//
//  FITShdu.cxx
//  FitsExtractor
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under PolyForm Noncommercial 1.0.0
//  You may use, share and adapt this code for noncommercial purposes.
//  Commercial use requires a separate licence from the author.
//  Licence text: https://polyformproject.org/licenses/noncommercial/1.0.0/


#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>
#include <DSTfits/FITSnumeric.h>

#include <limits>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <regex>
#include <string>
#include <cstdint>
#include <climits>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// Ensure basic integer sizes for narrowing casts / CFITSIO interoperability
static_assert(CHAR_BIT == 8, "This code assumes 8-bit chars");
static_assert(sizeof(short) >= 2, "Expected short >= 16 bits");
static_assert(sizeof(int) >= 4, "Expected int >= 32 bits");
static_assert(sizeof(long long) >= 8, "Expected long long >= 64 bits");

namespace DSL
{
    namespace
    {
        /**
         * @brief Shortest text that reads back as exactly @p v, for a FITS real keyword.
         *
         * @details Tries @p minDigits .. @p maxDigits significant digits and keeps the first
         * that round-trips through @p parse; @p maxDigits (max_digits10 of the type) always
         * does. Exact without padding: 0.0018888888 stays 0.0018888888, 150.5 stays 150.5.
         * The decimal point is a '.' whatever the locale.
         *
         * A decimal point is added to an integral result ("4" -> "4.0") so the value still
         * reads as a real, not an integer. Non-finite values keep their plain text, FITS
         * having no representation for them anyway.
         */
        template<typename T>
        std::string exactRealString(T v, int minDigits, int maxDigits, T (*parse)(const std::string&, size_t*))
        {
            if(!std::isfinite(v))
                return std::to_string(v);

            std::string s;
            for(int p = minDigits; p <= maxDigits; p++)
            {
                s = formatReal(static_cast<double>(v), p);
                try
                {
                    if(parse(s, nullptr) == v)
                        break;
                }
                catch(const std::out_of_range&)
                {
                    // Rounded up past the largest value of the type: it needs more digits.
                }
            }

            if(s.find_first_of(".E") == std::string::npos)
                s += ".0";

            return s;
        }

        template<typename W>
        W toInteger(const std::string& text, size_t* consumed)
        {
            const int base = integerBase(text);

            if constexpr(std::is_same_v<W, int>)
                return std::stoi(text, consumed, base);
            else if constexpr(std::is_same_v<W, long>)
                return std::stol(text, consumed, base);
            else if constexpr(std::is_same_v<W, long long>)
                return std::stoll(text, consumed, base);
            else if constexpr(std::is_same_v<W, unsigned long>)
                return std::stoul(text, consumed, base);
            else
                return std::stoull(text, consumed, base);
        }

        /**
         * @brief Integer value of a keyword, asked for through one of the integer getters.
         *
         * @details An integer conversion stops at the first character it can't read, which on
         * a real number gives the wrong answer: 1 for "1.5E+3", 1 for "0x1.8p1". So a real is
         * read whole and its integer part returned: 1500, 3.
         *
         * A warning says so when a fractional part is lost, and for any hexadecimal real.
         * A whole decimal such as "32768." stays silent: FITS files commonly store BZERO so.
         *
         * @param key Keyword name, for the warning.
         * @param text Keyword value.
         * @param getter Name of the calling getter, for the warning.
         * @throws std::out_of_range if the integer part doesn't fit @p W.
         */
        template<typename W>
        W integerKeywordValue(const std::string& key, const std::string& text, const char* getter)
        {
            size_t consumed = 0;
            const W integer = toInteger<W>(text, &consumed);
            if(consumed == text.size())
                return integer;

            size_t realConsumed = 0;
            const double real = parseDouble(text, &realConsumed);
            if(realConsumed <= consumed)
                return integer; // Not a real: what follows the number isn't part of it.

            const double integerPart = std::trunc(real);

            const double limit = std::ldexp(1.0, std::numeric_limits<W>::digits);
            const double lowest = std::is_signed_v<W> ? -limit : 0.0;
            if(integerPart >= limit || integerPart < lowest)
                throw std::out_of_range(std::string(getter) + ": value " + text + " of key " + key + " is out of range");

            const W value = static_cast<W>(integerPart);

            if(integerPart != real || isHexNumber(text))
                std::cerr << FITSwarning("FITShdu", getter, "Value "+text+" of key "+key+" isn't an integer: returning its integer part, "+std::to_string(value)+".").what() << std::flush;

            return value;
        }

        /**
         * @brief What a keyword of this type can hold and be written with: the capacity
         *        of its Write...ValueForKey() and of its CFITSIO type.
         * @details Not the ranges Process() uses, which only pick the smallest type for a value
         *          read from a file: fInt is written as TINT, so it takes 12345678.
         * @throws std::invalid_argument, std::out_of_range if the value doesn't match.
         * @throws FITSexception if the type isn't a numeric one.
         */
        void checkWrittenCapacity(const std::string& value, const key_type& type)
        {
            switch(type)
            {
                case fShort:     parseInteger<int16_t>(value);  break;
                case fInt:       parseInteger<int32_t>(value);  break;
                case fLong:      parseInteger<int32_t>(value);  break;
                case fLongLong:  parseInteger<int64_t>(value);  break;
                case fByte:      parseInteger<uint8_t>(value);  break;
                case fUShort:    parseInteger<uint16_t>(value); break;
                case fUInt:      parseInteger<uint32_t>(value); break;
                case fULong:     parseInteger<uint32_t>(value); break;
                case fULongLong: parseInteger<uint64_t>(value); break;

                case fBool:
                    if(parseInteger<uint8_t>(value) > 1)
                        throw std::out_of_range("Only 0 or 1: "+value);
                    break;

                case fFloat:     parseWholeReal<float>(value);  break;
                case fDouble:    parseWholeReal<double>(value); break;

                default:
                    throw FITSexception(BAD_KEYCHAR, "FITSkeyword", "setValue", "Unknown target type when setting value: "+value);
            }
        }

        bool holdsUnsignedInteger(const std::string& text)
        {
            try
            {
                parseInteger<uint64_t>(text);
                return true;
            }
            catch(const std::invalid_argument&)
            {
                return false;
            }
            catch(const std::out_of_range&)
            {
                return false;
            }
        }

        /**
         * @brief A whole, non-negative value as integer text, whichever way the file wrote it.
         * @details "0.00000000E+00" and "2.147483648E9" are how files commonly store BZERO.
         * @return The integer as text, empty if the value isn't such a number.
         */
        std::string wholeUnsignedText(const std::string& value)
        {
            if(holdsUnsignedInteger(value))
                return value;

            try
            {
                const double real = parseWholeReal<double>(value);

                if(real >= 0.0 && real < std::ldexp(1.0, 64) && std::trunc(real) == real)
                    return std::to_string(static_cast<uint64_t>(real));
            }
            catch(const std::invalid_argument&) {}
            catch(const std::out_of_range&) {}

            return std::string();
        }

        void validateBeforeWriting(FITSDictionary::const_iterator it)
        {
            try
            {
                it->second.validate();
            }
            catch(const FITSexception&)
            {
                throw FITSexception(BAD_KEYCHAR, "FITShdu", "Write", "KEYWORD "+it->first+" holds '"+it->second.value()+"', which doesn't match its type "+FITSkeyword::GetDataType(it->second.type())+".");
            }
        }
    }

#pragma mark - FITSkeyword class implementation
    
    std::string FITSkeyword::GetDataType(const key_type& tt)
    {
        switch (tt)
        {
            case fChar:
                return "TSTRING";
                break;
                
            case fShort:
                return "TSHORT";
                break;
                
            case fUShort:
                return "TUSHORT";
                break;
                
            case fInt:
                return "TINT";
                break;
                
            case fUInt:
                return "TUINT";
                break;
                
            case fLong:
                return "TLONG";
                break;

            case fULong:
                return "TULONG";
                break;
                
            case fLongLong:
                return "TLONGLONG";
                break;
                
            case fULongLong:
                return "TULONGLONG";
                break;
                
            case fBool:
                return "TLOGICAL";
                break;
                
            case fFloat:
                return "TFLOAT";
                break;
                
            case fDouble:
                return "DOUBLE";
                break;
                
            case fByte:
                return "TBYTE";
                break;
                
            default:
                return "UNDEF";
                break;break;
        }
    }
    
#pragma mark * ctor/dtor
    /**
     *  @details Construct KEYWORD container from value and comment.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     */
    FITSkeyword::FITSkeyword(const std::string& val, const std::string& cmt)
    {
        Process(val, cmt);
    }
    
    /**
     *  @details Construct KEYWORD container from value and comment.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     */
    FITSkeyword::FITSkeyword(const std::string& val, const std::string& cmt, const key_type& kt)
    {
        if(kt == key_type::fUndef)
            Process(val, cmt);
        else
        {
            fcomment = std::string(cmt);
            fvalue   = std::string(val);
            ftype    = kt;
            validate();
        }
    }

    /**
     *  @details Construct KEYWORD container from what a file holds. A file can't be refused for
     *  being unusual, so the type isn't checked against the value here. Writing it out will be.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     *  @param kt KEYWORD type
     */
    FITSkeyword FITSkeyword::unchecked(const std::string& val, const std::string& cmt, const key_type& kt)
    {
        FITSkeyword keyword(val, cmt, fChar);
        keyword.ftype = kt;
        return keyword;
    }
    
    /**
     *  @details Construct KEYWORD container from value and comment.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     */
    FITSkeyword::FITSkeyword(const std::string& val, const key_type& kt)
    {
        if(kt == key_type::fUndef)
            Process(val,  std::string());
        else
        {
            fcomment = std::string();
            fvalue   = std::string(val);
            ftype    = kt;
            validate();
        }
    }
    
    /**
     *  @details Construct KEYWORD container from value.
     *  @param val KEYWORD physical value as string
     *  @note The KEYWORD description is left blank
     */
    FITSkeyword::FITSkeyword(const std::string& val)
    {
        Process(val, std::string());
    }
    
    /**
     *  @details Copy constructor
     *  @param key FITSkeyword container to be copied
     */
    FITSkeyword::FITSkeyword(const FITSkeyword& key)
    {
        fvalue = key.fvalue;
        fcomment = key.fcomment;
        ftype = key.ftype;
    }
    
    /**
     *  @details Destructor
     */
    FITSkeyword::~FITSkeyword()
    {
        if(fvalue.size() > 0)
            fvalue.clear();
        
        if(fcomment.size() > 0)
            fcomment.clear();
        
        ftype = fUndef;
    }
    
#pragma mark * Processing
    /**
     *  @details Fill KEYWORD container.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     */
    void FITSkeyword::Process(const std::string& val, const std::string& cmt)
    {
        fcomment = std::string(cmt);
        fvalue   = std::string(val);
        ftype=fUndef;
        
        if( fvalue.size() < 1 )
            return;
                
        bool has_only_digits = false;
        
        // Accept integers, decimals and scientific notation: optional sign, digits, optional ".digits", optional exponent
        static const std::regex numeric_re(R"(^[+-]?(?:\d+)(?:\.\d*)?(?:[eE][+-]?\d+)?$)");
        if (std::regex_match(fvalue, numeric_re))
            has_only_digits = true;


    
        if(! has_only_digits )
        {
            ftype = fChar;
        }
        // An exponent makes a real, with or without a point: 1E-5 isn't the integer 1.
        else if(fvalue.find('e') != std::string::npos ||
                fvalue.find('E') != std::string::npos    )
            {
                double val = parseDouble(fvalue, nullptr);
                if(std::abs(val) > static_cast<double>(std::numeric_limits<float>::max()) || std::abs(val) < 1.e-7) 
                    ftype = fDouble;
                else
                    ftype = fFloat;
            }
        else if(fvalue.find('.') != std::string::npos)
            {
                double val = parseDouble(fvalue, nullptr);
                if(std::abs(val) > static_cast<double>(std::numeric_limits<float>::max()) || std::abs(val) < 1.e-7) 
                    ftype = fDouble;
                else
                    ftype = fFloat;
            }
        else if(fvalue.find('-') != std::string::npos )
        {
            int64_t num = std::stoll(fvalue, nullptr, 10);
            
            if(std::llabs(num) <= INT8_MAX)
                ftype = fShort;
            else if(std::llabs(num) <= INT16_MAX)
                ftype = fInt;
            else if(std::llabs(num) <= INT32_MAX)
                ftype = fLong;
            else if(std::llabs(num) <= INT64_MAX)
                ftype = fLongLong;
            else
                throw FITSexception(BAD_KEYCHAR,"FITSkeyword","Process","numerical value out of range "+val+" ["+std::to_string(__LINE__)+"]");
        }
        else
        {
            uint64_t num = std::stoull(fvalue, nullptr, 10);
            
            if(num <= 1)
                ftype = fByte;
            else if(num <= UINT8_MAX)
                ftype = fUShort;
            else if(num <= UINT16_MAX)
                ftype = fUInt;
            else if(num <= UINT32_MAX)
                ftype = fULong;
            else if(num <= UINT64_MAX)
                ftype = fULongLong;
            else
                throw FITSexception(BAD_KEYCHAR,"FITSkeyword","Process","numerical value out of range "+val+" ["+std::to_string(__LINE__)+"]");
        }

        if(ftype == fUndef)
            throw FITSexception(BAD_KEYCHAR,"FITSkeyword","Process",std::string("Keyword type undefined")+std::string(" [")+std::to_string(__LINE__)+std::string("]"));
        
        try
        {
            if(fvalue.size()+fcomment.size() >= 80)
                throw FITSwarning("FITSkeyword","Process","FITS key card string is too long. It may be troncated later.\n       \033[33m'"+fvalue+" / "+fcomment+"'\033[33m");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
        }
    }
    
#pragma mark * Modifier
    /**
     *  Modify the value associated to the keyword.
     *  @param value The value to associate to the keyword.
     *
     *  @note A check of type consistancy is performed. The type isn't modified but the value is adapted to the base type.
     */
    void FITSkeyword::setValue(const std::string& value)
    {
        // For character types accept any value
        if(ftype == fChar)
        {
            fvalue = value;
            return;
        }

        if(value.empty())
            throw FITSexception(0, "FITSkeyword", "setValue", "Non string KEYWORD can't be empty.");

        try
        {
            checkWrittenCapacity(value, ftype);
        }
        catch(const std::invalid_argument&)
        {
            throw FITSexception(0, "FITSkeyword", "setValue", "Value '"+value+"' doesn't match the KEYWORD type "+GetDataType(ftype)+".");
        }
        catch(const std::out_of_range&)
        {
            throw FITSexception(0, "FITSkeyword", "setValue", "Value '"+value+"' is out of range for the KEYWORD type "+GetDataType(ftype)+".");
        }

        fvalue = value;
    }

    /**
     *  @details Check that the value is what the KEYWORD type can hold and be written with. Only
     *  a KEYWORD made with unchecked() can hold one that isn't.
     *  @throws FITSexception if it isn't.
     */
    void FITSkeyword::validate() const
    {
        if(ftype == fChar || ftype == fUndef)
            return;

        try
        {
            checkWrittenCapacity(fvalue, ftype);
        }
        catch(const std::invalid_argument&)
        {
            throw FITSexception(0, "FITSkeyword", "validate", "Value '"+fvalue+"' doesn't match the KEYWORD type "+GetDataType(ftype)+".");
        }
        catch(const std::out_of_range&)
        {
            throw FITSexception(0, "FITSkeyword", "validate", "Value '"+fvalue+"' is out of range for the KEYWORD type "+GetDataType(ftype)+".");
        }
    }

    /**
     *  @details Change the KEYWORD type.
     *  @param type New KEYWORD type.
     *  @throws FITSexception if the current value doesn't match it, in which case nothing changes.
     */
    void FITSkeyword::setType(const key_type& type)
    {
        unchecked(fvalue, fcomment, type).validate();
        ftype = type;
    }

#pragma mark * Dump
    /**
     *  @details Print out the KEYWORD physical value and its descritpion.
     *  @param out the output stream where value and descritpion are printed out.
     */
    void FITSkeyword::Dump( std::ostream& out) const
    {
        std::string tmp_string = fvalue;
        if(tmp_string.size() > 80)
            tmp_string.insert(80,"\n              ");
        
        while (tmp_string.size() < 30)
        {
            tmp_string+=" ";
        }
        
        if(fcomment.size() > 0)
        {
            tmp_string += " | " + fcomment;
        }
        
        while ((tmp_string.size()%85))
        {
            tmp_string+=" ";
        }
        
        out<<tmp_string<<" \033[34m["<<ftype<<": "<<FITSkeyword::GetDataType(ftype)<<"]\033[0m";
    }

    /**
     * @brief 
     * 
     */
    std::string FITSkeyword::asString() const
    {
        std::string tmp_string = std::string();
        
        if(ftype == fChar || ftype == fUndef)
        {
            tmp_string += "'"+fvalue+"'";
            while ((tmp_string.size()+10)<30)
                tmp_string+=" ";
        }
        else
        {
            while((tmp_string.size()+10+fvalue.size()) < 30)
                tmp_string+=" ";

            tmp_string += fvalue;
        }
        
        if(tmp_string.size()+10 > 90)
            return tmp_string;

        tmp_string += " / ";
        tmp_string += fcomment;

        while( ((tmp_string.size()+10)%80))
            tmp_string+=" ";

        if(tmp_string.size()+10 > 80)
            tmp_string.resize(80-10);  

        return tmp_string;
    }
    
#pragma mark - FITShdu class implementation
#pragma mark * ctor/dtor
    
    /**
     *  @details Allocate memory to an empty header data block
     */
    FITShdu::FITShdu()
    { }
    
    /**
     *  @brief Constructor
     */
    FITShdu::FITShdu(const std::shared_ptr<fitsfile>& fptr)
    {
        char *header = NULL;
        int  nKey    = 0;
        int  status  = 0, cflags = 0;
        
        {
            CFITSIOGuard  guard;
            cflags = fits_hdr2str(fptr.get(), 0, NULL, 0, &header, &nKey, &status);
        }
        
        if(cflags || header == NULL)
        {
            throw FITSexception(status,"FITShdu","FITShdu");
        }
        else
        {
            Process( std::string(header) );
        }
        
        if(header)
            free(header);
    }

    /**
     *  @brief Constructor
     *  @param h The FITS header as a string
     */
    FITShdu::FITShdu(const std::string & h)
    {
        Process( h );
    }
    
    /**
     *  @brief Constructor
     */
    FITShdu::FITShdu(const FITShdu& in_hdu)
    {
        for(FITSDictionary::const_iterator it = in_hdu.hdu.begin(); it != in_hdu.hdu.end(); it++)
        {
            if(it->first == "PCOUNT" || it->first == "GCOUNT")
                hdu.insert(std::pair<key_code,FITSkeyword>(it->first,FITSkeyword::unchecked(it->second.value(), it->second.comment(), DSL::key_type::fULongLong)));
            else
                hdu.insert(std::pair<key_code,FITSkeyword>(it->first,FITSkeyword(it->second)));
        }
    }
    
    /**
     *  @brief Destructor
     */
    FITShdu::~FITShdu()
    {
        if(hdu.size() > 0)
            hdu.clear();
    }

    void FITShdu::swap(FITShdu& other) noexcept
    {
        hdu.swap(other.hdu);
    }

#pragma mark * Processing
    /**
     *  @details Read FITS header and extract value for each KEYWORD.
     *  @param h : Input header
     */
    void FITShdu::Process(const std::string& h)
    {
        size_t pos = 0;
        while (pos < h.size())
        {
            size_t length = (pos + 80 < h.size())? 80 : h.size() - pos;
            std::string hdu_entry = h.substr(pos, length);
            
            std::string value;
            std::string comment;
            
            key_code key = hdu_entry.substr(0, 8);
            key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char c){ return std::isspace(c); }), key.end());
            if(key.size() < 1 || key == "END")
            {
                pos += 80;
                continue;
            }
            
            size_t step = 10;

            // Locate the comment separator. The value field starts at column 11
            // and, for a character keyword, is a single quoted string in which a
            // doubled '' stands for an embedded apostrophe. Only a '/' *outside*
            // that string opens the comment, so the closing quote has to be
            // found by scanning the value instead of searching the card backwards:
            // an apostrophe in the comment text, or a '/' written right after the
            // closing quote, would otherwise anchor the search past the separator
            // and merge the comment into the value.
            size_t cmt_start = step;

            bool   is_string = false;          // value field is a properly closed quoted string
            size_t str_start = std::string::npos;
            size_t str_stop  = std::string::npos;

            if(step < hdu_entry.size())
            {
                size_t val_start = hdu_entry.find_first_not_of(" ", step);

                if(val_start != std::string::npos && hdu_entry[val_start] == '\'')
                {
                    size_t iChar = val_start + 1;

                    while(iChar < hdu_entry.size())
                    {
                        if(hdu_entry[iChar] != '\'')
                        {
                            iChar++;
                        }
                        else if(iChar + 1 < hdu_entry.size() && hdu_entry[iChar + 1] == '\'')
                        {
                            iChar += 2;               // escaped apostrophe: still part of the value
                        }
                        else
                        {
                            is_string = true;         // iChar is the closing quote
                            break;
                        }
                    }

                    if(is_string)
                    {
                        str_start = val_start + 1;
                        str_stop  = iChar;
                        cmt_start = iChar + 1;        // just past the closing quote
                    }
                    else
                        cmt_start = hdu_entry.size(); // unterminated string: no comment field
                }
            }

            size_t cmt_pos = (cmt_start < hdu_entry.size())? hdu_entry.find_first_of("/",cmt_start)
                                                           : std::string::npos;

            if(cmt_pos == std::string::npos)
                comment = std::string();
            else
                comment = hdu_entry.substr(cmt_pos + 1, hdu_entry.size() - cmt_pos - 1 );

            if(is_string)
                value = hdu_entry.substr(str_start, str_stop - str_start);
            else if(cmt_pos == std::string::npos)
                value = hdu_entry.substr(step, hdu_entry.size() - step);
            else
                value = hdu_entry.substr(step, cmt_pos - step);

            if(   key != "COMMENT"
               && key != "HISTORY" )
            {
                if(is_string)
                {
                    // Inside a character value only trailing blanks are insignificant,
                    // leading ones belong to the value, and a doubled '' encodes a
                    // single apostrophe.
                    size_t last = value.find_last_not_of(" ");
                    value.erase((last == std::string::npos)? 0 : last + 1);

                    for(size_t iChar = value.find("''"); iChar != std::string::npos; iChar = value.find("''", iChar + 1))
                        value.erase(iChar, 1);
                }
                else
                {
                    value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char c){ return std::isspace(c); }), value.end());
                    while(value.find_first_of("'") != std::string::npos)
                        value.erase(value.find_first_of("'"), 1);
                }
            }

            FITSDictionary::iterator iKey = hdu.end();
            if(hdu.size() > 1)
                iKey = hdu.find(key);
            
            if(iKey == hdu.end())
            {
                if(key.find("NAXIS") != std::string::npos && holdsUnsignedInteger(value))
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(value, comment, fULongLong)));
                else if(key == "BITPIX")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(value, comment, fInt)));
                else if(key == "BZERO" && !wholeUnsignedText(value).empty())
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(wholeUnsignedText(value), comment, fULongLong)));
                else if(key == "BSCALE")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(value, comment, fDouble)));
                else if(key == "PCOUNT")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(value, comment, fULongLong)));
                else if(key == "GCOUNT")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword::unchecked(value, comment, fULongLong)));
                else
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment)));
            }
            else if(   iKey->first == "COMMENT"
                    || iKey->first == "HISTORY" )
            {
                ((iKey->second).value()) += std::string(" ") + value;
            }
            
            pos += 80;
        }
    }
    
#pragma mark * Dump
    /**
     *  @details Print out FITS HDU content.
     *  @param out the output stream where value and descritpion are printed out.
     */
        void FITShdu::Dump( std::ostream& out) const
    {
        for(FITSDictionary::const_iterator iKey = hdu.begin(); iKey != hdu.end(); iKey++)
        {
            key_code tmp_string = iKey->first;
            while (tmp_string.size() < 11)
            {
                tmp_string+=" ";
            }
            
            out<<tmp_string<<" = ";
            iKey->second.Dump( out );
            out<<std::endl;
        }
    }

    /**
     * @details Return FITS HDU content as a string.
     * @return string containing the FITS HDU KEY CARDS
     */
    std::string FITShdu::asString() const
    {
        std::string out = std::string();
        for(FITSDictionary::const_iterator iKey = hdu.cbegin(); iKey != hdu.cend(); iKey++)
        {
            std::string tmp_string = std::string();

            if(iKey->first.size() > 8)
                tmp_string += iKey->first.substr(0,8);
            else
                 tmp_string += iKey->first;

            while (tmp_string.size() < 8)
                tmp_string += " ";
            
            if(iKey->first == "COMMENT" || iKey->first == "HISTORY")
                tmp_string += "  ";
            else
                tmp_string += "= ";

            tmp_string += iKey->second.asString();

            if(iKey->first == "COMMENT" || iKey->first == "HISTORY")
            {
                while(tmp_string.find_first_of("'") != std::string::npos)
                    tmp_string.erase(tmp_string.find_first_of("'"), 1);
                
                if(tmp_string.size() > 80)
                {
                    size_t pos =1;
                    while ((tmp_string.size()%80))
                    {
                        if(pos*80 < tmp_string.size())
                            tmp_string.insert(pos*80, iKey->first+std::string("  "));
                        else
                        {
                            while ((tmp_string.size()%80))
                                tmp_string+=" ";
                            continue;
                        }

                        if((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
                            std::cout<<"\033[34m[DEBUG]\033[0m FITShdu::asString() - Long COMMENT/HISTORY string detected. Inserted line break at pos "<<pos*80<<"."<<std::endl
                            <<tmp_string<<std::endl;
                        pos++;
                    }
                }
                else
                {
                    while ((tmp_string.size()%80))
                        tmp_string+=" ";
                }
            }
            else
            {
                // For regular keywords, ensure exactly 80 characters
                if (tmp_string.size() > 80)
                {
                    // Truncate if too long
                    tmp_string = tmp_string.substr(0, 80);
                }
                else
                {
                    // Pad with spaces to exactly 80 characters
                    while ((tmp_string.size() % 80))
                        tmp_string += " ";
                }
            }

            out += tmp_string;
        }

        std::string endStr = "END     ";
        while ((out.size()+endStr.size()) % 2880)
            out += " ";
        out += endStr;

        return out;
    }

#pragma mark * Accessor
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return boolean value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, false is returned
     */
    bool FITShdu::GetBoolValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return false;
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetBoolValueForKey","Value for key "+key+" isn't a numerical value.");
        
        try
        {
            if(outString.size() > 1 || (outString != "1" && outString != "0" ) )
                throw FITSwarning("FITShdu","GetBoolValueForKey","Value for key "+key+" may not be boolean.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return static_cast<bool>( std::stoi(outString, nullptr, integerBase(outString)) );
    }
    
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Short integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    int8_t FITShdu::GetInt8ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<int8_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetShortValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int8_t>( integerKeywordValue<int>(key, outString, "GetInt8ValueForKey") );
    }

    uint8_t FITShdu::GetUInt8ValueForKey  (const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<uint8_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetSUhortValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint8_t>( integerKeywordValue<unsigned long>(key, outString, "GetUInt8ValueForKey") );
    }

    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    int16_t FITShdu::GetInt16ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<int16_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetIntValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return  static_cast<int16_t>(integerKeywordValue<int>(key, outString, "GetInt16ValueForKey"));
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Unsigned Integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    uint16_t FITShdu::GetUInt16ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<uint16_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetUIntValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return  static_cast<uint16_t>( integerKeywordValue<int>(key, outString, "GetUInt16ValueForKey") );
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    int32_t FITShdu::GetInt32ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<int32_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int32_t>(integerKeywordValue<long>(key, outString, "GetInt32ValueForKey"));
    }

    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    uint32_t FITShdu::GetUInt32ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<uint32_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint32_t>(integerKeywordValue<unsigned long>(key, outString, "GetUInt32ValueForKey"));
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    int64_t FITShdu::GetInt64ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<int64_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetLongLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int64_t>(integerKeywordValue<long long>(key, outString, "GetInt64ValueForKey"));
    }

    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    uint64_t FITShdu::GetUInt64ValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<uint64_t>::min();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetLongLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint64_t>(integerKeywordValue<unsigned long long>(key, outString, "GetUInt64ValueForKey"));
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Float value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    float FITShdu::GetFloatValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<float>::quiet_NaN();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetFloatValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return parseFloat(outString);
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Double value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    double FITShdu::GetDoubleValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
        
        if(tt == fUndef || (tt == fChar && !isHexNumber(outString)))
            throw FITSexception(0,"FITShdu","GetDoubleValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return parseDouble(outString);
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @param type Upon return base type of the KEYWORD physical value
     *  @return the value associated to the KEYWORD into a std::string
     *  @note If the KEYWORD doesn't exist, an empty std::string is returned and type is set to fUndef
     */
    std::string FITShdu::GetValueForKey(const std::string& key, key_type& type) const
    {
        type = fUndef;
        
        FITSDictionary::const_iterator iKey = hdu.find(key);
        if(iKey == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","GetValueForKey","Key "+key+" not found in HDU");
        
        type = iKey->second.type();
        
        return iKey->second.value();
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return the value associated to the KEYWORD into a std::string
     *  @note If the KEYWORD doesn't exist, an empty std::string is returned and type is set to fUndef
     */
    std::string FITShdu::GetValueForKey(const std::string& key) const
    {
        key_type type = fUndef;
       
        return GetValueForKey(key, type);
    }
    
    /**
     *  Estimate the total dimention of a FITS cube or FITS table based on the information embeded into the Header.
     *  @return The total size of the FITS cube or table.
     */
    size_t FITShdu::GetDimension() const
    {
        FITSDictionary::const_iterator it= hdu.find("NAXIS1");
        if(it == hdu.end())
            return 0;
        
        size_t n = 1;
        uint64_t dim64 = std::stoull(it->second.value(), nullptr, integerBase(it->second.value()));
        size_t dim = static_cast<size_t>(dim64);
        while(it != hdu.end())
        {
            n++;
            it = hdu.find(std::string("NAXIS")+std::to_string(n));
            if(it != hdu.end())
                dim *= std::stoull(it->second.value(), nullptr, integerBase(it->second.value()));
        }
        
        return dim;
        
    }
    
#pragma mark * I/O
    
    /**
     *  @details Write FITS header to the current HDU
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::Write(const std::shared_ptr<fitsfile>& _fptr_) const
    {
        if(_fptr_ == NULL)
        {
            throw std::invalid_argument("\033[31m[FITShdu::Write]\033[0mreceived nullptr");
            return;
        }
        
        if((verbose&verboseLevel::VERBOSE_HDU)==verboseLevel::VERBOSE_HDU)
            std::cout<<"\033[31m[FITShdu::Write]\033[0m"<<std::endl;

        std::shared_ptr<fitsfile> fptr;
        {
            CFITSIOGuard  guard;
            fptr = _fptr_;
        }
        
        FITSDictionary::const_iterator it;
        
        for(it = hdu.begin(); it != hdu.end(); it++)
        {
            if(it->first == "DATE")
                continue;
                        
            if( it->first == "EXTEND" || it->first == "SIMPLE" || it->first == "XTENSION" )
                continue;
            
            if( it->first == "COMMENT")
                continue;
            
            if( it->first == "HISTORY")
                continue;

            if( it->first == "BSCALE" )
                continue;

            if( it->first == "BZERO" )
                continue;

            validateBeforeWriting(it);
            
            switch(it->second.type())
            {
                case fChar:
                    WriteCharValueForKey( it, fptr );
                    break;
                    
                case fShort:
                    WriteInt8ValueForKey( it, fptr );
                    break;
                
                case fByte:
                case fBool:
                case fUShort:
                    WriteUInt8ValueForKey( it, fptr );
                    break;
                    
                case fInt:
                    WriteInt16ValueForKey( it, fptr );
                    break;
                
                case fUInt:
                    WriteUInt16ValueForKey( it, fptr );
                    break;
                    
                case fLong:
                    WriteInt32ValueForKey( it, fptr );
                    break;
                    
                case fULong:
                    WriteUInt32ValueForKey( it, fptr );
                    break;
                    
                case fLongLong:
                    WriteInt64ValueForKey( it, fptr );
                    break;
                
                case fULongLong:
                    WriteUInt64ValueForKey( it, fptr );
                    break;
                    
                case fFloat:
                    WriteFloatValueForKey( it, fptr );
                    break;
                    
                case fDouble:
                    WriteDoubleValueForKey( it, fptr );
                    break;
                    
                default:
                    WriteCharValueForKey( it, fptr );
                    break;
            }
        }
        
        it = hdu.find("DATE");
        if( it != hdu.end() )
            WriteCharValueForKey( it, fptr );
        
        it = hdu.find("COMMENT");
        if( it != hdu.end() )
            WriteCharValueForKey( it, fptr );
        
        it = hdu.find("HISTORY");
        if( it != hdu.end() )
            WriteCharValueForKey( it, fptr );

        bool has_bzero = false;
        bool has_bscale = false;
        double bzero = 0.0;
        double bscale = 1.0;
        
        FITSDictionary::const_iterator ibz = hdu.find("BZERO");
        if( ibz != hdu.end() )
        {
            bzero = GetDoubleValueForKey("BZERO");
            has_bzero = (std::abs(bzero) > std::numeric_limits<double>::epsilon());
        }

        FITSDictionary::const_iterator ibs = hdu.find("BSCALE");
        if( ibs != hdu.end() )
        {
            bscale = GetDoubleValueForKey("BSCALE");
            has_bscale = (std::abs(bscale-1) > std::numeric_limits<double>::epsilon());
        }

        FITSDictionary::const_iterator ibb = hdu.find("BITPIX");
        if( ibb == hdu.end() )
            throw FITSexception(KEY_NO_EXIST,"FITShdu","Write","BITPIX key not found in HDU");

        if(has_bscale || has_bzero)
        {
            if(ibz == hdu.end() || ibs == hdu.end())
                throw FITSexception(KEY_NO_EXIST,"FITShdu","Write","BZERO and BSCALE must be both present to be written");

            validateBeforeWriting(ibs);
            validateBeforeWriting(ibz);

            if(ibs->second.type() == fDouble || ibs->second.type() == fFloat)
                WriteDoubleValueForKey( ibs, fptr );
            else if (parseDouble(ibs->second.value()) < 0)
                WriteInt64ValueForKey( ibs, fptr );
            else
                WriteUInt64ValueForKey( ibs, fptr );

            if(ibz->second.type() == fDouble || ibz->second.type() == fFloat)
                WriteDoubleValueForKey( ibz, fptr );
            else if (parseDouble(ibz->second.value()) < 0)
                WriteInt64ValueForKey( ibz, fptr );
            else 
                WriteUInt64ValueForKey( ibz, fptr );
        }

    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteCharValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& _fptr_) const
    {
        std::shared_ptr<fitsfile> fptr;
        {
            CFITSIOGuard  guard;
            fptr = _fptr_;
        }

        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteCharValueForKey","received nullptr");
        
        if(it == hdu.end())
            throw FITSexception(KEY_NO_EXIST,"FITShdu","WriteCharValueForKey","Reach end of header");
        
        // Build local null-terminated buffers for key, value and comment to avoid casting away const
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string val_str = it->second.value();
        std::vector<char> valbuf(val_str.c_str(), val_str.c_str()+val_str.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        if(fits_update_key(fptr.get(), TSTRING, keybuf.data(), valbuf.data(), cmtbuf.data(), &status ) )
            throw FITSexception(status,"FITShdu","Write");
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteBoolValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteBoolValueForKey","received nullptr");
        }
        
        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteBoolValueForKey","Reach end of header");
        
        int val = std::stoi(it->second.value(), nullptr, integerBase(it->second.value()));
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TLOGICAL, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteInt8ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteShortValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteShortValueForKeyv","Reach end of header");

        // parse into fixed-width then cast to platform 'short' for CFITSIO
        int16_t tmp = static_cast<int16_t>(std::stoi(it->second.value(), nullptr, integerBase(it->second.value())));
        short val = static_cast<short>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TSHORT, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }

    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteUInt8ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteShortValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteShortValueForKeyv","Reach end of header");

        uint16_t tmp = static_cast<uint16_t>(std::stoul(it->second.value(), nullptr, integerBase(it->second.value())));
        unsigned short val = static_cast<unsigned short>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TUSHORT, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }

    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteInt16ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteIntValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteIntValueForKey","Reach end of header");

        int32_t tmp = static_cast<int32_t>(std::stoi(it->second.value(), nullptr, integerBase(it->second.value())));
        int val = static_cast<int>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if(fits_update_key(fptr.get(), TINT, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }

    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteUInt16ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteIntValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteIntValueForKey","Reach end of header");

        uint32_t tmp = static_cast<uint32_t>(std::stoul(it->second.value(), nullptr, integerBase(it->second.value())));
        unsigned int val = static_cast<unsigned int>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if(fits_update_key(fptr.get(), TUINT, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteInt32ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");

        if(it == hdu.cend())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");

        int32_t tmp = static_cast<int32_t>(std::stol(it->second.value(), nullptr, integerBase(it->second.value())));
        long val = static_cast<long>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TLONG, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteUInt32ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");

        uint32_t tmp = static_cast<uint32_t>(std::stoul(it->second.value(), nullptr, integerBase(it->second.value())));
        unsigned long val = static_cast<unsigned long>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TULONG, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteInt64ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");

        int64_t tmp = static_cast<int64_t>(std::stoll(it->second.value(), nullptr, integerBase(it->second.value())));
        long long val = static_cast<long long>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TLONGLONG, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }

        return;
    }

    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteUInt64ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");

        uint64_t tmp = static_cast<uint64_t>(std::stoull(it->second.value(), nullptr, integerBase(it->second.value())));
        unsigned long long val = static_cast<unsigned long long>(tmp);
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TULONGLONG, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteFloatValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");
        }
        
        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        
        float val = parseFloat(it->second.value());
        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TFLOAT, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteDoubleValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");
        }
        
        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        

        double val = parseDouble(it->second.value());

        std::string keystr = it->first;
        std::vector<char> keybuf(keystr.c_str(), keystr.c_str()+keystr.size()+1);

        std::string cmt_str = it->second.comment();
        std::vector<char> cmtbuf(cmt_str.c_str(), cmt_str.c_str()+cmt_str.size()+1);

        int status = 0;

        {
            CFITSIOGuard  guard;
            if( fits_update_key(fptr.get(), TDOUBLE, keybuf.data(), &val, cmtbuf.data(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        
        return;
    }
    
#pragma mark * Modifier

    void FITShdu::Import(const FITShdu& other, const std::set<std::string>& key_codes_to_skip)
    {
        for(FITSDictionary::const_iterator it = other.hdu.cbegin(); it != other.hdu.cend(); it++)
        {
            if(
                it->first == "SIMPLE"   ||
                it->first == "XTENSION" ||
                it->first == "EXTEND"   ||
                it->first == "EXTNAME"  ||
                it->first == "COMMENT"  ||
                it->first == "HISTORY"  ||
                it->first == "BSCALE"   ||
                it->first == "BZERO"    ||
                it->first == "PCOUNT"   ||
                it->first == "GCOUNT"   ||
                it->first == "BITPIX"   ||
                it->first == "DATASUM"  ||
                it->first == "CHECKSUM" ||
                it->first == "HISTORY"  ||
                it->first.rfind("NAXIS", 0) == 0
                )
                continue;

            if(key_codes_to_skip.find(it->first) != key_codes_to_skip.end())
                continue;

            hdu.insert(std::pair<key_code,FITSkeyword>(it->first, it->second));
        }
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param kt    Base type of the keyword value.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const std::string& value, const key_type& kt)
    {
        ValueForKey(keyword, value, kt, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param kt    Base type of the keyword value.
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const std::string& value, const key_type& requested, const std::string& cmt)
    {
        // Structural keywords are integers whatever the caller says: the string
        // overloads default to fChar, and a quoted PCOUNT/GCOUNT/BITPIX/NAXISn makes
        // the extension unreadable for CFITSIO. Same types as reading a file assigns.
        key_type kt = requested;
        if(kt == fChar)
        {
            if(keyword == "BITPIX")
                kt = fInt; // Negative for real images.
            else if(keyword == "PCOUNT" || keyword == "GCOUNT")
                kt = fULongLong;
            else if(keyword.rfind("NAXIS", 0) == 0)
                kt = fULongLong;
        }

        // Before anything changes, so a refused value leaves the keyword as it was.
        const FITSkeyword updated(value, cmt, kt);

        FITSDictionary::iterator it = hdu.find(keyword);
        if(it != hdu.end())
        {
            try
            {
                if(it->second.type() != kt)
                {
                    std::stringstream ss;
                    ss<<"KEYWORD "<<keyword<<" ALREADY EXIST BUT IT ISN'T OF THE SAME TYPE."<<std::endl
                        <<"TYPE WILL BE MODIFIED.\033[34m "<<keyword<<"\033[0m "<<it->second.type()<<" ["<<FITSkeyword::GetDataType(it->second.type())<<"] -> "<<kt<<" ["<<FITSkeyword::GetDataType(kt)<<"]"<<std::endl
                        <<"\033[0m"<<std::flush;
                    it->second.Dump(ss);
                    
                    throw FITSwarning("FITShdu","ValueForKey",ss.str());
                }
            }
            catch(std::exception &e)
            {
                if((verbose & DSL::verboseLevel::VERBOSE_DETAIL)==DSL::verboseLevel::VERBOSE_DETAIL)
                    std::cerr<<e.what()<<std::endl;
            }

            it->second = FITSkeyword(value, it->second.comment(), kt);
            return;
        }

        // New keyword: created with the requested type, not the FITSkeyword default —
        // otherwise a numeric keyword added this way is written as a quoted string.
        hdu.insert(std::pair<key_code,FITSkeyword>(keyword, updated));
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const std::string& value, const std::string& cmt)
    {
        ValueForKey(keyword, value, fChar, cmt);
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const std::string& value)
    {
        ValueForKey(keyword, value, fChar, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const int8_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fInt,cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const int8_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const int16_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fInt ,cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const int16_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const int32_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fLong,cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const int32_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const int64_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fLongLong,cmt);
        
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const int64_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }
    
#if defined(__APPLE__)
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const size_t& value, const std::string& cmt)
    {
        // size_t is unsigned: fLongLong would reject anything above INT64_MAX,
        // e.g. the BZERO = 2^63 of an unsigned 64 bit image.
        ValueForKey(keyword, std::to_string(value), fULongLong,cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const size_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }
#endif
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const bool& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fBool, cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const bool& value)
    {
        ValueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const float& value, const std::string& cmt)
    {
        // Shortest exact text. std::to_string kept only 6 decimals between 1e-4 and 1e4,
        // which truncated any value carrying more (WCS terms first among them).
        ValueForKey(keyword,
                    exactRealString<float>(value, std::numeric_limits<float>::digits10, std::numeric_limits<float>::max_digits10,
                                           parseFloat),
                    fFloat, cmt);
    }
    
    void FITShdu::ValueForKey(const key_code& keyword, const float& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const double& value, const std::string& cmt)
    {
        // Shortest exact text. std::to_string kept only 6 decimals between 1e-4 and 1e4,
        // which truncated CDi_j, PVi_m, CRVAL, ... written through this overload.
        ValueForKey(keyword,
                    exactRealString<double>(value, std::numeric_limits<double>::digits10, std::numeric_limits<double>::max_digits10,
                                            parseDouble),
                    fDouble, cmt);
    }

    void FITShdu::ValueForKey(const key_code& keyword, const double& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const uint8_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fUShort, cmt);
    }

    void FITShdu::ValueForKey(const key_code& keyword, const uint8_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const uint16_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fUInt, cmt);
    }

    void FITShdu::ValueForKey(const key_code& keyword, const uint16_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const uint32_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fULong, cmt);
    }

    void FITShdu::ValueForKey(const key_code& keyword, const uint32_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::ValueForKey(const key_code& keyword, const uint64_t& value, const std::string& cmt)
    {
        ValueForKey(keyword, std::to_string(value), fULongLong, cmt);
    }

    void FITShdu::ValueForKey(const key_code& keyword, const uint64_t& value)
    {
        ValueForKey(keyword, value, std::string());
    }
    
    
    /**
     *  Remove Keyword from HDU
     *  @param keyword keyword to be removed
     */
    void FITShdu::DeleteKey(const key_code& keyword)
    {
        FITSDictionary::iterator it = hdu.find(keyword);
        
        if(it == hdu.end())
            return;
        
        hdu.erase(it);
    }    
}

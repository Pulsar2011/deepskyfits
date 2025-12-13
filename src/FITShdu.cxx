//
//  FITShdu.cxx
//  FitsExtractor
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>

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

// Ensure basic integer sizes for narrowing casts / CFITSIO interoperability
static_assert(CHAR_BIT == 8, "This code assumes 8-bit chars");
static_assert(sizeof(short) >= 2, "Expected short >= 16 bits");
static_assert(sizeof(int) >= 4, "Expected int >= 32 bits");
static_assert(sizeof(long long) >= 8, "Expected long long >= 64 bits");

namespace DSL
{

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
        }
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
        else if(fvalue.find('.') != std::string::npos && (
                fvalue.find('e') != std::string::npos ||
                fvalue.find('E') != std::string::npos    ))
            {
                double val = std::stod(fvalue, nullptr);
                if(std::abs(val) > static_cast<double>(std::numeric_limits<float>::max()) || std::abs(val) < 1.e-7) 
                    ftype = fDouble;
                else
                    ftype = fFloat;
            }
        else if(fvalue.find('.') != std::string::npos)
            {
                double val = std::stod(fvalue, nullptr);
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

        // Empty value -> treat as clearing
        if(value.empty() && ftype == fChar)
        {
            fvalue = value;
            return;
        }
        else if(value.empty())
            throw FITSexception(0, "FITSkeyword", "setValue", "Non string KEYWORD can't be empty.");

        // Numeric format check (integer, float, or exponent)
        static const std::regex num_re("^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$");
        if(!std::regex_match(value, num_re))
        {
            throw FITSexception(0, "FITSkeyword", "setValue", "KEYWORD type unconsistancy for key "+value+".");
        }

        try
        {
            switch(ftype)
            {
                case fShort: // mapped from Process: abs(num) <= 127
                {
                    int64_t num = std::stoll(value);
                    if(std::llabs(num) > INT8_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fShort: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fInt: // abs(num) <= 32767
                {
                    int64_t num = std::stoll(value);
                    if(std::llabs(num) > INT16_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fInt: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fLong: // abs(num) <= 2147483647
                {
                    int64_t num = std::stoll(value);
                    if(std::llabs(num) > INT32_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fLong: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fLongLong:
                {
                    int64_t num = std::stoll(value);
                    if(std::llabs(num) > INT64_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fLong: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fByte: // unsigned small: num <= 1
                {
                    uint64_t num = std::stoll(value);
                    if(num > 1)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fByte: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fUShort: // num <= 255
                {
                    uint64_t num = std::stoll(value);
                    if(num > UINT8_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fUShort: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fUInt: // num <= 65535
                {
                    uint64_t num = std::stoll(value);
                    if(num > UINT16_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fUInt: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fULong: // larger unsigned
                {
                    uint64_t num = std::stoll(value);
                    if(num > UINT32_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fUInt: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fULongLong: // larger unsigned
                {
                    uint64_t num = std::stoull(value);
                    if(num > UINT64_MAX)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fUInt: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fBool:
                {
                    int b = std::stoi(value);
                    if(std::abs(b) > 1)
                        throw FITSexception(0, "FITSkeyword", "setValue", "Value out of range for fBool: "+value);
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fFloat:
                {
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                case fDouble:
                {
                    fvalue.clear();
                    fvalue += std::string(value);
                    break;
                }

                default:
                    // Unknown type -> refuse
                    throw FITSexception(BAD_KEYCHAR, "FITSkeyword", "setValue", "Unknown target type when setting value: "+value);
            }
        }
        catch(const std::invalid_argument& e)
        {
            throw FITSexception(0, "FITSkeyword", "setValue", std::string("Invalid numeric value: ")+value);
        }
        catch(const std::out_of_range& e)
        {
            throw FITSexception(0, "FITSkeyword", "setValue", std::string("Numeric value out of range: ")+value);
        }
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
            hdu.insert(std::pair<key_code,FITSkeyword>(it->first,FITSkeyword(it->second)));
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
            
            size_t cmt_start = hdu_entry.find("\'/");
            size_t cmt_stop  = hdu_entry.rfind("/\'");
            size_t end_str   = hdu_entry.rfind("\'");
            
            if(cmt_start == std::string::npos )
                cmt_start = 0;
            
            if(cmt_stop == std::string::npos )
                cmt_stop = 0;
            
            if(end_str == std::string::npos )
                end_str = 0;
            
            cmt_start=(cmt_stop > cmt_start)?cmt_stop:cmt_start;
            
            cmt_start=(end_str > cmt_start)?end_str:cmt_start;
            
            size_t cmt_pos = hdu_entry.find_first_of("/",cmt_start+2);
            
            size_t step = 10;
            
            if(cmt_pos == std::string::npos)
            {
                value = hdu_entry.substr(step,hdu_entry.size() - step);
                comment = std::string();
            }
            else
            {
                value   = hdu_entry.substr(step,cmt_pos - step);
                comment = hdu_entry.substr(cmt_pos + 1, hdu_entry.size() - cmt_pos - 1 );
            }
            
            if(   key != "COMMENT"
               && key != "HISTORY" )
            {
                value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char c){ return std::isspace(c); }), value.end());
                while(value.find_first_of("'") != std::string::npos)
                    value.erase(value.find_first_of("'"), 1);
            }
            
            FITSDictionary::iterator iKey = hdu.end();
            if(hdu.size() > 1)
                iKey = hdu.find(key);
            
            if(iKey == hdu.end())
            {
                if(key.find("NAXIS") != std::string::npos)
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment, fULongLong)));
                else if(key == "BITPIX")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment, fInt)));
                else if(key == "BZERO")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment, fDouble)));
                else if(key == "BSCALE")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment, fDouble)));
                else if(key == "BLANK")
                    hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment, fUInt)));
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
        
        if(tt == fUndef || tt == fChar)
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
        
        return static_cast<bool>( std::stoi(outString) );
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetShortValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int8_t>( std::stoi(outString) );
    }

    uint8_t FITShdu::GetUInt8ValueForKey  (const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<uint8_t>::min();
        }
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetSUhortValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint8_t>( std::stoul(outString) );
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetIntValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return  static_cast<int16_t>(std::stoi(outString));
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetUIntValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return  static_cast<uint16_t>( std::stoi(outString) );
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int32_t>(std::stol(outString));
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint32_t>(std::stoul(outString));
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetLongLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<int64_t>(std::stoll(outString));
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetLongLongValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return static_cast<uint64_t>(std::stoull(outString));
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetFloatValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return std::stof(outString);
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
        
        if(tt == fUndef || tt == fChar)
            throw FITSexception(0,"FITShdu","GetDoubleValueForKey","Value for key "+key+" isn't a numerical value.");
        
        return std::stod(outString);
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
        uint64_t dim64 = std::stoull(it->second.value());
        size_t dim = static_cast<size_t>(dim64);
        while(it != hdu.end())
        {
            n++;
            it = hdu.find(std::string("NAXIS")+std::to_string(n));
            if(it != hdu.end())
                dim *= std::stoull(it->second.value());
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
            
            switch(it->second.type())
            {
                case fChar:
                    WriteCharValueForKey( it, fptr );
                    break;
                    
                case fShort:
                    WriteInt8ValueForKey( it, fptr );
                    break;
                
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

            if(ibs->second.type() == fDouble || ibs->second.type() == fFloat)
                WriteDoubleValueForKey( ibs, fptr );
            else if (std::stod(ibs->second.value()) < 0)
                WriteInt64ValueForKey( ibs, fptr );
            else
                WriteUInt64ValueForKey( ibs, fptr );

            if(ibz->second.type() == fDouble || ibz->second.type() == fFloat)
                WriteDoubleValueForKey( ibz, fptr );
            else if (std::stod(ibz->second.value()) < 0)
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
        
        int val = std::stoi(it->second.value());
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
        int16_t tmp = static_cast<int16_t>(std::stoi(it->second.value()));
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

        uint16_t tmp = static_cast<uint16_t>(std::stoul(it->second.value()));
        unsigned short val = static_cast<unsigned short>(tmp);
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
    void FITShdu::WriteInt16ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteIntValueForKey","received nullptr");

        if(it == hdu.end())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteIntValueForKey","Reach end of header");

        int32_t tmp = static_cast<int32_t>(std::stoi(it->second.value()));
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

        uint32_t tmp = static_cast<uint32_t>(std::stoul(it->second.value()));
        int val = static_cast<int>(static_cast<int32_t>(tmp));
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
    void FITShdu::WriteInt32ValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");

        if(it == hdu.cend())
            throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");

        int32_t tmp = static_cast<int32_t>(std::stol(it->second.value()));
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

        uint32_t tmp = static_cast<uint32_t>(std::stoul(it->second.value()));
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

        int64_t tmp = static_cast<int64_t>(std::stoll(it->second.value()));
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

        uint64_t tmp = static_cast<uint64_t>(std::stoull(it->second.value()));
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
        
        float val = std::stof(it->second.value());
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
        

        double val = std::stod(it->second.value());

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
    void FITShdu::ValueForKey(const key_code& keyword, const std::string& value, const key_type& kt, const std::string& cmt)
    {
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

                    it->second.setType(kt);
                    
                    throw FITSwarning("FITShdu","ValueForKey",ss.str());
                }
            }
            catch(std::exception &e)
            {
                if((verbose & DSL::verboseLevel::VERBOSE_DETAIL)==DSL::verboseLevel::VERBOSE_DETAIL)
                    std::cerr<<e.what()<<std::endl;
            }

            it->second.setValue(value);
            return;
        }
        
        hdu.insert(std::pair<key_code,FITSkeyword>(keyword, FITSkeyword(value,cmt)));
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
    
#ifdef Darwinx86_64
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
        ValueForKey(keyword, std::to_string(value), fLongLong,cmt);
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
        if(std::fabs(value) > 1e-4 && std::fabs(value) < 1e4)
            ValueForKey(keyword, std::to_string(value), fFloat, cmt);
        else
        {
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::scientific << value;
            ValueForKey(keyword, oss.str(), fFloat, cmt);
        }
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
        if(std::fabs(value) > 1e-4 && std::fabs(value) < 1e4)
            ValueForKey(keyword, std::to_string(value), fDouble, cmt);
        else
        {
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::max_digits10) << std::scientific << value;
            ValueForKey(keyword, oss.str(), fDouble, cmt);
        }
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

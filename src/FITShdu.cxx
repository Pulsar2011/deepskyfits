//
//  FITShdu.cxx
//  FitsExtractor
//
//  Created by GILLARD William on 31/03/15.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>

#include <limits>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <regex>

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
                
            case fLongLong:
                return "TLONGLONG";
                break;
                
            case fULong:
                return "TULONG";
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
    
#pragma mark • ctor/dtor
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
    
#pragma mark • Processing
    /**
     *  @details Fill KEYWORD container.
     *  @param val KEYWORD physical value as string
     *  @param cmt KEYWORD description
     */
    void FITSkeyword::Process(const std::string& val, const std::string& cmt)
    {
        fcomment = std::string(cmt);
        fvalue   = std::string(val);
        
        if( fvalue.size() < 1 )
            return;
        
        bool has_only_digits = false;
        
        if(std::regex_match ( fvalue.begin(), fvalue.end(), std::regex("[+-]*[0-9]+[\\.]*[0-9]*([eE][+-][0-9]+)*")))
            has_only_digits = true;
    
        if(! has_only_digits )
            ftype = fChar;
        else if(fvalue.find('.') != std::string::npos ||
                fvalue.find('e') != std::string::npos ||
                fvalue.find('E') != std::string::npos )
            ftype = fDouble;
        else if(fvalue.find('-') == std::string::npos )
            ftype = fULong;
        else
            ftype = fLongLong;
        
        try
        {
            if(fvalue.size()+fcomment.size() >= 80)
                throw FITSwarning("FITSkeyword","Process","Comment string is too long. The comment string will be trunctated as \n       \033[33m'"+fcomment+"'\033[33m");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            fcomment.resize(80-fvalue.size()-1);
        }
    }
    
#pragma mark • Modifier
    /**
     *  Modify the value associated to the keyword.
     *  @param value The value to associate to the keyword.
     *
     *  @note A check of type consistancy is performed. The type isn't modified but the value is adapted to the base type.
     */
    void FITSkeyword::setValue(const std::string& value)
    {
        if(!is_digits(value) && ftype != fChar)
        {
            throw FITSexception(0, "FITSkeyword","setValue","KEYWORD type unconsistancy for key "+value+".");
        }
        
        if(fvalue == value)
            return;
        
        fvalue.clear();
        
        switch (ftype)
        {
            case fChar:
                fvalue = value;
                break;
                
            case fShort:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fUShort:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fInt:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fUInt:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fLong:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fLongLong:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fULong:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            case fBool:
                fvalue = std::to_string(std::stoi(value) != 0);
                break;
                
            case fFloat:
                fvalue = std::to_string(std::stof(value));
                break;
                
            case fDouble:
                fvalue = std::to_string(std::stod(value));
                break;
                
            case fByte:
                fvalue = std::to_string(std::stoi(value));
                break;
                
            default:
                fvalue = value;
                break;
        }
        
    }
    
#pragma mark • Dump
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
    
#pragma mark - FITShdu class implementation
#pragma mark • ctor/dtor
    
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
        
        cflags = fits_hdr2str(fptr.get(), 0, NULL, 0, &header, &nKey, &status);
        
        if(cflags || header == NULL)
        {
            throw FITSexception(status,"FITShdu","FITShdu");
        }
        else
        {
            Process( std::string(header) );
        }
        
        if(header)
            delete header;
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

#pragma mark • Processing
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
            key.erase(remove_if(key.begin(), key.end(), isspace), key.end());
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
                value.erase(remove_if(value.begin(), value.end(), isspace), value.end());
                while(value.find_first_of("'") != std::string::npos)
                    value.erase(value.find_first_of("'"), 1);
            }
            
            FITSDictionary::iterator iKey = hdu.end();
            if(hdu.size() > 1)
                iKey = hdu.find(key);
            
            if(iKey == hdu.end())
                hdu.insert(std::pair<key_code,FITSkeyword>(key,FITSkeyword(value, comment)));
            else if(   iKey->first == "COMMENT"
                    || iKey->first == "HISTORY" )
            {
                ((iKey->second).value()) += std::string(" ") + value;
            }
            
            pos += 80;
        }
    }
    
#pragma mark • Dump
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

#pragma mark • Accessor
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    int FITShdu::GetIntValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<int>::min();
        }
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetIntValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<int>::min();
        }
        
        return  std::stoi(outString);
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Unsigned Integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    unsigned int FITShdu::GetUIntValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<unsigned int>::min();
        }
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetUIntValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<unsigned int>::min();
        }
        
        return  static_cast<unsigned int>( std::stoi(outString) );
    }
    
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
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetBoolValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            return false;
        }
        
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
    short FITShdu::GetShortValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<short>::min();
        }
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetShortValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<short>::min();
        }
        
        return static_cast<short>( std::stoi(outString) );
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    long FITShdu::GetLongValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<long>::min();
        }
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetLongValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<long>::min();
        }
        
        return std::stol(outString);
    }
    
    /**
     *  @details Retrive physical value for a specific KEYWORD.
     *  @param key KEYWORD name
     *  @return Long long integer value associated to the KEYWORD
     *  @note If the KEYWORD value isn't a numerical value OR if the KEYWORD doesn't exist, NaN is returned
     */
    long long FITShdu::GetLongLongValueForKey(const std::string& key) const
    {
        key_type tt = fUndef;
        
        std::string outString = GetValueForKey(key, tt);
        if(outString.size() < 1)
        {
            return std::numeric_limits<long long>::min();
        }
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetLongLongValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<long long>::min();
        }
        
        return std::stoll(outString);
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
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetFloatValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<float>::min();
        }
        
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
        
        try
        {
            if(tt == fUndef || tt == fChar)
                throw FITSexception(0,"FITShdu","GetDoubleValueForKey","Value for key "+key+" isn't a numerical value.");
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
            
            return std::numeric_limits<double>::min();
        }
        
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
            return std::string();
        
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
    long long FITShdu::GetDimension() const
    {
        FITSDictionary::const_iterator it= hdu.find("NAXIS1");
        if(it == hdu.end())
            return 0;
        
        long long int n = 1;
        long long dim = std::stoll(it->second.value());
        while(it != hdu.end())
        {
            n++;
            it = hdu.find(std::string("NAXIS")+std::to_string(n));
            if(it != hdu.end())
                dim *= std::stoll(it->second.value());
        }
        
        return dim;
        
    }
    
#pragma mark • I/O
    
    /**
     *  @details Write FITS header to the current HDU
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::Write(const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == NULL)
        {
            throw std::invalid_argument("\033[31m[FITShdu::Write]\033[0mreceived nullptr");
            return;
        }
        
        std::cout<<"\033[31m[FITShdu::Write]\033[0m"<<std::endl;
        
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
            
            switch(it->second.type())
            {
                case fChar:
                    WriteCharValueForKey( it, fptr );
                    break;
                    
                case fShort:
                    WriteShortValueForKey( it, fptr );
                    break;
                    
                case fInt:
                    WriteIntValueForKey( it, fptr );
                    break;
                    
                case fLong:
                    WriteLongValueForKey( it, fptr );
                    break;
                    
                case fULong:
                    WriteULongValueForKey( it, fptr );
                    break;
                    
                case fLongLong:
                    WriteLongLongValueForKey( it, fptr );
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

    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteCharValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteCharValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteCharValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        // FIX: Avoid dangling pointer from temporary string
        std::string val_str = std::string();
        val_str += it->second.value();
        char * val = (char*) val_str.c_str();
        int status = 0;
        
        try
        {
            if(fits_update_key(fptr.get(), TSTRING, (char*) it->first.c_str(), val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception &e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteIntValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteIntValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteIntValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        int val = std::stoi(it->second.value());
        int status = 0;
        
        try
        {
            if(fits_update_key(fptr.get(), TINT, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
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
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteBoolValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        int val = std::stoi(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TLOGICAL, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteShortValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteShortValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteShortValueForKeyv","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        short val = std::stoi(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TSHORT, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteLongValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        long int val = std::stol(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TLONG, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteULongValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        unsigned long int val = std::stoul(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TULONG, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
    /**
     *  @details Write or update FITS header KEYWORD and its associated VALUE and COMMENT to the current HDU
     *  @param it : FITSDictionary iterator encapsulating KEYWORD, VALUE and COMMENT filed of the FITS Header keyword
     *  @param fptr : HDU where this header will be written
     */
    void FITShdu::WriteLongLongValueForKey(FITSDictionary::const_iterator it, const std::shared_ptr<fitsfile>& fptr) const
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITShdu","WriteLongValueForKey","received nullptr");
        }
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        long long int val = std::stoll(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TLONGLONG, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
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
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        float val = std::stof(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TFLOAT, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
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
        
        try
        {
            if(it == hdu.end())
                throw FITSexception(SEEK_ERROR,"FITShdu","WriteLongValueForKey","Reach end of header");
        }
        catch (std::exception& e)
        {
            if((verbose & verboseLevel::VERBOSE_BASIC)== verboseLevel::VERBOSE_BASIC)
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        double val = std::stod(it->second.value());
        int status = 0;
        
        try
        {
            if( fits_update_key(fptr.get(), TDOUBLE, (char*) it->first.c_str(), &val, (char*) it->second.comment().c_str(), &status ) )
                throw FITSexception(status,"FITShdu","Write");
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        return;
    }
    
#pragma mark • Modifier

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param kt    Base type of the keyword value.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const std::string& value, const key_type& kt)
    {
        valueForKey(keyword, value, kt, std::string());
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
    void FITShdu::valueForKey(const key_code& keyword, const std::string& value, const key_type& kt, const std::string& cmt)
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
                      <<"VALUE WON'T BE MODIFIED.\033[34m"<<keyword<<"\033[0m -> "<<kt<<" % "<<it->second.type()<<std::endl
                      <<"\033[34m"<<it->first;
                    it->second.Dump(ss);
                    std::cerr<<"\033[0m"<<std::endl;
                    
                    throw FITSexception(BAD_ORDER,"FITShdu","valueForKey",ss.str());
                }
            }
            catch(std::exception &e)
            {
                std::cerr<<e.what()<<std::flush;
                return;
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
    void FITShdu::valueForKey(const key_code& keyword, const std::string& value, const std::string& cmt)
    {
        valueForKey(keyword, value, fChar, cmt);
    }

    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const std::string& value)
    {
        valueForKey(keyword, value, fChar, std::string());
    }
        
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const short& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fShort,cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const short& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const int& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fInt ,cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const int& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const long& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fLong,cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const long& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const long long& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fLongLong,cmt);
        
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const long long& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const size_t& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fLongLong,cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const size_t& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const bool& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fBool, cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const bool& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword   Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const float& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fFloat, cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const float& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const double& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fDouble, cmt);
    }
    
    /**
     *  Modify the value associated to a FITS keyword. If the keyword doesn't exists, a new filed is added to the disctionary with the given KEYWORD and its associated value.
     *
     *  @param keyword Keyword name
     *  @param value Keyword value, has a string
     *  @param cmt commentair describing the keyword.
     *  @note the total number of char of the Keyword value + commentair string shall not exceed 80 character
     */
    void FITShdu::valueForKey(const key_code& keyword, const uint32_t& value, const std::string& cmt)
    {
        valueForKey(keyword, std::to_string(value), fDouble, cmt);
    }
    
    void FITShdu::valueForKey(const key_code& keyword, const double& value)
    {
        valueForKey(keyword, value, std::string());
    }
    
    
    /**
     *  Remove Keyword from HDU
     *  @param keyword keyword to be removed
     */
    void FITShdu::deleteKey(const key_code& keyword)
    {
        FITSDictionary::iterator it = hdu.find(keyword);
        
        if(it == hdu.end())
            return;
        
        hdu.erase(it);
    }
    
#pragma mark • static member
    bool FITShdu::debug = false;
    
}

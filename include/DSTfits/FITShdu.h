//
//  FITShdu.h
//  FitsExtractor
//
//  Created by GILLARD William on 31/03/15.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
/*! @file
 */

#ifndef _DSL_FITShdu_
#define _DSL_FITShdu_

#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include <limits>

#include <fitsio.h>

namespace DSL
{

    /*!
     * @typedef DSL::key_code
     * @details DSL::key_code is a short-cut type used to describe HDU name
     */
    typedef std::string key_code;
    
#pragma mark - FITSkeyword class definition
    /*!
     * @class DSL::FITSkeyword FITShdu.h "fitsExtractor/FITShdu.h"
     * @author GILLARD William
     * @version 1.0
     * @date 31/03/2015
     * @brief HDU value manager.
     * @details Container that is used to manage values and comments of a given HDU KEYWORD
     */
    class FITSkeyword
    {
    public:
        enum key_type
        {
            /*!
             *  @enum DSL::FITSkeyword::key_type
             *  @details FITSkeyword::key_type defines the base type of the FITS KEYWORD value.
             */
            fChar    = TSTRING,       //!< string
            fShort   = TSHORT ,       //!< short integer
            fUShort  = TUSHORT ,       //!< short integer
            fInt     = TINT   ,       //!< integer
            fUInt    = TUINT   ,       //!< integer
            fLong    = TLONG  ,       //!< long integer
            fLongLong= TLONGLONG,     //!< long long integer
            fULong   = TULONG,     //!< long long integer
            fBool    = TLOGICAL,      //!< boolean
            fFloat   = TFLOAT,        //!< floiting poind
            fDouble  = TDOUBLE,       //!< double floiting point
            fByte    = TBYTE,
            fUndef,     //!< undefined
        };
        
        static std::string GetDataType(const key_type&);

#pragma mark • ctor/dtor
        FITSkeyword(std::string, std::string);                //!< Constructor
        FITSkeyword(std::string, std::string, key_type);     //!< Constructor
        FITSkeyword(std::string);                            //!< Constructor
        FITSkeyword(std::string, key_type);     //!< Constructor
        FITSkeyword(const FITSkeyword&);                     //!< Copy constructor
        ~FITSkeyword();                                      //!< Destructor
        
#pragma mark • Accessor
        inline std::string* value() {return &fvalue;}       //!< Get pointer to the FITS KEYWORD value
        inline std::string* comment() {return &fcomment;}   //!< Get pointer to the FITS KEYWORD description
        inline key_type type(){return ftype;}               //!< Get type of the FITS KEYWORD value
        
        inline std::string value() const {return fvalue;}       //!< Get pointer to the FITS KEYWORD value
        inline std::string comment() const {return fcomment;}   //!< Get pointer to the FITS KEYWORD description
        inline key_type type() const {return ftype;}            //!< Get type of the FITS KEYWORD value
        
#pragma mark • Modifier
        void setValue(std::string);                         //!< Set value for key
        
#pragma mark • Dump
        void Dump( std::ostream& ) const;                   //!< Print out FITS KEYWORD value and descritpion
    
    private:
        std::string fvalue;                                 //!< FITS KEEYWORD value
        std::string fcomment;                               //!< FITS KEEYWORD description
        key_type ftype;                                     //!< FITS KEEYWORD base type
        
        bool is_digits(const std::string &str)
        {
            return str.find_first_not_of("-+0123456789.") == std::string::npos;
        }

#pragma mark • Processing
        void Process(std::string, std::string);             //!< Retrive FITS KEYWORD value and description
    
    };
    
    /*!
     @typedef DSL::FITSDictionary 
     @details DSL::FITSDictionary is a map<DSL::key_code,DSL::FITSkeyword> type which describe the FITS HDU dictionary. Each pair of <DSL::key_code,DSL::FITSkeyword> represent a key-value pair with the key-code the HDU name and the DSL::FITSkeyword the HDU value associated to the DSL::key-code.
     */
    typedef std::map<key_code,FITSkeyword> FITSDictionary ;
    
#pragma mark - FITShdu class definition
    /*!
     * @class DSL::FITShdu FITShdu.h "fitsExtractor/FITShdu.h"
     * @author GILLARD William
     * @version 1.0
     * @date 31/03/2015
     * @brief FITS HDU manager
     * @details Store and manage access to FITS header parameters.
     */
    class FITShdu
    {
    private:
        FITSDictionary hdu;                 //!< FITS KEYWORD manager
    
    protected:
#pragma mark • Processing
        void Process(std::string);          //!< Decipher FITS HEADER
        
#pragma mark • I/O
        void        WriteCharValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;
        void        WriteIntValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;     //!< Write KEYWORD integer value to the HDU
        void        WriteBoolValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;    //!< Write KEYWORD boolean value to the HDU
        void        WriteShortValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;   //!< Write KEYWORD short value to the HDU
        void        WriteLongValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;    //!< Write KEYWORD long value to the HDU
        void        WriteULongValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;    //!< Write KEYWORD long value to the HDU
        void        WriteLongLongValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;//!< Write KEYWORD lon long value to the HDU
        void        WriteFloatValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;   //!< Write KEYWORD float value to the HDU
        void        WriteDoubleValueForKey(FITSDictionary::const_iterator, fitsfile *fptr) const;  //!< Write KEYWORD double value to the HDU
        
    public:
#pragma mark • ctor/dtor
        FITShdu();                          //!< Default constructor
        FITShdu(fitsfile *fptr);            //!< Construct from fitsfile
        FITShdu(const FITShdu &);           //!< Copy constructor
        virtual ~FITShdu();                 //!< Destructor
        
#pragma mark • Accessor
        int          GetIntValueForKey(std::string) const;     //!< Retrive KEYWORD integer value
        unsigned int GetUIntValueForKey(std::string) const;     //!< Retrive KEYWORD integer value
        bool         GetBoolValueForKey(std::string) const;    //!< Retrive KEYWORD boolean value
        short        GetShortValueForKey(std::string) const;   //!< Retrive KEYWORD short value
        long         GetLongValueForKey(std::string) const;    //!< Retrive KEYWORD long value
        long long    GetLongLongValueForKey(std::string) const;//!< Retrive KEYWORD lon long value
        float        GetFloatValueForKey(std::string) const;   //!< Retrive KEYWORD float value
        double       GetDoubleValueForKey(std::string) const;  //!< Retrive KEYWORD double value
        
        std::string GetValueForKey(std::string, FITSkeyword::key_type&) const; //!< Retrive KEYWORD  value as a string.
        std::string GetValueForKey(std::string) const;                         //!< Retrive KEYWORD  value as a string.
        
        inline FITSDictionary::const_iterator begin() const {return hdu.begin();}
        inline FITSDictionary::const_iterator end() const {return hdu.end();}
        inline FITSDictionary::const_iterator GetEntry(key_code keyword) const {return hdu.find(keyword);}
        inline bool FindKey(key_code keyword) const {return hdu.find(keyword) != hdu.end();}
        
        long long GetDimension() const;

#pragma mark • Modifier

        void valueForKey(key_code, std::string, FITSkeyword::key_type tk = FITSkeyword::fChar, std::string cmt = std::string());         //!< Set value for key
        void valueForKey(key_code, short    , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, int      , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, long     , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, long long, std::string cmt);           //!< Set value for key
        void valueForKey(key_code, size_t   , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, bool     , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, float    , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, double   , std::string cmt);           //!< Set value for key
        void valueForKey(key_code, uint32_t, std::string cmt);           //!< Set value for key
       
        void valueForKey(key_code, short    );           //!< Set value for key
        void valueForKey(key_code, int      );           //!< Set value for key
        void valueForKey(key_code, long     );           //!< Set value for key
        void valueForKey(key_code, long long);           //!< Set value for key
        void valueForKey(key_code, size_t   );           //!< Set value for key
        void valueForKey(key_code, bool     );           //!< Set value for key
        void valueForKey(key_code, float    );           //!< Set value for key
        void valueForKey(key_code, double   );           //!< Set value for key
        
        void deleteKey(key_code);

#pragma mark • I/O
        void Write(fitsfile *) const;
        
#pragma mark • Dump
        void Dump( std::ostream& ) const;               //!< Print out KEYWORD and their associated value and description.
        
#pragma mark • Static member
        static bool debug;
        
    };
}

#endif /* defined(__FitsExtractor__FITShdu__) */

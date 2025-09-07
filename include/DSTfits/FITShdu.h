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

#include <DSTfits/FITSdata.h>

namespace DSL
{

     /*!
     * @typedef DSL::key_code
     * @details DSL::key_code is a short-cut type used to describe HDU name
     */
    typedef std::string key_code;
    
#pragma region  - FITSkeyword class definition
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
            static std::string GetDataType(const key_type&);

#pragma region  • ctor/dtor
            FITSkeyword(const std::string&, const std::string&);                        //!< Constructor
            FITSkeyword(const std::string&, const std::string&, const key_type&);       //!< Constructor
            FITSkeyword(const std::string&);                                            //!< Constructor
            FITSkeyword(const std::string&, const key_type&);                           //!< Constructor
            FITSkeyword(const FITSkeyword&);                                            //!< Copy constructor
            ~FITSkeyword();                                                             //!< Destructor
        
#pragma endregion
#pragma region  • Accessor
            inline std::string& value()   {return fvalue;}                 //!< Get pointer to the FITS KEYWORD value
            inline std::string& comment() {return fcomment;}               //!< Get pointer to the FITS KEYWORD description
            inline key_type&    type()    {return ftype;}                  //!< Get type of the FITS KEYWORD value

            inline const std::string& value()   const {return fvalue;}     //!< Get pointer to the FITS KEYWORD value
            inline const std::string& comment() const {return fcomment;}   //!< Get pointer to the FITS KEYWORD description
            inline const key_type&    type()    const {return ftype;}      //!< Get type of the FITS KEYWORD value
        
#pragma endregion
#pragma region  • Modifier
            void setValue(const std::string&);                         //!< Set value for key
        
#pragma endregion
#pragma region  • Dump
            void Dump( std::ostream& ) const;                   //!< Print out FITS KEYWORD value and descritpion
    
        private:
            std::string fvalue;                                 //!< FITS KEEYWORD value
            std::string fcomment;                               //!< FITS KEEYWORD description
            key_type ftype;                                     //!< FITS KEEYWORD base type
            
            bool is_digits(const std::string &str)
            {
                return str.find_first_not_of("-+0123456789.") == std::string::npos;
            }

#pragma endregion
#pragma region  • Processing
            void Process(const std::string&, const std::string&);             //!< Retrive FITS KEYWORD value and description
#pragma endregion
    };
#pragma endregion
#pragma region  - FITSDictionary definition
    
    /*!
     @typedef DSL::FITSDictionary 
     @details DSL::FITSDictionary is a map<DSL::key_code,DSL::FITSkeyword> type which describe the FITS HDU dictionary. Each pair of <DSL::key_code,DSL::FITSkeyword> represent a key-value pair with the key-code the HDU name and the DSL::FITSkeyword the HDU value associated to the DSL::key-code.
     */
    typedef std::map<key_code,FITSkeyword> FITSDictionary ;
    

#pragma endregion

#pragma region  - FITShdu class definition
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
#pragma region  • Processing
        void Process(const std::string&);          //!< Decipher FITS HEADER
        
#pragma endregion
#pragma region  • I/O
        void        WriteCharValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;
        void        WriteIntValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;     //!< Write KEYWORD integer value to the HDU
        void        WriteBoolValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;    //!< Write KEYWORD boolean value to the HDU
        void        WriteShortValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;   //!< Write KEYWORD short value to the HDU
        void        WriteLongValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;    //!< Write KEYWORD long value to the HDU
        void        WriteULongValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;    //!< Write KEYWORD long value to the HDU
        void        WriteLongLongValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;//!< Write KEYWORD lon long value to the HDU
        void        WriteFloatValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;   //!< Write KEYWORD float value to the HDU
        void        WriteDoubleValueForKey(FITSDictionary::const_iterator, const std::shared_ptr<fitsfile>& fptr) const;  //!< Write KEYWORD double value to the HDU
        
    public:
#pragma endregion
#pragma region  • ctor/dtor
        FITShdu();                          //!< Default constructor
        FITShdu(const std::shared_ptr<fitsfile>& fptr);            //!< Construct from fitsfile
        FITShdu(const FITShdu &);           //!< Copy constructor
        virtual ~FITShdu();                 //!< Destructor
        
#pragma endregion
#pragma region  • Accessor
        int          GetIntValueForKey     (const std::string&) const;     //!< Retrive KEYWORD integer value
        unsigned int GetUIntValueForKey    (const std::string&) const;     //!< Retrive KEYWORD integer value
        bool         GetBoolValueForKey    (const std::string&) const;    //!< Retrive KEYWORD boolean value
        short        GetShortValueForKey   (const std::string&) const;   //!< Retrive KEYWORD short value
        long         GetLongValueForKey    (const std::string&) const;    //!< Retrive KEYWORD long value
        long long    GetLongLongValueForKey(const std::string&) const;//!< Retrive KEYWORD lon long value
        float        GetFloatValueForKey   (const std::string&) const;   //!< Retrive KEYWORD float value
        double       GetDoubleValueForKey  (const std::string&) const;  //!< Retrive KEYWORD double value
        
        std::string GetValueForKey(const std::string&, key_type&) const; //!< Retrive KEYWORD  value as a string.
        std::string GetValueForKey(const std::string&) const;                         //!< Retrive KEYWORD  value as a string.
        
        inline FITSDictionary::const_iterator begin() const {return hdu.begin();}
        inline FITSDictionary::const_iterator end() const {return hdu.end();}
        inline FITSDictionary::const_iterator GetEntry(key_code keyword) const {return hdu.find(keyword);}
        inline bool FindKey(key_code keyword) const {return hdu.find(keyword) != hdu.end();}
        
        long long GetDimension() const;

#pragma endregion
#pragma region  • Modifier

        void valueForKey(const key_code&, const std::string&, const key_type& tk);                             //!< Set value for key
        void valueForKey(const key_code&, const std::string&, const key_type& tk, const std::string&);         //!< Set value for key
        void valueForKey(const key_code&, const std::string&, const std::string&);                             //!< Set value for key
        void valueForKey(const key_code&, const std::string&);                                                 //!< Set value for key
        void valueForKey(const key_code&, const short&      , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const int&        , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const long&       , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const long long&  , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const size_t&     , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const bool&       , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const float&      , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const double&     , const std::string&);           //!< Set value for key
        void valueForKey(const key_code&, const uint32_t&   , const std::string&);           //!< Set value for key
       
        void valueForKey(const key_code&, const short&    );           //!< Set value for key
        void valueForKey(const key_code&, const int&      );           //!< Set value for key
        void valueForKey(const key_code&, const long&     );           //!< Set value for key
        void valueForKey(const key_code&, const long long&);           //!< Set value for key
        void valueForKey(const key_code&, const size_t&   );           //!< Set value for key
        void valueForKey(const key_code&, const bool&     );           //!< Set value for key
        void valueForKey(const key_code&, const float&    );           //!< Set value for key
        void valueForKey(const key_code&, const double&   );           //!< Set value for key
        
        void deleteKey(const key_code&);

#pragma endregion
#pragma region  • I/O
        void Write(const std::shared_ptr<fitsfile>& ) const;
        
#pragma endregion
#pragma region  • Dump
        void Dump( std::ostream& ) const;               //!< Print out KEYWORD and their associated value and description.
        
#pragma endregion
#pragma region  • Static member
        static bool debug;
#pragma endregion
    };
#pragma endregion
}

#endif /* defined(__FitsExtractor__FITShdu__) */

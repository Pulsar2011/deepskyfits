//
//  FITSdata.h
//  DeepSkyLib
//
//  Created by GILLARD William on 07/08/17.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#ifndef _DSL_FITSdata_
#define _DSL_FITSdata_

#include <fitsio.h>

namespace DSL
{
    
    enum class verboseLevel: uint8_t {
        VERBOSE_NONE    = 0x00,
        VERBOSE_BASIC   = 0x01,
        VERBOSE_DETAIL  = 0x0F,
        VERBOSE_HDU     = 0x02,
        VERBOSE_IMG     = 0x04,
        VERBOSE_TBL     = 0x08,
        VERBOSE_DEBUG   = 0xFF
    };

    inline verboseLevel verbose = verboseLevel::VERBOSE_NONE;

    verboseLevel operator|(verboseLevel a, verboseLevel b);
    verboseLevel operator&(verboseLevel a, verboseLevel b);
    verboseLevel& operator|=(verboseLevel& a, verboseLevel b);
    verboseLevel& operator&=(verboseLevel& a, verboseLevel b);
    verboseLevel operator~(verboseLevel a);
    std::ostream& operator<<(std::ostream& os, verboseLevel v);

    enum ttype
    {
        /**
         * @enum Possible type for FITS HDU bloc
         */
        tascii = ASCII_TBL, //!< FITS ASCII table
        tbinary= BINARY_TBL, //!< FITS binary table
        timg   = IMAGE_HDU, //!< FITS image or datacube
        tany   = ANY_HDU,  //!< Any of those listed above
    };
    
    enum dtype
    {
        /**
         * @enum Possible type for FITS embeded data
         */
        tsbyte     = TSBYTE,
        tshort     = TSHORT,    // int8_t
        tushort    = TUSHORT,   // uint8_t
        tint       = TINT,      // int16_t
        tuint      = TUINT,     // uint16_t
        tlong      = TLONG,     // int32_t
        tulong     = TULONG,    // uint32_t
        tlonglong  = TLONGLONG, // int64_t
        tulonglong = TULONGLONG,// uint64_t
        tfloat     = TFLOAT,
        tdouble    = TDOUBLE,
        tstring    = TSTRING,
        tlogical   = TLOGICAL,
        tbit       = TBIT,
        tbyte      = TBYTE,
        tcplx      = TCOMPLEX,
        tdbcplx    = TDBLCOMPLEX,
        tnone      = 666,
    };

    enum key_type
    {
        /*!
         *  @enum Possible type for FITS keywords
         *  @details key_type defines the base type of the FITS KEYWORD value.
         */
        fChar     = TSTRING,       //!< string
        fShort    = TSHORT ,       //!< int8_t
        fUShort   = TUSHORT ,      //!< uint8_t
        fInt      = TINT   ,       //!< int16_t
        fUInt     = TUINT   ,      //!< uint16_t
        fLong     = TLONG  ,       //!< int32_t
        fULong    = TULONG  ,      //!< uint32_t
        fLongLong = TLONGLONG,     //!< int64_t
        fULongLong= TULONGLONG,        //!< uint64_t
        fBool     = TLOGICAL,      //!< boolean
        fFloat    = TFLOAT,        //!< floiting poind
        fDouble   = TDOUBLE,       //!< double floiting point
        fByte     = TBYTE,
        fUndef,                   //!< undefined
    };
    
}

#endif

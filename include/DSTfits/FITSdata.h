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
        tsbyte   = TSBYTE,
        tshort   = TSHORT,
        tushort  = TUSHORT,
        tint     = TINT,
        tuint    = TUINT,
        tlong    = TLONG,
        tlonglong= TLONGLONG,
        tulong   = TULONG,
        tfloat   = TFLOAT,
        tdouble  = TDOUBLE,
        tstring  = TSTRING,
        tlogical = TLOGICAL,
        tbit     = TBIT,
        tbyte    = TBYTE,
        tcplx    = TCOMPLEX,
        tdbcplx  = TDBLCOMPLEX,
        tnone    = 666,
    };
    
}

#endif

//
//  FITSwcs.h
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#ifndef _FITSwcs_
#define _FITSwcs_

#include <memory>
#include <string>

// WCSLIB includes
#include <wcslib/wcs.h>
#include <wcslib/wcshdr.h>

// DeepSkyTools dependency
#include "FITShdu.h"



namespace DSL
{
#pragma region - FITSwcs class definition
    /**
     *  @class FITSwcs
     *  @brief Class to handle WCS information in FITS files
     *  @details This class is used to handle WCS information in FITS files. It is C++ wrapper to the WCSLIB library it depend to.
     */
    class FITSwcs
    {
        protected:

            std::shared_ptr<struct wcsprm> fwcs;  //!< Pointer to the WCS structure from WCSLIB
            int fwcs_status;                      //!< Status of the WCS structure
            int fnwcs;                             //!< Number of WCS in the WCSLIB structures
#pragma region * protected member function
        
        /**
         * @brief Initialize WCS from a FITS image HDU
         *  @param fptr: Pointer to the FITS file
         *  @param relax: Relaxation parameter for WCS parsing
         *  @param ctrl: Control parameter for WCS parsing
         */
        void initFromImg(const std::shared_ptr<fitsfile>& fptr, const int& relax, const int& ctrl);

        /**
         * @brief Initialize WCS from a FITS image HDU
         *  @param fptr: Pointer to the FITS header
         *  @param relax: Relaxation parameter for WCS parsing
         *  @param ctrl: Control parameter for WCS parsing
         */
        void initFromImg(const FITShdu& fptr, const int& relax, const int& ctrl);

        /**
         * @brief Initialize WCS from a FITS image HDU
         *  @param fptr: Pointer to the FITS header
         *  @param relax: Relaxation parameter for WCS parsing
         *  @param ctrl: Control parameter for WCS parsing
         */
        void initFromImg(const std::shared_ptr<FITShdu>& fptr, const int& relax, const int& ctrl);

        /**
         * @brief Initialize WCS from a FITS image HDU
         *  @param header: FITS HDU header string
         *  @param relax: Relaxation parameter for WCS parsing
         *  @param ctrl: Control parameter for WCS parsing
         */
        void initFromString(const std::string& header, const int& relax, const int& ctrl);

#pragma endregion


        public:

#pragma region * ctor/dtor
            /**
             *  @details Default constructor
             */
            FITSwcs():fwcs(nullptr), fwcs_status(0),fnwcs(0) {};

            /**
             *  @brief Constructor from a FITS HDU
             *  @details FITSwcs constructor from a FITS HDU: it reads the HDU to extract WCS information if any are found. 
             *  @param hdu: Reference to the FITS HDU
             *  @param relax: Relaxation parameter for WCS parsing (default: WCSHDR_all)
             *  @param ctrl: Control parameter for WCS parsing (default: 0)
             */
            FITSwcs(const FITShdu& hdu, int relax = WCSHDR_all, int ctrl = 0);

            /**
             *  @brief Constructor from a FITS HDU
             *  @details FITSwcs constructor from a FITS HDU: it reads the HDU to extract WCS information if any are found. 
             *  @param hdu: Reference to the FITS HDU
             *  @param relax: Relaxation parameter for WCS parsing (default: WCSHDR_all)
             *  @param ctrl: Control parameter for WCS parsing (default: 0)
             */
            FITSwcs(const std::shared_ptr<FITShdu>& hdu, int relax = WCSHDR_all, int ctrl = 0);

            /**
             *  @brief Constructor from a FITS file pointer
             *  @details FITSwcs constructor from a FITS file pointer: it reads the current HDU to extract WCS information if any are found. 
             *  @param fptr: Pointer to the FITS file
             *  @param relax: Relaxation parameter for WCS parsing (default: WCSHDR_all)
             *  @param ctrl: Control parameter for WCS parsing (default: 0)
             */
            FITSwcs(const std::shared_ptr<fitsfile>& fptr, int relax = WCSHDR_all, int ctrl = 0);

            /**
             *  @details Destructor
             */
            ~FITSwcs();
#pragma endregion
#pragma region * Accessor

            inline int getStatus() const { return fwcs_status; }   //!< Get the WCS status
            inline int getNumberOfWCS() const { return fnwcs; }    //!< Get the number of WCS structures
            
            double CRPIX(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CRPIX(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            double CRVAL(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CRVAL(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            double CDELT(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CDELT(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            
            
#pragma endregion
    };
#pragma endregion

    
} // namespace DST



#endif
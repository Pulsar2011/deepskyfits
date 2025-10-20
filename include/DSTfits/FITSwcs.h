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
    typedef std::vector<double> pixelCoords;
    typedef std::vector<double> worldCoords;

    typedef std::vector<pixelCoords> pixelVectors;
    typedef std::vector<worldCoords> worldVectors;

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

#pragma region * Conversion
            /**
             * @brief Convert pixel coordinates to world coordinates
             * 
             * @param wcsIndex World Coordinate System index
             * @param npixels Number of pixel coordinates
             * @param pixcrd Array of pixel coordinates
             * @param world Array to store world coordinates
             */
            worldVectors pixel2world(const size_t&, const pixelVectors&);

            /**
             * @brief Convert world coordinates to pixel coordinates
             * 
             * @param wcsIndex World Coordinate System index
             * @param npixels Number of world coordinates
             * @param world Array of world coordinates
             * @param pixcrd Array to store pixel coordinates
             */
            pixelVectors world2pixel(const size_t& wcsIndex, const worldVectors&);

            /**
             * @brief Convert wcs to header string
             * 
             * @param wcsIndex Index of the WCS to convert. Default value is 0, all WCS present in the structure will be converted to aa header string.
             * @return std::string 
             */
            std::string asString(const int& wcsIndex=-1); //!< Get WCS header as a string for a given WCS index
            FITShdu asFITShdu(const int& wcsIndex=-1); //!< Get WCS header as a string for a given WCS index
#pragma region 
    };
#pragma endregion

    
} // namespace DST



#endif
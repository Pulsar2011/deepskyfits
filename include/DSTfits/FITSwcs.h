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
            mutable int fwcs_status;                      //!< Status of the WCS structure
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
            FITSwcs():fwcs(nullptr), fwcs_status(WCSERR_UNSET),fnwcs(0) {};

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


            FITSwcs(const FITSwcs& other); //!< Copy constructor
            FITSwcs(const FITSwcs& other, const size_t& idx); //!< Copy constructor

            /**
             * @brief Construct a new FITSwcs object for a subimage defined by pixel ranges
             * 
             * @param other Input FITSwcs object
             * @param idx Index of the WCS to extract
             * @param crpix Index of the pixel to be used as new reference in the subfrme WCS
             */
            FITSwcs(const FITSwcs& other, const size_t& idx, const std::vector<size_t>& crpix); //!< Copy constructor
            FITSwcs(const FITSwcs& other, const size_t& idx, const std::initializer_list<size_t>& crpix): FITSwcs(other, idx, std::vector<size_t>(crpix)) {} //!< Copy constructor

            FITSwcs(const FITSwcs& other, const size_t& idx, const std::vector<double>& crpix); //!< Copy constructor
            FITSwcs(const FITSwcs& other, const size_t& idx, const std::initializer_list<double>& crpix): FITSwcs(other, idx, std::vector<double>(crpix)) {} //!< Copy constructor
            

            /**
             *  @details Destructor
             */
            ~FITSwcs();

            /**
             * @brief Swap content of two FITSwcs objects
             * 
             * @param first wsc object to swap with \c second
             * @param second wsc object to swap with \c first
             */
            static void swap(FITSwcs& first, FITSwcs& second) noexcept; //!< Swap content of two FITSwcs objects

#pragma endregion
#pragma region * Accessor

            inline int getStatus() const { return fwcs_status; }   //!< Get the WCS status
            inline int getNumberOfWCS() const { return fnwcs; }    //!< Get the number of WCS structures
            size_t getNumberOfAxis(const size_t&) const;              //!< Get the number of WCS structures
            const std::string getSuffix(const size_t&) const;        //!< Get the WCS suffix for a given WCS index
            
            double CRPIX(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CRPIX(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            double CRVAL(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CRVAL(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            double CDELT(const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis
            double CDELT(const size_t&, const size_t&) const; //!< Get the CRPIX value for a given WCS index and axis

            std::vector< std::string > CTYPE(const size_t&) const; //!< Get the CTYPE values for a given WCS index
            
#pragma endregion

#pragma region * Conversion

            /**
             * @brief Change the celestial coordinate system to new references
             * 
             * @param wcsIndex World Coordinate System index
             * @param newRefPole Longitude and latitude in the new celestial coordinate system of the pole in the old reference system [deg]
             * @param oldLon Longitude in the original celestial coordinate system of the pole of the new system [deg]
             * @param newTYPE Pair of strings defining the new celestial coordinate types for the new system (e.g. ("RA","DEC"), ("GLON","GLAT"), ("ELON","ELAT"), ...)
             * @param alt Code for alternate coordinate descriptions (i.e. the 'a' in keyword names such as CTYPEia). This is blank for the primary coordinate description, or one of the 26 upper-case letters, A-Z. May be set to the null pointer, or null string if no change is required.
             */
            void changeCelestialCorrds(const size_t& wcsIndex,
                const worldCoords& newRefPole,
                const double& oldLon,
                const std::pair<std::string,std::string>& newTYPE,
                const std::string& alt);

            /**
             * @brief Change the celestial coordinate system to new references
             * 
             * @param wcsIndex World Coordinate System index
             * @param newRefPole Longitude and latitude in the new celestial coordinate system of the pole in the old reference system [deg]
             * @param oldLon Longitude in the original celestial coordinate system of the pole of the new system [deg]
             * @param newTYPE Pair of strings defining the new celestial coordinate types for the new system (e.g. ("RA","DEC"), ("GLON","GLAT"), ("ELON","ELAT"), ...)
             * @param radsys New reference frame (e.g. "FK5", "FK4", "ICRS", "GALACTIC", "ECLIPTIC", ...). Used when transforming to equatorial coordinates, identified by \c newTYPE ( \c "RA" ; \c "DEC" ). May be set to empty string to preserve the current value. 
             * @param equinox Equinox of the new reference frame in years. Used when transforming to equatorial coordinates, identified by \c newTYPE ( \c "RA" ; \c "DEC" ). 
             * @param alt Code for alternate coordinate descriptions (i.e. the 'a' in keyword names such as CTYPEia). This is blank for the primary coordinate description, or one of the 26 upper-case letters, A-Z. May be set to the null pointer, or null string if no change is required.
             */
            void changeCelestialCorrds(const size_t& wcsIndex,
                const worldCoords& newRefPole,
                const double& oldLon,
                const std::pair<std::string,std::string>& newTYPE,
                const std::string& radsys,
                const double& equinox,
                const std::string& alt);

            /**
             * @brief Convert pixel coordinates to world coordinates
             * 
             * @param wcsIndex World Coordinate System index
             * @param npixels Number of pixel coordinates
             * @param pixcrd Array of pixel coordinates
             * @param world Array to store world coordinates
             */
            worldVectors pixel2world(const size_t&, const pixelVectors&) const;

            /**
             * @brief Convert world coordinates to pixel coordinates
             * 
             * @param wcsIndex World Coordinate System index
             * @param npixels Number of world coordinates
             * @param world Array of world coordinates
             * @param pixcrd Array to store pixel coordinates
             */
            pixelVectors world2pixel(const size_t& wcsIndex, const worldVectors&) const;

            /**
             * @brief Convert wcs to header string
             * 
             * @param wcsIndex Index of the WCS to convert. Default value is -1, all WCS present in the structure will be converted to aa header string.
             * @return std::string 
             */
            std::string asString(const int& wcsIndex=-1) const; //!< Get WCS header as a string for a given WCS index

            /**
             * @brief Convert wcs to header 
             * 
             * @param wcsIndex Index of the WCS to convert. Default value is -1, all WCS present in the structure will be converted to a FITS header.
             * @return FITShdu dictionary with WCS keycards 
             */
            FITShdu asFITShdu(const int& wcsIndex=-1) const; //!< Get WCS header as a string for a given WCS index
#pragma endregion 
#pragma region * I/O
            void Dump(const int& wcsIndex=-1) const; //!< Dump WCS information to an output stream
#pragma endregion
    };
#pragma endregion

    
} // namespace DST



#endif
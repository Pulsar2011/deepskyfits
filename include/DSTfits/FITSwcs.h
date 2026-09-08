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

#include <limits>
#include <memory>
#include <string>

// WCSLIB includes
#include <wcslib/wcs.h>
#include <wcslib/wcshdr.h>

// DeepSkyTools dependency
#include "FITShdu.h"
#include "FITSdata.h"



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
            std::vector< std::string > CUNIT(const size_t&) const; //!< Get the CTYPE values for a given WCS index
            
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

#pragma region SkyCoordinates calss definition
    /**
     *  @class SkyCoordinates
     *  @brief Abstract base of the celestial coordinate hierarchy: one direction on the sky,
     *         together with the star's space motion and the date it was measured at.
     *
     *  @details
     *  A position is described by three independent things, and keeping them apart is what the
     *  whole hierarchy is organised around:
     *
     *  - **System** (\c CoordSystem) — which pole and origin the two angles are measured from:
     *    equatorial (RA/Dec), galactic (l/b) or ecliptic (&lambda;/&beta;). The concrete
     *    subclasses \c EquatorialCoordinates, \c GalacticCoordinates and \c EclipticCoordinates
     *    each fix one system and name the angles accordingly.
     *  - **Frame** (\c CoordFrame) — the orientation of the axes: FK4, FK5, ICRS, HIPPARCOS.
     *    Changing frame *rotates* the position.
     *  - **Epoch** (\c getEpoch()) — the date the position is valid at. A star with proper motion
     *    has moved by the time you observe it; \c EquatorialCoordinates::atEpoch() *transports*
     *    the position along its own motion without rotating anything.
     *
     *  The conversions are implemented on top of SuperNOVAS, which owns every constant and every
     *  rotation matrix involved; nothing is reimplemented here.
     *
     *  @par Units
     *  Angles are in **degrees** throughout the public interface (the SuperNOVAS convention of
     *  right ascension in hours appears only in \c EquatorialCoordinates::convertFrame(), where
     *  it is called out explicitly). Proper motions are in **mas/yr**, parallax in **mas**,
     *  radial velocity in **km/s**, and epochs are **Julian dates on the TT scale**.
     *
     *  @par Output-parameter convention
     *  Conversions do not return a new object; they fill one you supply:
     *
     *  ```c++
     *      DSL::EquatorialCoordinates m31(10.68470, 41.26875);   // ICRS by default
     *      DSL::GalacticCoordinates  gal;
     *      m31.toGalactic(&gal);                                 // gal now holds (l, b)
     *  ```
     *
     *  The destination may be any concrete \c SkyCoordinates, which is why \c assign() is public:
     *  it lets a conversion populate a sibling type it does not inherit from. Every conversion
     *  throws \c std::invalid_argument on a null destination.
     *
     *  @par Choosing a frame
     *  Each conversion has a one-argument form that keeps the current frame and a two-argument
     *  form that also rotates into a requested one:
     *
     *  ```c++
     *      DSL::EclipticCoordinates ecl;
     *      m31.toEcliptic(&ecl);                          // stays in M31's own frame
     *      m31.toEcliptic(&ecl, DSL::CoordFrame::FK4);    // ... and precess to B1950.0
     *  ```
     *
     *  @sa EquatorialCoordinates, GalacticCoordinates, EclipticCoordinates
     */
    class SkyCoordinates
    {
        public:
            /**
             * @brief The spherical coordinate system the two angles are measured in.
             * @warning \c UNDEFINED and \c EQUATORIAL are both 0, so an unset system is
             *          indistinguishable from an equatorial one. Treat \c UNDEFINED as
             *          "equatorial by default" rather than as a detectable error state.
             */
            enum class CoordSystem
            {
                UNDEFINED  = 0, //!< No system recorded; numerically identical to \c EQUATORIAL
                EQUATORIAL = 0, //!< Right ascension and declination, about the celestial pole
                GALACTIC   = 1, //!< Galactic longitude and latitude, about the north galactic pole
                ECLIPTIC   = 2  //!< Ecliptic longitude and latitude, about the ecliptic pole
            };

            /**
             * @brief Celestial reference frames.
             * @details A frame fixes the *orientation* of the axes. Each one also has a
             *          conventional epoch at which catalogues referred to it are published
             *          (see \c epochOf()), but orientation and epoch are independent here:
             *          changing frame rotates the axes, while \c EquatorialCoordinates::atEpoch()
             *          moves the star along its own space motion. HIPPARCOS shares the ICRS
             *          orientation and differs from it only by its J1991.25 epoch.
             */
            enum class CoordFrame
            {
                UNDEFINED = 0,
                FK4       = 1, //!< FK4, mean equator and equinox of B1950.0
                FK5       = 2, //!< FK5, mean equator and equinox of J2000.0
                ICRS      = 3, //!< International Celestial Reference System, epoch J2000.0
                HIPPARCOS = 4  //!< ICRS orientation at the Hipparcos catalogue epoch, J1991.25
            };

        protected :
#pragma region * Generic accessors
            // The angles are named phi/theta here because the base class does not know which
            // system it is holding. Each derived class re-exposes them under the name its own
            // system uses: getRA()/getDEC(), getGLON()/getGLAT(), getELON()/getELAT().

            void setPhi(const double& phi);                //!< @param phi [deg] longitude-like angle
            void setTheta(const double& theta);            //!< @param theta [deg] latitude-like angle

            /**
             * @brief Set the proper motion.
             * @param pmPhi   [mas/yr] rate along phi, already multiplied by cos(theta)
             * @param pmTheta [mas/yr] rate along theta
             */
            void setProperMotion(const double& pmPhi, const double& pmTheta);

            void setParallax(const double& parallax);              //!< @param parallax [mas] 0 means unknown
            void setRadialVelocity(const double& radialVelocity);  //!< @param radialVelocity [km/s] barycentric
            void setEpoch(const double& epoch);                    //!< @param epoch [day] Julian date (TT)

            double getPhi() const;               //!< @return [deg] the longitude-like angle
            double getTheta() const;             //!< @return [deg] the latitude-like angle
            CoordFrame getFrame() const;         //!< @return the reference frame the position is expressed in
            CoordSystem getSystem() const;       //!< @return the spherical system the angles are measured in
            double getProperMotionPhi() const;   //!< [mas/yr] proper motion in phi (pre-multiplied by cos(theta) for equatorial RA)
            double getProperMotionTheta() const; //!< [mas/yr] proper motion in theta
#pragma endregion

#pragma region * ctor
            // Protected: the class is abstract, and a bare direction with no system attached is
            // not a thing callers should be able to build. Construct a concrete subclass instead.

            //! @brief Origin of an undefined system and frame, at epoch J2000.0.
            SkyCoordinates();

            /**
             * @brief A direction with no system, frame or motion attached.
             * @param phi   [deg] longitude-like angle
             * @param theta [deg] latitude-like angle
             */
            SkyCoordinates(const double& phi, const double& theta);

            /**
             * @brief A direction in a named system and frame.
             * @param phi   [deg] longitude-like angle
             * @param theta [deg] latitude-like angle
             * @param sys   Spherical system the angles are measured in
             * @param frame Reference frame; the epoch defaults to that frame's standard epoch
             */
            SkyCoordinates(const double& phi, const double& theta, CoordSystem sys, CoordFrame frame=CoordFrame::UNDEFINED);

            /**
             * @brief A moving source with no system or frame attached.
             * @param pmPhi   [mas/yr] proper motion along phi, including the cos(theta) factor
             * @param pmTheta [mas/yr] proper motion along theta
             */
            SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta);

            /**
             * @brief A moving source in a named system and frame.
             * @param pmPhi   [mas/yr] proper motion along phi, including the cos(theta) factor
             * @param pmTheta [mas/yr] proper motion along theta
             * @param sys     Spherical system the angles are measured in
             * @param frame   Reference frame; the epoch defaults to that frame's standard epoch
             */
            SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta, CoordSystem sys, CoordFrame frame=CoordFrame::UNDEFINED);

            /**
             * @brief Fully specified constructor, including the distance, the line-of-sight
             *        velocity and the epoch the position refers to.
             * @param phi             [deg] longitude-like angle
             * @param theta           [deg] latitude-like angle
             * @param pmPhi           [mas/yr] proper motion along phi, including the cos(theta) factor
             * @param pmTheta         [mas/yr] proper motion along theta
             * @param parallax        [mas] trigonometric parallax; 0 means "unknown/infinitely far"
             * @param radialVelocity  [km/s] barycentric radial velocity
             * @param epoch           [day] Julian date (TT) at which \c phi and \c theta are valid.
             *                        Pass NaN to mean "the standard epoch of \c frame".
             * @param sys             Spherical system the angles are measured in
             * @param frame           Reference frame the position is expressed in
             */
            SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                           const double& parallax, const double& radialVelocity, const double& epoch,
                           CoordSystem sys, CoordFrame frame=CoordFrame::UNDEFINED);
#pragma endregion

        public:
            virtual ~SkyCoordinates() = default;

#pragma region * Space motion
            double getParallax() const;       //!< [mas] trigonometric parallax; 0 means unknown
            double getRadialVelocity() const; //!< [km/s] barycentric radial velocity
            double getEpoch() const;          //!< [day] Julian date (TT) at which the stored position is valid
#pragma endregion

#pragma region * Epochs
            // Thin forwards to SuperNOVAS; no date arithmetic is reimplemented here.

            /**
             * @brief The epoch a reference frame is conventionally published at.
             * @details Resolved by SuperNOVAS \c novas_epoch() from the frame's system name,
             *          so the values follow the library rather than a local copy of them:
             *          FK4 -> B1950.0, FK5 and ICRS -> J2000.0, HIPPARCOS -> J1991.25.
             *          An undefined frame yields J2000.0.
             * @return [day] Julian date (TT) of the frame's standard epoch
             */
            static double epochOf(CoordFrame frame);

            /**
             * @brief The epoch named by an astronomical epoch string.
             * @details Forwards to SuperNOVAS \c novas_epoch(), which accepts the fixed names
             *          ("ICRS", "FK4", "FK5", "HIP", "B1950.0", "J2000.0") as well as any
             *          Besselian or Julian year ("B1933.193", "J2022.033"). Without a leading
             *          'B' or 'J', epochs before 1984.0 are read as Besselian and later ones
             *          as Julian.
             * @param spec Epoch specification, e.g. \c "J2026.7"
             * @return [day] the corresponding Julian date (TT)
             * @throws std::invalid_argument if \c spec is not a recognised epoch
             */
            static double epochOf(const std::string& spec);

            /**
             * @brief The current instant, as a Julian date suitable for \c atEpoch().
             * @details Forwards to SuperNOVAS \c novas_set_current_time() with the leap-second
             *          count from \c novas_lookup_leap(), then reads the result on the TT scale.
             *          UT1-UTC is taken as zero: it is bounded by 0.9 s, which is nine orders of
             *          magnitude below the accuracy of any proper motion.
             * @return [day] Julian date (TT) of now
             * @throws std::runtime_error if the system clock cannot be converted
             */
            static double currentEpoch();
#pragma endregion

            /**
             * @brief Great-circle angle between this direction and another.
             * @details Uses the haversine formula, which stays accurate for very small
             *          separations where the law of cosines loses precision, and switches to
             *          the law of cosines beyond 90 deg where the haversine loses it instead.
             * @param other The direction to measure to
             * @return [deg] the angular separation, in the range 0 to 180
             * @warning The two angles are compared as stored. If \c other is in a different
             *          system, frame or epoch, convert it first or the result is meaningless.
             */
            double angularSeparation(const SkyCoordinates& other) const;

#pragma region * Assignment
            /**
             * @brief Assign new spherical coordinates, system and frame to this object.
             * @details Public so that conversion routines of sibling derived classes (e.g.
             *          EquatorialCoordinates::toGalactic) can populate an \c output object
             *          of another concrete type without requiring shared inheritance.
             *          Proper motion is reset to zero; the parallax, radial velocity and epoch
             *          already held by this object are kept.
             * @param phi   [deg] longitude-like angle
             * @param theta [deg] latitude-like angle
             * @param sys   Spherical system the angles are measured in
             * @param frame Reference frame the position is expressed in
             */
            void assign(const double& phi, const double& theta, CoordSystem sys, CoordFrame frame);

            /**
             * @brief Assign new spherical coordinates, proper motion, system and frame to this object.
             * @details The parallax, radial velocity and epoch already held by this object are kept.
             * @param phi     [deg] longitude-like angle
             * @param theta   [deg] latitude-like angle
             * @param pmPhi   [mas/yr] proper motion in phi (RA proper motion already includes the cos(theta) factor)
             * @param pmTheta [mas/yr] proper motion in theta
             * @param sys     Spherical system the angles are measured in
             * @param frame   Reference frame the position is expressed in
             */
            void assign(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                        CoordSystem sys, CoordFrame frame);

            /**
             * @brief Assign new spherical coordinates, proper motion, distance, line-of-sight
             *        velocity, epoch, system and frame to this object.
             * @details This is the overload the conversion routines use, because it is the only
             *          one that replaces every field rather than preserving some of them.
             * @param phi             [deg] longitude-like angle
             * @param theta           [deg] latitude-like angle
             * @param pmPhi           [mas/yr] proper motion along phi, including the cos(theta) factor
             * @param pmTheta         [mas/yr] proper motion along theta
             * @param parallax        [mas] trigonometric parallax; 0 means "unknown/infinitely far"
             * @param radialVelocity  [km/s] barycentric radial velocity
             * @param epoch           [day] Julian date (TT) at which \c phi and \c theta are valid.
             *                        Pass NaN to mean "the standard epoch of \c frame".
             * @param sys             Spherical system the angles are measured in
             * @param frame           Reference frame the position is expressed in
             */
            void assign(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                        const double& parallax, const double& radialVelocity, const double& epoch,
                        CoordSystem sys, CoordFrame frame);
#pragma endregion

#pragma region * System conversion
            /**
             * @brief Convert to equatorial coordinates, keeping the current reference frame.
             * @param output Destination object, filled with the right ascension and declination
             * @throws std::invalid_argument if \c output is null
             */
            void toEquatorial(SkyCoordinates* output) const {toEquatorial(output, getFrame());}

            /**
             * @brief Convert to galactic coordinates, keeping the current reference frame.
             * @param output Destination object, filled with the galactic longitude and latitude
             * @throws std::invalid_argument if \c output is null
             */
            void toGalactic  (SkyCoordinates* output) const {toGalactic  (output, getFrame());}

            /**
             * @brief Convert to ecliptic coordinates, keeping the current reference frame.
             * @param output Destination object, filled with the ecliptic longitude and latitude
             * @throws std::invalid_argument if \c output is null
             */
            void toEcliptic  (SkyCoordinates* output) const {toEcliptic  (output, getFrame());}

            /**
             * @brief Convert to equatorial coordinates in a chosen reference frame.
             * @details Converts the system first, then rotates into \c frame. The parallax and
             *          radial velocity travel with the position; \c output records \c frame and
             *          the epoch the rotation leaves the star at.
             * @param output Destination object, filled with the right ascension and declination
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            virtual void toEquatorial(SkyCoordinates* output, CoordFrame frame) const = 0;

            /**
             * @brief Convert to galactic coordinates in a chosen reference frame.
             * @param output Destination object, filled with the galactic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            virtual void toGalactic  (SkyCoordinates* output, CoordFrame frame) const = 0;

            /**
             * @brief Convert to ecliptic coordinates in a chosen reference frame.
             * @param output Destination object, filled with the ecliptic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            virtual void toEcliptic  (SkyCoordinates* output, CoordFrame frame) const = 0;
#pragma endregion

        private:
            double fphi;   //!< [deg] longitude-like angle (RA, galactic or ecliptic longitude)
            double ftheta; //!< [deg] latitude-like angle (Dec, galactic or ecliptic latitude)
            double fpmPhi;   //!< [mas/yr] proper motion in phi, including the cos(theta) factor
            double fpmTheta; //!< [mas/yr] proper motion in theta
            double fparallax;       //!< [mas] trigonometric parallax; 0 means unknown
            double fradialVelocity; //!< [km/s] barycentric radial velocity
            double fepoch;          //!< [day] Julian date (TT) at which fphi/ftheta are valid
            CoordSystem fcoordSys;  //!< Spherical system the angles are measured in
            CoordFrame fcoordFrame; //!< Reference frame the position is expressed in
    };

    // Type aliases for convenience
    using SkyCoord = SkyCoordinates;      //!< Shorthand for \c SkyCoordinates
    using CoordFrame = SkyCoord::CoordFrame;   //!< \c SkyCoordinates::CoordFrame at namespace scope
    using CoordSystem = SkyCoord::CoordSystem; //!< \c SkyCoordinates::CoordSystem at namespace scope

#pragma endregion

#pragma region SkyCoordinates derived classes
#pragma region EquatorialCoordinates class definition
    /**
     *  @class EquatorialCoordinates
     *  @brief A position in right ascension and declination, optionally with the full space
     *         motion of a catalogue entry.
     *
     *  @details
     *  This is the pivot of the hierarchy. Frame conversions and epoch propagation are both
     *  defined on equatorial coordinates, so \c GalacticCoordinates and \c EclipticCoordinates
     *  route through this class whenever they are asked for a frame other than their own.
     *
     *  It is also the only system that carries proper motion through a conversion: galactic and
     *  ecliptic results come back with zero proper motion, because the rate would have to be
     *  re-projected onto different axes.
     *
     *  @par A catalogue entry, end to end
     *  ```c++
     *      using namespace DSL;
     *
     *      // Barnard's Star, as published by Hipparcos at epoch J1991.25
     *      EquatorialCoordinates barnard(269.45207, 4.66829,   // [deg] RA, Dec
     *                                    -798.58, 10328.12,    // [mas/yr] pmRA*, pmDec
     *                                    547.45, -110.51,      // [mas] parallax, [km/s] v_rad
     *                                    CoordFrame::HIPPARCOS);
     *
     *      EquatorialCoordinates today;
     *      barnard.atCurrentEpoch(&today);       // where it is now
     *
     *      std::cout << barnard.angularSeparation(today) << " deg travelled\n";
     *  ```
     *
     *  @par Frame versus epoch
     *  These are two separate operations and they compose in either order:
     *
     *  ```c++
     *      EquatorialCoordinates fk5, atJ2000;
     *      barnard.toFK5(&fk5);              // rotate the axes, date untouched
     *      barnard.atEpoch("J2000.0", &atJ2000);  // move the star, axes untouched
     *  ```
     *
     *  The one exception is FK4: converting into or out of it goes through a NOVAS
     *  \c CHANGE_EPOCH transformation, which transports the star over the 50 years between
     *  B1950.0 and J2000.0 as well as rotating it. The resulting epoch follows the frame there,
     *  and \c getEpoch() reports it.
     *
     *  @sa SkyCoordinates, GalacticCoordinates, EclipticCoordinates
     */
    class EquatorialCoordinates : public SkyCoord
    {
        public:
#pragma region * ctor/dtor
            //! @brief The ICRS origin (RA = 0, Dec = 0) at epoch J2000.0.
            EquatorialCoordinates();

            /**
             * @brief A fixed direction in the ICRS.
             * @param ra  [deg] right ascension
             * @param dec [deg] declination
             */
            EquatorialCoordinates(const double& ra, const double& dec);

            /**
             * @brief A fixed direction in a chosen frame, at that frame's standard epoch.
             * @param ra    [deg] right ascension
             * @param dec   [deg] declination
             * @param frame Reference frame the position is expressed in
             */
            EquatorialCoordinates(const double& ra, const double& dec, CoordFrame frame);

            /**
             * @brief A moving source in the ICRS, at epoch J2000.0.
             * @param ra    [deg] right ascension
             * @param dec   [deg] declination
             * @param pmRA  [mas/yr] proper motion in right ascension (already includes the cos(dec) factor)
             * @param pmDEC [mas/yr] proper motion in declination
             */
            EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC);

            /**
             * @brief A moving source in a chosen frame, at that frame's standard epoch.
             * @param ra    [deg] right ascension
             * @param dec   [deg] declination
             * @param pmRA  [mas/yr] proper motion in right ascension (already includes the cos(dec) factor)
             * @param pmDEC [mas/yr] proper motion in declination
             * @param frame Reference frame the position is expressed in
             */
            EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC, CoordFrame frame);

            /**
             * @brief Fully specified catalogue entry.
             * @details The distance and radial velocity are what make \c atEpoch() rigorous for
             *          nearby fast movers: with them the star is transported along its true
             *          space-motion vector, picking up the perspective acceleration. Leave the
             *          parallax at 0 for a distant source and the propagation becomes purely
             *          angular, which is the correct treatment there.
             * @param ra             [deg] right ascension
             * @param dec            [deg] declination
             * @param pmRA           [mas/yr] proper motion in right ascension (includes the cos(dec) factor)
             * @param pmDEC          [mas/yr] proper motion in declination
             * @param parallax       [mas] trigonometric parallax; 0 means "unknown/infinitely far"
             * @param radialVelocity [km/s] barycentric radial velocity
             * @param frame          Reference frame the position is expressed in
             * @param epoch          [day] Julian date (TT) the position refers to. Defaults to a
             *                       NaN sentinel meaning "the standard epoch of \c frame", i.e.
             *                       B1950.0 for FK4, J1991.25 for HIPPARCOS, J2000.0 otherwise.
             */
            EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC,
                                  const double& parallax, const double& radialVelocity,
                                  CoordFrame frame = CoordFrame::ICRS,
                                  const double& epoch = std::numeric_limits<double>::quiet_NaN());

            ~EquatorialCoordinates(); //!< Destructor
#pragma endregion

#pragma region * Accessor
            double getRA() const;    //!< @return [deg] right ascension, in the range 0 to 360
            double getDEC() const;   //!< @return [deg] declination, in the range -90 to +90
            double getPMRA()  const; //!< [mas/yr] proper motion in right ascension
            double getPMDEC() const; //!< [mas/yr] proper motion in declination
#pragma endregion

#pragma region * Conversion
            // Overriding the two-argument form below hides the one-argument
            // convenience overloads of the base class; re-import them here.
            using SkyCoord::toEquatorial;
            using SkyCoord::toGalactic;
            using SkyCoord::toEcliptic;

            /**
             * @brief Rotate into another equatorial reference frame.
             * @details Proper motion, parallax and radial velocity are all carried through. An
             *          FK4 leg also transports the star in time (see the class description), so
             *          \c output may come back at a different epoch than this object.
             * @param output Destination object, filled with the rotated position
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEquatorial(SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Convert to galactic coordinates, optionally rotating the frame first.
             * @param output Destination object, filled with the galactic longitude and latitude.
             *               Its proper motion is set to zero: the rate is not re-projected onto
             *               the galactic axes.
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toGalactic  (SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Convert to ecliptic coordinates, optionally rotating the frame first.
             * @param output Destination object, filled with the ecliptic longitude and latitude.
             *               Its proper motion is set to zero, as for \c toGalactic().
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEcliptic  (SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Convert to the ICRS frame (mean equator/equinox, dynamical J2000 frame tie applied).
             * @param output Destination object, filled with the ICRS right ascension/declination and
             *               its proper motion propagated to the ICRS frame.
             */
            void toICRS(EquatorialCoordinates* output) const;

            /**
             * @brief Convert to the FK5 frame (mean equator/equinox of J2000.0).
             * @param output Destination object, filled with the FK5 right ascension/declination and
             *               its proper motion propagated to the FK5 frame/epoch.
             */
            void toFK5(EquatorialCoordinates* output) const;

            /**
             * @brief Convert to the FK4 frame (mean equator/equinox of B1950.0). E-terms of
             *        aberration are not applied, consistent with the precession-only treatment
             *        used elsewhere in this class.
             * @param output Destination object, filled with the FK4 right ascension/declination and
             *               its proper motion propagated to the FK4 frame/epoch.
             */
            void toFK4(EquatorialCoordinates* output) const;
#pragma endregion

#pragma region * Static frame conversion
            /**
             * @brief Rotate a bare RA/Dec pair and its proper motion between reference frames.
             * @details The stateless form of \c toEquatorial(), for callers holding loose numbers
             *          rather than objects. Equivalent to the wider overload below with the
             *          parallax and radial velocity set to zero.
             * @warning Unlike everywhere else in this header, the right ascension is in **hours**,
             *          not degrees, following the SuperNOVAS catalogue convention. Declination is
             *          still in degrees.
             * @param raHours          [h] right ascension
             * @param decDeg           [deg] declination
             * @param pmRaMasPerYr     [mas/yr] proper motion in right ascension, including cos(dec)
             * @param pmDecMasPerYr    [mas/yr] proper motion in declination
             * @param from             Frame the input is expressed in
             * @param to               Frame to rotate into
             * @param raHoursOut       [h] resulting right ascension
             * @param decDegOut        [deg] resulting declination
             * @param pmRaMasPerYrOut  [mas/yr] resulting proper motion in right ascension
             * @param pmDecMasPerYrOut [mas/yr] resulting proper motion in declination
             * @throws std::invalid_argument if either frame is \c UNDEFINED
             */
            static void convertFrame(double raHours, double decDeg, double pmRaMasPerYr, double pmDecMasPerYr,
                                      CoordFrame from, CoordFrame to,
                                      double& raHoursOut, double& decDegOut,
                                      double& pmRaMasPerYrOut, double& pmDecMasPerYrOut);

            /**
             * @brief Frame conversion carrying the full space-motion vector.
             * @details The distance and line-of-sight velocity matter on the FK4 legs, which
             *          transport the star over the 50 years between B1950.0 and J2000.0. The
             *          shorter overload above is this one with both set to zero.
             *
             *          HIPPARCOS and ICRS name the same axes, so converting between them is a
             *          no-op here; what separates them is the epoch, which is \c atEpoch()'s job.
             * @warning As above, \c raHours is in **hours** and \c decDeg in degrees.
             * @param raHours          [h] right ascension
             * @param decDeg           [deg] declination
             * @param pmRaMasPerYr     [mas/yr] proper motion in right ascension, including cos(dec)
             * @param pmDecMasPerYr    [mas/yr] proper motion in declination
             * @param parallaxMas      [mas] trigonometric parallax; 0 means "unknown/infinitely far"
             * @param rvKmPerSec       [km/s] barycentric radial velocity
             * @param from             Frame the input is expressed in
             * @param to               Frame to rotate into
             * @param raHoursOut       [h] resulting right ascension
             * @param decDegOut        [deg] resulting declination
             * @param pmRaMasPerYrOut  [mas/yr] resulting proper motion in right ascension
             * @param pmDecMasPerYrOut [mas/yr] resulting proper motion in declination
             * @param parallaxMasOut   [mas] resulting parallax
             * @param rvKmPerSecOut    [km/s] resulting radial velocity
             * @throws std::invalid_argument if either frame is \c UNDEFINED
             */
            static void convertFrame(double raHours, double decDeg, double pmRaMasPerYr, double pmDecMasPerYr,
                                      double parallaxMas, double rvKmPerSec,
                                      CoordFrame from, CoordFrame to,
                                      double& raHoursOut, double& decDegOut,
                                      double& pmRaMasPerYrOut, double& pmDecMasPerYrOut,
                                      double& parallaxMasOut, double& rvKmPerSecOut);
#pragma endregion

#pragma region * Epoch propagation

            /**
             * @brief Move the star along its own space motion to another date.
             * @details The reference frame is untouched: only the date changes. The position,
             *          the proper motion, the parallax and the radial velocity are all
             *          propagated together from \c getEpoch() to \c targetEpoch, so a nearby
             *          fast mover picks up its perspective acceleration correctly. With an
             *          unknown parallax (0) the propagation is purely angular, which is the
             *          right answer for a distant source.
             * @param targetEpoch [day] Julian date (TT) to move the position to
             * @param output      Destination object, filled with the position at \c targetEpoch
             *                    and carrying that epoch afterwards
             * @throws std::invalid_argument if \c output is null
             * @throws std::runtime_error if the underlying SuperNOVAS call fails
             */
            void atEpoch(const double& targetEpoch, EquatorialCoordinates* output) const;

            /**
             * @brief Move the star to the date named by an epoch string.
             * @param epochSpec Anything \c SkyCoordinates::epochOf(const std::string&) accepts,
             *                  e.g. \c "J2026.7", \c "B1950.0" or \c "HIP"
             * @param output    Destination object, filled with the position at that date
             * @throws std::invalid_argument if \c output is null or \c epochSpec is unrecognised
             */
            void atEpoch(const std::string& epochSpec, EquatorialCoordinates* output) const;

            /**
             * @brief Move the star to the standard epoch of a reference frame.
             * @details Equivalent to \c atEpoch(epochOf(frame), output). Note that this changes
             *          only the date, never the orientation: use \c toFK4() / \c toFK5() /
             *          \c toICRS() to rotate the axes.
             * @param frame  Frame whose standard epoch is wanted, e.g. \c CoordFrame::HIPPARCOS
             *               for J1991.25
             * @param output Destination object, filled with the position at that date
             * @throws std::invalid_argument if \c output is null
             */
            void atEpoch(CoordFrame frame, EquatorialCoordinates* output) const;

            /**
             * @brief Move the star to the current instant.
             * @details Shorthand for \c atEpoch(currentEpoch(), output) — the usual way to ask
             *          "where is this object tonight?".
             * @param output Destination object, filled with the position now
             * @throws std::invalid_argument if \c output is null
             * @throws std::runtime_error if the system clock cannot be converted
             */
            void atCurrentEpoch(EquatorialCoordinates* output) const;
#pragma endregion
    };
#pragma endregion
#pragma region GalacticCoordinates class definition
    /**
     *  @class GalacticCoordinates
     *  @brief A position in galactic longitude and latitude (\e l, \e b).
     *
     *  @details
     *  The galactic system is defined by the IAU/Hipparcos convention: the north galactic pole
     *  at RA 192.85948 deg, Dec +27.12825 deg (J2000), with the north celestial pole at galactic
     *  longitude 122.93192 deg. The galactic centre therefore sits at (\e l, \e b) = (0, 0).
     *
     *  Objects are built in the ICRS. Asking for another frame converts the system first and
     *  then rotates, so the result is the galactic position as it would be seen on that frame's
     *  axes:
     *
     *  ```c++
     *      DSL::GalacticCoordinates m31(121.17432, -21.57292);
     *
     *      DSL::EquatorialCoordinates icrs, b1950;
     *      m31.toEquatorial(&icrs);                          // (10.6847, 41.2688)
     *      m31.toEquatorial(&b1950, DSL::CoordFrame::FK4);   // precessed to B1950.0
     *  ```
     *
     *  @note Proper motion is not carried into or out of this system: the rate would have to be
     *        re-projected onto the galactic axes, which the class does not do. Parallax, radial
     *        velocity and epoch do travel through, since they are properties of the star rather
     *        than of the axes used to point at it.
     *
     *  @sa SkyCoordinates, EquatorialCoordinates, EclipticCoordinates
     */
    class GalacticCoordinates : public SkyCoord
    {
        public:
#pragma region * ctor/dtor
            //! @brief The galactic centre direction (\e l = 0, \e b = 0), in the ICRS.
            GalacticCoordinates();

            /**
             * @brief A direction in the galactic system, expressed on ICRS axes.
             * @param glon [deg] galactic longitude, in the range 0 to 360
             * @param glat [deg] galactic latitude, in the range -90 to +90
             */
            GalacticCoordinates(const double& glon, const double& glat);

            ~GalacticCoordinates(); //!< Destructor
#pragma endregion

#pragma region * Accessor
            double getGLON() const; //!< @return [deg] galactic longitude, in the range 0 to 360
            double getGLAT() const; //!< @return [deg] galactic latitude, in the range -90 to +90
#pragma endregion

#pragma region * Conversion
            // Overriding the two-argument form below hides the one-argument
            // convenience overloads of the base class; re-import them here.
            using SkyCoord::toEquatorial;
            using SkyCoord::toGalactic;
            using SkyCoord::toEcliptic;

            /**
             * @brief Convert to equatorial coordinates, optionally in another reference frame.
             * @param output Destination object, filled with the right ascension and declination
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEquatorial(SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Re-express these galactic coordinates on another frame's axes.
             * @details Round-trips through the equatorial system to apply the rotation, so the
             *          result is the galactic position of the same star as seen from \c frame.
             * @param output Destination object, filled with the galactic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toGalactic  (SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Convert to ecliptic coordinates, optionally in another reference frame.
             * @param output Destination object, filled with the ecliptic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEcliptic  (SkyCoord* output, CoordFrame frame) const override;
#pragma endregion
    };
#pragma endregion
#pragma region EclipticCoordinates class definition
    /**
     *  @class EclipticCoordinates
     *  @brief A position in ecliptic longitude and latitude (&lambda;, &beta;).
     *
     *  @details
     *  The ecliptic system shares its origin with the equatorial one — the vernal equinox is
     *  (&lambda;, &beta;) = (0, 0) and (RA, Dec) = (0, 0) at the same time — and is tilted from
     *  it by the obliquity, 23.4393 deg at J2000.0. Useful whenever the Solar System matters:
     *  planets and the Moon stay within a few degrees of &beta; = 0.
     *
     *  ```c++
     *      DSL::EclipticCoordinates solstice(90.0, 0.0);   // the June solstice point
     *
     *      DSL::EquatorialCoordinates equ;
     *      solstice.toEquatorial(&equ);                    // RA 90 deg, Dec +23.4393 deg
     *  ```
     *
     *  @note The obliquity used is the one at J2000.0, on both ends of a frame change. That
     *        makes a frame conversion a clean rotation about the ecliptic pole — the longitude
     *        precesses by the general precession (about 0.6985 deg between B1950.0 and J2000.0)
     *        and the latitude is left essentially untouched — and makes the conversion exactly
     *        invertible. It is not the mean obliquity of the target epoch.
     *
     *  @note As with \c GalacticCoordinates, proper motion is not carried into or out of this
     *        system; parallax, radial velocity and epoch are.
     *
     *  @sa SkyCoordinates, EquatorialCoordinates, GalacticCoordinates
     */
    class EclipticCoordinates : public SkyCoord
    {
        public:
#pragma region * ctor/dtor
            //! @brief The vernal equinox (&lambda; = 0, &beta; = 0), in the ICRS.
            EclipticCoordinates();

            /**
             * @brief A direction in the ecliptic system, expressed on ICRS axes.
             * @param elon [deg] ecliptic longitude, in the range 0 to 360
             * @param elat [deg] ecliptic latitude, in the range -90 to +90
             */
            EclipticCoordinates(const double& elon, const double& elat);

            ~EclipticCoordinates(); //!< Destructor
#pragma endregion

#pragma region * Accessor
            double getELON() const; //!< @return [deg] ecliptic longitude, in the range 0 to 360
            double getELAT() const; //!< @return [deg] ecliptic latitude, in the range -90 to +90
#pragma endregion

#pragma region * Conversion
            // Overriding the two-argument form below hides the one-argument
            // convenience overloads of the base class; re-import them here.
            using SkyCoord::toEquatorial;
            using SkyCoord::toGalactic;
            using SkyCoord::toEcliptic;

            /**
             * @brief Convert to equatorial coordinates, optionally in another reference frame.
             * @param output Destination object, filled with the right ascension and declination
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEquatorial(SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Convert to galactic coordinates, optionally in another reference frame.
             * @param output Destination object, filled with the galactic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toGalactic  (SkyCoord* output, CoordFrame frame) const override;

            /**
             * @brief Re-express these ecliptic coordinates on another frame's axes.
             * @details Round-trips through the equatorial system to apply the rotation. Because
             *          the same J2000.0 obliquity is used on both ends, this reduces to a
             *          rotation about the ecliptic pole: the longitude moves by the general
             *          precession and the latitude barely moves at all.
             * @param output Destination object, filled with the ecliptic longitude and latitude
             * @param frame  Reference frame to express the result in
             * @throws std::invalid_argument if \c output is null, or if either frame is \c UNDEFINED
             */
            void toEcliptic  (SkyCoord* output, CoordFrame frame) const override;
#pragma endregion
    };
#pragma endregion
#pragma endregion

} // namespace DSL



#endif
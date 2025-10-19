//
//  FITSwcs.cxx
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#include <stdexcept>

#include <fitsio.h>

#include <DSTfits/FITSwcs.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>

namespace DSL
{
#pragma region - FITSwcs member function implementation

#pragma region * protected member function

        void FITSwcs::initFromString(const std::string& header, const int& relax, const int& ctrl)
        {
            fwcs.reset();
            fnwcs      = 0;
            fwcs_status= 0;
            

            if(header.empty())
            {
                fwcs_status = VALUE_UNDEFINED;
                throw FITSexception(fwcs_status,"FITSwcs","initFromString","Empty header string");
            }

            int nkeyrec = static_cast<int>(header.size()/80);
            int nreject=0;
            struct wcsprm* _wcs = nullptr;

            char* hdr_ptr = new char[header.size()];
            memcpy(hdr_ptr,header.c_str(),header.size());

            if ( (fwcs_status = wcspih( hdr_ptr, nkeyrec, relax, ctrl, &nreject, &fnwcs, &_wcs)) )
            {
                fwcs.reset();
                delete[] hdr_ptr;
                throw WCSexception(fwcs_status,"FITSwcs","initFromString");
            }

            if((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<"Number of Key read         : "<<nkeyrec<<std::endl;
                std::cout<<"Number of rejected WCS key : "<<nreject<<std::endl;
                std::cout<<"Number of WCS found        : "<<fnwcs<<std::endl;
                std::cout<<"WCS status                 : "<<fwcs_status<<std::endl;
                for(size_t k = 0; k < static_cast<size_t>(fnwcs); k++)
                {
                    std::cout<<"WCS name       : "<<_wcs[k].wcsname<<std::endl;
                    if((verbose&verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS&verboseLevel::VERBOSE_BASIC))
                    {
                        for(size_t j = 0; j < static_cast<size_t>(_wcs[k].naxis); j++)
                        {
                            std::cout<<"AXIS           : "<<_wcs[k].ctype[j]<<std::endl;
                            std::cout<<"CRPIX["<<j<<"] : "<<_wcs[k].crpix[j]<<std::endl;
                            std::cout<<"CRVAL["<<j<<"] : "<<_wcs[k].crval[j]<<std::endl;
                            std::cout<<"CDELT["<<j<<"] : "<<_wcs[k].cdelt[j]<<std::endl;
                        }
                    }
                }

            }

            if(fnwcs > 0)
            {
                int nw = fnwcs;
                fwcs = std::shared_ptr<struct wcsprm>( _wcs, [nw](struct wcsprm* p){if(!p) return; int cnt = nw; struct wcsprm* tmp = p;wcsvfree(&cnt,&tmp);} );
                fwcs_status = 0;
            }
            else
            {
                fwcs.reset();
                fwcs_status = WCSERR_BAD_WORLD_COORD;
            }

            delete[] hdr_ptr;
            return;
        }

        void FITSwcs::initFromImg(const FITShdu& fptr, const int& relax, const int& ctrl)
        {
            std::string header = fptr.asString();
            initFromString(header, relax, ctrl);
        }

        void FITSwcs::initFromImg(const std::shared_ptr<FITShdu>& fptr, const int& relax, const int& ctrl)
        {
            std::string header = fptr->asString();
            initFromString(header, relax, ctrl);
        }

        void FITSwcs::initFromImg(const std::shared_ptr<fitsfile>& fptr, const int& relax, const int& ctrl)
        {
            if(fptr == nullptr || fptr.use_count() < 1)
            {
                fwcs_status = SHARED_NULPTR;
                throw FITSexception(fwcs_status,"FITSwcs","initFromImg");
            }

            if ((verbose & verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_WCS)
            {
                std::cerr << "FITSwcs::initFromImg: fitsfile*=" << static_cast<const void*>(fptr.get())<< " use_count=" << fptr.use_count() << '\n';
            }

            int hdu_type = 0;
            fwcs_status = 0;
            if( fits_get_hdu_type(fptr.get(), &hdu_type, &fwcs_status) )
            {
                fwcs.reset();
                throw FITSexception(fwcs_status,"FITSwcs","initFromImg");
            }

            if(hdu_type != IMAGE_HDU)
            {
                fwcs_status = NOT_IMAGE;
                throw FITSexception(fwcs_status,"FITSwcs","initFromImg","Current HDU isn't an FITS image");
            }

            char* header = nullptr;
            int nkeyrec = 0;
            fwcs_status = 0;
            if ( fits_hdr2str(fptr.get(), 1, NULL, 0, &header, &nkeyrec, &fwcs_status) )
            {
                fwcs.reset();
                throw FITSexception(fwcs_status,"FITSwcs","initFromImg");
            }

            std::string shdr(header);
            free(header);

            return initFromString(shdr, relax, ctrl);
        }

#pragma endregion
#pragma region * ctor/dtor
        FITSwcs::FITSwcs(const FITShdu& hdu, int relax, int ctrl):fwcs(),fwcs_status(0),fnwcs(0)
        {
            initFromImg(hdu, relax, ctrl);
        }

        FITSwcs::FITSwcs(const std::shared_ptr<FITShdu>& hdu, int relax, int ctrl):fwcs(),fwcs_status(0),fnwcs(0)
        {
            initFromImg(hdu, relax, ctrl);
        }

        FITSwcs::FITSwcs(const std::shared_ptr<fitsfile>& fptr, int relax, int ctrl):fwcs(),fwcs_status(0),fnwcs(0)
        {
            initFromImg(fptr, relax, ctrl);
        }

        FITSwcs::~FITSwcs()
        {
            fwcs.reset();
            fwcs_status = 0;
            fnwcs       = 0;
        }

#pragma endregion
#pragma region * Accessor

        /**
         * @brief Get the \c CRPIX value for a given axis for the default WCS
         * 
         * @param axis Axis index
         * @return double \c CRPIXi value
         */
        double FITSwcs::CRPIX(const size_t& axis) const
        {
            return CRPIX(0, axis);
        }


        /**
        * @brief Get the \c CRPIX value for a given WCS index and axis
        * 
        * @param wcsIndex World Coordinate System index
        * @param axis Axis index in World Coordinate System
        * @return double \c CRPIXia value with i=axis and a=wcsIndex
        */
        double FITSwcs::CRPIX(const size_t& wcsIndex, const size_t& axis) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","CRPIX",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CRPIX",errmsg);
            }

            if(axis > static_cast<size_t>(fwcs->naxis) || axis == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CRPIX",errmsg);
            }

            return fwcs.get()[wcsIndex].crpix[ axis-1 ];
        }

        /**
         * @brief Get the CRVAL value for a given axis for the default WCS
         * 
         * @param axis Axis index
         * @return double \c CRVALi value
         */
        double FITSwcs::CRVAL(const size_t& axis) const
        {
            return CRVAL(0, axis);
        }


        /**
        * @brief Get the CRVAL value for a given WCS index and axis
        * 
        * @param wcsIndex World Coordinate System index
        * @param axis Axis index in World Coordinate System
        * @return double \c CRVALia value with i=axis and a=wcsIndex
        */
        double FITSwcs::CRVAL(const size_t& wcsIndex, const size_t& axis) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","CRVAL",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CRVAL",errmsg);
            }

            if(axis > static_cast<size_t>(fwcs->naxis) || axis == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CRVAL",errmsg);
            }

            return fwcs.get()[wcsIndex].crval[ axis-1 ];
        }

        /**
         * @brief Get the CDELT value for a given axis for the default WCS
         * 
         * @param axis Axis index
         * @return double \c CDELTi value
         */
        double FITSwcs::CDELT(const size_t& axis) const
        {
            return CDELT(0, axis);
        }


        /**
        * @brief Get the CDELT value for a given WCS index and axis
        * 
        * @param wcsIndex World Coordinate System index
        * @param axis Axis index in World Coordinate System
        * @return double \c CDELTia value with i=axis and a=wcsIndex
        */
        double FITSwcs::CDELT(const size_t& wcsIndex, const size_t& axis) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","CDELT",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CDELT",errmsg);
            }

            if(axis > static_cast<size_t>(fwcs->naxis) || axis == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CDELT",errmsg);
            }

            return fwcs.get()[wcsIndex].cdelt[ axis-1 ];
        }

}
#pragma endregion
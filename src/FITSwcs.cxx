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
#include <thread>
#include <iostream>
#include <sstream>

#include <fitsio.h>

#include <DSTfits/FITSwcs.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>

#include <wcslib/wcserr.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcsprintf.h>
#include <wcslib/wcsutil.h>

namespace DSL
{
#pragma region - FITSwcs member function implementation

#pragma region * protected member function

        void FITSwcs::initFromString(const std::string& header, const int& relax, const int& ctrl)
        {
            fwcs.reset();
            fnwcs      = 0;
            fwcs_status= WCSERR_UNSET;
            

            if(header.empty())
            {
                fwcs_status = VALUE_UNDEFINED;
                throw FITSexception(fwcs_status,"FITSwcs","initFromString","Empty header string");
            }

            if((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
            {
                std::cout<<std::endl<<std::endl<<"\033[31m--- FITSwcs::initFromString() ---\033[0m]"<<std::endl<<std::endl;
                std::cout<<"Header size (bytes) : "<<header.size()<<std::endl
                    <<header<<std::endl<<std::endl;
            }

            int nkeyrec = static_cast<int>(header.size()/80);
            int nreject=0;
            struct wcsprm* _wcs = nullptr;

            std::vector<char> hdr_buf(header.begin(), header.end());
            hdr_buf.push_back('\0'); // ensure NUL-terminated
            char* hdr_ptr = hdr_buf.data();

            //// DUMP FOR DEBUGGING PURPOSES
            // Diagnostic: dump cards and highlight WCS-like keywords to find which wa

            if((verbose & verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
                wcserr_enable(1);

            if ( (fwcs_status = wcspih( hdr_ptr, nkeyrec, relax, ctrl, &nreject, &fnwcs, &_wcs)) )
            {
                fwcs.reset();
                throw WCSexception(fwcs_status,"FITSwcs","initFromString");
            }

            if((verbose & verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
                wcserr_enable(0);

            if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<"Number of Key read         : "<<nkeyrec<<std::endl;
                std::cout<<"Number of rejected WCS key : "<<nreject<<std::endl;
                std::cout<<"Number of WCS found        : "<<fnwcs<<std::endl;
                std::cout<<"WCS status                 : "<<fwcs_status<<std::endl;
                for(size_t k = 0; k < static_cast<size_t>(fnwcs); k++)
                {
                    std::cout<<"WCS name       : "<<_wcs[k].wcsname<<std::endl;
                    if((verbose&verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS|verboseLevel::VERBOSE_BASIC))
                    {
                        for(int j = 0; j < _wcs[k].naxis; j++)
                        {
                            std::cout<<((j+1<_wcs[k].naxis)?"\033[31m   |-":"\033[31m   `-")<<"\033[34m AXIS     : \033[0m"<<_wcs[k].ctype[j]<<std::endl;
                            std::cout<<((j+1<_wcs[k].naxis)?"\033[31m   | ":"\033[31m     ")<<"   |- \033[34m CRPIX["<<j<<"] : \033[0m"<<_wcs[k].crpix[j]<<std::endl;
                            std::cout<<((j+1<_wcs[k].naxis)?"\033[31m   | ":"\033[31m     ")<<"   |- \033[34m CRVAL["<<j<<"] : \033[0m"<<_wcs[k].crval[j]<<std::endl;
                            std::cout<<((j+1<_wcs[k].naxis)?"\033[31m   | ":"\033[31m     ")<<"   `- \033[34m CDELT["<<j<<"] : \033[0m"<<_wcs[k].cdelt[j]<<std::endl;
                        }
                    }
                }
            }

            if(fnwcs > 0)
            {
                wcserr_enable(1);

                int stat[NWCSFIX];
                wcserr statmsg[NWCSFIX];
                int fix_status = wcsfixi(7,0,_wcs,stat,statmsg);

                if((verbose&verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS|verboseLevel::VERBOSE_BASIC))
                {
                    std::cout<<std::endl<<"\033[31m*****\033[34m WCSFIX STATUS \033[0m\n";
                    std::vector<std::string> func({"CDFIX", "DATFIX", "OBSFIX", "UNITFIX", "SPCFIX", "CELFIX","CYLFIX"});
                    for (int i = 0; i < NWCSFIX; i++)
                    {
                        std::cout<<"\033[31m    "<<((i+1)!=NWCSFIX?"|- ":"`- ")<<"\033[34m"<<func[i]<<" : \033[0m"<<((stat[i]<1)?"\033[32m":"\033[31m")<<stat[i]<<"\033[0m"<<std::endl;
                    }
                    std::cout<<std::endl;
                }
             
                for (int i = 0; i < NWCSFIX; i++)
                {
                    std::vector<std::string> func({"CDFIX", "DATFIX", "OBSFIX", "UNITFIX", "SPCFIX", "CELFIX","CYLFIX"});
                    
                    if (stat[i] > 0)
                    {
                        std::cerr<<"\033[31m[WCSFIX "<<func[i]<<"]\033[0m WCSFIX reported a warning/status code "<<stat[i]<<":"<<std::endl;
                        wcserr_prt(statmsg+i, 0x0);
                 
                        // Free memory used to store the message.
                        if (statmsg[i].msg) wcsdealloc(statmsg[i].msg);
                        std::cout<<std::endl;
                    }
                }

                if (fix_status != 0)
                {
                    std::cerr<<"\033[31m[WCSFIX]\033[0m WCSFIX failed with status code "<<fix_status<<std::endl;
                }

                wcserr_enable(0);

                if(wcsset(_wcs))
                {
                    fwcs.reset();
                    fwcs_status = WCSERR_UNSET;
                    throw WCSexception(fwcs_status,"FITSwcs","initFromString","wcsset failed after wcsfixi");
                }

                if((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
                {
                    std::cout<<std::endl<<"\033[31m*****\033[34m WCS HEADER PARSED \033[0m\n";
                    wcsprt(_wcs);
                }

                int nw = fnwcs;
                fwcs = std::shared_ptr<struct wcsprm>( _wcs, [nw](struct wcsprm* p){if(!p) return; int cnt = nw; struct wcsprm* tmp = p;wcsvfree(&cnt,&tmp);} );
                fwcs_status = 0;
            }
            else
            {
                fwcs.reset();
                fwcs_status = WCSERR_BAD_WORLD_COORD;
            }

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
        FITSwcs::FITSwcs(const FITShdu& hdu, int relax, int ctrl):fwcs(),fwcs_status(WCSERR_UNSET),fnwcs(0)
        {
            initFromImg(hdu, relax, ctrl);
        }

        FITSwcs::FITSwcs(const std::shared_ptr<FITShdu>& hdu, int relax, int ctrl):fwcs(),fwcs_status(WCSERR_UNSET),fnwcs(0)
        {
            initFromImg(hdu, relax, ctrl);
        }

        FITSwcs::FITSwcs(const std::shared_ptr<fitsfile>& fptr, int relax, int ctrl):fwcs(),fwcs_status(WCSERR_UNSET),fnwcs(0)
        {
            initFromImg(fptr, relax, ctrl);
        }

        FITSwcs::FITSwcs(const FITSwcs& other):fwcs(other.fwcs), fwcs_status(other.fwcs_status), fnwcs(other.fnwcs)
        { }

        FITSwcs::FITSwcs(const FITSwcs& other, const size_t& idx):fwcs(), fwcs_status(WCSERR_UNSET), fnwcs(0)
        {
            if(other.fwcs == nullptr)
            {
                fwcs_status = WCSERR_UNSET;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object has no WCS defined");
            }

            if(idx >= static_cast<size_t>(other.fnwcs))
            {
                fwcs_status = WCSERR_BAD_PARAM;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object index is out of range");
            }

            
            struct wcsprm* _wcs = new wcsprm;
            _wcs->flag=-1;
            if ( wcssub(1, &(other.fwcs.get()[idx]),0x0,0x0, _wcs) )
            {
                fwcs_status = WCSERR_UNSET;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","wcsdup failed");
            }

            fwcs_status = wcsset(_wcs);

            if(fwcs_status)
            {
                    fwcs.reset();
                    fwcs_status = WCSERR_UNSET;
                    throw WCSexception(fwcs_status,"FITSwcs","initFromString","wcsset failed after wcsfixi");
            }

            fwcs = std::shared_ptr<struct wcsprm>( _wcs, [](struct wcsprm* p){if(!p) return; struct wcsprm* tmp = p;wcsfree(tmp);} );
            fnwcs       = 1;
        }

        FITSwcs::FITSwcs(const FITSwcs& other, const size_t& idx, const std::vector<size_t>& crpix):fwcs(), fwcs_status(WCSERR_UNSET), fnwcs(0)
        {
            if(crpix.size() != other.getNumberOfAxis(idx))
            {
                fwcs_status = WCSERR_BAD_PARAM;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","CRPIX vector size doesn't match number of axis in WCS");
            }

            if(other.fwcs == nullptr)
            {
                fwcs_status = WCSERR_UNSET;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object has no WCS defined");
            }

            if(other.getNumberOfWCS() == 0)
            {
                return;
            }
            else if(other.getNumberOfWCS() <= idx)
            {
                fwcs_status = WCSERR_BAD_PARAM;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object index is out of range");
            }

            FITShdu _origin = other.asFITShdu(idx);
            
            //GET WCS SUFFIX
            const std::string suff = other.getSuffix(idx);

            if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<std::endl<<"\033[32m[FITSwcs Copy Constructor]\033[0m OLD sub-WCS HDU:"<<std::endl;
                _origin.Dump(std::cerr);
            }

            // ESTIMATE THE VALUE THAT WOULD HAVE 'OLD' CRPIX IN THE NEW SUBIMAGE
            std::vector<size_t> new_crpix;
            for(size_t i = 0; i < crpix.size(); i++)
            {
                double delta = other.CRPIX(idx, i+1) - static_cast<double>(crpix[i]);
                _origin.ValueForKey("CRPIX"+std::to_string(i+1)+suff, delta, "Pixel coordinate of reference point");
            }

            if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<std::endl<<"\033[32m[FITSwcs Copy Constructor]\033[0m NNEWw sub-WCS HDU:"<<std::endl;
                _origin.Dump(std::cerr);
            }

            // Fallback sequence without nested try/catch
            const std::vector<int> relax_order = { WCSHDR_all, WCSHDR_reject, WCSHDR_strict, WCSHDR_none };

            bool initialized = false;
            std::ostringstream errs;
            for (int relax : relax_order)
            {
                try
                {
                    initFromImg(_origin, relax, 0);
                    initialized = true;
                    break;
                }
                catch(const WCSexception& e)
                {
                    if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
                    {
                        std::cerr<<"\033[31m[ERROR]\033[0m FITSwcs Copy Constructor failed with relax="<<relax<<":\n";
                        std::cerr<<e.what()<<std::endl;
                    }
                    errs << " relax=" << relax << " failed: " << e.what() << "\n";
                }
            }
        
            if (!initialized)
            {
                throw WCSexception(WCSERR_UNSET, "FITSwcs", "Copy Constructor",
                        std::string("Failed to init from subimage HDU after trying relax sequence:\n") + errs.str());
            }
        }

        FITSwcs::FITSwcs(const FITSwcs& other, const size_t& idx, const std::vector<double>& crpix):fwcs(), fwcs_status(WCSERR_UNSET), fnwcs(0)
        {
            if(crpix.size() != other.getNumberOfAxis(idx))
            {
                fwcs_status = WCSERR_BAD_PARAM;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","CRPIX vector size doesn't match number of axis in WCS");
            }

            if(other.fwcs == nullptr)
            {
                fwcs_status = WCSERR_UNSET;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object has no WCS defined");
            }

            if(other.getNumberOfWCS() == 0)
            {
                return;
            }
            else if(other.getNumberOfWCS() <= idx)
            {
                fwcs_status = WCSERR_BAD_PARAM;
                throw WCSexception(fwcs_status,"FITSwcs","Copy Constructor","Input FITSwcs object index is out of range");
            }

            FITShdu _origin = other.asFITShdu(idx);
            const std::string suff = other.getSuffix(idx);

            if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<std::endl<<"\033[32m[FITSwcs Copy Constructor]\033[0m OLD sub-WCS HDU:"<<std::endl;
                _origin.Dump(std::cerr);
            }

            // ESTIMATE THE VALUE THAT WOULD HAVE 'OLD' CRPIX IN THE NEW SUBIMAGE
            std::vector<double> new_crpix;
            for(size_t i = 0; i < crpix.size(); i++)
            {
                double delta = other.CRPIX(idx, i+1) - crpix[i];
                _origin.ValueForKey("CRPIX"+std::to_string(i+1)+suff, delta, "Pixel coordinate of reference point");
            }

            if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
            {
                std::cout<<std::endl<<"\033[32m[FITSwcs Copy Constructor]\033[0m NNEWw sub-WCS HDU:"<<std::endl;
                _origin.Dump(std::cerr);
            }

            // Fallback sequence without nested try/catch
            const std::vector<int> relax_order = { WCSHDR_all, WCSHDR_reject, WCSHDR_strict, WCSHDR_none };

            bool initialized = false;
            std::ostringstream errs;
            for (int relax : relax_order)
            {
                try
                {
                    initFromImg(_origin, relax, 0);
                    initialized = true;
                    break;
                }
                catch(const WCSexception& e)
                {
                    if((verbose&verboseLevel::VERBOSE_WCS)==verboseLevel::VERBOSE_WCS)
                    {
                        std::cerr<<"\033[31m[ERROR]\033[0m FITSwcs Copy Constructor failed with relax="<<relax<<":\n";
                        std::cerr<<e.what()<<std::endl;
                    }
                    errs << " relax=" << relax << " failed: " << e.what() << "\n";
                }
            }
        
            if (!initialized)
            {
                throw WCSexception(WCSERR_UNSET, "FITSwcs", "Copy Constructor",
                        std::string("Failed to init from subimage HDU after trying relax sequence:\n") + errs.str());
            }
        }

        FITSwcs::~FITSwcs()
        {
            fwcs.reset();
            fwcs_status = WCSERR_UNSET;
            fnwcs       = 0;
        }

        void FITSwcs::swap(FITSwcs& first, FITSwcs& second) noexcept
        {
            using std::swap;
            swap(first.fwcs, second.fwcs);
            swap(first.fwcs_status, second.fwcs_status);
            swap(first.fnwcs, second.fnwcs);

            for(int k =0; k < first.fnwcs; k++)
                first.fwcs_status = wcsset(&(first.fwcs.get()[k]));

            for(size_t k =0; k < static_cast<size_t>(second.fnwcs); k++)
                second.fwcs_status = wcsset(&(second.fwcs.get()[k]));
        }


#pragma endregion
#pragma region * Accessor

        /**
         * @brief Get the number of axis for a given WCS index
         * 
         * @param wcsIndex World Coordinate System index
         * @return size_t Number of axis in WCS
         */
        size_t FITSwcs::getNumberOfAxis(const size_t& wcsIndex) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","getNumberOfAxis",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","getNumberOfAxis",errmsg);
            }

            return static_cast<size_t>(fwcs.get()[wcsIndex].naxis);
        }

        const std::string FITSwcs::getSuffix(const size_t& wcsIndex) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","getNumberOfAxis",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","getNumberOfAxis",errmsg);
            }

            std::string suff=std::string();
            FITShdu _origin = asFITShdu(wcsIndex);
            
            for(FITSDictionary::const_iterator it = _origin.begin(); it != _origin.end(); ++it)
            {
                std::string key = it->first;
                if(key.find("WCSAXES") != std::string::npos ||
                   key.find("CTYPE")   != std::string::npos ||
                   key.find("CRVAL")   != std::string::npos ||
                   key.find("CRPIX")   != std::string::npos ||
                   key.find("CDELT")   != std::string::npos )
                {
                    //EXTRACT THE SUFFIX
                    size_t pos = key.find_last_of("0123456789");
                    if(pos != std::string::npos)
                    {
                        suff += key.substr(pos+1);
                    }
                }

                if(!suff.empty())
                    break;
            }

            return std::string(suff);
        }

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

        std::vector< std::string> FITSwcs::CTYPE(const size_t& wcsIndex) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","getCTYPEs",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CDELT",errmsg);
            }

            std::vector<std::string> ctype_list;
            size_t axis_cnt = static_cast<size_t>(fwcs.get()[wcsIndex].naxis);
            ctype_list.reserve(axis_cnt);

            for(size_t i = 0; i < axis_cnt; i++)
            {
                std::string ctype_axis(fwcs.get()[wcsIndex].ctype[i]);
                ctype_list.push_back(ctype_axis);
            }

            return ctype_list;
        } 

#pragma endregion
#pragma region * Conversion

        void FITSwcs::changeCelestialCorrds(const size_t& wcsIndex,
                const worldCoords& newRefPole,
                const double& oldLon,
                const std::pair<std::string,std::string>& newTYPE,
                const std::string& alt)
        {
            changeCelestialCorrds(wcsIndex, newRefPole, oldLon, newTYPE, "", 0.0, alt);
        }

        void FITSwcs::changeCelestialCorrds(const size_t& wcsIndex,
                const worldCoords& newRefPole,
                const double& oldLon,
                const std::pair<std::string,std::string>& newTYPE,
                const std::string& radsys,
                const double& equinox,
                const std::string& alt)
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","pix2world",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PARAM];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","pix2world",errmsg);
            }

            std::vector<char> ctype1_buf(newTYPE.first.begin(), newTYPE.first.end());
            ctype1_buf.push_back('\0'); // ensure NUL-terminated
            char* _ctype1 = ctype1_buf.data();

            std::vector<char> ctype2_buf(newTYPE.second.begin(), newTYPE.second.end());
            ctype2_buf.push_back('\0'); // ensure NUL-terminated
            char* _ctype2=ctype2_buf.data();

            // Allocate these here so they live until function exit (avoid dangling pointers).
            std::vector<char> radsys_buf;
            std::vector<char> alt_buf;
            char* _radsys = 0x0;
            if(!radsys.empty())
            {
                radsys_buf.assign(radsys.begin(), radsys.end());
                radsys_buf.push_back('\0'); // ensure NUL-terminated
                _radsys = radsys_buf.data();
            }

            char * _alt = 0x0;
            if(!alt.empty())
            {
                alt_buf.assign(alt.begin(), alt.end());
                alt_buf.push_back('\0'); // ensure NUL-terminated
                _alt = alt_buf.data();
            }

            if( (fwcs_status=wcsccs( &fwcs.get()[wcsIndex], newRefPole[0], newRefPole[1], oldLon, _ctype1, _ctype2, _radsys, equinox, _alt) ))
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","changeCelestialCorrds","Failed to change celestial coordinate system");
            }

            fwcs.get()[wcsIndex].flag = 0; // force recalculation of derived parameters
            

            if(( fwcs_status=wcsset( &fwcs.get()[wcsIndex] ) ))
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","changeCelestialCorrds","Failed to change celestial coordinate system");
            }

            // Ensure wcslib allocates/normalizes internal string storage now,
            // so pointers to local buffers (ctype*_buf, radsys_buf, alt_buf) are not retained.
            wcstrim(&fwcs.get()[wcsIndex]);

            if((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
            {
                std::cout<<std::endl<<"\033[31m*****\033[34m WCS AFTER CHANGE OF CELESTIAL COORDINATE SYSTEM \033[0m\n";
                // trim/print resulting wcsprm for debugging
                if((verbose & verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)
                {
                    wcserr_enable(1);
                    wcsprt(&fwcs.get()[wcsIndex]);
                    wcserr_enable(0);
                }
            }

        }

        worldVectors FITSwcs::pixel2world(const size_t& wcsIndex, const pixelVectors& px) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET]+std::string("\033[34m [")+std::to_string(__LINE__)+std::string("]\033[0m]");
                throw WCSexception(WCSERR_UNSET,"FITSwcs","pix2world",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PARAM]+std::string("\033[34m [")+std::to_string(__LINE__)+std::string("]\033[0m]");
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","pix2world",errmsg);
            }

            if(px.size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX]+std::string("\033[34m [")+std::to_string(__LINE__)+std::string("]\033[0m]");
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pix2world",errmsg);
            }

            if(px[0].size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX]+std::string("\033[34m [")+std::to_string(__LINE__)+std::string("]\033[0m]");
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pix2world",errmsg);
            }

            // Correct semantics:
            // npix  = number of points (rows) = px.size()
            // ncoord = number of coordinates per point (axes) expected by wcslib = wcsprm->ncoord
            const int ncoord = static_cast<int>(px.size());        // number of points
            const int nelem  = static_cast<int>(px[0].size());     // elements per point (stride)

            if (ncoord <= 0 || nelem <= 0)
            {
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pixel2world","Empty pixel vector");
            }
            // require nelem >= naxis when multiple points
            const int naxis = static_cast<int>(std::max(fwcs.get()[wcsIndex].naxis,nelem));
            if (ncoord > 0 && nelem != naxis)
            {
                std::ostringstream os;
                os << "pixel2world: nelem (" << nelem << ") != naxis (" << naxis << ")";
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pixel2world",os.str());
            }

            // Flatten px into pixC with layout [ncoord][nelem] -> index = i*nelem + j
            std::vector<double> pixC(static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            for (int i = 0; i < ncoord; ++i)
            {
                const size_t base = static_cast<size_t>(i) * static_cast<size_t>(nelem);
                for (int j = 0; j < nelem; ++j)
                {
                    pixC[base + static_cast<size_t>(j)] = px[static_cast<size_t>(i)][static_cast<size_t>(j)];
                }
            }

            // allocate output buffers sized ncoord * nelem
            std::vector<double> imgcrd_vec(static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> phi_vec   (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> theta_vec (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> world_vec (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<int>    stat_vec  (static_cast<size_t>(ncoord));

            if( (fwcs_status = wcsp2s(&fwcs.get()[wcsIndex],
                                      ncoord,
                                      nelem,
                                      pixC.data(),
                                      imgcrd_vec.data(),
                                      phi_vec.data(),
                                      theta_vec.data(),
                                      world_vec.data(),
                                      stat_vec.data())) )
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","pixel2world",errmsg);
            }

            // Build output per point: read world elements at world_vec[i*nelem + axis]
            worldVectors wv(static_cast<size_t>(ncoord));
            for (int i = 0; i < ncoord; ++i)
            {
                if (stat_vec[static_cast<size_t>(i)] > 0)
                    continue;

                const size_t base = static_cast<size_t>(i) * static_cast<size_t>(nelem);
                auto &out = wv[static_cast<size_t>(i)];
                out.reserve(static_cast<size_t>(naxis));
                for (int a = 0; a < naxis; ++a)
                    out.push_back(world_vec[base + static_cast<size_t>(a)]);
            }
            return wv;
        }

        pixelVectors FITSwcs::world2pixel(const size_t& wcsIndex, const worldVectors& wc) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","world2pixel",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PARAM];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","world2pixel",errmsg);
            }

            if(wc.size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel",errmsg);
            }

            if(wc[0].size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel",errmsg);
            }

            // wcslib semantics:
            // ncoord = number of points (rows)
            // nelem  = number of elements per point (stride)
            const int ncoord  = static_cast<int>(wc.size());
            const int nelem   = static_cast<int>(wc[0].size());
            const int naxis   = static_cast<int>(fwcs.get()[wcsIndex].naxis);

            if (ncoord <= 0 || nelem <= 0)
            {
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel","Empty world vector");
            }

            if (ncoord > 1 && nelem < naxis)
            {
                std::ostringstream os;
                os << "world2pixel: nelem (" << nelem << ") < naxis (" << naxis << ")";
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel",os.str());
            }

            // Flatten input: layout [ncoord][nelem] => index = i*nelem + j
            std::vector<double> worldC(static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            for (int i = 0; i < ncoord; ++i)
            {
                const size_t base = static_cast<size_t>(i) * static_cast<size_t>(nelem);
                for (int j = 0; j < nelem; ++j)
                    worldC[base + static_cast<size_t>(j)] = wc[static_cast<size_t>(i)][static_cast<size_t>(j)];
            }

            // output buffers sized ncoord * nelem ; stat sized ncoord
            std::vector<double> imgcrd_vec(static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> phi_vec   (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> theta_vec (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<double> pixel_vec (static_cast<size_t>(ncoord) * static_cast<size_t>(nelem));
            std::vector<int>    stat_vec  (static_cast<size_t>(ncoord)); // <-- FIXED: one status per point

            // Correct arg order for wcss2p:
            // wcss2p(wcs, ncoord, nelem, world, imgcrd, phi, theta, pixcrd, stat)
            wcserr_enable(1);
            if( (fwcs_status = wcss2p(&fwcs.get()[wcsIndex],
                                      ncoord,
                                      nelem,
                                      worldC.data(),
                                      phi_vec.data(),
                                      theta_vec.data(),
                                      imgcrd_vec.data(),
                                      pixel_vec.data(),
                                      stat_vec.data())) )
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","world2pixel",errmsg);
            }

            // Build output per point: read pixel elements at pixel_vec[i*nelem + axis]
            pixelVectors pv(static_cast<size_t>(ncoord)); // <-- FIXED: correct return type
            for (int i = 0; i < ncoord; ++i)
            {
                if (stat_vec[static_cast<size_t>(i)] != 0)
                    continue;

                const size_t base = static_cast<size_t>(i) * static_cast<size_t>(nelem);
                auto &out = pv[static_cast<size_t>(i)];
                out.reserve(static_cast<size_t>(naxis));
                for (int a = 0; a < naxis; ++a)
                    out.push_back(pixel_vec[base + static_cast<size_t>(a)]);
            }
            return pv;
        }

        std::string FITSwcs::asString(const int& wcsIndex) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","asHeader",errmsg);
            }

            if(wcsIndex >= fnwcs)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","asHeader",errmsg);
            }

            
            int nkeyrec = 0;
            fwcs_status = 0;

            if(wcsIndex < 0)
            {
                std::string hdr = std::string();
                for(int i = 0; i < fnwcs; i++)
                    hdr += asString(i);

                return hdr;
            }

            char* header = nullptr;
            fwcs_status = wcshdo(WCSHDO_all,&fwcs.get()[wcsIndex],&nkeyrec,&header);
                
            if (fwcs_status > 0)
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","asHeader",errmsg);
            }

            std::string shdr(header);
            free(header);

            return shdr;
        }

        FITShdu FITSwcs::asFITShdu(const int& wcsIndex) const
        {
            std::string header = asString(wcsIndex);
            FITShdu shdr(header);

            return shdr;
        }

#pragma endregion
#pragma region * I/O
            void FITSwcs::Dump(const int& wcsIndex) const
            {
                if(fwcs == nullptr)
                {
                    std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                    throw WCSexception(WCSERR_UNSET,"FITSwcs","Dump",errmsg);
                }

                if(wcsIndex >= fnwcs)
                {
                    std::string errmsg = wcs_errmsg[WCSERR_BAD_PARAM];
                    throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","Dump",errmsg);
                }

                if(wcsIndex < 0)
                {
                    for(int i = 0; i < fnwcs; i++)
                        Dump(i);
                    return;
                }

                wcserr_enable(1);
                wcstrim(&(fwcs.get()[wcsIndex]));
                wcsprt(&(fwcs.get()[wcsIndex]));
                wcserr_enable(0);
            }
#pragma endregion

}
#pragma endregion
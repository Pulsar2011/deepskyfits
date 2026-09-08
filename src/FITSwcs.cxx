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
#include <cmath>
#include <ctime>
#include <algorithm>

#include <fitsio.h>

#include <DSTfits/FITSwcs.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>

#include <wcslib/wcserr.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcsprintf.h>
#include <wcslib/wcsutil.h>

// novas.h has no extern "C" guard of its own; wrap it so the linker resolves libsupernovas's C symbols.
extern "C" {
#include <novas.h>
}

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
            else if(static_cast<size_t>(other.getNumberOfWCS()) <= idx)
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
            else if(static_cast<size_t>(other.getNumberOfWCS()) <= idx)
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
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CTYPE",errmsg);
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
        
        std::vector< std::string> FITSwcs::CUNIT(const size_t& wcsIndex) const
        {
            if(fwcs == nullptr)
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_UNSET,"FITSwcs","getCUNITs",errmsg);
            }

            if(wcsIndex >= static_cast<size_t>(fnwcs))
            {
                std::string errmsg = wcs_errmsg[WCSERR_UNSET];
                throw WCSexception(WCSERR_BAD_PARAM,"FITSwcs","CUNIT",errmsg);
            }

            std::vector<std::string> cunit_list;
            size_t axis_cnt = static_cast<size_t>(fwcs.get()[wcsIndex].naxis);
            cunit_list.reserve(axis_cnt);

            for(size_t i = 0; i < axis_cnt; i++)
            {
                std::string cunit_axis(fwcs.get()[wcsIndex].cunit[i]);
                cunit_list.push_back(cunit_axis);
            }

            return cunit_list;
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

#pragma region SkyCoordinates calss implementation
        SkyCoordinates::SkyCoordinates():fphi(0.0),ftheta(0.0),fpmPhi(0.0),fpmTheta(0.0),fparallax(0.0),fradialVelocity(0.0),fepoch(NOVAS_JD_J2000),fcoordSys(CoordSystem::UNDEFINED),fcoordFrame(CoordFrame::UNDEFINED)
        { }

        SkyCoordinates::SkyCoordinates(const double& phi, const double& theta):fphi(phi),ftheta(theta),fpmPhi(0.0),fpmTheta(0.0),fparallax(0.0),fradialVelocity(0.0),fepoch(NOVAS_JD_J2000),fcoordSys(CoordSystem::UNDEFINED),fcoordFrame(CoordFrame::UNDEFINED)
        { }

        SkyCoordinates::SkyCoordinates(const double& phi, const double& theta, CoordSystem sys, CoordFrame frame):fphi(phi),ftheta(theta),fpmPhi(0.0),fpmTheta(0.0),fparallax(0.0),fradialVelocity(0.0),fepoch(epochOf(frame)),fcoordSys(sys),fcoordFrame(frame)
        { }

        SkyCoordinates::SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta):fphi(phi),ftheta(theta),fpmPhi(pmPhi),fpmTheta(pmTheta),fparallax(0.0),fradialVelocity(0.0),fepoch(NOVAS_JD_J2000),fcoordSys(CoordSystem::UNDEFINED),fcoordFrame(CoordFrame::UNDEFINED)
        { }

        SkyCoordinates::SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta, CoordSystem sys, CoordFrame frame):fphi(phi),ftheta(theta),fpmPhi(pmPhi),fpmTheta(pmTheta),fparallax(0.0),fradialVelocity(0.0),fepoch(epochOf(frame)),fcoordSys(sys),fcoordFrame(frame)
        { }

        SkyCoordinates::SkyCoordinates(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                                       const double& parallax, const double& radialVelocity, const double& epoch,
                                       CoordSystem sys, CoordFrame frame):fphi(phi),ftheta(theta),fpmPhi(pmPhi),fpmTheta(pmTheta),fparallax(parallax),fradialVelocity(radialVelocity),fepoch(std::isnan(epoch) ? epochOf(frame) : epoch),fcoordSys(sys),fcoordFrame(frame)
        { }

        void SkyCoordinates::setPhi(const double& phi) { fphi = phi; }
        void SkyCoordinates::setTheta(const double& theta) { ftheta = theta; }
        void SkyCoordinates::setProperMotion(const double& pmPhi, const double& pmTheta) { fpmPhi = pmPhi; fpmTheta = pmTheta; }
        void SkyCoordinates::setParallax(const double& parallax) { fparallax = parallax; }
        void SkyCoordinates::setRadialVelocity(const double& radialVelocity) { fradialVelocity = radialVelocity; }
        void SkyCoordinates::setEpoch(const double& epoch) { fepoch = epoch; }
        double SkyCoordinates::getPhi() const { return fphi; }
        double SkyCoordinates::getTheta() const { return ftheta; }
        SkyCoordinates::CoordFrame SkyCoordinates::getFrame() const { return fcoordFrame; }
        SkyCoordinates::CoordSystem SkyCoordinates::getSystem() const { return fcoordSys; }
        double SkyCoordinates::getProperMotionPhi() const { return fpmPhi; }
        double SkyCoordinates::getProperMotionTheta() const { return fpmTheta; }
        double SkyCoordinates::getParallax() const { return fparallax; }
        double SkyCoordinates::getRadialVelocity() const { return fradialVelocity; }
        double SkyCoordinates::getEpoch() const { return fepoch; }

#pragma region * Epochs
        // Every one of these forwards to SuperNOVAS: the library owns the definition of
        // what "B1950.0" or "J1991.25" means, and of how the system clock maps onto TT.

        //! ICRS and HIPPARCOS name the same set of axes; they differ only in their standard epoch.
        static bool isICRSOriented(const CoordFrame& frame)
        {
            return frame == CoordFrame::ICRS || frame == CoordFrame::HIPPARCOS;
        }

        //! The date a position carries after a frame conversion.
        //! An FK4 leg goes through transform_cat(CHANGE_EPOCH, ...), which advances the star
        //! along its proper motion as well as rotating it; every other leg is a pure rotation
        //! and leaves the date alone.
        static double frameConversionEpoch(const CoordFrame& from, const CoordFrame& to, const double& currentEpoch)
        {
            if(to == CoordFrame::FK4)
                return NOVAS_JD_B1950;  // carried back to B1950.0 on the way into FK4

            if(from == CoordFrame::FK4)
                return NOVAS_JD_J2000;  // ... and forward to J2000.0 on the way out of it

            return currentEpoch;
        }

        //! The NOVAS system name a frame is known by, and therefore the epoch it resolves to.
        static const char* novasSystemOf(const CoordFrame& frame)
        {
            switch(frame)
            {
                case CoordFrame::FK4:       return NOVAS_SYSTEM_FK4;
                case CoordFrame::FK5:       return NOVAS_SYSTEM_FK5;
                case CoordFrame::ICRS:      return NOVAS_SYSTEM_ICRS;
                case CoordFrame::HIPPARCOS: return NOVAS_SYSTEM_HIP;
                default:                    return NOVAS_SYSTEM_J2000;  // undefined frame: assume J2000.0
            }
        }

        double SkyCoordinates::epochOf(CoordFrame frame)
        {
            const double jd = novas_epoch(novasSystemOf(frame));
            if(std::isnan(jd))
                throw std::runtime_error("[ERROR] SkyCoordinates::epochOf: novas_epoch() rejected the system name");

            return jd;
        }

        double SkyCoordinates::epochOf(const std::string& spec)
        {
            const double jd = novas_epoch(spec.c_str());
            if(std::isnan(jd))
                throw std::invalid_argument("[ERROR] SkyCoordinates::epochOf: unrecognised epoch specification '" + spec + "'");

            return jd;
        }

        double SkyCoordinates::currentEpoch()
        {
            const std::time_t unixTime = std::time(nullptr);
            novas_timespec now = NOVAS_TIMESPEC_INIT;

            // UT1-UTC is left at zero: it never exceeds 0.9 s, which is far below the
            // resolution of any proper motion this epoch is going to be used with.
            if(novas_set_current_time(novas_lookup_leap(unixTime), 0.0, &now) != 0)
                throw std::runtime_error("[ERROR] SkyCoordinates::currentEpoch: novas_set_current_time() failed");

            return novas_get_time(&now, NOVAS_TT);
        }
#pragma endregion

        void SkyCoordinates::assign(const double& phi, const double& theta, CoordSystem sys, CoordFrame frame)
        {
            assign(phi, theta, 0.0, 0.0, sys, frame);
        }

        void SkyCoordinates::assign(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                                     CoordSystem sys, CoordFrame frame)
        {
            // Preserve the distance and the date: neither is changed by a plain re-assignment
            // of the angular coordinates.
            assign(phi, theta, pmPhi, pmTheta, fparallax, fradialVelocity, fepoch, sys, frame);
        }

        void SkyCoordinates::assign(const double& phi, const double& theta, const double& pmPhi, const double& pmTheta,
                                     const double& parallax, const double& radialVelocity, const double& epoch,
                                     CoordSystem sys, CoordFrame frame)
        {
            fphi            = phi;
            ftheta          = theta;
            fpmPhi          = pmPhi;
            fpmTheta        = pmTheta;
            fparallax       = parallax;
            fradialVelocity = radialVelocity;
            fepoch          = std::isnan(epoch) ? epochOf(frame) : epoch;
            fcoordSys       = sys;
            fcoordFrame     = frame;
        }

        double SkyCoordinates::angularSeparation(const SkyCoordinates& other) const
        {
            const double lon1 = fphi         * NOVAS_DEGREE;
            const double lat1 = ftheta       * NOVAS_DEGREE;
            const double lon2 = other.fphi   * NOVAS_DEGREE;
            const double lat2 = other.ftheta * NOVAS_DEGREE;

            const double dlon = lon2 - lon1;
            const double dlat = lat2 - lat1;

            // Haversine formula: stable for small separations
            const double sinDLatHalf = std::sin(dlat / 2.0);
            const double sinDLonHalf = std::sin(dlon / 2.0);

            double hav = sinDLatHalf * sinDLatHalf +
                         std::cos(lat1) * std::cos(lat2) * sinDLonHalf * sinDLonHalf;

            // Clamp to handle numerical rounding
            hav = std::max(0.0, std::min(1.0, hav));

            double sep = 2.0 * std::asin(std::sqrt(hav));

            // For very large angles (>~90°), law of cosines is more accurate
            // Use it as a fallback if haversine gives suspiciously large values
            if (sep > M_PI / 2.0) {
                double cosSep = std::sin(lat1)*std::sin(lat2) + 
                               std::cos(lat1)*std::cos(lat2)*std::cos(dlon);
                cosSep = std::max(-1.0, std::min(1.0, cosSep));
                sep = std::acos(cosSep);
            }

            return sep / NOVAS_DEGREE;
        }

#pragma endregion

#pragma region EquatorialCoordinates implementation

        EquatorialCoordinates::EquatorialCoordinates():SkyCoordinates(0.0, 0.0, CoordSystem::EQUATORIAL, CoordFrame::ICRS)
        {
        }

        EquatorialCoordinates::EquatorialCoordinates(const double& ra, const double& dec):SkyCoordinates(ra,dec, CoordSystem::EQUATORIAL, CoordFrame::ICRS)
        {
        }

        EquatorialCoordinates::EquatorialCoordinates(const double& ra, const double& dec, CoordFrame frame):SkyCoordinates(ra,dec, CoordSystem::EQUATORIAL, frame)
        {
        }

        EquatorialCoordinates::EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC):SkyCoordinates(ra, dec, pmRA, pmDEC, CoordSystem::EQUATORIAL, CoordFrame::ICRS)
        {
        }

        EquatorialCoordinates::EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC, CoordFrame frame):SkyCoordinates(ra, dec, pmRA, pmDEC, CoordSystem::EQUATORIAL, frame)
        {
        }

        EquatorialCoordinates::EquatorialCoordinates(const double& ra, const double& dec, const double& pmRA, const double& pmDEC,
                                                     const double& parallax, const double& radialVelocity,
                                                     CoordFrame frame, const double& epoch):SkyCoordinates(ra, dec, pmRA, pmDEC, parallax, radialVelocity, epoch, CoordSystem::EQUATORIAL, frame)
        {
        }

        EquatorialCoordinates::~EquatorialCoordinates()
        { }

        double EquatorialCoordinates::getRA()  const { return getPhi();   }
        double EquatorialCoordinates::getDEC() const { return getTheta(); }
        double EquatorialCoordinates::getPMRA()  const { return getProperMotionPhi();   }
        double EquatorialCoordinates::getPMDEC() const { return getProperMotionTheta(); }

        void EquatorialCoordinates::toEquatorial(SkyCoordinates* output, CoordFrame frame) const
        {
           if(!output)
               throw std::invalid_argument("[ERROR] EquatorialCoordinates::toEquatorial: output pointer is null");
        
           if (getFrame() == frame) {
               // No conversion needed, just copy
               output->assign(getRA(), getDEC(), getPMRA(), getPMDEC(),
                             getParallax(), getRadialVelocity(), getEpoch(),
                             CoordSystem::EQUATORIAL, frame);
               return;
           }

           // Convert frame: convertFrame wants RA in hours
           double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0, parallax = 0.0, rv = 0.0;
           EquatorialCoordinates::convertFrame(getRA() / 15.0, getDEC(),
                                              getPMRA(), getPMDEC(),
                                              getParallax(), getRadialVelocity(),
                                              getFrame(), frame,
                                              ra, dec, pmRA, pmDEC, parallax, rv);

           // Output in degrees
           output->assign(ra * 15.0, dec, pmRA, pmDEC, parallax, rv,
                          frameConversionEpoch(getFrame(), frame, getEpoch()),
                          CoordSystem::EQUATORIAL, frame);
        }

        void EquatorialCoordinates::toEcliptic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::toEcliptic: output pointer is null");
        
            // Get coordinates in requested frame
            double ra = getRA() / 15.0, dec = getDEC();
            double parallax = getParallax(), rv = getRadialVelocity();

            if (getFrame() != frame) {
                double pmRA = 0.0, pmDEC = 0.0;
                EquatorialCoordinates::convertFrame(ra, dec, getPMRA(), getPMDEC(),
                                                   parallax, rv,
                                                   getFrame(), frame,
                                                   ra, dec, pmRA, pmDEC, parallax, rv);
            }

            // Convert equatorial (in requested frame) to ecliptic
            double elon = 0.0, elat = 0.0;
            equ2ecl(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY, ra, dec, &elon, &elat);

            output->assign(elon, elat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::ECLIPTIC, frame);
        }

        void EquatorialCoordinates::toGalactic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::toGalactic: output pointer is null");
        
            // Get coordinates in requested frame
            double ra = getRA() / 15.0, dec = getDEC();
            double parallax = getParallax(), rv = getRadialVelocity();

            if (getFrame() != frame) {
                double pmRA = 0.0, pmDEC = 0.0;
                EquatorialCoordinates::convertFrame(ra, dec, getPMRA(), getPMDEC(),
                                                   parallax, rv,
                                                   getFrame(), frame,
                                                   ra, dec, pmRA, pmDEC, parallax, rv);
            }

            // Convert equatorial (in requested frame) to galactic
            double glon = 0.0, glat = 0.0;
            equ2gal(ra, dec, &glon, &glat);  // ← Use frame-converted coords

            output->assign(glon, glat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::GALACTIC, frame);
        }

        void EquatorialCoordinates::convertFrame(double raHours, double decDeg, double pmRaMasPerYr, double pmDecMasPerYr,
                                                  CoordFrame from, CoordFrame to,
                                                  double& raHoursOut, double& decDegOut,
                                                  double& pmRaMasPerYrOut, double& pmDecMasPerYrOut)
        {
            double parallaxOut = 0.0, rvOut = 0.0;
            convertFrame(raHours, decDeg, pmRaMasPerYr, pmDecMasPerYr, 0.0, 0.0, from, to,
                         raHoursOut, decDegOut, pmRaMasPerYrOut, pmDecMasPerYrOut, parallaxOut, rvOut);
        }

        // Converts an RA/Dec (+ space motion) entry between FK4 (B1950 dynamical mean equator/equinox),
        // FK5 (J2000 dynamical mean equator/equinox), ICRS and HIPPARCOS, via transform_cat() which
        // propagates the position and the proper motion together (CHANGE_EPOCH for FK4<->FK5,
        // CHANGE_*_TO_* for the FK5/J2000<->ICRS frame tie). The frame tie legs are time-independent
        // (see novas transform_cat()).
        //
        // HIPPARCOS shares the ICRS orientation, so it takes the same rotation path; what separates the
        // two is the epoch (J1991.25 vs J2000.0), which is the business of atEpoch(), not of a rotation.
        void EquatorialCoordinates::convertFrame(double raHours, double decDeg, double pmRaMasPerYr, double pmDecMasPerYr,
                                                  double parallaxMas, double rvKmPerSec,
                                                  CoordFrame from, CoordFrame to,
                                                  double& raHoursOut, double& decDegOut,
                                                  double& pmRaMasPerYrOut, double& pmDecMasPerYrOut,
                                                  double& parallaxMasOut, double& rvKmPerSecOut)
        {
            if(from == CoordFrame::UNDEFINED || to == CoordFrame::UNDEFINED)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::convertFrame: source/target frame is undefined");

            // HIPPARCOS and ICRS are the same set of axes, so a conversion between them is a no-op.
            const bool sameOrientation = (from == to)
                || (isICRSOriented(from) && isICRSOriented(to));

            if(sameOrientation)
            {
                raHoursOut       = raHours;
                decDegOut        = decDeg;
                pmRaMasPerYrOut  = pmRaMasPerYr;
                pmDecMasPerYrOut = pmDecMasPerYr;
                parallaxMasOut   = parallaxMas;
                rvKmPerSecOut    = rvKmPerSec;
                return;
            }

            cat_entry in, pivot, result;
            make_cat_entry("SRC", "USER", 0, raHours, decDeg, pmRaMasPerYr, pmDecMasPerYr, parallaxMas, rvKmPerSec, &in);

            // Step 1: bring the star to the J2000-dynamical frame pivot
            switch(from)
            {
                case CoordFrame::FK4:
                    transform_cat(CHANGE_EPOCH, NOVAS_JD_B1950, &in, NOVAS_JD_J2000, NOVAS_SYSTEM_FK5, &pivot);
                    break;
                case CoordFrame::FK5:
                    pivot = in;
                    break;
                case CoordFrame::ICRS:
                case CoordFrame::HIPPARCOS:
                    transform_cat(CHANGE_ICRS_TO_J2000, NOVAS_JD_J2000, &in, NOVAS_JD_J2000, NOVAS_SYSTEM_J2000, &pivot);
                    break;
                default:
                    throw std::invalid_argument("[ERROR] EquatorialCoordinates::convertFrame: unsupported source frame");
            }

            // Step 2: from the J2000-dynamical pivot to the requested target frame
            switch(to)
            {
                case CoordFrame::FK4:
                    transform_cat(CHANGE_EPOCH, NOVAS_JD_J2000, &pivot, NOVAS_JD_B1950, NOVAS_SYSTEM_FK4, &result);
                    break;
                case CoordFrame::FK5:
                    result = pivot;
                    break;
                case CoordFrame::ICRS:
                case CoordFrame::HIPPARCOS:
                    transform_cat(CHANGE_J2000_TO_ICRS, NOVAS_JD_J2000, &pivot, NOVAS_JD_J2000, NOVAS_SYSTEM_ICRS, &result);
                    break;
                default:
                    throw std::invalid_argument("[ERROR] EquatorialCoordinates::convertFrame: unsupported target frame");
            }

            raHoursOut       = result.ra;
            decDegOut        = result.dec;
            pmRaMasPerYrOut  = result.promora;
            pmDecMasPerYrOut = result.promodec;
            parallaxMasOut   = result.parallax;
            rvKmPerSecOut    = result.radialvelocity;
        }

#pragma region * Epoch propagation

        void EquatorialCoordinates::atEpoch(const double& targetEpoch, EquatorialCoordinates* output) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::atEpoch: output pointer is null");

            cat_entry in, moved;
            make_cat_entry("SRC", "USER", 0, getRA() / 15.0, getDEC(), getPMRA(), getPMDEC(),
                           getParallax(), getRadialVelocity(), &in);

            // PROPER_MOTION carries the star along its own space-motion vector between the two
            // dates and leaves the reference frame untouched, which is exactly the split this
            // class draws between "frame" (orientation) and "epoch" (date).
            if(transform_cat(PROPER_MOTION, getEpoch(), &in, targetEpoch, "USER", &moved) != 0)
                throw std::runtime_error("[ERROR] EquatorialCoordinates::atEpoch: transform_cat() failed");

            output->assign(moved.ra * 15.0, moved.dec, moved.promora, moved.promodec,
                           moved.parallax, moved.radialvelocity, targetEpoch,
                           CoordSystem::EQUATORIAL, getFrame());
        }

        void EquatorialCoordinates::atEpoch(const std::string& epochSpec, EquatorialCoordinates* output) const
        {
            atEpoch(SkyCoordinates::epochOf(epochSpec), output);
        }

        void EquatorialCoordinates::atEpoch(CoordFrame frame, EquatorialCoordinates* output) const
        {
            atEpoch(SkyCoordinates::epochOf(frame), output);
        }

        void EquatorialCoordinates::atCurrentEpoch(EquatorialCoordinates* output) const
        {
            atEpoch(SkyCoordinates::currentEpoch(), output);
        }
#pragma endregion

        // The three named helpers are toEquatorial() with the target frame fixed; keeping them as
        // one-line forwards means the distance, velocity and epoch bookkeeping lives in one place.
        void EquatorialCoordinates::toICRS(EquatorialCoordinates* output) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::toICRS: output pointer is null");

            toEquatorial(output, CoordFrame::ICRS);
        }

        void EquatorialCoordinates::toFK5(EquatorialCoordinates* output) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::toFK5: output pointer is null");

            toEquatorial(output, CoordFrame::FK5);
        }

        void EquatorialCoordinates::toFK4(EquatorialCoordinates* output) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EquatorialCoordinates::toFK4: output pointer is null");

            toEquatorial(output, CoordFrame::FK4);
        }

#pragma endregion

#pragma region GalacticCoordinates implementation

        GalacticCoordinates::GalacticCoordinates():SkyCoordinates()
        {
            assign(0.0, 0.0, CoordSystem::GALACTIC, CoordFrame::ICRS);
        }

        GalacticCoordinates::GalacticCoordinates(const double& glon, const double& glat):SkyCoordinates()
        {
            assign(glon, glat, CoordSystem::GALACTIC, CoordFrame::ICRS);
        }

        GalacticCoordinates::~GalacticCoordinates()
        { }

        double GalacticCoordinates::getGLON() const { return getPhi();   }
        double GalacticCoordinates::getGLAT() const { return getTheta(); }

        void GalacticCoordinates::toGalactic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] GalacticCoordinates::toGalactic: output pointer is null");
        
            if (frame == getFrame()) {
                // No conversion needed
                output->assign(getGLON(), getGLAT(), 0.0, 0.0,
                               getParallax(), getRadialVelocity(), getEpoch(),
                               CoordSystem::GALACTIC, frame);
                return;
            }

            // Step 1: Convert galactic → equatorial (stays in current frame)
            double ra = 0.0, dec = 0.0;
            gal2equ(getGLON(), getGLAT(), &ra, &dec);  // ra in hours

            // Step 2: Convert frame
            double raOut = ra, decOut = dec, pmRA = 0.0, pmDEC = 0.0;
            double parallax = getParallax(), rv = getRadialVelocity();
            EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                               getFrame(), frame,
                                               raOut, decOut, pmRA, pmDEC, parallax, rv);

            // Step 3: Convert back to galactic in new frame
            double glon = 0.0, glat = 0.0;
            equ2gal(raOut, decOut, &glon, &glat);

            output->assign(glon, glat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::GALACTIC, frame);
        }

        void GalacticCoordinates::toEquatorial(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] GalacticCoordinates::toEquatorial: output pointer is null");
        
            // Step 1: Convert galactic → equatorial (stays in our current frame)
            double ra = 0.0, dec = 0.0;
            gal2equ(getGLON(), getGLAT(), &ra, &dec);  // ra in hours, dec in degrees
            
            // Step 2: Apply frame conversion if needed
            double raOut = ra, decOut = dec, pmRA = 0.0, pmDEC = 0.0;
            double parallax = getParallax(), rv = getRadialVelocity();
            if (frame != getFrame()) {
                EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                                   getFrame(), frame,
                                                   raOut, decOut, pmRA, pmDEC, parallax, rv);
            }

            // Step 3: Output in degrees (ra*15 converts hours→degrees)
            output->assign(raOut * 15.0, decOut, pmRA, pmDEC, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::EQUATORIAL, frame);
        }

        void GalacticCoordinates::toEcliptic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] GalacticCoordinates::toEcliptic: output pointer is null");

            // Step 1: Convert galactic → equatorial (stays in current frame)
            double ra = 0.0, dec = 0.0;
            gal2equ(getGLON(), getGLAT(), &ra, &dec);  // ra in hours, dec in degrees

            // Step 2: Convert frame if needed
            double parallax = getParallax(), rv = getRadialVelocity();
            if(getFrame() != frame)
            {
                double pmRA = 0.0, pmDEC = 0.0;
                EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                                   getFrame(), frame,
                                                   ra, dec, pmRA, pmDEC, parallax, rv);
            }

            // Step 3: Convert equatorial (in requested frame) to ecliptic
            double elon = 0.0, elat = 0.0;
            equ2ecl(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY, ra, dec, &elon, &elat);

            output->assign(elon, elat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::ECLIPTIC, frame);
        }

#pragma endregion

#pragma region EclipticCoordinates implementation

        EclipticCoordinates::EclipticCoordinates():SkyCoordinates()
        {
            assign(0.0, 0.0, CoordSystem::ECLIPTIC, CoordFrame::ICRS);
        }

        EclipticCoordinates::EclipticCoordinates(const double& elon, const double& elat):SkyCoordinates()
        {
            assign(elon, elat, CoordSystem::ECLIPTIC, CoordFrame::ICRS);
        }

        EclipticCoordinates::~EclipticCoordinates()
        { }

        double EclipticCoordinates::getELON() const { return getPhi();   }
        double EclipticCoordinates::getELAT() const { return getTheta(); }

        void EclipticCoordinates::toEcliptic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EclipticCoordinates::toEcliptic: output pointer is null");

            double elon = getELON(), elat = getELAT();
            double parallax = getParallax(), rv = getRadialVelocity();

            if(frame != getFrame())
            {
                // The frame change is defined on equatorial coordinates, so round-trip
                // through them: ecliptic → equatorial, change frame, equatorial → ecliptic.
                double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0;
                ecl2equ(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY,
                        elon, elat, &ra, &dec);  // ra in hours, dec in degrees

                EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                                   getFrame(), frame,
                                                   ra, dec, pmRA, pmDEC, parallax, rv);

                equ2ecl(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY, ra, dec, &elon, &elat);
            }

            output->assign(elon, elat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::ECLIPTIC, frame);
        }

        void EclipticCoordinates::toEquatorial(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EclipticCoordinates::toEquatorial: output pointer is null");

            // Step 1: Convert ecliptic → equatorial (stays in current frame)
            double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0;
            ecl2equ(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY,
                    getELON(), getELAT(), &ra, &dec);  // ra in hours, dec in degrees

            // Step 2: Convert frame if needed
            double parallax = getParallax(), rv = getRadialVelocity();
            if(frame != getFrame())
            {
                EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                                   getFrame(), frame,
                                                   ra, dec, pmRA, pmDEC, parallax, rv);
            }

            // Step 3: Output in degrees (ra*15 converts hours→degrees)
            output->assign(ra * 15.0, dec, pmRA, pmDEC, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::EQUATORIAL, frame);
        }

        void EclipticCoordinates::toGalactic(SkyCoordinates* output, CoordFrame frame) const
        {
            if(!output)
                throw std::invalid_argument("[ERROR] EclipticCoordinates::toGalactic: output pointer is null");
        
            // Step 1: Convert ecliptic → equatorial (stays in current frame)
            double ra = 0.0, dec = 0.0;
            ecl2equ(NOVAS_JD_J2000, NOVAS_GCRS_EQUATOR, NOVAS_FULL_ACCURACY, 
                    getELON(), getELAT(), &ra, &dec);
            
            // Step 2: Convert frame if needed
            double raOut = ra, decOut = dec, pmRA = 0.0, pmDEC = 0.0;
            double parallax = getParallax(), rv = getRadialVelocity();
            if (frame != getFrame())
            {
                EquatorialCoordinates::convertFrame(ra, dec, 0.0, 0.0, parallax, rv,
                                                   getFrame(), frame,
                                                   raOut, decOut, pmRA, pmDEC, parallax, rv);
            }

            // Step 3: Convert to galactic
            double glon = 0.0, glat = 0.0;
            equ2gal(raOut, decOut, &glon, &glat);

            output->assign(glon, glat, 0.0, 0.0, parallax, rv,
                           frameConversionEpoch(getFrame(), frame, getEpoch()),
                           CoordSystem::GALACTIC, frame);
        }

#pragma endregion
}
#pragma endregion
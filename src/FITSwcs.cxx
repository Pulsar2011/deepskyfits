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

#pragma endregion
#pragma region * Conversion

        worldVectors FITSwcs::pixel2world(const size_t& wcsIndex, const pixelVectors& px)
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

            if(px .size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pix2world",errmsg);
            }

            if(px[0].size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","pix2world",errmsg);
            }

            // Use heap-backed flattened arrays to avoid VLAs (Clang warns about VLA extension)
            const int npix  = static_cast<const int>(px.size());
            const int ncoord = static_cast<const int>(px[0].size());
            std::vector<double> pixC(static_cast<size_t>(npix) * ncoord);

            // Parallel fill: split rows among threads; each thread writes a disjoint range -> no data races
            unsigned int hw = std::thread::hardware_concurrency();
            int nthreads = (hw == 0) ? 1 : static_cast<int>(std::min<unsigned int>(static_cast<unsigned int>(npix), hw));

            if (nthreads <= 1)
            {
                for (int i = 0; i < npix; ++i)
                    for (int j = 0; j < ncoord; ++j)
                        pixC[static_cast<size_t>(i) * ncoord + j] = px[static_cast<size_t>(i)][static_cast<size_t>(j)];
            }
            else
            {
                // Define number of worker threads
                std::vector<std::thread> workers;
                workers.reserve(static_cast<size_t>(nthreads));

                // Worker lambda to fill a chunk of rows
                auto worker = [&](int start, int end){
                    for (int ii = start; ii < end; ++ii)
                    {
                        const size_t base = static_cast<size_t>(ii) * ncoord;
                        for (int j = 0; j < ncoord; ++j)
                            pixC[base + j] = px[static_cast<size_t>(ii)][static_cast<size_t>(j)];
                    }
                };

                // Divide work among threads
                int chunk = npix / nthreads;
                int rem = npix % nthreads;
                int cur = 0;
                for (int t = 0; t < nthreads; ++t)
                {
                    int start = cur;
                    int size = chunk + (t < rem ? 1 : 0);
                    int end = start + size;
                    workers.emplace_back(worker, start, end);
                    cur = end;
                }

                // Wait for all threads to finish
                for (auto &th : workers) th.join();
            }

            std::vector<double> imgcrd_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> phi_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> theta_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> world_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<int> stat_vec(static_cast<size_t>(npix));

            // wcsp2s expects (const struct wcsprm*, ncoord, npix, pixC, imgcrd, phi, theta, world, stat)
            // pass flattened buffers by casting to the expected pointer-to-array type (this is the usual approach
            // when wrapping C APIs that use runtime 2D array params).
            
            if( (fwcs_status = wcsp2s(&fwcs.get()[wcsIndex],
                                      ncoord,
                                      npix,
                                      pixC.data(),
                                      imgcrd_vec.data(),
                                      phi_vec.data(),
                                      theta_vec.data(),
                                      world_vec.data(),
                                      stat_vec.data())) )
            {
                std::string errmsg = wcs_errmsg[fwcs_status];
                throw WCSexception(fwcs_status,"FITSwcs","pix2world",errmsg);
            }

            // Parallelize building the worldVectors output.
            // Preallocate and let each thread write distinct indices (no synchronization needed).
            worldVectors wv(static_cast<size_t>(npix)); // each slot will be filled (or left empty on error)

            // Reuse earlier computed of number of threads nthreads
            if (nthreads <= 1)
            {

                for (int i = 0; i < npix; ++i)
                {
                    if (stat_vec[static_cast<size_t>(i)] > 0)
                    {
                        WCSexception e(stat_vec[static_cast<size_t>(i)],"FITSwcs","pix2world","Error converting pixel to world coordinates");
                        if((verbose & verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS&verboseLevel::VERBOSE_BASIC))
                            std::cerr << e.what() << std::endl;
                        
                        // leave wv[i] empty
                        continue;
                    }
                    
                    //Reserve and fill in the world coordinates
                    wv[static_cast<size_t>(i)].reserve(static_cast<size_t>(ncoord));
                    const size_t base = static_cast<size_t>(i) * ncoord;
                    for (int j = 0; j < ncoord; ++j)
                        wv[static_cast<size_t>(i)].push_back(world_vec[base + j]);
                }
            }
            else
            {
                // Define number of worker threads
                std::vector<std::thread> workers;
                workers.reserve(static_cast<size_t>(nthreads));

                // Worker lambda to fill a chunk of rows
                auto worker = [&](int start, int end){
                    for (int ii = start; ii < end; ++ii)
                    {
                        if (stat_vec[static_cast<size_t>(ii)] > 0)
                        {
                            WCSexception e(stat_vec[static_cast<size_t>(ii)],"FITSwcs","pix2world","Error converting pixel to world coordinates");
                            if((verbose & verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS&verboseLevel::VERBOSE_BASIC))
                                std::cerr << e.what() << std::endl;

                            continue; // leave wv[ii] empty
                        }

                        //Reserve and fill in the world coordinates
                        const size_t base = static_cast<size_t>(ii) * ncoord;
                        auto &out = wv[static_cast<size_t>(ii)];
                        out.reserve(static_cast<size_t>(ncoord));
                        for (int j = 0; j < ncoord; ++j)
                            out.push_back(world_vec[base + j]);
                    }
                };

                // Divide work among threads
                int chunk = npix / nthreads;
                int rem = npix % nthreads;
                int cur = 0;
                for (int t = 0; t < nthreads; ++t)
                {
                    int start = cur;
                    int size = chunk + (t < rem ? 1 : 0);
                    int end = start + size;
                    workers.emplace_back(worker, start, end);
                    cur = end;
                }

                // Wait for all threads to finish
                for (auto &th : workers) th.join();
            }

            return wv;
        }

        pixelVectors FITSwcs::world2pixel(const size_t& wcsIndex, const worldVectors& px)
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

            if(px .size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel",errmsg);
            }

            if(px[0].size() == 0)
            {
                std::string errmsg = wcs_errmsg[WCSERR_BAD_PIX];
                throw WCSexception(WCSERR_BAD_PIX,"FITSwcs","world2pixel",errmsg);
            }

            // Use heap-backed flattened arrays to avoid VLAs (Clang warns about VLA extension)
            const int npix  = static_cast<const int>(px.size());
            const int ncoord = static_cast<const int>(px[0].size());
            std::vector<double> worldC(static_cast<size_t>(npix) * ncoord);

            // Parallel fill: split rows among threads; each thread writes a disjoint range -> no data races
            unsigned int hw = std::thread::hardware_concurrency();
            int nthreads = (hw == 0) ? 1 : static_cast<int>(std::min<unsigned int>(static_cast<unsigned int>(npix), hw));

            if (nthreads <= 1)
            {
                for (int i = 0; i < npix; ++i)
                    for (int j = 0; j < ncoord; ++j)
                        worldC[static_cast<size_t>(i) * ncoord + j] = px[static_cast<size_t>(i)][static_cast<size_t>(j)];
            }
            else
            {
                // Define number of worker threads
                std::vector<std::thread> workers;
                workers.reserve(static_cast<size_t>(nthreads));

                // Worker lambda to fill a chunk of rows
                auto worker = [&](int start, int end){
                    for (int ii = start; ii < end; ++ii)
                    {
                        const size_t base = static_cast<size_t>(ii) * ncoord;
                        for (int j = 0; j < ncoord; ++j)
                            worldC[base + j] = px[static_cast<size_t>(ii)][static_cast<size_t>(j)];
                    }
                };

                // Divide work among threads
                int chunk = npix / nthreads;
                int rem = npix % nthreads;
                int cur = 0;
                for (int t = 0; t < nthreads; ++t)
                {
                    int start = cur;
                    int size = chunk + (t < rem ? 1 : 0);
                    int end = start + size;
                    workers.emplace_back(worker, start, end);
                    cur = end;
                }

                // Wait for all threads to finish
                for (auto &th : workers) th.join();
            }

            std::vector<double> imgcrd_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> phi_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> theta_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<double> pixel_vec(static_cast<size_t>(npix) * ncoord);
            std::vector<int> stat_vec(static_cast<size_t>(npix));

            // wcss2p expects (const struct wcsprm*, ncoord, npix, pixC, imgcrd, phi, theta, world, stat)
            // pass flattened buffers by casting to the expected pointer-to-array type (this is the usual approach
            // when wrapping C APIs that use runtime 2D array params).
            if( (fwcs_status = wcss2p(&fwcs.get()[wcsIndex],
                                      ncoord,
                                      npix,
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

            // Parallelize building the worldVectors output.
            // Preallocate and let each thread write distinct indices (no synchronization needed).
            pixelVectors wv(static_cast<size_t>(npix)); // each slot will be filled (or left empty on error)

            // Reuse earlier computed of number of threads nthreads
            if (nthreads <= 1)
            {

                for (int i = 0; i < npix; ++i)
                {
                    if (stat_vec[static_cast<size_t>(i)] > 0)
                    {
                        WCSexception e(stat_vec[static_cast<size_t>(i)],"FITSwcs","world2pixel","Error converting world to pixel coordinates");
                        if((verbose & verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS&verboseLevel::VERBOSE_BASIC))
                            std::cerr << e.what() << std::endl;
                        
                        // leave wv[i] empty
                        continue;
                    }
                    
                    //Reserve and fill in the pixel coordinates
                    wv[static_cast<size_t>(i)].reserve(static_cast<size_t>(ncoord));
                    const size_t base = static_cast<size_t>(i) * ncoord;
                    for (int j = 0; j < ncoord; ++j)
                        wv[static_cast<size_t>(i)].push_back(pixel_vec[base + j]);
                }
            }
            else
            {
                // Define number of worker threads
                std::vector<std::thread> workers;
                workers.reserve(static_cast<size_t>(nthreads));

                // Worker lambda to fill a chunk of rows
                auto worker = [&](int start, int end){
                    for (int ii = start; ii < end; ++ii)
                    {
                        if (stat_vec[static_cast<size_t>(ii)] > 0)
                        {
                            WCSexception e(stat_vec[static_cast<size_t>(ii)],"FITSwcs","world2pixel","Error converting world to pixel coordinates");
                            if((verbose & verboseLevel::VERBOSE_DEBUG)==(verboseLevel::VERBOSE_WCS&verboseLevel::VERBOSE_BASIC))
                                std::cerr << e.what() << std::endl;

                            continue; // leave wv[ii] empty
                        }

                        //Reserve and fill in the world coordinates
                        const size_t base = static_cast<size_t>(ii) * ncoord;
                        auto &out = wv[static_cast<size_t>(ii)];
                        out.reserve(static_cast<size_t>(ncoord));
                        for (int j = 0; j < ncoord; ++j)
                            out.push_back(pixel_vec[base + j]);
                    }
                };

                // Divide work among threads
                int chunk = npix / nthreads;
                int rem = npix % nthreads;
                int cur = 0;
                for (int t = 0; t < nthreads; ++t)
                {
                    int start = cur;
                    int size = chunk + (t < rem ? 1 : 0);
                    int end = start + size;
                    workers.emplace_back(worker, start, end);
                    cur = end;
                }

                // Wait for all threads to finish
                for (auto &th : workers) th.join();
            }
            
            return wv;
        }

        std::string FITSwcs::asString(const int& wcsIndex)
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

        FITShdu FITSwcs::asFITShdu(const int& wcsIndex)
        {
            std::string header = asString(wcsIndex);
            FITShdu shdr(header);

            return shdr;
        }


#pragma endregion

}
#pragma endregion
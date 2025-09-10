//
//  FITSimg.h++
//  
//
//  Created by GILLARD William on 31/03/15.
//	Copyright (c) 2015, All rights reserved
//  CPPM
//

#ifndef _DSL_FITSimg_
#define _DSL_FITSimg_

#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <map>
#include <vector>
#include <valarray>
#include <limits>
#include <stdexcept>
#include <functional>
#include <thread>
#include <mutex>
#include <algorithm> // <-- added for std::min

#include <Minuit2/FCNBase.h>
#include <Minuit2/MinimumBuilder.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnUserParameterState.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnMinos.h>
#include <Minuit2/MnContours.h>
#include <Minuit2/MnPlot.h>
#include <Minuit2/MnPrint.h>

#include "FITShdu.h"
#include "FITSexception.h"

namespace DSL
{
    typedef std::valarray<bool> pxMask;

    namespace stat
    {
        class Percentil: public ROOT::Minuit2::FCNBase
        {
        private:
            const std::valarray<double> val;
            double fpp;
            double sum;

            // helper: sequential sum for a range
            static double partial_sum(const std::valarray<double>& v, size_t start, size_t end)
            {
                double s = 0.0;
                for (size_t i = start; i < end; ++i) s += v[i];
                return s;
            }

            void summation()
            {
                unsigned int threads = std::thread::hardware_concurrency();

                const size_t n = val.size();
                if (n == 0)
                {
                    sum = 0.0;
                    return;
                }

                unsigned int t = (threads == 0) ? 1 : threads;

                // do not spawn more threads than elements
                if (static_cast<size_t>(t) > n) t = static_cast<unsigned int>(n);

                const size_t chunk = (n + t - 1) / t;
                
                std::vector<std::thread> workers;
                std::vector<double> partials(t, 0.0);

                for (unsigned int i = 0; i < t; ++i)
                {
                    const size_t start = i * chunk;
                    const size_t end = std::min(n, start + chunk);
                    if (start >= end) continue;
                    // capture index by value to avoid race on i
                    workers.emplace_back([this, start, end, &partials, idx = i]() {
                        partials[idx] = partial_sum(this->val, start, end);
                    });
                }

                for (auto &th : workers)
                {
                    if (th.joinable())
                        throw std::runtime_error("internal error: expected joinable worker thread (detached or moved)");
                    th.join();
                }

                for (std::vector<double>::const_iterator i = partials.cbegin(); i != partials.cend(); ++i) sum += (*i);
            }

        public:
            
            Percentil(const std::valarray<double>& array): val(array), fpp(0.5), sum(0.0)
            {
                summation();
            }

            Percentil(const std::vector<double>& array): val(array.data(), array.size()), fpp(0.5), sum(0.0)
            {
                summation();
            }

            Percentil(const std::vector<double>& array, const double& pp): val(array.data(), array.size()), fpp(pp), sum(0.0)
            {
                summation();
            }

            // proper move constructor
            Percentil(const Percentil& other) noexcept : val(other.val), fpp(other.fpp), sum(other.sum)
            {
                summation();
            }

            // Accept integer vectors by converting to vector<double> and delegating
            Percentil(const std::vector<uint16_t>& array)
                : Percentil(std::vector<double>(array.begin(), array.end()))
            {}

            Percentil(const std::vector<uint16_t>& array, const double& pp)
                : Percentil(std::vector<double>(array.begin(), array.end()), pp)
            {}

            Percentil(const std::vector<uint32_t>& array)
                : Percentil(std::vector<double>(array.begin(), array.end()))
            {}

            Percentil(const std::vector<uint32_t>& array, const double& pp)
                : Percentil(std::vector<double>(array.begin(), array.end()), pp)
            {}

            Percentil(const std::vector<int16_t>& array)
                : Percentil(std::vector<double>(array.begin(), array.end()))
            {}

            Percentil(const std::vector<int16_t>& array, const double& pp)
                : Percentil(std::vector<double>(array.begin(), array.end()), pp)
            {}

            Percentil(const std::vector<int32_t>& array)
                : Percentil(std::vector<double>(array.begin(), array.end()))
            {}

            Percentil(const std::vector<int32_t>& array, const double& pp)
                : Percentil(std::vector<double>(array.begin(), array.end()), pp)
            {}

            Percentil(const std::vector<float>& array)
                : Percentil(std::vector<double>(array.begin(), array.end()))
            {}

            Percentil(const std::vector<float>& array, const double& pp)
                : Percentil(std::vector<double>(array.begin(), array.end()), pp)
            {}

            // Accept valarray<uint16_t> by delegating via a temporary vector<double>
            Percentil(const std::valarray<uint16_t>& array)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)))
            {}

            Percentil(const std::valarray<uint16_t>& array, const double& pp)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)), pp)
            {}

            // Accept valarray<uint32_t> by delegating via a temporary vector<double>
            Percentil(const std::valarray<uint32_t>& array)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)))
            {}

            Percentil(const std::valarray<uint32_t>& array, const double& pp)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)), pp)
            {}

            // Accept valarray<uint32_t> by delegating via a temporary vector<double>
            Percentil(const std::valarray<int16_t>& array)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)))
            {}

            Percentil(const std::valarray<int16_t>& array, const double& pp)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)), pp)
            {}

            // Accept valarray<int32_t> by delegating via a temporary vector<double>
            Percentil(const std::valarray<int32_t>& array)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)))
            {}

            Percentil(const std::valarray<int32_t>& array, const double& pp)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)), pp)
            {}

            // Accept valarray<float> by delegating via a temporary vector<double>
            Percentil(const std::valarray<float>& array)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)))
            {}

            Percentil(const std::valarray<float>& array, const double& pp)
                : Percentil(std::vector<double>(std::begin(array), std::end(array)), pp)
            {}

            ~Percentil() {}

            void SetPercentil(double pp) { fpp = pp; }

            double operator()(const std::vector<double>& param) const
            {
    	        return pow(  static_cast<double>((val[ val <= param[0] ]).size())  / static_cast<double>(val.size()) - fpp, 2.) ;
            }

            double Eval(double th) const
            {
    	        return  static_cast<double>( (val[ val <= th ]).size() ) / static_cast<double>(val.size()) ;
            }

            virtual double Up() const { return 4; }
        };
    }
    
#pragma mark - FITScube class definition
    class FITScube
    {
        /**
         * @class DSL::FITScube FITSimg.h "fitsExtractor/FITSimg.h"
         * @author GILLARD William
         * @version 1.0
         * @date 31/03/2015
         * @brief Based class used to manage FITS data cube.
         * @details This parent class is used to acces FITS data cube of any base type, plot and process data in from the FITS datacube. This class is purely virtual class which call the methods of its child class accouting for the base type of the raw data.
         */
    protected:
#pragma mark • Protected member
        pxMask mask;
        FITShdu *hdu;                           //!< Header of the image
        
        std::vector<size_t> Naxis;  //!< Dimenssion of the image axis
        int eqBITPIX, BITPIX;                   //!< Type of data contained into the image
        std::string name;                       //!< Extension name
        int img_status;

#pragma mark • Protected member function
        static std::vector<size_t> Build_axis(unsigned int, size_t, va_list args);
        
    private:
#pragma mark • Initialization
        void init();                    //!< Initialization
        virtual void WriteDataCube(const std::shared_ptr<fitsfile>&) {}; //!< pur virtual class methods used by child classes to write data to a fits file on disk

#pragma mark • Purely virtual & protected methos
    protected:
        virtual void img_init(){};      //!< Child class initialization
        
#pragma mark • ctor/dtor
    protected:
        FITScube(const std::shared_ptr<fitsfile>& );            //!< Constructor
        FITScube();
        
    public:
        FITScube(const FITScube&);      //!< Copy constructor
        virtual ~FITScube();            //!< Destructor
        
        
        virtual void Resize(size_t, size_t, size_t, size_t){};
        
        static FITScube* UByteFITSimg      (unsigned int, size_t, ...);
        static FITScube* ByteFITSimg       (unsigned int, size_t, ...);
        static FITScube* UShortFITSimg     (unsigned int, size_t, ...);
        static FITScube* ShortFITSimg      (unsigned int, size_t, ...);
        static FITScube* IntFITSimg        (unsigned int, size_t, ...);
        static FITScube* UIntFITSimg       (unsigned int, size_t, ...);
        static FITScube* LongFITSimg       (unsigned int, size_t, ...);
        static FITScube* ULongFITSimg      (unsigned int, size_t, ...);
        static FITScube* LongLongFITSimg   (unsigned int, size_t, ...);
        static FITScube* FloatFITSimg      (unsigned int, size_t, ...);
        static FITScube* DoubleFITSimg     (unsigned int, size_t, ...);
        
        static FITScube* UByteFITSimg       (std::vector<size_t>);
        static FITScube* ByteFITSimg        (std::vector<size_t>);
        static FITScube* UShortFITSimg      (std::vector<size_t>);
        static FITScube* ShortFITSimg       (std::vector<size_t>);
        static FITScube* IntFITSimg         (std::vector<size_t>);
        static FITScube* UIntFITSimg        (std::vector<size_t>);
        static FITScube* LongFITSimg        (std::vector<size_t>);
        static FITScube* ULongFITSimg       (std::vector<size_t>);
        static FITScube* LongLongFITSimg    (std::vector<size_t>);
        static FITScube* FloatFITSimg       (std::vector<size_t>);
        static FITScube* DoubleFITSimg      (std::vector<size_t>);
        
#pragma mark • Static member
        static bool debug;
        
#pragma mark • Accessor
        unsigned long long Size(unsigned int i = 0) const ;              //!< Get number of pixel of the axe
        size_t           Nelements() const;                            //!< Get total number of pixel
        inline const int Status() const {return img_status;}            //!< Get fits error status
        inline const int GetBitPerPixel() const {return BITPIX;}        //!< Get the number of Bit per image pixel
        inline const size_t GetDimension() const {return Naxis.size();}
        
        enum zScale
        {
            /**
             *  @enum DSL::FITScube::zScale
             *  @details Defines the type of the z axis during Qt display. It do not affect the ROOT display.
             */
            fLIN,   ///< Linear scale
            fLOG,   ///< Logaritmic scale
            fSINH,  ///< Sinus hyperbolic scale
            fQUAD,  ///< Quadratic scale
            fSQRT,  ///< Squared root scale
        };
        
        std::vector<double>                     WorldCoordinates(size_t) const; //!< Get world coordinates
        virtual std::vector<double>             WorldCoordinates(std::vector<unsigned long long>) const; //!< Get world coordinates
        virtual std::vector<double>             WorldCoordinates(std::vector<double>) const; //!< Get world coordinates
        
        virtual std::vector<double> World2Pixel( std::vector<double> ) const; //!< Get pixel coordinates based on WCS
        
        virtual std::vector<unsigned long long> PixelCoordinates(size_t) const; //!< Get pixel coordinates
        
        /**
         *  Obtain the pixel index from the 1D FITS datacube based on the cartesian coordinate of the pixel
         *  @param iPx: Cartésian coordinates of the pixel
         *  @return Pixel index in the 1D FITS datacube
         */
        template<typename T>
        size_t PixelIndex(T iPx, ...) const           //!< Get pixel index
        {
            T arg = iPx;
            std::vector<unsigned long long> pix_index;
            pix_index.push_back(arg);
            
            va_list ap;
            va_start(ap, iPx);
            
            for(size_t naxe = 1; naxe < Naxis.size(); naxe++)
            {
                arg = va_arg(ap, T);
                
                if(arg == 0)
                    break;
                
                pix_index.push_back(static_cast<unsigned long long>(arg));
                
                if(static_cast<double>(arg) - static_cast<double>(pix_index[pix_index.size()-1]) >= 0.5)
                    pix_index[pix_index.size()-1]++;
            }
            va_end(ap);
            

#if __cplusplus >= 199711L
            auto the_index =  std::bind(static_cast<size_t(FITScube::*)(std::vector<unsigned long long>) const>(&FITScube::PixelIndex), this,std::placeholders::_1);
            
            size_t index = the_index(pix_index);
#else
            size_t index = PixelIndex(pix_index);
#endif
            
            pix_index.clear();
            
            return index;
        }
        
        /**
         *  Obtain the pixel index from the 1D FITS datacube based on the cartesian coordinate of the pixel
         *  @param iPx: Cartésian coordinates of the pixel
         *  @return Pixel index in the 1D FITS datacube
         */
        template<typename T>
        std::vector<double> World2Pixel(T iPx, ...) const           //!< Get pixel index
        {
            T arg = iPx;
            std::vector<double> pix_coo;
            
            pix_coo.push_back(static_cast<double>( arg ));
            
            va_list ap;
            va_start(ap, iPx);
            
            for(size_t naxe = 1; naxe < Naxis.size(); naxe++)
            {
                arg = va_arg(ap, T);
                
                if(arg == 0)
                    break;
                
                pix_coo.push_back(static_cast<double>(arg));
            }
            va_end(ap);
            
            
#if __cplusplus >= 199711L
            auto the_index =  std::bind(static_cast< std::vector<double>(FITScube::*)(std::vector<double>) const>( &FITScube::World2Pixel ), this,std::placeholders::_1);
            
            std::vector<double> coo = the_index(pix_coo);
#else
            std::vector<double> coo = World2Pixel(pix_coo);
#endif
            
            pix_coo.clear();
            
            return coo;
        }
        
        virtual size_t PixelIndex(std::vector<unsigned long long>) const;           //!< Get pixel index
        virtual size_t PixelIndex(std::vector<double>) const;
        
        inline const FITShdu& pHDU() const {return *hdu;}
        inline  FITShdu* HDU()  {return hdu;}
        
        virtual double GetSum ()                  const =0;
        virtual double GetMean()                  const =0;
        virtual double GetQuadraticMean()         const =0;
        virtual double GetVariance()              const =0;
        virtual double GetMedian(bool fast=false) const =0;
        virtual double GetMinimum()               const =0;
        virtual double GetMaximum()               const =0;
        virtual double Getpercentil(double, bool fast=false) const =0;
        virtual double Get5thpercentil()          const =0;
        virtual double Get25thpercentil()         const =0;
        virtual double Get75thpercentil()         const =0;
        virtual double Get95thpercentil()         const =0;
        
        inline std::valarray<bool> GetMask() const {return mask;}
        
        
        virtual const double* GetDataCube() const = 0 ;
        virtual const std::valarray<double>& GetData() const = 0;
        
        virtual uint8_t      UByteValueAtPixel(size_t)             const {return 0;}
        virtual uint8_t      UByteValueAtPixel(unsigned long long) const {return 0;}
        virtual int8_t       ByteValueAtPixel(size_t)              const {return 0;}
        virtual int8_t       ByteValueAtPixel(unsigned long long)  const {return 0;}
        virtual uint16_t     UShortValueAtPixel(size_t)            const {return 0;}
        virtual uint16_t     UShortValueAtPixel(unsigned long long)const {return 0;}
        virtual int16_t      ShortValueAtPixel(size_t)             const {return 0;}
        virtual int16_t      ShortValueAtPixel(unsigned long long) const {return 0;}
        virtual unsigned int UIntValueAtPixel(size_t)              const {return 0;}
        virtual unsigned int UIntValueAtPixel(unsigned long long)  const {return 0;}
        virtual int          IntValueAtPixel(size_t)               const {return 0;}
        virtual int          IntValueAtPixel(unsigned long long)   const {return 0;}
        virtual uint32_t     ULongValueAtPixel(size_t)             const {return 0;}
        virtual uint32_t     ULongValueAtPixel(unsigned long long) const {return 0;}
        virtual int32_t      LongValueAtPixel(size_t)              const {return 0;}
        virtual int32_t      LongValueAtPixel(unsigned long long)  const {return 0;}
        virtual int64_t      LongLongValueAtPixel(size_t)          const {return 0;}
        virtual int64_t      LongLongValueAtPixel(unsigned long long)const {return 0;}
        virtual float        FloatValueAtPixel( size_t )           const {return 0;}
        virtual float        FloatValueAtPixel(unsigned long long) const {return 0;}
        virtual double       DoubleValueAtPixel( size_t )          const {return 0;}
        virtual double       DoubleValueAtPixel(unsigned long long)const {return 0;}
        

#pragma mark • I/O
        
        void Write(const std::shared_ptr<fitsfile>& );                          //!< Write FITS image to a fitsfile
        void Write(std::string, bool replace = false);  //!< Write FITS image to a new fitsfile
        
#pragma mark • Modifier
        void SetAxisLength(unsigned long int n, long long size);
        void DeleteLastAxis();
        void BitPerPixel(int, int eq = 0);
        void SetName    (std::string);
        virtual void Bscale(double ){};
        virtual void Bzero (double ){};
        virtual void Blank (uint16_t ){};
        
#pragma mark • Operator
        template <typename T>
        void operator+=(const T& val)
        {
            switch (BITPIX)
            {
                case 8:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetUByteAtPixel(UByteValueAtPixel(k) + static_cast<uint8_t>(val),k);
                    break;
                    
                case 10:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetByteAtPixel(ByteValueAtPixel(k) + static_cast<int8_t>(val),k);
                    break;
                    
                case 16:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetShortAtPixel(ShortValueAtPixel(k) + static_cast<int16_t>(val),k);
                    break;
                    
                case 20:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetUShortAtPixel(UShortValueAtPixel(k) + static_cast<uint16_t>(val),k);
                    break;
                    
                case 32:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetLongAtPixel(LongValueAtPixel(k) + static_cast<int32_t>(val),k);
                    break;
                    
                case 40:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetULongAtPixel(ULongValueAtPixel(k) + static_cast<uint32_t>(val),k);
                    break;
                    
                case 64:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetLongLongAtPixel(LongLongValueAtPixel(k) + static_cast<int64_t>(val),k);
                    break;
                    
                case -32:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetFloatAtPixel(FloatValueAtPixel(k) + static_cast<float>(val),k);
                    break;
                    
                case -64:
                    for(size_t k =0; k < mask.size(); k++)
                        if(!mask[k])SetDoubleAtPixel(DoubleValueAtPixel(k) + static_cast<double>(val),k);
                    break;
                    
                default:
#if __cplusplus < 201103L
                    throw FITSexception(BAD_BITPIX,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(static_cast<long long>(BITPIX))+" IS UNKNOWN.");
#else
                    throw FITSexception(BAD_BITPIX,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN.");
#endif
            }
        }
        
        template <typename T>
        void operator-=(const T& val)
        {
            operator+=(-1*val);
        }
        
    
#pragma mark • Altere data
        virtual void SetUByteAtPixel      (uint8_t, size_t){};
        virtual void SetByteAtPixel       (int8_t, size_t){};
        virtual void SetUShortAtPixel     (uint16_t, size_t){};
        virtual void SetShortAtPixel      (int16_t, size_t){};
        virtual void SetIntAtPixel        (int, size_t){};
        virtual void SetUIntAtPixel       (unsigned int, size_t){};
        virtual void SetLongAtPixel       (int32_t, size_t){};
        virtual void SetULongAtPixel      (uint32_t, size_t){};
        virtual void SetLongLongAtPixel   (int64_t, size_t){};
        virtual void SetFloatAtPixel      (float, size_t){};
        virtual void SetDoubleAtPixel     (double, size_t){};

        
        virtual void SetUByteAtPixel      (uint8_t, unsigned long long, ...){};
        virtual void SetByteAtPixel       (int8_t, unsigned long long, ...){};
        virtual void SetUShortAtPixel     (uint16_t, unsigned long long, ...){};
        virtual void SetShortAtPixel      (int16_t, unsigned long long, ...){};
        virtual void SetIntAtPixel        (int, unsigned long long, ...){};
        virtual void SetUIntAtPixel       (unsigned int, unsigned long long, ...){};
        virtual void SetLongAtPixel       (int32_t, unsigned long long, ...){};
        virtual void SetULongAtPixel      (uint32_t, unsigned long long, ...){};
        virtual void SetLongLongAtPixel   (int64_t, unsigned long long, ...){};
        virtual void SetFloatAtPixel      (float, unsigned long long, ...){};
        virtual void SetDoubleAtPixel     (double, unsigned long long, ...){};
        
        void MaskPixel(unsigned int long long, ...);
        void MaskPixel(const std::valarray<bool>);
        
        void UnmaskPixel(unsigned int long long, ...);
        void UnmaskPixel(const std::valarray<bool>);
        
        bool Masked(unsigned int long long, ...) const;
        bool Masked(size_t) const;
        
        
        
#pragma mark • Pure virtual methods
        
        virtual FITScube *Layer(unsigned int) const  {return NULL;}
        virtual FITScube *Window (size_t, size_t, size_t, size_t) const {return const_cast<FITScube *>(this);}
        virtual FITScube *Rebin  (std::vector<size_t> ) const {return const_cast<FITScube *>(this);}
        
        virtual void Print() const {};
        
#ifdef _HAS_Qt4_
#pragma mark – Qt display capability
        virtual QCustomPlot* QtDisplay(std::string OPTION = "") const { return NULL;}
        virtual QCustomPlot* QtDisplay(unsigned int iMin, unsigned int iMax, std::string OPTION="") const { return NULL;}
        virtual QCustomPlot* QtDisplayLayer(unsigned int iLayer, std::string OPTION="") const {return NULL;}
        
        virtual void Overlay(QCustomPlot*, std::string OPTION = "") const { return;}
        virtual void Overlay(QCustomPlot*, unsigned int iMin, unsigned int iMax, std::string OPTION="") const { return;}
        
        
#endif

#ifdef _HAS_ROOT_
#pragma mark – ROOT export capability
        virtual void PixelToTH1(TH1&, unsigned int) const {};
        virtual void PixelToTH2(TH2&) const {};
        virtual void PixelToTH3(TH3&) const {};
        virtual void PixelToTree(TTree&)  const {};
        virtual void PixelDistribution(TH1&) const {};
#endif
        
    };
    
#pragma mark - FITSimg class definition
    template< typename T >
    class FITSimg : public FITScube
    {
        /**
         * @class DSL::FITSimg FITSimg.h "fitsExtractor/FITSimg.h"
         * @author GILLARD William
         * @version 1.0
         * @date 31/03/2015
         * @brief Template class to manage ND FITS image
         * @details Template class representing fits images of N dimensions. This class inherits from FITScube class. It stores data from FITS datacube into an array of \f$\pod_{k=1}^{N}n_k\f$ dimensions, with \f$N\f$ the total number of axis and \f$n_k\f$ the number of pixel of the axis \f$k\f$. The array is filled following FITS standard, starting from axis \f$N\f$ to axis \f$1\f$. Data are of the same base type as the FITS datacube.
         */
    private:
        std::valarray<double> data;          //!< Image data
        
        double BSCALE;                       //!< Data scalling factor
        double BZERO;                        //!< Data offset
        uint16_t BLANK;                      //!< Transparent pixel reference value

#pragma mark • Initialization
        virtual void img_init();
        void template_init();
        
#pragma mark • I/O
        void WriteDataCube(const std::shared_ptr<fitsfile>& );  //!< Write data to Fits HDU
        template<typename S> void WriteData(const std::shared_ptr<fitsfile>& , int);
    
    public:
#pragma mark • ctor/dtor
        
        FITSimg(const std::shared_ptr<fitsfile>& fptr  );      //!< Default constructor
        FITSimg(const FITSimg<T>&);     //!< Copy constructor
        FITSimg(unsigned int, size_t, ...);
        FITSimg(std::vector<size_t>);
        virtual ~FITSimg();             //!< Destructor
        
#pragma mark • Modifier
        void Bscale(double _bs );
        void Bzero (double _b0);
        void Blank (uint16_t nanVal);
        
        void AddLayer(const FITSimg<T>&);

#pragma mark • data operation
        
    private:
        template< typename S > void ReadArray(const std::shared_ptr<fitsfile>& fptr); //!< Read and interpret FITS BINARY data
        
    public:
        
        FITSimg<T>& operator=(const FITSimg<T>&);		///< Get pixel content
        
        const double operator [](const unsigned long long) const;  ///< Get pixel content
              double operator [](const unsigned long long);	      ///< Get pixel content
        
        virtual uint8_t      UByteValueAtPixel(size_t)             const;
        virtual uint8_t      UByteValueAtPixel(unsigned long long) const;
        virtual int8_t       ByteValueAtPixel(size_t)             const;
        virtual int8_t       ByteValueAtPixel(unsigned long long) const;
        virtual uint16_t     UShortValueAtPixel(size_t)             const;
        virtual uint16_t     UShortValueAtPixel(unsigned long long) const;
        virtual int16_t      ShortValueAtPixel(size_t)             const;
        virtual int16_t      ShortValueAtPixel(unsigned long long) const;
        virtual unsigned int UIntValueAtPixel(size_t)             const;
        virtual unsigned int UIntValueAtPixel(unsigned long long) const;
        virtual int          IntValueAtPixel(size_t)             const;
        virtual int          IntValueAtPixel(unsigned long long) const;
        virtual uint32_t     ULongValueAtPixel(size_t)             const;
        virtual uint32_t     ULongValueAtPixel(unsigned long long) const;
        virtual int32_t      LongValueAtPixel(size_t)             const;
        virtual int32_t      LongValueAtPixel(unsigned long long) const;
        virtual int64_t      LongLongValueAtPixel(size_t)             const;
        virtual int64_t      LongLongValueAtPixel(unsigned long long) const;
        virtual float        FloatValueAtPixel( size_t )           const;
        virtual float        FloatValueAtPixel(unsigned long long) const;
        virtual double       DoubleValueAtPixel( size_t )           const;
        virtual double       DoubleValueAtPixel(unsigned long long) const;
        
        void operator*= (const T&);///< Scale up data
        void operator/= (const T&);///< Scale down data
        void operator+= (const T&);///< Apply positive offset to data
        void operator-= (const T&);///< Apply negative offset to data
       
        void operator*= (const std::valarray<T>&);///< Scale up data
        void operator/= (const std::valarray<T>&);///< Scale down data
        void operator+= (const std::valarray<T>&);///< Apply positive offset to data
        void operator-= (const std::valarray<T>&);///< Apply negative offset to data
        
        template<typename D>
        void operator+= (const D&);
        
        void operator*= (const FITSimg<T>&);///< Multiply images
        void operator/= (const FITSimg<T>&);///< Divide images
        void operator+= (const FITSimg<T>&);///< Add images
        void operator-= (const FITSimg<T>&);///< substract images
        
        void SetPixelValue(T, std::vector<unsigned long long>);
        void SetPixelValue(T, unsigned long long);
        
        void SetUByteAtPixel      (uint8_t, size_t);
        void SetByteAtPixel       (int8_t, size_t);
        void SetUShortAtPixel     (uint16_t, size_t);
        void SetShortAtPixel      (int16_t, size_t);
        void SetIntAtPixel        (int, size_t);
        void SetUIntAtPixel       (unsigned int, size_t);
        void SetLongAtPixel       (int32_t, size_t);
        void SetULongAtPixel      (uint32_t, size_t);
        void SetLongLongAtPixel   (int64_t, size_t);
        void SetFloatAtPixel      (float, size_t);
        void SetDoubleAtPixel     (double, size_t);
        
        void SetUByteAtPixel      (uint8_t, unsigned long long, ...);
        void SetByteAtPixel       (int8_t, unsigned long long, ...);
        void SetUShortAtPixel     (uint16_t, unsigned long long, ...);
        void SetShortAtPixel      (int16_t, unsigned long long, ...);
        void SetIntAtPixel        (int, unsigned long long, ...);
        void SetUIntAtPixel       (unsigned int, unsigned long long, ...);
        void SetLongAtPixel       (int32_t, unsigned long long, ...);
        void SetULongAtPixel      (uint32_t, unsigned long long, ...);
        void SetLongLongAtPixel   (int64_t, unsigned long long, ...);
        void SetFloatAtPixel      (float, unsigned long long, ...);
        void SetDoubleAtPixel     (double, unsigned long long, ...);
        
        void MaskPixel            (unsigned long long, ...);
        
#pragma mark • Accessor
 
        double GetSum()                     const;
        double GetMean()                    const;
        double GetQuadraticMean()           const;
        double GetVariance()                const;
        double GetMedian(bool fast=false)   const;
        double GetMinimum()                 const;
        double GetMaximum()                 const;
        double Getpercentil(double, bool fast=false) const;
        double Get5thpercentil()            const;
        double Get25thpercentil()           const;
        double Get75thpercentil()           const;
        double Get95thpercentil()           const;
        
        const double* GetDataCube() const {return &data[0];}
        const std::valarray<double>& GetData() const {return data;}
        
#pragma mark • Display methods

#pragma mark • Data export methods
        
#pragma mark • Extraction method
        FITScube *Layer(unsigned int) const;
        FITScube *Window (size_t, size_t, size_t, size_t) const;
        FITScube *Rebin  (std::vector<size_t> ) const;
        
        void Resize(size_t, size_t, size_t, size_t);
        
        void Print()const;
        
    };
    
#pragma mark - FITSimg class implementation
    
#pragma mark • Initialization
    
    /**
     * @details Initialize type dependent FITS variable to decent default value.
     */
    //template< typename T >
    //void FITSimg<T>::template_init()
    //{}
    
    /**
     * @details Initialize private variable to decent default value.
     */
    template< typename T >
    void FITSimg<T>::img_init()
    {
        data = std::valarray<double>(mask.size());
        
#if __cplusplus < 201103L
        data.resize(mask.size(), 0);
#endif
        
        BSCALE =  1.;
        BZERO  =  0.;
        BLANK  = std::numeric_limits<uint16_t>::quiet_NaN();
    }
    
#pragma mark • I/O
    
    /**
     *  @details Write, or update data, to the current HDU images
     *  @param fptr : Fits file header where data will be written.
     */
    template< typename T>
    void FITSimg<T>::WriteDataCube(const std::shared_ptr<fitsfile>&  fptr)
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw std::invalid_argument("\033[31m[FITSimg::WriteDataCube]\033[0mreceived nullptr");
            return;
        }
        
        switch (BITPIX)
        {
            case 8:
                WriteData<uint8_t>(fptr, TBYTE);
                break;
                
            case 16:
                WriteData<int16_t>(fptr, TSHORT);
                break;
                
            case 32:
                WriteData<int32_t>(fptr, TLONG);
                break;
                
            case 64:
                WriteData<uint64_t>(fptr, TLONGLONG);
                break;
                
            case -32:
                WriteData<float>(fptr, TFLOAT);
                break;
                
            case -64:
                WriteData<double>(fptr, TDOUBLE);
                break;
                
            default:
                throw FITSexception(BAD_BITPIX,"FITSimg","WriteDataCube","invalid BITPIX");
                return;
        }
        
        return;
    }
    
    template<typename T>
    template<typename S>
    void FITSimg<T>::WriteData(const std::shared_ptr<fitsfile>& fptr, int DATA_TYPE)
    {
        if (!fptr || fptr.use_count() < 1)
        {
            throw std::invalid_argument("\033[31m[FITSimg::WriteDataCube]\033[0mreceived nullptr");
        }

        if (mask.sum() > 0 && BLANK != std::numeric_limits<T>::quiet_NaN())
        {
            hdu->valueForKey("BLANK", BLANK);
        }
        
        const long long    num_axis = static_cast<const long long>( Naxis.size() );
        const long long array_size  = static_cast<const long long>( Nelements() );
    
        long long *fpixel;
        fpixel     = new long long[num_axis];
        for(long long i=0; i < num_axis; i++)
            fpixel[i]    = 1;
        
        S *array;
        array      = new S[array_size];
        for(long long i=0; i < array_size; i++)
            array[i]      = static_cast<S>( (mask[i]) ? ((BLANK != std::numeric_limits<T>::quiet_NaN())?BLANK:std::numeric_limits<S>::min()): (data[i] - BZERO)/BSCALE );
        
        std::cout<<"\033[31m[FITSimg::WriteDataCube]\033[0m"<<std::endl
                 <<"        \033[31m|- BITPIX       : \033[0m"<<BITPIX<<std::endl
                 <<"        \033[31m|- EQV BITPIX   : \033[0m"<<eqBITPIX<<std::endl
                 <<"        \033[31m|- DATA_TYPE    : \033[0m"<<DATA_TYPE<<std::endl
                 <<"        \033[31m|- BLANK        : \033[0m"<<BLANK<<std::endl
                 <<"        \033[31m`- Number of pix: \033[0m"<<Nelements()<<std::endl;
        for(size_t i = 0; i < Naxis.size()-1; i++)
            std::cout<<"             \033[34m|- Axis "<<i<<"    : \033[0m"<<Naxis[i]<<std::endl;
        std::cout<<"             \033[34m`- Axis "<<Naxis.size()-1<<"    : \033[0m"<<Naxis[Naxis.size()-1]<<std::endl;
        
        
        if( fits_write_pixll(fptr.get(), DATA_TYPE, fpixel, array_size, array, &img_status) )
        {
            throw FITSexception(img_status,"FITSimg","WriteDataCube");
        }
        
        delete [] fpixel;
        delete [] array;
        
    }

#pragma mark • ctor/dtor
    /**
     *  @details Read current HDU of the fitsfile to extract a 2D images
     *  @param fptr: Pointer to the fitfile
     */
    template< typename T >
    FITSimg<T>::FITSimg(const std::shared_ptr<fitsfile>& fptr): FITScube(fptr)
    {
        img_init();
        
        if(img_status)
            return;
        
        //  • GET BSCALE
        double tmp_bscale = hdu->GetDoubleValueForKey("BSCALE");
        if(tmp_bscale != tmp_bscale)
            tmp_bscale = 1.;
        BSCALE = tmp_bscale;
        
        //  • GET BZERO
        double tmp_bzero = hdu->GetDoubleValueForKey("BZERO");
        if(tmp_bzero != tmp_bzero)
            tmp_bzero = 0.;
        BZERO = tmp_bzero;
        
        //  • GET BLANCK
        uint16_t tmp_blank = hdu->GetUInt16ValueForKey("BLANK");
        if( tmp_blank == std::numeric_limits<uint16_t>::quiet_NaN() )
            tmp_blank = -1.*std::numeric_limits<uint16_t>::max();
        else
            BLANK = tmp_blank; std::cout<<"BLANK : "<<BLANK<<std::endl;
        
        // • GET THE WHOLE IMAGE DATA
        switch (BITPIX)
        {
            case 8:
                ReadArray<uint8_t>(fptr);
                break;
                
            case 16:
                ReadArray<uint16_t>(fptr);
                break;
                
            case 32:
                ReadArray<uint32_t>(fptr);
                break;
                
            case 64:
                ReadArray<uint64_t>(fptr);
                break;
                
            case -32:
                ReadArray<float>(fptr);
                break;
                
            case -64:
                ReadArray<double>(fptr);
                break;
                
            default:
#if __cplusplus < 201103L
                throw FITSexception(BAD_BITPIX,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(static_cast<long long>(BITPIX))+" IS UNKNOWN.");
#else
                throw FITSexception(BAD_BITPIX,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN.");
#endif
        }
    }
    
    /**GetValueForKey
     *  @details Copy memory content of an input FITSimg to this
     *  @param img: FITS image to copie
     */
    template< typename T >
    FITSimg<T>::FITSimg(const FITSimg<T>& img):FITScube(img),
                                               BSCALE(img.BSCALE),
                                               BZERO(img.BZERO),
                                               BLANK(img.BLANK)
    {
        data = std::valarray<double>(img.data.size());

#if __cplusplus < 201103L
        data.resize(img.data.size());
        mask.resize(img.mask.size(),0);
        mask |= img.mask;
#endif
        
        for(unsigned int i = 0; i < img.data.size(); i++)
            data[i] = img.data[i];
        
        hdu->valueForKey("BSCALE",BSCALE,"");
        hdu->valueForKey("BZERO" ,BZERO  ,""  );
        hdu->valueForKey("BLANK" ,BLANK  ,""  );
        hdu->valueForKey("BITPIX",BITPIX,"");

    }
    
    /**
     *  @details Create new empty image of finite dimension.
     *  @param _naxis: Number of dimenssion of the new image
     *  @param _iaxis: Number of pixel in the \f$i^{\rm ieme}\f$ dimension
     */
    template< typename T>
    FITSimg<T>::FITSimg(unsigned int _naxis, size_t _iaxis, ...) : FITScube()
    {
        
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
	for(size_t i = 0; i < axis.size(); i++)
	  Naxis.push_back(axis[i]);

        mask = pxMask(static_cast<size_t>( Nelements() ));

#if __cplusplus < 201103L
        mask.resize(static_cast<size_t>( Nelements() ),0);
#endif

        img_init();

        hdu = new FITShdu();
        hdu->valueForKey("NAXIS",static_cast<int>(axis.size() ));
        for(size_t i = 0; i < Naxis.size(); i++)
	  {
#if __cplusplus >= 201103L
            hdu->valueForKey("NAXIS"+std::to_string(i+1),Naxis[i]);
#else
            hdu->valueForKey("NAXIS"+std::to_string(static_cast<long long int>( i+1 )),Naxis[i]);
#endif
	  }
        
        template_init();
        
    }
    
    /**
     *  @details Create new empty image of finite dimension.
     *  @param _axis: STL vector tat list the number of pixel of the image in each of its dimenssion
     */
    template< typename T>
    FITSimg<T>::FITSimg(std::vector<size_t> _axis) : FITScube()
    {
        for(size_t i = 0; i < _axis.size(); i++)
            Naxis.push_back(_axis[i]);
        
        mask = pxMask(static_cast<size_t>( Nelements() ));

#if __cplusplus < 201103L
        mask.resize(static_cast<size_t>( Nelements() ),0);
#endif
        
        img_init();
        
        hdu = new FITShdu();
        hdu->valueForKey("NAXIS",static_cast<int>(_axis.size() ));
        for(size_t i = 0; i < Naxis.size(); i++)
        {
#if __cplusplus >= 201103L
            hdu->valueForKey("NAXIS"+std::to_string(i+1),Naxis[i]);
#else
            hdu->valueForKey("NAXIS"+std::to_string(static_cast<long long int>( i+1 )),Naxis[i]);
#endif
        }
        
        template_init();
    }
    
    /**
     *  @details Free memory
     */
    template< typename T >
    FITSimg<T>::~FITSimg()
    {
        data.resize(0);
    }
    
#pragma mark • modifier
    template< typename T >
    void FITSimg<T>::Bscale(double _bs )
    {
        BSCALE = _bs;
        hdu->valueForKey("BSCALE",BSCALE);
    }
    
    template< typename T >
    void FITSimg<T>::Bzero (double _b0)
    {
        BZERO = _b0;
        hdu->valueForKey("BZERO",BZERO);
    }
    
    template< typename T >
    void FITSimg<T>::Blank(uint16_t nanVal)
    {
        BLANK = nanVal;
        hdu->valueForKey("BLANK",BLANK);
    }
    
    template< typename T >
    void FITSimg<T>::AddLayer(const FITSimg<T>& iLayer)
    {

        try
        {
            if(iLayer.Naxis.size() < 2 ||
               iLayer.Naxis.size() > 3)
                throw FITSwarning("FITSimg<T>","AddLayer","Can't add 1D plot or +4D data-cube to a 2D images");
        
            // Check the two images have the same X and Y dimension
            if((Naxis[0] != iLayer.Naxis[0]) ||
               (Naxis[1] != iLayer.Naxis[1]) )
                throw FITSwarning("FITSimg<T>","AddLayer","The two images didn't have same x and y dimensions");
        
            // Check the data are of the same type
            if(BITPIX != iLayer.BITPIX)
               throw FITSwarning("FITSimg<T>","AddLayer","The two images have't the same data type");
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        // Increment the number of dimenssion to account for the new Layer
        if(Naxis.size() < 3)
        {
            Naxis.push_back(1);
            hdu->valueForKey("NAXIS",Naxis.size());
        }
        size_t nLayer = Naxis[2];
        
        if(iLayer.Naxis.size() == 3)
        {
            Naxis[2] += iLayer.Naxis[2];
        }
        else
        {
            Naxis[2] ++;
        }
        
        hdu->valueForKey("NAXIS3",Naxis[2]);
        
        // Copy data array to avoid lost any data.
        std::valarray<double> cpy_data = std::valarray<double>( data.size() );
#if __cplusplus < 201103L
        cpy_data.resize(data.size());
        mask.resize(img.mask.size(),0);
        mask |= img.mask;
#endif
        cpy_data += data;
        
        
        pxMask cpy_mask = pxMask( mask.size() );
#if __cplusplus < 201103L
        cpy_mask.resize(mask.size());
#endif
        cpy_mask |= mask;
        
        
        size_t data_size = data.size();
        size_t other_size= iLayer.data.size();
        
        data.resize(data_size+other_size);
        mask.resize(data_size+other_size);
        
        if(nLayer > 1)
        {
            data[std::gslice(0,{nLayer,Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] += cpy_data;
            mask[std::gslice(0,{nLayer,Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] |= cpy_mask;
        }
        else
        {
            data[std::gslice(0,{Naxis[1],Naxis[0]},{Naxis[0],1})] += cpy_data;
            mask[std::gslice(0,{Naxis[1],Naxis[0]},{Naxis[0],1})] |= cpy_mask;
        }
        
        if(iLayer.Naxis.size() > 2)
        {
            data[std::gslice(nLayer*Naxis[1]*Naxis[0],{iLayer.Naxis[2],Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] += iLayer.data;
            mask[std::gslice(nLayer*Naxis[1]*Naxis[0],{iLayer.Naxis[2],Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] |= iLayer.mask;
        }
        else
        {
            data[std::gslice(nLayer*Naxis[1]*Naxis[0],{Naxis[1],Naxis[0]},{Naxis[0],1})] += iLayer.data;
            mask[std::gslice(nLayer*Naxis[1]*Naxis[0],{Naxis[1],Naxis[0]},{Naxis[0],1})] |= iLayer.mask;
        }
        
        cpy_mask.resize(0);
        cpy_data.resize(0);
        
        
    }
    
#pragma mark • data operation
    
    /**
     *  @details Read the RAW data from the FITS file and extract the pixel content
     *  @param fptr; Input fitsfile
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::ReadArray(const std::shared_ptr<fitsfile>& fptr)
    {
        
        if(fptr == nullptr || fptr.use_count() < 1)
            return;
            
        // • GET THE WHOLE IMAGE DATA
        //    - GET PIXEL DIMENSION
        
        if(FITScube::debug)
            hdu->Dump(std::cout);
        
        const long      num_axis    = static_cast<const long>( Naxis.size() );
        const LONGLONG array_size  = static_cast<const long long>( Nelements() );
        
        //    - ALLOCATE BUFFER MEMORY
        int any_null = 0;
        img_status   = 0;
        
        LONGLONG *fpixel;
        fpixel     = new LONGLONG[num_axis];
        for(long i=0; i < num_axis; i++)
            fpixel[i]    = 1;
        
        S *array;
        array      = new S[array_size];
        for(long long i=0; i < array_size; i++)
            array[i]      = 0;
        
        char *null_array;
        null_array = new char[array_size];
        for(int i=0; i < array_size; i++)
            null_array[i] = 0;
        
        int TTYPE = 0;
        switch (eqBITPIX)
        {
            case 8:
                TTYPE = TBYTE;
                break;
            
            case 12:
                TTYPE = TSBYTE;
                break;

            case 16:
                TTYPE = TSHORT;
                break;
                
            case 20:
                TTYPE = TUSHORT;
                break;
                
            case 30:
                TTYPE = TUINT;
                break;
                
            case 31:
                TTYPE = TINT;
                break;
                
            case 32:
                TTYPE = TLONG;
                break;
                
            case 40:
                TTYPE = TULONG;
                
            case 64:
                TTYPE = TLONGLONG;
                break;
                
            case -32:
                TTYPE = TFLOAT;
                break;
                
            case -64:
                TTYPE = TDOUBLE;
                break;
                
            default:
                img_status=BAD_BITPIX;
#if __cplusplus < 201103L
                throw FITSexception(img_status,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(static_cast<long long>(BITPIX))+" IS UNKNOWN.");
#else
                throw FITSexception(img_status,"FITSimg","operator=+()","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN.");
#endif
        }
        
        //    - EXTRACT BINARY DATA AND CONVERT TO NUMERICAL VALUE
        
        try
        {
            if( fits_read_pixnullll(fptr.get(), TTYPE, fpixel, array_size, array, null_array, &any_null, &img_status  ) )
                throw FITSexception(img_status,"FITSimg<T>","ReadArray");
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            switch (img_status)
            {
                case 410:
                    std::cerr<<"    |- IMG DATA TYPE : "<<TTYPE<<std::endl
                             <<"    `- this DATA TYPE: "<<BITPIX;
                    throw;
                    break;
                    
                case 854:
                    std::cerr<<"    |- IMG PIXEL INDEX IS OUT OF RANGE"<<std::endl;
                    throw;
                    break;
               
                default:
                    break;
            }
            
            std::cerr<<"    |- IMG DATA TYPE : "<<TTYPE<<std::endl
                     <<"    |- BITPIX        : "<<BITPIX<<std::endl
                     <<"    |- EQUIV. BITPIX : "<<eqBITPIX<<std::endl
                     <<"    |- NAXIS         : "<<num_axis<<std::endl;
            for(size_t i=0; i < Naxis.size()-1; i++)
            std::cerr<<"    |    |- NAXIS["<<i<<"] : "<<Naxis[i]<<std::endl;
            std::cerr<<"    |    `- NAXIS["<<Naxis.size()-1<<"] : "<<Naxis[Naxis.size()-1]<<std::endl
                     <<"    |- START COO     : "<<std::endl;
            for(unsigned int i=0; i < num_axis-1; i++)
            std::cerr<<"    |   |- NAXIS"<<i<<"[0] : "<<fpixel[i]<<std::endl;
            std::cerr<<"    |   `- NAXIS"<<num_axis-1<<"[0] : "<<fpixel[num_axis-1]<<std::endl
                     <<"    |- DATA SIZE     : "<<array_size<<std::endl
                     <<"    |- ARRAY[0]      : "<<array[0]<<std::endl
                     <<"    |- null_array[0] : "<<null_array[0]<<std::endl
                     <<"    `- HAS NULL      : "<<any_null;
            
            //for(LONGLONG n = 0; n < array_size; n++)
            //    std::cerr<<"["<<n<<"]    "<<array[n]<<std::endl;
            
            std::cerr<<"\033[0m"<<std::endl;
            //return;
        }
        
        //std::cout<<"==============="<<std::endl;
        
        for(unsigned int i = 0; i < array_size; i++)
        {
            if(BLANK != std::numeric_limits<uint16_t>::quiet_NaN() &&
               (array[i]  <= static_cast<S>( BLANK ) || null_array[i]) )
            {
                std::cout<<"MASK PIXEL "<<array[i]<<" % "<<BLANK<<std::endl;
                if(mask.size() > i)
                    mask[i] = true;
            }
            
            data[i] = static_cast<double>(array[i]) ;
            
            
            //std::cout<<PixelCoordinates(i)[0]<<"   "<<PixelCoordinates(i)[1]<<"   "<<data[i]<<"   "<<mask[i]<<"   "<<DoubleValueAtPixel(static_cast<size_t>(i))<<std::endl;
            
        }
        
        //std::cout<<"+++++++++++++++++"<<std::endl;
        
        delete [] fpixel;
        delete [] null_array;
        delete [] array;
        
        return;
    }
    
    /**
     *  @brief Assignement opperator
     *  @details Assign memory content of img to this
     *  @param img: FITS image to be copyed
     *
     *  @return this = img
     */
    template< typename T >
    FITSimg<T>& FITSimg<T>::operator=(const FITSimg<T>& img)
    {
        if(this == &img)
            return *this;
        
	if(hdu != NULL)
	  delete hdu;
	
	hdu = new FITShdu(img.pHDU());
	
        name.clear();
        Naxis.clear();
            
#if __cplusplus >= 201103L
        data.resize(0);
        mask.resize(0);
#endif
      
        name = img.name;
        
        data = std::valarray<double>( img.data.size());
        mask = std::valarray<bool>  ( img.mask.size());
        
#if __cplusplus < 201103L
        data.resize(img.data.size());
        mask.resize(img.data.size());
#endif
        
        hdu = new FITShdu(img.pHDU());
        
        for(size_t i = 0; i < img.data.size(); i++)
            data[i] = img.data[i];
        
        for(size_t i = 0; i < img.Naxis.size(); i++)
            Naxis.push_back(img.Naxis[i]);
        
        
        for(size_t i = 0; i < img.mask.size(); i++)
            mask[i] = img.mask[i];
        
        return *this;
    }
    
    template< typename T >
    uint8_t FITSimg<T>::UByteValueAtPixel(size_t iPx) const
    {
        return UByteValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    uint8_t FITSimg<T>::UByteValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<uint8_t>(this->operator[](iPx));
    }
    
    template< typename T >
    int8_t FITSimg<T>::ByteValueAtPixel(size_t iPx) const
    {
        return ByteValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    int8_t FITSimg<T>::ByteValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<int8_t>(this->operator[](iPx));
    }
    
    template< typename T >
    uint16_t FITSimg<T>::UShortValueAtPixel(size_t iPx) const
    {
        return UShortValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    uint16_t FITSimg<T>::UShortValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<uint16_t>(this->operator[](iPx));
    }
    
    template< typename T >
    int16_t FITSimg<T>::ShortValueAtPixel(size_t iPx) const
    {
        return ShortValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    int16_t FITSimg<T>::ShortValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<int16_t>(this->operator[](iPx));
    }

    template< typename T >
    unsigned int FITSimg<T>::UIntValueAtPixel(size_t iPx) const
    {
        return UIntValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    unsigned int FITSimg<T>::UIntValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<int>(this->operator[](iPx));
    }
    
    template< typename T >
    int FITSimg<T>::IntValueAtPixel(size_t iPx) const
    {
        return IntValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    int FITSimg<T>::IntValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<int>(this->operator[](iPx));
    }
    
    
    template< typename T >
    uint32_t FITSimg<T>::ULongValueAtPixel(size_t iPx) const
    {
        return ULongValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    uint32_t FITSimg<T>::ULongValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<uint32_t>(this->operator[](iPx));
    }
    
    template< typename T >
    int32_t FITSimg<T>::LongValueAtPixel(size_t iPx) const
    {
        return LongValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    int32_t FITSimg<T>::LongValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<uint32_t>(this->operator[](iPx));
    }
    
    template< typename T >
    int64_t FITSimg<T>::LongLongValueAtPixel(size_t iPx) const
    {
        return LongLongValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    int64_t FITSimg<T>::LongLongValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<uint64_t>(this->operator[](iPx));
    }
    
    template< typename T >
    float FITSimg<T>::FloatValueAtPixel(size_t iPx) const
    {
        return FloatValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    float FITSimg<T>::FloatValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<float>(this->operator[](iPx));
    }

    
    template< typename T >
    double FITSimg<T>::DoubleValueAtPixel(size_t iPx) const
    {
        return DoubleValueAtPixel(static_cast<unsigned long long>( iPx ));
    }
    
    template< typename T >
    double FITSimg<T>::DoubleValueAtPixel(unsigned long long iPx) const
    {
        return static_cast<double>(this->operator[](iPx));
        //return this->operator[](iPx);
    }
    
    /**
     *  @details Optain the physical value of a pixel in the image.
     *  @param iPx: pixel of the image. Pixel is numbered for Naxis[0] to Naxis[n].
     *  @return Content of the pixel or 0 if the pixel is masked.
     */
    template< typename T >
    const double FITSimg<T>::operator [](const unsigned long long iPx) const
    {
        return ( mask[iPx])? 0. : data[iPx];
    }
    
    /**
     *  @details Optain the physical value of a pixel in the image.
     *  @param iPx: pixel of the image. Pixel is numbered for Naxis[0] to Naxis[n].
     *  @return Content of the pixel or 0 if the pixel is masked.
     */
    template< typename T >
    double FITSimg<T>::operator [](const unsigned long long iPx)
    {
        return ( mask[iPx])? 0. : data[iPx];
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator*=(const T& val)
    {

#if __cplusplus >= 201103L
      size_t k = 0;
      for (auto it = begin(data); it!=end(data); ++it)
	{
	  if(!mask[k]) (*it)*=static_cast<double>(val);
	  k++;
	}
#else
        for(size_t k = 0; k < data.size(); k++)
            if(!mask[k])
	      data[k] = data[k]*static_cast<double>(val);
#endif
        //data[!mask] *= val;
    }
    
    /**
     *  @details Scale down the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator/= (const T& val)
    {
#if __cplusplus >= 201103L
      size_t k = 0;
      for (auto it = begin(data); it!=end(data); ++it)
	{
	  if(!mask[k]) (*it)/=static_cast<double>(val);
	  k++;
        }
#else
        for(size_t k = 0; k < data.size(); k++)
            if(!mask[k])
                data[k] = data[k]/static_cast<double>(val);
#endif
        //data[!mask] /= val;
    }
    
    
    /**
     *  @details Offset uppward the whole image by a value of val.
     *  @param val: upward offset.
     *  @note masked pixel aren't affected by the offset.
     */
    template< typename T >
    void FITSimg<T>::operator+= (const T& val)
    {
#if __cplusplus >= 201103L
      size_t k = 0;
      for (auto it = begin(data); it!=end(data); ++it)
	{
	  if(!mask[k]) (*it)+=static_cast<double>(val);
	  k++;
        }
#else
        for(size_t k = 0; k < data.size(); k++)
            if(!mask[k])
                data[k] = data[k] + static_cast<double>(val);
#endif
    }
    
    
    /**
     *  @details Offset downward the whole image by a value of val.
     *  @param val: downward offset.
     *  @note masked pixel aren't affected by the offset.
     */
    template< typename T >
    void FITSimg<T>::operator-= (const T& val)
    {
#if __cplusplus >= 201103L
      size_t k = 0;
      for (auto it = begin(data); it!=end(data); ++it)
	{
	  if(!mask[k]) (*it)-=static_cast<double>(val);
	  k++;
        }
#else
        for(size_t k = 0; k < data.size(); k++)
            if(!mask[k])
                data[k] = data[k] - static_cast<double>(val);
#endif
    }
    
    /**
     *  @details Offset uppward the whole image by a value of val.
     *  @param val: upward offset.
     *  @note masked pixel aren't affected by the offset.
     */
    template< typename T >
    template< typename D >
    void FITSimg<T>::operator+= (const D& val)
    {
        operator+= ( static_cast<double>( val ) );
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator*=(const std::valarray<T>& val)
    {
        if(data.size() != val.size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator*=(const std::valarray<T>& val)","val must have same dimension as this");

	for(size_t k = 0; k < data.size(); k++)
	  if(!mask[k])
	    data[k] *= static_cast<double>(val[k]);
	//        data[!mask] *= val[!mask];
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator/=(const std::valarray<T>& val)
    {
        if(data.size() != val.size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator/=(const std::valarray<T>& val)","val must have same dimension as this");
        
	for(size_t k = 0; k < data.size(); k++)
	  if(!mask[k])
	    data[k] /= static_cast<double>(val[k]);
	//        data[!mask] /= val[!mask];
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator+=(const std::valarray<T>& val)
    {
        if(data.size() != val.size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator+=(const std::valarray<T>& val)","val must have same dimension as this");
        
	for(size_t k = 0; k < data.size(); k++)
	  if(!mask[k])
	    data[k] += static_cast<double>(val[k]);
	//        data[!mask] += val[!mask];
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    void FITSimg<T>::operator-=(const std::valarray<T>& val)
    {
        if(data.size() != val.size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator-=(const std::valarray<T>& val)","val must have same dimension as this");
        
	
	for(size_t k = 0; k < data.size(); k++)
	  if(!mask[k])
	    data[k] -= static_cast<double>(val[k]);
	//        data[!mask] -= val[!mask];
    }
    
                                
    
    /**
     *  @details Multiply, pixel by pixel, two images.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator*= (const FITSimg<T>& img)
    {
        data *= img.data;
        mask |= img.mask;
    }
    
    /**
     *  @details Divide, pixel by pixel, two images.
     *  @param img: Image to divide to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator/= (const FITSimg<T>& img)
    {
        data /= img.data;
        mask |= img.mask;
    }
    
    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator+= (const FITSimg<T>& img)
    {
        data += img.data;
        mask |= img.mask;
    }
    
    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator-= (const FITSimg<T>& img)
    {
        data -= img.data;
        mask |= img.mask;
    }
    
    template< typename T >
    void FITSimg<T>::SetPixelValue(T val, std::vector<unsigned long long> iPx)
    {
        
        img_status = 0;
        size_t index = PixelIndex(iPx);
        
        if(index >= Nelements())
            return;
        
        unsigned long long sum = 0;
        
        for(unsigned int i = 0 ; i < iPx.size(); i++)
            sum+= iPx[i];
        
        try
        {
            if(index == 0 && sum != 0)
            {
                img_status = BAD_DIMEN;
                std::stringstream ss;
                ss<<"Pixel ["<<index<<"] @ coo (";
                for(size_t k = 0; k < iPx.size()-1; k++)
                    ss<<iPx[k]<<",";
                ss<<iPx[iPx.size()-1]<<") is out of range"<<std::flush;
                
                throw FITSexception(img_status,"FITScube","SetPixelValue",ss.str());
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        SetPixelValue(val, index);
    }
    
    template< typename T >
    void FITSimg<T>::SetPixelValue(T val, unsigned long long index)
    {
        try
        {
            if(index >= data.size())
            {
                img_status = BAD_DIMEN;
#if __cplusplus < 201103L
                throw FITSexception(img_status,"FITScube","SetPixelValue","Pixel ["+std::to_string(static_cast<long long>(index))+"] is out of range");
#else
                throw FITSexception(img_status,"FITScube","SetPixelValue","Pixel ["+std::to_string(index)+"] is out of range");
#endif
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        data[index] = val;
    }
    
    template< typename T >
    void FITSimg<T>::SetUByteAtPixel      (uint8_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetByteAtPixel       (int8_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetUShortAtPixel     (uint16_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetShortAtPixel      (int16_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetIntAtPixel        (int val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetUIntAtPixel       (unsigned int val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetLongAtPixel       (int32_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetULongAtPixel      (uint32_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetLongLongAtPixel   (int64_t val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetFloatAtPixel      (float val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetDoubleAtPixel     (double val, size_t n)
    {
        SetPixelValue(static_cast<double>( val ), PixelCoordinates(n));
    }
    
    template< typename T >
    void FITSimg<T>::SetUByteAtPixel      (uint8_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);

        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetByteAtPixel       (int8_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetUShortAtPixel     (uint16_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetShortAtPixel      (int16_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetIntAtPixel        (int val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetUIntAtPixel       (unsigned int val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetLongAtPixel       (int32_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetULongAtPixel      (uint32_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetLongLongAtPixel   (int64_t val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetFloatAtPixel      (float val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
    template< typename T >
    void FITSimg<T>::SetDoubleAtPixel     (double val, unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(unsigned int naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        SetPixelValue(static_cast<double>( val ), pix_index);
        
        pix_index.clear();
    }
    
#pragma mark • Accessor
    
    template< typename T>
    double FITSimg<T>::GetMean() const
    {
        if(GetSum() == 0)
            return 0;
        
        if(mask.sum() && mask.size() > 0)
            return static_cast<double>( ( data[!mask] ).sum() ) / static_cast<double>( ( data[!mask] ).size() );
        else
            return static_cast<double>( data.sum() ) / static_cast<double>( data.size() );
    }
    
    template< typename T>
    double FITSimg<T>::GetSum() const
    {
        double sum = 0;

#if __cplusplus >= 201103L
        if(mask.sum() && mask.size() > 0)
            sum = static_cast<double>(  data[!mask].sum() );
        else
            sum = static_cast<double>(  data.sum() );
#else
        for(size_t i = 0; i < data.size(); i++)
        {
            if(mask[i])
                continue;
            
            sum +=  data [i] ;
        }
#endif
        return sum;
    }
    
    template< typename T>
    double FITSimg<T>::GetQuadraticMean() const
    {
        
#if __cplusplus >= 201103L
        if(mask.sum() && mask.size() > 0)
            return (std::pow(data[!mask], 2.)).sum() / static_cast<double>( ( data[!mask] ).size() );
        else
            return (std::pow(data, 2.)).sum() / static_cast<double>( data.size() );
#else
      double QMean = 0;
      double count = 0;
      for(unsigned int i = 0; i < data.size(); i++)
	{
	  if(mask[i])
	    continue;

	  QMean += static_cast<double>( data [i] )*static_cast<double>(data [i] );
	  count++;
	}

      return (count > 0)? QMean/count : 0;
#endif
    }
    
    template< typename T>
    double FITSimg<T>::GetVariance() const
    {
        
#if __cplusplus >= 201103L
        if(mask.sum() && mask.size() > 0)
            return sqrt( std::pow(data[!mask] - GetMean(), 2.) ).sum()  / static_cast<double>( ( data[!mask] ).size() -1 );
        else
            return sqrt( std::pow(data - GetMean(), 2.) ).sum()  / static_cast<double>( data.size() -1 );
#else
      double mean = GetMean();
      double count = 0;
      double var   = 0;
 
      for(unsigned int i = 0; i < data.size(); i++)
        {
          if(mask[i])
            continue;

          var += ( data [i]  - mean)*( data [i]  - mean);
          count++;
	}

      return (count-1. > 0)? sqrt(var/(count-1.)) : 0;
#endif
    }
    
    
    template< typename T>
    double FITSimg<T>::GetMedian(bool fast) const
    {
        std::vector<double> tmp;
        for(size_t i = 0; i < data.size(); i++)
        {
            if(!mask[i]) tmp.push_back( data[i] );
        }
    
        std::sort(tmp.begin(), tmp.end());
        
        if(tmp.size() < 1)
            return 0;
        
        double median = 0;
        if(!( tmp.size() % 2 ))
            median = ( tmp[tmp.size()/2] + tmp[tmp.size()/2+1] ) / 2.;
        else
            median = ( tmp[tmp.size()/2] );
        
        if(GetSum() == 0)
            return median;
        
        if(fast)
            return median;
        
        return Getpercentil(0.5);
    }
    
    template< typename T>
    double FITSimg<T>::Getpercentil(double fpp, bool fast) const
    {
        std::vector<double> tmp;
        for(size_t i = 0; i < data.size(); i++)
            if(!mask[i] && fabs(data[i]) > std::numeric_limits<double>::min()) tmp.push_back( data[i] );
        
        std::sort(tmp.begin(), tmp.end());
        
        if(tmp.size() < 1)
            return 0;
        
        double pos = static_cast<double>(tmp.size())*fpp;
        size_t n   = static_cast<size_t>(pos) + ((pos - static_cast<double>(static_cast<size_t>(pos)) > 0.5)? 1 : 0);
        
        if(GetSum() == 0)
            return tmp[n];
        
        if(fast)
            return tmp[n];
        
        
        double pp_value = std::numeric_limits<double>::max();
        stat::Percentil stat(tmp, fpp);
        
        //std::cout<<"\nEST. "<<fpp<<" percentil :"<<tmp[n]<<" -> "<<data[!mask && data<tmp[n]].sum()/data[!mask].sum()<<std::endl;
        
                 
            std::valarray<double> vtmp(tmp.data(), tmp.size());
            
            bool match = false;
            int32_t inc = +1;
            size_t step = 1;
            double err     = std::numeric_limits<double>::max()/2;
            double tmp_err = std::numeric_limits<double>::max();
            size_t tmp_n = 0;
            
            double lower_bound = std::numeric_limits<double>::min();
	    
            
            while( !match && n > 0 && (n < vtmp.size()-1) )
            {
	      lower_bound = static_cast<double>(std::valarray<double>(vtmp[std::slice(0,n,1)]  ).size())/static_cast<double>(vtmp.size());

	      if(n>0)
		lower_bound = static_cast<double>(std::valarray<double>(vtmp[std::slice(0,n,1)]  ).size())/static_cast<double>(vtmp.size());
                
                err = std::abs(lower_bound-fpp)/fpp;
                
                if(err > tmp_err)
                {
                    n = (inc > 0)? tmp_n+step/2:tmp_n-step/2;
                    step = step/2;
                    continue;
                }
                
                if(lower_bound <= fpp)
                    inc = + 1;
                else if(lower_bound > fpp)
                    inc = - 1;
                
                if(std::abs(lower_bound - fpp)/fpp < std::numeric_limits<double>::min()*1e4 ||
                   std::abs(err - tmp_err)/tmp_err < std::numeric_limits<double>::min()*1e4 ||
                   step < 1 )
                {
                    match = true;
                  
		  continue;
                }
                
                step = static_cast<size_t>(err*static_cast<double>(n));
                tmp_n = n;
                
                if(inc > 0)
                {
                    while(n+step > vtmp.size()-1) step /= 2;
                    
                    n += step;
                }
                else
                {
                    while(step > n) step /= 2;
                    
                    n -= step;
                }
                
                tmp_err = err;
            }
            
            pp_value = vtmp[n];
            
            vtmp.resize(0);
        
        if(FITScube::debug)
            std::cout<<"ALT. EVAL "<<fpp<<" percentil :"<<pp_value<<" -> "<<stat.Eval(pp_value)<<" -> "<<stat({pp_value})<<std::endl;
            
        tmp.clear();
        
        return pp_value;
    }
    
    template< typename T>
    double FITSimg<T>::Get5thpercentil() const
    {
        return Getpercentil(0.05);
    }
    
    template< typename T>
    double FITSimg<T>::Get25thpercentil() const
    {
        return Getpercentil(0.25);
    }
    
    template< typename T>
    double FITSimg<T>::Get75thpercentil() const
    {
        return Getpercentil(0.75);
    }
    
    template< typename T>
    double FITSimg<T>::Get95thpercentil() const
    {
        return Getpercentil(0.95);
    }
    
    template< typename T>
    double FITSimg<T>::GetMinimum() const
    {
        
        if(std::valarray<bool>(mask[!mask]).size() == 0)
            throw std::invalid_argument("\033[31m[FITSimg::GetMinimum]\033[0mAll pixel are masked. I can't estimate a minima");
        
        double min_val =  std::valarray<double>(data[!mask]).min() ;
        
        return min_val;
    }
    
    template< typename T>
    double FITSimg<T>::GetMaximum() const
    {
        if(std::valarray<bool>(mask[!mask]).size() == 0)
            throw std::invalid_argument("\033[31m[FITSimg::GetMaximum]\033[0mAll pixel are masked. I can't estimate a maxima");
        
        double max_val =  std::valarray<double>(data[!mask]).max() ;
        
        return max_val;
    }
    
    
#pragma mark • Display methods
#ifdef _HAS_Qt4_
#pragma mark – Qt display capability
    /**
     *  Display FITS 2D images onto the scrren.
     *  @param OPTION: Optional parameters used to customize the plot:
     *      - LOGZ  : Display color scale in LOG10 scale
     *      - QUADZ : Display color scale in QUADRATIC scale
     *      - SQRTZ : Display color scale in SQUARE_ROOT scale
     *      - SINH  : Display color scale in SINH scale
     *  @return Pointer to the displayed plot structur for further customization and overplotting
     */
    template< typename T >
    QCustomPlot* FITSimg<T>::QtDisplay(std::string OPTION) const
    {
        return QtDisplay(0 , Nelements(), OPTION);
    }
    
    /**
     *  Display sub FITS 2D images onto the scrren.
     *  @param iMin: index of the pixel used as the lower left corner
     *  @param iMax: index of the pixel used as the upper left corner
     *  @param OPTION: Optional parameters used to customize the plot:
     *      - LOGZ  : Display color scale in LOG10 scale
     *      - QUADZ : Display color scale in QUADRATIC scale
     *      - SQRTZ : Display color scale in SQUARE_ROOT scale
     *      - SINH  : Display color scale in SINH scale
     *  @return Pointer to the displayed plot structur for further customization and overplotting
     */
    template< typename T >
    QCustomPlot* FITSimg<T>::QtDisplay(unsigned int iMin, unsigned int iMax, std::string OPTION) const
    {
        //- SEARCH FOR EXISTING QtMainWindows
        QtFITSviewer *mw = new QtFITSviewer();
        mw->setWindowTitle(QString(hdu->GetValueForKey("EXTNAME").c_str()));
        
        const unsigned long long nx = static_cast<unsigned long long>( Naxis[0] );
        const unsigned long long ny = static_cast<unsigned long long>( Naxis[1] );
        
        int xsize = (nx*2 < 900)? nx*2 : 900;
        xsize = (xsize < 542 )? 542 : xsize;
        
        int ysize = xsize*ny/nx-100;
        ysize = (ysize < 390)? 390 : ysize;
        ysize = (ysize > 900-100)? 900 : ysize;
        
        mw->setGeometry(400, 250, xsize, ysize);
        
        //- Obtain the plot
        QCustomPlot *qPlot = mw->GetPlot();
        qPlot->yAxis->setScaleRatio(qPlot->xAxis,1.0);
        
        //- Fill the graph
        qPlot->xAxis->setLabel("pixel [px]");
        qPlot->yAxis->setLabel("pixel [px]");
        
        Overlay(qPlot, iMin, iMax, OPTION);
        
        mw->SetPlot(qPlot);
        mw->show();
        
        return qPlot;
        
    }
    
    template< typename T >
    void FITSimg<T>::Overlay(QCustomPlot* qPlot, std::string OPTION) const
    {
        return Overlay(qPlot, 0 , Nelements(), OPTION);
    }
    
    template< typename T >
    void FITSimg<T>::Overlay(QCustomPlot* qPlot, unsigned int iMin, unsigned int iMax, std::string OPTION) const
    {
#if __cplusplus >= 199711L
        //auto pixels_coo = std::bind( &FITScube::PixelCoordinates, this,std::placeholders::_1);
        auto pixels_coo   = std::bind( static_cast<std::vector<double>(FITScube::*)(size_t) const>(&FITScube::WorldCoordinates), this,std::placeholders::_1);
#endif
        
        std::vector<unsigned long long> xPixels;
        
        std::transform(OPTION.begin(), OPTION.end(), OPTION.begin(), ::tolower);
        

        const unsigned long long nx = static_cast<unsigned long long>( Naxis[0] );
        const unsigned long long ny = static_cast<unsigned long long>( Naxis[1] );
        
        FITScube::zScale useScale = fLIN;
        if(OPTION.find("logz",0) != std::string::npos )
            useScale  = fLOG;
        else if(OPTION.find("sinhz",0) != std::string::npos )
            useScale  = fSINH;
        else if(OPTION.find("quadz",0) != std::string::npos )
            useScale  = fQUAD;
        else if(OPTION.find("sqrtz",0) != std::string::npos )
            useScale  = fSQRT;
        else
            useScale = fLIN;
        
        QCPColorMap *colorMap = new QCPColorMap(qPlot->xAxis, qPlot->yAxis);
        colorMap->data()->setSize(nx, ny);								// we want the color map to have nx * ny data points
                                                                        // set the color gradient of the color map to one of the presets:

        double min_x, max_x, min_y, max_y;
        
#if __cplusplus >= 199711L
        min_x = std::min(pixels_coo(0)[0],pixels_coo(data.size()-1)[0]);
        max_x = std::max(pixels_coo(0)[0],pixels_coo(data.size()-1)[0]);
        
        min_y = std::min(pixels_coo(0)[1],pixels_coo(data.size()-1)[1]);
        max_y = std::max(pixels_coo(0)[1],pixels_coo(data.size()-1)[1]);
#else
        min_x = std::min(WorldCoordinates(0)[0],WorldCoordinates(data.size()-1)[0]);
        max_x = std::max(WorldCoordinates(0)[0],WorldCoordinates(data.size()-1)[0]);
        
        min_y = std::min(WorldCoordinates(0)[1],WorldCoordinates(data.size()-1)[1]);
        max_y = std::max(WorldCoordinates(0)[1],WorldCoordinates(data.size()-1)[1]);
#endif
        
        colorMap->data()->setRange(QCPRange(min_x, max_x), QCPRange(min_y, max_y));	// and span the coordinate range in both key (x) and value (y) dimensions
        colorMap->data()->fill(0);
        
        // now we assign some data, by accessing the QCPColorMapData instance of the color map:
        
        unsigned int k = iMin;
        double* Intensity = new double[nx*ny];
        for(unsigned int ik = 0; ik< nx*ny; ik++) Intensity[ik]=0;
        
        for(k = iMin; k < iMax; k++)
        {
            if(*(&mask[0]+k))
                continue;
                     
            Intensity[k%(nx*ny)] += *(&data[0]+k);
        }
        
        bool is_increasing_xAxis = true;
        bool is_increasing_yAxis = true;
        
        if(WorldCoordinates(0)[0] > WorldCoordinates(data.size()-1)[0])
            is_increasing_xAxis  = false;
        
        if(WorldCoordinates(0)[1] > WorldCoordinates(data.size()-1)[1])
            is_increasing_yAxis  = false;
        
        for(unsigned int ik = 0; ik< nx*ny; ik++)
        {
            
            k = iMin + ik;
            
            xPixels.clear();
            
#if __cplusplus >= 199711L
            xPixels = FITSimg<T>::PixelCoordinates(k);

            double xIndex = (is_increasing_xAxis)? xPixels[0] : Naxis[0] - (xPixels[0]+1);//-PixelCoordinates(0)[0];
            double yIndex = (is_increasing_yAxis)? xPixels[1] : Naxis[1] - (xPixels[1]+1);//-PixelCoordinates(0)[1];
#else
            xPixels = PixelCoordinates(k);
            
            double xIndex = xPixels[0];//-WorldCoordinates(0)[0];
            double yIndex = xPixels[1];//-WorldCoordinates(0)[1];
#endif
            
            if(Intensity[ik] == 0)
                continue;
                
            switch (useScale)
            {
                case fLIN:
                    Intensity[ik] *= 1.;
                    break;
                    
                case fLOG:
                    Intensity[ik] = log10(Intensity[ik]);
                    break;
                    
                case fQUAD:
                    Intensity[ik] = pow(Intensity[ik], 2.);
                    break;
                    
                case fSQRT:
                    Intensity[ik] = sqrt(Intensity[ik]);
                    break;
                    
                case fSINH:
                    Intensity[ik] = sinh(Intensity[ik]);
                    break;
                    
                default:
                    Intensity[ik] *= 1;
                    break;
            }
            
            colorMap->data()->setCell(xIndex, yIndex, static_cast<double>( Intensity[ik]) );
        }
        
        
        delete [] Intensity;
        
        std::string UNITS = hdu->GetValueForKey("UNITS");
        if(UNITS.size() < 1)
            UNITS += std::string("Intensity");
        
        switch (useScale)
        {
            case fLOG:
                UNITS = "log10("+UNITS+")";
                break;
                
            case fQUAD:
                UNITS += "^2";
                break;
                
            case fSQRT:
                UNITS += "^(1/2)";
                break;
                
            case fSINH:
                UNITS += "sinh("+UNITS+")";
                break;
                
            default:
                break;
        }

        
        QCPColorScale *colorScale = NULL;
        
        if(	!(qPlot->plotLayout()->hasElement (0, 1)) )
        {
            // add a color scale:
            colorScale = new QCPColorScale(qPlot);
            colorScale->setGradient(QCPColorGradient::gpThermal);
            //colorMap->setGradient(QCPColorGradient::gpThermal);
        
            colorScale->axis()->setLabel(UNITS.c_str());
            qPlot->plotLayout()->addElement(0, 1, colorScale);          // add it to the right of the main axis rect
            colorScale->setType(QCPAxis::atRight);						// scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
            
            // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
            QCPMarginGroup *marginGroup = new QCPMarginGroup(qPlot);
            qPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
            colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
        }
        else
            colorScale = static_cast<QCPColorScale*>(qPlot->plotLayout()->element(0,1));
            
        
        colorMap->setColorScale(colorScale);						// associate the color map with the color scale
        // we could have also created a QCPColorGradient instance and added own	colors to
        // the gradient, see the documentation of QCPColorGradient for what's possible.
        
        // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
        colorMap->rescaleDataRange();
        
        if(fabs(GetMaximum()/GetMinimum()) > 100)
            colorMap->setDataRange(QCPRange(Get5thpercentil(),Get95thpercentil()));
        
        colorMap->setInterpolate(false);
        
        //qPlot->addPlottable(colorMap);
        
        // rescale the key (x) and value (y) axes so the whole color map is visible:
        qPlot->rescaleAxes();
        qPlot->replot();
    }
    
    /**
     *  Display a 2D layer (or a single slice) of 3D FITS datacube onto Qt Widget output.
     *
     *  @param iLayer: The Layer to display
     *  @param OPTION: Dysplay option
     *  @return Pointer to the displayed plot structur for further customization and overplotting.
     */
    template< typename T >
    QCustomPlot* FITSimg<T>::QtDisplayLayer(unsigned int iLayer, std::string OPTION) const
    {
        if(Naxis.size() <= 2)
            QtDisplay(OPTION);
        
        try
        {
            if(iLayer > Naxis[2])
            {
                iLayer=Naxis.size();
#if __cplusplus < 201103L
                throw FITSwarning("FITSimg","QtDisplayLayer","Image only contains "+std::to_string(static_cast<long long>(Naxis[2]))+" layers.\nThe last layer will be displayed");
#else
                throw FITSwarning("FITSimg","QtDisplayLayer","Image only contains "+std::to_string(Naxis[2])+" layers.\nThe last layer will be displayed");
#endif
            }
        
            if(iLayer < 1)
            {
                iLayer = 1;
#if __cplusplus < 201103L
                throw FITSwarning("FITSimg","QtDisplayLayer","Layer should be comprised between 1 to "+std::to_string(static_cast<long long>(Naxis[2]))+".\nThe first layer will be displayed");
#else
                throw FITSwarning("FITSimg","QtDisplayLayer","Layer should be comprised between 1 to "+std::to_string(Naxis[2])+".\nThe first layer will be displayed");
#endif
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        unsigned int nx = static_cast<unsigned int>( Size(1) );
        unsigned int ny = static_cast<unsigned int>( Size(2) );
        
        unsigned int iMin       = nx*ny*(iLayer-1);
        unsigned int iMax       = nx*ny + iMin;
        
        return QtDisplay(iMin, iMax, OPTION);
    }
    
#endif
    
#pragma mark • Data export methods
#ifdef _HAS_ROOT_
#pragma mark – ROOT export capability
    /**
     *  Project the 2D FITS image onto one of its primary axis and convert the resulting histograms into a ROOT:TH1 Histograms
     *  @param hin: Upon return ROOT::TH1 histograms.
     *  @param xAxes: FITS axes one whish to project the data
     */
    template< typename T >
    void FITSimg<T>::PixelToTH1(TH1& hin, unsigned int xAxes) const
    {
        
        if(FITScube::debug)
            std::cout<<"\033[33m>>>>> [FITSimg::PixelToTH2]: \033[32mExport FITS data cube to 1D Histogram.\033[0m"<<std::endl;
        
        if(xAxes > Naxis.size())
        {
            std::cerr<<"\033[1;35;47m*** [FITSimg::PixelToTH1]: Errors *** "
                     <<"DATA CUBE HAS LESS DIMENSION THAN "<<xAxes<<"\033[0m"<<std::endl;
            return;
        }
        
        std::vector<unsigned long long> xPixel;
        
        for(unsigned int k = 0; k < data.size(); k++)
        {
            if(mask[k])
                continue;
            
            if(data[k] == 0)
                continue;
            
            xPixel.clear();
            xPixel = PixelCoordinates(k);
            
            if(hin.InheritsFrom("TH1I"))
                hin.Fill( xPixel[xAxes], static_cast<int>( data[k] ));
            else if(hin.InheritsFrom("TH1F"))
                hin.Fill( xPixel[xAxes], static_cast<float>( data[k] ));
            else if(hin.InheritsFrom("TH1D"))
                hin.Fill( xPixel[xAxes], static_cast<double>( data[k] ));
            else if(hin.InheritsFrom("TH1C"))
                hin.Fill( xPixel[xAxes], static_cast<int8_t>( data[k] ));
            else if(hin.InheritsFrom("TH1S"))
                hin.Fill( xPixel[xAxes], static_cast<short>( data[k] ));
            else
                hin.Fill( xPixel[xAxes], data[k]);
        }
        
        xPixel.clear();
        
        if(FITScube::debug)
            std::cout<<"\033[33m<<<<< [FITSimg::PixelToTH1]:\033[32m DONE.\033[34m"<<std::endl;
    }
    
    /**
     *  Convert 2D FITS image into a ROOT:TH2 Histograms
     *  @param hin: Upon return ROOT::TH2 histograms.
     */
    template< typename T >
    void FITSimg<T>::PixelToTH2(TH2& hin) const
    {
        
        double xMin = hin.GetXaxis()->GetBinLowEdge(1);
        double xMax = hin.GetXaxis()->GetBinUpEdge(hin.GetNbinsX());
        
        double yMin = hin.GetYaxis()->GetBinLowEdge(1);
        double yMax = hin.GetYaxis()->GetBinUpEdge(hin.GetNbinsY());
        
        if(FITScube::debug)
            std::cout<<"\033[33m>>>>> [FITSimg::PixelToTH2]: \033[32mExport FITS data cube to 2D Histogram.\033[0m"<<std::endl;
        
        std::vector<unsigned long long> xPixel;
        
        for(unsigned int k = 0; k < data.size(); k++)
        {
            if(mask[k])
                continue;
    
            if(data[k] == 0)
                continue;
            
            xPixel.clear();
            xPixel = PixelCoordinates(k);
            
            if(xPixel[0] < xMin || xPixel[0] > xMax)
                continue;
            
            if(xPixel[1] < yMin || xPixel[1] > yMax)
                continue;
            
            if(hin.InheritsFrom("TH2I"))
                hin.Fill( xPixel[0], xPixel[1], static_cast<int>( data[k] ));
            else if(hin.InheritsFrom("TH2F"))
                hin.Fill( xPixel[0], xPixel[1], static_cast<float>( data[k] ));
            else if(hin.InheritsFrom("TH2D"))
                hin.Fill( xPixel[0], xPixel[1], static_cast<double>( data[k] ));
            else if(hin.InheritsFrom("TH2C"))
                hin.Fill( xPixel[0], xPixel[1], static_cast<int8_t>( data[k] ));
            else if(hin.InheritsFrom("TH2S"))
                hin.Fill( xPixel[0], xPixel[1], static_cast<short>( data[k] ));
            else
                hin.Fill( xPixel[0], xPixel[1], data[k]);
        }
        
        if(FITScube::debug)
            std::cout<<"\033[33m<<<<< [FITSimg::PixelToTH2]:\033[32m DONE.\033[34m"<<std::endl;
    }
    
    /**
     *  Convert nD FITS data cube  into a ROOT:TH3 Histograms
     *  @param hin: Upon return ROOT::TH3 histograms.
     */
    template< typename T >
    void FITSimg<T>::PixelToTH3(TH3& hin) const
    {
        
        if(FITScube::debug)
            std::cout<<"\033[33m>>>>> [FITSimg::PixelToTH3]: \033[32mExport FITS data cube to 2D Histogram.\033[0m"<<std::endl;
        
        if(Naxis.size() < 3)
        {
            std::cerr<<"\033[1;35;47m*** [FITSimg::PixelToTH3]: Errors *** "
                     <<"CAN'T EXPORT 2D IMAGE to ROOT::TH3 3D HISTOGRAMS\033[0m"<<std::endl;
            return;
        }
        
        std::vector<unsigned long long> xPixel;
        
        for(unsigned int k = 0; k < data.size(); k++)
        {
            if(mask[k])
                continue;
            
            if(data[k] == 0)
                continue;
            
            xPixel.clear();
            xPixel = PixelCoordinates(k);
            
            if(hin.InheritsFrom("TH3I"))
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], static_cast<int>( data[k] ));
            else if(hin.InheritsFrom("TH3F"))
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], static_cast<float>( data[k] ));
            else if(hin.InheritsFrom("TH3D"))
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], static_cast<double>( data[k] ));
            else if(hin.InheritsFrom("TH3C"))
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], static_cast<int8_t>( data[k] ));
            else if(hin.InheritsFrom("TH3S"))
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], static_cast<short>( data[k] ));
            else
                hin.Fill( xPixel[0], xPixel[1], xPixel[2], data[k]);
        }
        
        if(FITScube::debug)
            std::cout<<"\033[33m<<<<< [FITSimg::PixelToTH3]:\033[32m DONE.\033[34m"<<std::endl;
    }
    
    /**
     *  Convert FITS data cube into ROOT tree.
     *
     *  @param outTree: The tree that will be used to store FITS data after conversion to TTree.
     */
    template< typename T >
    void FITSimg<T>::PixelToTree(TTree &outTree) const
    {
        double Val = 0;
        
        std::vector<unsigned int> pxAxis(Naxis.size());
        //std::vector<double>       WCS(Naxis.size());
        
        if(outTree.FindBranch("PIXEL"))
            outTree.SetBranchAddress("PIXEL",&Val);
        else
            outTree.Branch("PIXEL", &Val, "PIXEL/D");
        
        for(unsigned int k = 0; k < pxAxis.size(); k++ )
        {
            if(outTree.FindBranch(Form("NAXIS%i",k+1)))
                outTree.SetBranchAddress(Form("NAXIS%i",k+1),&pxAxis[k]);
            else
                outTree.Branch(Form("NAXIS%i",k+1), &pxAxis[k],	Form("NAXIS%i/i",k+1));
        }
        
        /*
         for(unsigned int k = 0; k < pxAxis.size(); k++ )
        {
            if(outTree.FindBranch(Form("WCS%i",k+1)))
                outTree.SetBranchAddress(Form("WCS%i",k+1),&WCS[k]);
            else
                outTree.Branch(Form("WCS%i",k+1), &WCS[k],	Form("WCS%i/D",k+1));
        }
         */
        
        for(unsigned int nPx = 0; nPx < data.size(); nPx++)
        {
            for(unsigned int iX = 1; iX < pxAxis.size(); iX++ )
            {
                pxAxis[iX] = PixelCoordinates(nPx)[iX];
                //WCS   [iX] = Axis[iX]->PixelCenter(pxAxis[iX]);
            }
            
            Val = data[nPx];
            
            outTree.Fill();
        }
        
        pxAxis.clear();
        //WCS.clear();
        
    }
    
    template< typename T >
    void FITSimg<T>::PixelDistribution(TH1& hin) const
    {
        if(FITScube::debug)
            std::cout<<"\033[33m>>>>> [FITSimg::PixelDistribution]\033[0m"<<std::endl;
        
        for(unsigned int k = 0; k < data.size(); k++)
        {
            if(mask[k])
                continue;
            
            if(data[k] == 0)
                continue;
            
            if(hin.InheritsFrom("TH1I"))
                hin.Fill( static_cast<int>( data[k] ));
            else if(hin.InheritsFrom("TH1F"))
                hin.Fill( static_cast<float>( data[k] ));
            else if(hin.InheritsFrom("TH1D"))
                hin.Fill( static_cast<double>( data[k] ));
            else if(hin.InheritsFrom("TH1C"))
                hin.Fill( static_cast<int8_t>( data[k] ));
            else if(hin.InheritsFrom("TH1S"))
                hin.Fill( static_cast<short>( data[k] ));
            else
                hin.Fill( data[k]);
        }
        
        if(FITScube::debug)
            std::cout<<"\033[33m<<<<< [FITSimg::PixelDistribution]\033[0m"<<std::endl;
    }
    
#endif
    
#pragma mark • Extraction method
    
    /**
     *  Extract layer from a 3D FITS cube.
     *  @param iLayer: The layer one want to extract in single FITSimg. Layer numbering start at 1
     *  @return Return pointer to a 2D FITScube for the selected layer. Header information are updated to account for new image dimenssion.
     */
    template< typename T >
    FITScube *FITSimg<T>::Layer(unsigned int iLayer) const
    {
        FITSimg<T> *copy = NULL;
        copy = new FITSimg<T>(*this);
        
        if(iLayer > 0)
            iLayer--;
        
        try
        {
            if(Naxis.size() <= 2)
            {
                throw FITSexception(BAD_DIMEN,"FITScube","Layer","Image do not contains layer.\nCopy of this will be returned.");
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return copy;
        }
        
        if(Naxis.size() > 3)
        {
            throw FITSexception(BAD_DIMEN,"FITScube","Layer","CAN'T EXTRACT LAYER OF nD DATA CUBE WITH n > 1.");
        }
        
        try
        {
            if(iLayer > Naxis[2])
            {
                iLayer=Naxis.size();
#if __cplusplus < 201103L
                throw FITSwarning("FITScube","Layer","Image only contains "+std::to_string(static_cast<long long>(Naxis[2]))+" layers.\nThe last layer will be displayed.");
#else
                throw FITSwarning("FITScube","Layer","Image only contains "+std::to_string(Naxis[2])+" layers.\nThe last layer will be displayed.");
#endif
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        long long nx = Size(1);
        long long ny = Size(2);
        
        long long n_elements = nx*ny;
        long long iBegin     = (nx*ny*iLayer-100 < 0) ? 0 : nx*ny*iLayer-100;
        long long iEnd       = (nx*ny*(1+iLayer)+100 > static_cast<long long>( data.size()) ) ? static_cast<long long>( data.size()):nx*ny*(1+iLayer)+100;
        long long i          = 0;
        
        copy->data.resize(n_elements);
        copy->mask.resize(n_elements);
        
        for(long long k = iBegin ; k < iEnd; k++)
        {
            
            long long iL = PixelCoordinates(k)[2];
            
            if(iL != iLayer)
                continue;
            
            copy->data[i] = data[k];
            copy->mask[i] = mask[k];
            
            i++;
            
        }
        
        if(FITScube::debug)
            copy->hdu->Dump(std::cout);
        copy->DeleteLastAxis();
        
        try
        {
            if(static_cast<long long>( copy->data.size() ) != copy->hdu->GetDimension() )
#if __cplusplus < 201103L
                throw FITSwarning("FITScube","Layer","DIMENSION MISSMATCH : "+std::to_string(static_cast<long long unsigned int>(copy->data.size()))+" != "+std::to_string(std::to_string(static_cast<long long unsigned int>(copy->hdu->GetDimension())));
#else
                throw FITSwarning("FITScube","Layer","DIMENSION MISSMATCH : "+std::to_string(copy->data.size())+" != "+std::to_string(copy->hdu->GetDimension()));
#endif
            
        }
        catch(std::exception &e)
        {
            std::cerr<<e.what()<<std::endl;
        }
        
        return copy;
    }
    
    /**
     *  Resize the current data array.
     *  @param xMin, yMin: The bottom left corner of the window used for the extraction
     *  @param xSize, ySize: The pixel size, along the x and y axis, of the windows
     *  @note Only work for two dimensional array
     */
    template< typename T >
    void FITSimg<T>::Resize (size_t xMin, size_t yMin, size_t xSize, size_t ySize)
    {
#ifdef _DEBUG_
        std::valarray<size_t> foo ( Nelements() );
        for (size_t i=0; i<foo.size(); i++) foo[i]=i;
#endif
        
        xSize = (xMin + xSize < Naxis[0])?xSize : Naxis[0]-xMin;
        ySize = (yMin + ySize < Naxis[1])?ySize : Naxis[1]-yMin;

        
        std::vector<unsigned long long> pixels;
        pixels.push_back(xMin);
        pixels.push_back(yMin);
        for(size_t i = 2; i < Naxis.size(); i++)
            pixels.push_back(0);
        
        size_t i_px = PixelIndex(pixels);
        pixels.clear();
        
        if(i_px >= Nelements())
            return;
        
        std::valarray<size_t> size(Naxis.size());
        std::valarray<size_t> stride(Naxis.size());
        
#if __cplusplus < 201103L
        size.resize(Naxis.size());
        stride.resize(Naxis.size());
#endif
        
        stride  += 1;
        
        for(size_t k = 0; k < Naxis.size(); k++)
        {
            if(k == Naxis.size()-1)
                size[k] = xSize;
            else if(k==Naxis.size()-2)
                size[k] = ySize;
            else
                size[k] = static_cast<size_t>(Naxis[Naxis.size() - 1 - k]);
            
            for(size_t l = 0; l < k; l++)
                stride[Naxis.size() - 1 - k] *= static_cast<size_t>(Naxis[l]);
        }
        
#ifdef _DEBUG_
        std::cout<<">>> FOO2 TEST for total "<<Nelements()<<std::endl<<std::endl;
        std::valarray<size_t> foo2 = foo[std::gslice(i_px,size,stride)];
        for (size_t i=0; i<foo2.size(); i++) std::cout<<foo2[i]<<std::endl;
        std::cout<<std::endl<<"<<< FOO2 TEST"<<std::endl;
#endif
        
        std::valarray<double> cpy     = data[std::gslice(i_px,size,stride)];
        std::valarray<bool>   mcpy = mask[std::gslice(i_px,size,stride)];
        
        data.resize(cpy.size());
        data *= 0.;
        
        mask.resize(mcpy.size());
        mask &= false;
        
        data += cpy;
        mask &= true;
        
        cpy.resize(0);
        mcpy.resize(0);
        size.resize(0);
        stride.resize(0);
        
        Naxis[0] = static_cast<unsigned long long>( xSize );
        Naxis[1] = static_cast<unsigned long long>( ySize );
        
        hdu->valueForKey("NAXIS1",static_cast<long long>(Naxis[0]));
        hdu->valueForKey("NAXIS2",static_cast<long long>(Naxis[1]));
        
        size_t n = 0;

#if __cplusplus >= 199711L
        auto pixels_pos   = std::bind( static_cast<std::vector<unsigned long long>(FITScube::*)(size_t) const>(&FITScube::PixelCoordinates), this,std::placeholders::_1);
        auto pixels_coo   = std::bind( static_cast<std::vector<double>(FITScube::*)(std::vector<unsigned long long>) const>(&FITScube::WorldCoordinates), this,std::placeholders::_1);
        
        hdu->valueForKey("CRPIX1",static_cast<double>( 0 ));
        hdu->valueForKey("CRPIX2",static_cast<double>( 0 ));
        
        hdu->valueForKey("CRVAL1",static_cast<double>( pixels_coo(pixels_pos(n))[0] ));
        hdu->valueForKey("CRVAL2",static_cast<double>( pixels_coo(pixels_pos(n))[1] ));
        
#else
        
        hdu->valueForKey("CRPIX1",static_cast<double>( PixelCoordinates(n)[0] ));
        hdu->valueForKey("CRPIX2",static_cast<double>( PixelCoordinates(n)[1] ));
        
        hdu->valueForKey("CRVAL1",static_cast<double>( WorldCoordinates(PixelCoordinates(n))[0] ));
        hdu->valueForKey("CRVAL2",static_cast<double>( WorldCoordinates(PixelCoordinates(n))[1] ));
#endif
        
    }
    
    /**
     *  Extract part of an image based on a cropping windows of the primary 2D slice of a N dimensional datacube.
     *  @param xMin, yMin: The bottom left corner of the window used for the extraction
     *  @param xSize, ySize: The pixel size, along the x and y axis, of the windows
     *  @return Return the FITScube for which x and y pixel coordinate are encapsulated in the specified window size.  Header information are updated to account for new image dimenssion.
     */
    template< typename T >
    FITScube *FITSimg<T>::Window (size_t xMin, size_t yMin, size_t xSize, size_t ySize) const
    {
        FITSimg<T> *copy = NULL;
        copy = new FITSimg<T>(*this);
        
        size_t xMax = xMin+xSize;
        size_t yMax = yMin+ySize;
        
        size_t n_elements = xSize*ySize;
        size_t n_deep     = 1;
        
        if(Naxis.size() > 2)
        {
            for(size_t iz = 2; iz < Naxis.size(); iz++)
                n_deep*= Naxis[iz];
        }
        
        n_elements *= n_deep;
        
        if(FITScube::debug)
            std::cout<<"\033[31m[FITScube::Window]\033[0m"<<std::endl
                     << "    \033[31m|- NEW IMAGE SIZE :\033[0m "<<n_elements<<std::endl
                     << "    \033[31m|- WINDOW         :\033[0m ("<<xMin<<","<<yMin<<")\033[34m -> \033[0m("<<xMax<<","<<yMax<<") [pix]"<<std::endl
                     << "    \033[31m`- PIXEL OFFSET   :\033[0m ("<<PixelCoordinates(0)[0]<<","<<PixelCoordinates(0)[1]<<")"<<std::endl;
        
        copy->Resize(xMin, yMin, xSize, ySize);
        
        return  copy;
    }
    
    /**
     *  Rebin image to smaller resolution.
     *  @param nbin Number of pixel to merge for each axis. If nbin[i]=0 or note defined, the i axis isn't rebined. Note that the number of pixel to be merged shall be a multiple of the total number of the pixel of that axis.
     *  @return Return rebined image as a new FITScube data cube.
     */
    template< typename T >
    FITScube *FITSimg<T>::Rebin (std::vector<size_t> nbin) const
    {
        double bin_size = 1;
        for(size_t iBin = 0; iBin < nbin.size(); iBin++)
            bin_size *= static_cast<double>(nbin[iBin]);
        
        std::vector<size_t> naxis;
        std::valarray<bool> msk = std::valarray<bool>(Nelements());
        
        std::vector<size_t> size;
        std::vector<size_t> stride;
        
        for(size_t k = 0; k < this->GetDimension(); k++)
        {
            stride.push_back(1);
            for(size_t j = 0; j < k; j++)
                stride[stride.size()-1] *= this->Size(j+1);
            
            if(nbin.size() <= k)
            {
                naxis.push_back(this->Size(k+1));
                size.push_back(1);
                continue;
            }
            
            if( nbin[k] == 0 || (this->Size(k+1)%nbin[k]) )
            {
                naxis.push_back(this->Size(k+1));
                size.push_back(1);
                continue;
            }
            
            naxis.push_back(Size(k+1)/nbin[k]);
            size.push_back(nbin[k]);
            
        }
        
        FITSimg<T> *copy = NULL;
        copy = new FITSimg<T>(naxis);
        
        copy->Bscale(BSCALE);
        copy->Bzero (BZERO);
        copy->Blank (BLANK);
        
#if __cplusplus < 201103L
        for(size_t k = 1; k <= this->GetDimension(); k++)
            if(pHDU().Exists("CDELT"+
                              std::to_string(static_cast<long long>(k))))
                copy->HDU()->valueForKey("CDELT"+std::to_string(static_cast<long long>(k)),
                                         this->pHDU().GetDoubleValueForKey("CDELT"+std::to_string(static_cast<long long>(k)))
                                         * static_cast<double>(Size(k)/copy->Size(k)));
            else
                copy->HDU()->valueForKey("CDELT"+std::to_string(static_cast<long long>(k)),
                                         static_cast<double>(Size(k)/copy->Size(k)),"");
        
        for(size_t k = 1; k <= this->GetDimension(); k++)
            if(pHDU().Exists("CRVAL"+std::to_string(static_cast<long long>(k))))
                copy->HDU()->valueForKey("CRVAL"+std::to_string(static_cast<long long>(k)),
                                         this->pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(static_cast<long long>(k)))
                                         + static_cast<double>(Size(k)/copy->Size(k))/2.);
            else
                copy->HDU()->valueForKey("CRVAL"+std::to_string(static_cast<long long>(k)),
                                         static_cast<double>(Size(k)/copy->Size(k))/2.);
#else
        for(size_t k = 1; k <= this->GetDimension(); k++)
            if(pHDU().Exists("CDELT"+
                              std::to_string(k)))
                copy->HDU()->valueForKey("CDELT"+std::to_string(k),
                                         this->pHDU().GetDoubleValueForKey("CDELT"+std::to_string(k))
                                         * static_cast<double>(Size(k)/copy->Size(k)));
            else
                copy->HDU()->valueForKey("CDELT"+std::to_string(k),
                                         static_cast<double>(Size(k)/copy->Size(k)),"");
        
        for(size_t k = 1; k <= this->GetDimension(); k++)
            if(pHDU().Exists("CRVAL"+std::to_string(k)))
                copy->HDU()->valueForKey("CRVAL"+std::to_string(k),
                                         this->pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(k))
                                         + static_cast<double>(Size(k)/copy->Size(k))/2.);
            else
                copy->HDU()->valueForKey("CRVAL"+std::to_string(k),
                                         static_cast<double>(Size(k)/copy->Size(k))/2.);
#endif
        
        std::cout<<" Rebining :";
        size_t pos = 0;
        for(size_t n = 0; n < copy->Size(); n++)
        {
            if(!(((n+1)/copy->Size(0)*100) % 10 ) && ((n+1)/copy->Size(0)*100) >= 1)
                std::cout<<"."<<std::flush;
            
            std::valarray<double> val = data[std::gslice(pos,
                                                    std::valarray<size_t>(size.data(), size.size()),
                                                    std::valarray<size_t>(stride.data(), stride.size()))];
            msk[std::gslice(pos,
                            std::valarray<size_t>(size.data(), size.size()),
                            std::valarray<size_t>(stride.data(), stride.size()))] = true;
            
            copy->SetPixelValue(val.sum()/bin_size, n);
            
            while(msk[pos] && pos < msk.size())
                pos++;
            
            //pos = ((std::valarray<size_t>)msk[msk > 0]).min();
            
            if( pos >= Nelements())
                break;
        }
        std::cout<<" \033[33m DONE\033[0m"<<std::endl;
        
        
        naxis.clear();
        stride.clear();
        size.clear();
        msk.resize(0);
        
        return  copy;
    }
    
    template < typename T >
    void FITSimg<T>::Print() const
    {
        for(unsigned int k = 0; k < data.size() ; k++)
        {
            std::cout<<k<<"    ";
            //for(unsigned int i = 0; i < Naxis.size(); i++)
            //   std::cout<<PixelCoordinates(k)[i]<<"    ";
            
            for(unsigned int i = 0; i < Naxis.size(); i++)
                std::cout<<WorldCoordinates(k)[i]<<"    ";
            
            std::cout<<*(&data[0]+k)<<"   "<<*(&mask[0]+k)<<std::endl;
        }
    }
    
    
#pragma mark - External opperator
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i]==img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator==(const FITSimg<T>& img1, const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data == img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i]<=img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator<=(const FITSimg<T>& img1,const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data <= img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] < img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator< (const FITSimg<T>& img1,const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data < img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] >= img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator>=(const FITSimg<T>& img1,const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data >= img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] > img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator> (const FITSimg<T>& img1,const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data > img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare two fits images.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to the content of the FITSimg img2.
     *  @param img1: Images to be compared to img2
     *  @param img2: Images to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] != img2[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator!=(const FITSimg<T>& img1,const FITSimg<T>& img2)
    {
        std::valarray<bool> test = ( img1.data != img2.data );
        test[img1.mask] = 0;
        test[img2.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] == val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator==(const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data == val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] <= val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator<=(const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data <= val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] < val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator< (const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data < val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] >= val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator>=(const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data >= val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] > val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator> (const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data > val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] != val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator!=(const FITSimg<T>& img1, const T& val)
    {
        std::valarray<bool> test = ( img1.data != val );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, img1[i] == val
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator==(const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val == img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, val <= img1[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator<=(const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val <= img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, val <= img1[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator< (const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val < img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, val >= img1[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator>=(const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val >= img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, val > img1[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator> (const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val > img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    
    /**
     *  @brief Compare fits images to a numerical offset.
     *  @details Compare, pixel by pixel, the content of the FITSimg img1 to a numerical value.
     *  @param img1: Images to be compared to val
     *  @param val: Value to be compared to img1
     *  @return An array of boolean with, for each pixels, val != img1[i]
     *  @note Masked pixel aren't not concidered and will always return false.
     */
    template< typename T >
    const std::valarray<bool> operator!=(const T& val, const FITSimg<T>& img1)
    {
        std::valarray<bool> test = ( val != img1.data );
        test[img1.mask] = 0;
        
        return test;
    }
    

}

#endif

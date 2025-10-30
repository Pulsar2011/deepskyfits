//
//  FITSimg.h++
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


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
#include <algorithm>

#include <thread>
#include <mutex>
#include <future>

#include "FITShdu.h"
#include "FITSexception.h"
#include "FITSstatistic.h"
#include "FITSdata.h" // <- add include for FitsArrayBase / FitsArray
#include "FITSwcs.h"

namespace DSL
{
    typedef std::valarray<bool> pxMask;
    
#pragma region - FITScube class definition
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
#pragma region * Protected member
        pxMask mask;
        FITShdu hdu;                              //!< Header of the image
        std::unique_ptr<DSL::FitsArrayBase> data; // Polymorphic valarray wrapper
        
        std::vector<size_t> Naxis;                //!< Dimenssion of the image axis
        int eqBITPIX, BITPIX;                 //!< Type of data contained into the image
        std::string name;                         //!< Extension name
        int img_status;

        FITSwcs fwcs;

        // helper: call fn with std::valarray<T>& when underlying storage is T; returns true if matched
        template<typename U, typename Fn>
        bool WithTypedData(Fn&& fn) const
        {
            if(!data)
                return false;
            
            // DataArrayBase::applyIfType is non-const; cast away constness only for type check + const ref route
            auto *mutable_ptr = const_cast<DSL::FitsArrayBase*>( data.get() );
            
            return mutable_ptr->template applyIfType<U>([&](std::valarray<U>& arr){fn(arr);});
        }

        template<typename U, typename Fn>
        bool WithTypedData(Fn&& fn)
        {
            if(!data) return false;
            return data->template applyIfType<U>([&](std::valarray<U>& arr){fn(arr);});
        }

#pragma endregion
#pragma region * Protected member function
        static std::vector<size_t> Build_axis(const size_t&, const std::initializer_list<size_t>&);
        
    private:
#pragma endregion
#pragma region * Initialization
        void init();                    //!< Initialization
        virtual void WriteDataCube(const std::shared_ptr<fitsfile>&) =0; //!< pur virtual class methods used by child classes to write data to a fits file on disk

#pragma endregion
#pragma region * Purely virtual & protected methos
    protected:
        virtual void img_init() =0;      //!< Child class initialization
        
#pragma endregion
#pragma region * ctor/dtor
    protected:
        FITScube(const std::shared_ptr<fitsfile>& );            //!< Constructor
        FITScube();
        
    public:
        FITScube(const FITScube&);                              //!< Copy constructor
        virtual ~FITScube();                                    //!< Destructor
        
        
        virtual void Resize(const size_t&, const size_t&, const size_t&, const size_t&) = 0;
        
        static FITScube* UByteFITSimg      (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* ByteFITSimg       (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* UShortFITSimg     (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* ShortFITSimg      (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* IntFITSimg        (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* UIntFITSimg       (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* LongFITSimg       (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* ULongFITSimg      (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* LongLongFITSimg   (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* FloatFITSimg      (const size_t&, const std::initializer_list<size_t>& );
        static FITScube* DoubleFITSimg     (const size_t&, const std::initializer_list<size_t>& );
        
        static FITScube* UByteFITSimg      (const std::vector<size_t>& );
        static FITScube* ByteFITSimg       (const std::vector<size_t>& );
        static FITScube* UShortFITSimg     (const std::vector<size_t>& );
        static FITScube* ShortFITSimg      (const std::vector<size_t>& );
        static FITScube* IntFITSimg        (const std::vector<size_t>& );
        static FITScube* UIntFITSimg       (const std::vector<size_t>& );
        static FITScube* LongFITSimg       (const std::vector<size_t>& );
        static FITScube* ULongFITSimg      (const std::vector<size_t>& );
        static FITScube* LongLongFITSimg   (const std::vector<size_t>& );
        static FITScube* FloatFITSimg      (const std::vector<size_t>& );
        static FITScube* DoubleFITSimg     (const std::vector<size_t>& );

#pragma endregion

#pragma region * Accessor
        size_t Size(const size_t& i = 0) const ;                       //!< Get number of pixel of the axe
        size_t           Nelements() const;                            //!< Get total number of pixel
        inline const int Status()          const {return img_status;}  //!< Get fits error status
        inline const int GetBitPerPixel()  const {return BITPIX;}      //!< Get the number of Bit per image pixel
        inline const int GetEqBitPerPixel()  const {return eqBITPIX;}  //!< Get the number of equivalent Bit per image pixel
        inline const size_t GetDimension() const {return Naxis.size();}
        inline const std::string GetName() const {if(hdu.Exists("EXTNAME")) return hdu.GetValueForKey("EXTNAME"); else return std::string("NO NAME");}       //!< Get the name of the image

        inline double at(const size_t& i) const { if(data == nullptr) throw FITSexception(SHARED_NOMEM,"FITScube","at","No data in memory"); return data->get(i); }
        
        virtual worldCoords             WorldCoordinates(const size_t&, const int& wcsIndex=0) const; //!< Get world coordinates
        virtual worldCoords             WorldCoordinates(const std::vector<size_t>&, const int& wcsIndex=0) const; //!< Get world coordinates
        virtual worldCoords             WorldCoordinates(const pixelCoords&, const int& wcsIndex=0) const; //!< Get world coordinates

        virtual worldVectors            WorldCoordinatesVector(const std::valarray<size_t>&, const int& wcsIndex=0) const; //!< Get world 
        virtual worldVectors            WorldCoordinatesVector(const std::initializer_list<size_t>&, const int& wcsIndex=0) const; //!< Get world coordinates
        virtual worldVectors            WorldCoordinatesVector(const std::vector<size_t>&, const int& wcsIndex=0) const; //!< Get world coordinates
        virtual worldVectors            WorldCoordinatesVector(const pixelVectors&, const int& wcsIndex=0) const; //!< Get world coordinates
        
        virtual pixelCoords World2Pixel( const worldCoords&, const int& wcsIndex=0) const; //!< Get pixel coordinates based on WCS
        virtual std::valarray<size_t> World2PixelArray( const worldVectors&, const int& wcsIndex=0) const; //!< Get pixel coordinates based on WCS
        virtual pixelVectors World2PixelVector( const worldVectors&, const int& wcsIndex=0) const; //!< Get pixel coordinates based on WCS
        
        virtual std::vector<size_t> PixelCoordinates(const size_t&) const; //!< Get pixel coordinates
        
        /**
         *  Obtain the pixel index from the 1D FITS datacube based on the cartesian coordinate of the pixel
         *  @param iPx: Cartésian coordinates of the pixel
         *  @return Pixel index in the 1D FITS datacube
         */
        template<typename T>
        size_t PixelIndex(const std::initializer_list<T>& arg) const           //!< Get pixel index
        {

            // fast path when caller already passes size_t coordinates
            if constexpr (std::is_same_v<T, size_t>)
            {
                // direct construct vector from initializer_list (one allocation)
                std::vector<size_t> pix_index(arg.begin(), arg.end());
                return PixelIndex(pix_index);
            }
            else
            {
                // reserve to avoid repeated reallocations
                std::vector<size_t> pix_index;
                pix_index.reserve(arg.size());
            
                if constexpr (std::is_floating_point_v<T>)
                    for (T v : arg) pix_index.push_back(static_cast<size_t>(std::llround(v)));
                else if constexpr (std::is_integral_v<T>)
                    for (T v : arg) pix_index.push_back(static_cast<size_t>(v));
                else
                    static_assert(always_false_v<T>, "PixelIndex initializer_list must hold integral or floating types convertible to size_t");

                return PixelIndex(pix_index);
            }
        }
        
        virtual size_t PixelIndex(const std::vector<size_t>&) const;           //!< Get pixel index
        virtual size_t PixelIndex(const std::vector<double>&) const;
        
        inline const FITShdu& HDU() const {return hdu;}
        inline       FITShdu& HDU() {return hdu;}

        inline size_t getNumberOfWCS() const {return fwcs.getNumberOfWCS(); }
        
        virtual double GetSum ()                   const =0;
        virtual double GetMean()                   const =0;
        virtual double GetQuadraticMean()          const =0;
        virtual double GetVariance()               const =0;
        virtual double GetStdDev()                 const =0;
        virtual double GetRMS()                    const =0;
        virtual double GetRMSE()                   const =0;
        virtual double GetMedian()                 const =0;
        virtual double GetMinimum()                const =0;
        virtual double GetMaximum()                const =0;
        virtual double Getpercentil(const double&) const =0;
        virtual double Get5thpercentil()           const =0;
        virtual double Get25thpercentil()          const =0;
        virtual double Get75thpercentil()          const =0;
        virtual double Get95thpercentil()          const =0;
        virtual double GetKurtosis()               const =0;
        virtual double GetSkewness()               const =0;
        
        inline std::valarray<bool> GetMask() const {return mask;}
        
        virtual uint8_t  UInt8ValueAtPixel    (const size_t&) const =0;
        virtual uint8_t  UByteValueAtPixel    (const size_t&) const =0;
        virtual int8_t   Int8ValueAtPixel     (const size_t&) const =0;
        virtual int8_t   ByteValueAtPixel     (const size_t&) const =0;
        virtual uint16_t UInt16ValueAtPixel   (const size_t&) const =0;
        virtual uint16_t UShortValueAtPixel   (const size_t&) const =0;
        virtual int16_t  Int16ValueAtPixel    (const size_t&) const =0;
        virtual int16_t  ShortValueAtPixel    (const size_t&) const =0;
        virtual uint32_t UInt32ValueAtPixel   (const size_t&) const =0;
        virtual uint32_t ULongValueAtPixel    (const size_t&) const =0;
        virtual int32_t  Int32ValueAtPixel    (const size_t&) const =0;
        virtual int32_t  LongValueAtPixel     (const size_t&) const =0;
        virtual uint64_t UInt64ValueAtPixel   (const size_t&) const =0;
        virtual uint64_t ULongLongValueAtPixel(const size_t&) const =0;
        virtual int64_t  Int64ValueAtPixel    (const size_t&) const =0;
        virtual int64_t  LongLongValueAtPixel (const size_t&) const =0;
        virtual float    FloatValueAtPixel    (const size_t&) const =0;
        virtual double   DoubleValueAtPixel   (const size_t&) const =0;


        virtual uint8_t  UInt8ValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual uint8_t  UByteValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual int8_t   Int8ValueAtPixel      (const std::initializer_list<size_t>&) const = 0;
        virtual int8_t   ByteValueAtPixel      (const std::initializer_list<size_t>&) const = 0;
        virtual uint16_t UInt16ValueAtPixel    (const std::initializer_list<size_t>&) const = 0;
        virtual uint16_t UShortValueAtPixel    (const std::initializer_list<size_t>&) const = 0;
        virtual int16_t  Int16ValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual int16_t  ShortValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual uint32_t UInt32ValueAtPixel    (const std::initializer_list<size_t>&) const = 0;
        virtual uint32_t ULongValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual int32_t  Int32ValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual int32_t  LongValueAtPixel      (const std::initializer_list<size_t>&) const = 0;
        virtual int64_t  Int64ValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual int64_t  LongLongValueAtPixel  (const std::initializer_list<size_t>&) const = 0;
        virtual uint64_t UInt64ValueAtPixel    (const std::initializer_list<size_t>&) const = 0;
        virtual uint64_t ULongLongValueAtPixel (const std::initializer_list<size_t>&) const = 0;
        virtual float    FloatValueAtPixel     (const std::initializer_list<size_t>&) const = 0;
        virtual double   DoubleValueAtPixel    (const std::initializer_list<size_t>&) const = 0;
        

        // Typed accessors: return pointer to the internal typed valarray<T> managed by FitsArray<T>.
        // Returns nullptr if data is not present or the stored type doesn't match T.
        template<typename T>
        const std::valarray<T>* GetData() const
        {
            if(!data) return nullptr;
            const std::valarray<T>* ptr = nullptr;
            // WithTypedData(const) casts away constness internally and invokes the lambda with non-const ref.
            WithTypedData<T>([&](std::valarray<T>& arr){ ptr = &arr; });
            return ptr;
        }

        template<typename T>
        std::valarray<T>* GetData()
        {
            if(!data) return nullptr;
            std::valarray<T>* ptr = nullptr;
            WithTypedData<T>([&](std::valarray<T>& arr){ ptr = &arr; });
            return ptr;
        }

#pragma endregion
#pragma region * I/O
        
        void Write(const std::shared_ptr<fitsfile>& );                          //!< Write FITS image to a fitsfile
        void Write(std::string, bool replace = false);  //!< Write FITS image to a new fitsfile
        
#pragma endregion
#pragma region * Modifier
        void SetAxisLength(const size_t& n, const size_t& size);
        void DeleteLastAxis();
        void BitPerPixel(const int& n) {BitPerPixel(n,0);}
        void BitPerPixel(const int&, const int& eq);
        void SetName    (const std::string&);

        virtual void SetPixelValue(const  uint8_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const   int8_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const uint16_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const  int16_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const uint32_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const  int32_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const uint64_t&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const  int64_t&, const std::vector<size_t>&) =0;
#ifdef Darwinx86_64
        virtual void SetPixelValue(const   size_t&, const std::vector<size_t>&) =0;
#endif
        virtual void SetPixelValue(const    float&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const   double&, const std::vector<size_t>&) =0;
        virtual void SetPixelValue(const  uint8_t&, const size_t&)        =0;
        virtual void SetPixelValue(const   int8_t&, const size_t&)        =0;
        virtual void SetPixelValue(const uint16_t&, const size_t&)        =0;
        virtual void SetPixelValue(const  int16_t&, const size_t&)        =0;
        virtual void SetPixelValue(const uint32_t&, const size_t&)        =0;
        virtual void SetPixelValue(const  int32_t&, const size_t&)        =0;
        virtual void SetPixelValue(const uint64_t&, const size_t&)        =0;
        virtual void SetPixelValue(const  int64_t&, const size_t&)        =0;
#ifdef Darwinx86_64
        virtual void SetPixelValue(const   size_t&, const size_t&)        =0;
#endif
        virtual void SetPixelValue(const    float&, const size_t&)        =0;
        virtual void SetPixelValue(const   double&, const size_t&)        =0;


        virtual void Bscale(const  uint8_t& )  =0;
        virtual void Bscale(const   int8_t& )  =0;
        virtual void Bscale(const uint16_t& )  =0;
        virtual void Bscale(const  int16_t& )  =0;
        virtual void Bscale(const uint32_t& )  =0;
        virtual void Bscale(const  int32_t& )  =0;
        virtual void Bscale(const uint64_t& )  =0;
        virtual void Bscale(const  int64_t& )  =0;
#ifdef Darwinx86_64
        virtual void Bscale(const size_t&   )  =0;
#endif
        virtual void Bscale(const float&    )  =0;
        virtual void Bscale(const double&   )  =0;
        
        virtual void Bzero(const  uint8_t& )   =0;
        virtual void Bzero(const   int8_t& )   =0;
        virtual void Bzero(const uint16_t& )   =0;
        virtual void Bzero(const  int16_t& )   =0;
        virtual void Bzero(const uint32_t& )   =0;
        virtual void Bzero(const  int32_t& )   =0;
        virtual void Bzero(const uint64_t& )   =0;
        virtual void Bzero(const  int64_t& )   =0;
#ifdef Darwinx86_64
        virtual void Bzero(const size_t&   )   =0;
#endif
        virtual void Bzero(const float&    )   =0;
        virtual void Bzero(const double&   )   =0;

        virtual void Blank(const  uint8_t& )   =0;
        virtual void Blank(const   int8_t& )   =0;
        virtual void Blank(const uint16_t& )   =0;
        virtual void Blank(const  int16_t& )   =0;
        virtual void Blank(const uint32_t& )   =0;
        virtual void Blank(const  int32_t& )   =0;
        virtual void Blank(const uint64_t& )   =0;
        virtual void Blank(const  int64_t& )   =0;
#ifdef Darwinx86_64
        virtual void Blank(const size_t&   )   =0;
#endif
        virtual void Blank(const float&    )   =0;
        virtual void Blank(const double&   )   =0;
        
#pragma endregion
#pragma region * Operator

#pragma region -- Operator +=
        virtual void operator+=(const  uint8_t& val){};
        virtual void operator+=(const   int8_t& val){};
        virtual void operator+=(const uint16_t& val){};
        virtual void operator+=(const  int16_t& val){};
        virtual void operator+=(const uint32_t& val){};
        virtual void operator+=(const  int32_t& val){};
        virtual void operator+=(const uint64_t& val){};
        virtual void operator+=(const  int64_t& val){};
#ifdef Darwinx86_64
        virtual void operator+=(const   size_t& val){};
#endif
        virtual void operator+=(const    float& val){};
        virtual void operator+=(const   double& val){};

        virtual void operator+=(const std::valarray< uint8_t>& val){};
        virtual void operator+=(const std::valarray<  int8_t>& val){};
        virtual void operator+=(const std::valarray<uint16_t>& val){};
        virtual void operator+=(const std::valarray< int16_t>& val){};
        virtual void operator+=(const std::valarray<uint32_t>& val){};
        virtual void operator+=(const std::valarray< int32_t>& val){};
        virtual void operator+=(const std::valarray<uint64_t>& val){};
        virtual void operator+=(const std::valarray< int64_t>& val){};
#ifdef Darwinx86_64
        virtual void operator+=(const std::valarray<  size_t>& val){};
#endif
        virtual void operator+=(const std::valarray<   float>& val){};
        virtual void operator+=(const std::valarray<  double>& val){};

        virtual void operator+=(const FITScube&){};

#pragma endregion
#pragma region -- Operator -=
        virtual void operator-=(const  uint8_t& val){};
        virtual void operator-=(const   int8_t& val){};
        virtual void operator-=(const uint16_t& val){};
        virtual void operator-=(const  int16_t& val){};
        virtual void operator-=(const uint32_t& val){};
        virtual void operator-=(const  int32_t& val){};
        virtual void operator-=(const uint64_t& val){};
        virtual void operator-=(const  int64_t& val){};
#ifdef Darwinx86_64
        virtual void operator-=(const   size_t& val){};
#endif
        virtual void operator-=(const    float& val){};
        virtual void operator-=(const   double& val){};

        virtual void operator-=(const std::valarray< uint8_t>& val){};
        virtual void operator-=(const std::valarray<  int8_t>& val){};
        virtual void operator-=(const std::valarray<uint16_t>& val){};
        virtual void operator-=(const std::valarray< int16_t>& val){};
        virtual void operator-=(const std::valarray<uint32_t>& val){};
        virtual void operator-=(const std::valarray< int32_t>& val){};
        virtual void operator-=(const std::valarray<uint64_t>& val){};
        virtual void operator-=(const std::valarray< int64_t>& val){};
#ifdef Darwinx86_64
        virtual void operator-=(const std::valarray<  size_t>& val){};
#endif
        virtual void operator-=(const std::valarray<   float>& val){};
        virtual void operator-=(const std::valarray<  double>& val){};

        virtual void operator-=(const FITScube&){};
#pragma endregion
#pragma region -- Operator *=
        virtual void operator*=(const  uint8_t& val){};
        virtual void operator*=(const   int8_t& val){};
        virtual void operator*=(const uint16_t& val){};
        virtual void operator*=(const  int16_t& val){};
        virtual void operator*=(const uint32_t& val){};
        virtual void operator*=(const  int32_t& val){};
        virtual void operator*=(const uint64_t& val){};
        virtual void operator*=(const  int64_t& val){};
#ifdef Darwinx86_64
        virtual void operator*=(const   size_t& val){};
#endif
        virtual void operator*=(const    float& val){};
        virtual void operator*=(const   double& val){};

        virtual void operator*=(const std::valarray< uint8_t>& val){};
        virtual void operator*=(const std::valarray<  int8_t>& val){};
        virtual void operator*=(const std::valarray<uint16_t>& val){};
        virtual void operator*=(const std::valarray< int16_t>& val){};
        virtual void operator*=(const std::valarray<uint32_t>& val){};
        virtual void operator*=(const std::valarray< int32_t>& val){};
        virtual void operator*=(const std::valarray<uint64_t>& val){};
        virtual void operator*=(const std::valarray< int64_t>& val){};
#ifdef Darwinx86_64
        virtual void operator*=(const std::valarray<  size_t>& val){};
#endif
        virtual void operator*=(const std::valarray<   float>& val){};
        virtual void operator*=(const std::valarray<  double>& val){};

        virtual void operator*=(const FITScube&){};
#pragma endregion
#pragma region -- Operator /=
        virtual void operator/=(const  uint8_t& val){};
        virtual void operator/=(const   int8_t& val){};
        virtual void operator/=(const uint16_t& val){};
        virtual void operator/=(const  int16_t& val){};
        virtual void operator/=(const uint32_t& val){};
        virtual void operator/=(const  int32_t& val){};
        virtual void operator/=(const uint64_t& val){};
        virtual void operator/=(const  int64_t& val){};
#ifdef Darwinx86_64
        virtual void operator/=(const   size_t& val){};
#endif
        virtual void operator/=(const    float& val){};
        virtual void operator/=(const   double& val){};

        virtual void operator/=(const std::valarray< uint8_t>& val){};
        virtual void operator/=(const std::valarray<  int8_t>& val){};
        virtual void operator/=(const std::valarray<uint16_t>& val){};
        virtual void operator/=(const std::valarray< int16_t>& val){};
        virtual void operator/=(const std::valarray<uint32_t>& val){};
        virtual void operator/=(const std::valarray< int32_t>& val){};
        virtual void operator/=(const std::valarray<uint64_t>& val){};
        virtual void operator/=(const std::valarray< int64_t>& val){};
#ifdef Darwinx86_64
        virtual void operator/=(const std::valarray<  size_t>& val){};
#endif
        virtual void operator/=(const std::valarray<   float>& val){};
        virtual void operator/=(const std::valarray<  double>& val){};

        virtual void operator/=(const FITScube&){};
#pragma endregion
    
#pragma endregion
#pragma region * Altere data
        
        void MaskPixels(const std::initializer_list<size_t>&);
        void MaskPixels(const std::valarray<bool>&);
        
        void UnmaskPixels(const std::initializer_list<size_t>&);
        void UnmaskPixels(const std::valarray<bool>&);
        
        bool Masked(const std::initializer_list<size_t>&) const;
        bool Masked(const std::vector<size_t>&) const;
        bool Masked(size_t) const;
        
        
        
#pragma endregion
#pragma region * Pure virtual methods
        
        virtual std::shared_ptr<FITScube> Layer(const size_t&) const  =0;
        virtual std::shared_ptr<FITScube> Window (size_t, size_t, size_t, size_t) const =0;
        virtual std::shared_ptr<FITScube> Rebin  (const std::initializer_list<size_t>& l, bool doMean=false) const {return Rebin(std::vector<size_t>(l),doMean);};
        virtual std::shared_ptr<FITScube> Rebin  (const std::vector<size_t>& , bool doMean=false) const =0;
        
        virtual void Print() const {};
        
    };
#pragma endregion
#pragma endregion
#pragma region - FITSimg class definition
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
        
        T BSCALE;   //!< Data scalling factor
        T BZERO;    //!< Data offset
        T BLANK;    //!< Transparent pixel reference value

#pragma region * Initialization
        virtual void img_init();
        void template_init();
        
#pragma endregion
#pragma region * I/O
        void WriteDataCube(const std::shared_ptr<fitsfile>& );  //!< Write data to Fits HDU
        template<typename S> void WriteData(const std::shared_ptr<fitsfile>& , int);
    
    public:
#pragma endregion
#pragma region * ctor/dtor
        
        FITSimg(const std::shared_ptr<fitsfile>& fptr  );      //!< Default constructor
        FITSimg(const FITSimg<T>&);     //!< Copy constructor
        FITSimg(const size_t&, const std::initializer_list<size_t>&);
        FITSimg(std::vector<size_t>);
        virtual ~FITSimg();             //!< Destructor
        
#pragma endregion
#pragma region * Modifier
        void Bscale(const  uint8_t& );
        void Bscale(const   int8_t& );
        void Bscale(const uint16_t& );
        void Bscale(const  int16_t& );
        void Bscale(const uint32_t& );
        void Bscale(const  int32_t& );
        void Bscale(const uint64_t& );
        void Bscale(const  int64_t& );
#ifdef Darwinx86_64
        void Bscale(const size_t&   );
#endif
        void Bscale(const float&    );
        void Bscale(const double&   );
        
        void Bzero(const  uint8_t& );
        void Bzero(const   int8_t& );
        void Bzero(const uint16_t& );
        void Bzero(const  int16_t& );
        void Bzero(const uint32_t& );
        void Bzero(const  int32_t& );
        void Bzero(const uint64_t& );
        void Bzero(const  int64_t& );
#ifdef Darwinx86_64
        void Bzero(const size_t&   );
#endif
        void Bzero(const float&    );
        void Bzero(const double&   );

        void Blank(const  uint8_t& );
        void Blank(const   int8_t& );
        void Blank(const uint16_t& );
        void Blank(const  int16_t& );
        void Blank(const uint32_t& );
        void Blank(const  int32_t& );
        void Blank(const uint64_t& );
        void Blank(const  int64_t& );
#ifdef Darwinx86_64
        void Blank(const size_t&   );
#endif
        void Blank(const float&    );
        void Blank(const double&   );
        
        void AddLayer(const FITSimg<T>&);

#pragma endregion

#pragma region * Statistical property
        double GetSum ()                   const;
        double GetMean()                   const;
        double GetQuadraticMean()          const;
        double GetVariance()               const;
        double GetStdDev()                 const;
        double GetRMS()                    const;
        double GetRMSE()                   const;
        double GetMedian()                 const;
        double GetMinimum()                const;
        double GetMaximum()                const;
        double Getpercentil(const double&) const;
        double Get5thpercentil()           const;
        double Get25thpercentil()          const;
        double Get75thpercentil()          const;
        double Get95thpercentil()          const;
        double GetKurtosis()               const;
        double GetSkewness()               const;

#pragma endregion
#pragma region * data operation

        
    private:
        template< typename S > void ReadArray(const std::shared_ptr<fitsfile>& fptr); //!< Read and interpret FITS BINARY data
        
    public:
        
        FITSimg<T>& operator=(const FITSimg<T>&);		///< Get pixel content
        
        const T operator [](const size_t&) const;  ///< Get pixel content
              T operator [](const size_t&);	      ///< Get pixel content
        
        uint8_t      UInt8ValueAtPixel   (const size_t& k) const {return UByteValueAtPixel   (k);}
        uint8_t      UByteValueAtPixel   (const size_t&) const;

        int8_t       Int8ValueAtPixel    (const size_t& k) const {return ByteValueAtPixel    (k);}
        int8_t       ByteValueAtPixel    (const size_t&) const;

        uint16_t     UInt16ValueAtPixel  (const size_t& k) const {return UShortValueAtPixel  (k);}
        uint16_t     UShortValueAtPixel  (const size_t&) const;

        int16_t      Int16ValueAtPixel   (const size_t& k) const {return ShortValueAtPixel   (k);}
        int16_t      ShortValueAtPixel   (const size_t&) const;

        uint32_t     UInt32ValueAtPixel  (const size_t& k) const {return ULongValueAtPixel   (k);}
        uint32_t     ULongValueAtPixel   (const size_t&) const;

        int32_t      Int32ValueAtPixel   (const size_t& k) const {return LongValueAtPixel    (k);}
        int32_t      LongValueAtPixel    (const size_t&) const;

        int64_t      Int64ValueAtPixel  (const size_t& k) const {return LongLongValueAtPixel(k);}
        int64_t      LongLongValueAtPixel(const size_t&) const;

        uint64_t     UInt64ValueAtPixel   (const size_t& k) const {return ULongLongValueAtPixel(k);}
        uint64_t     ULongLongValueAtPixel(const size_t&) const;

        float        FloatValueAtPixel   (const size_t&) const;
        double       DoubleValueAtPixel  (const size_t&) const;

        uint8_t      UInt8ValueAtPixel   (const std::initializer_list<size_t>& k) const {return UByteValueAtPixel(k);}
        uint8_t      UByteValueAtPixel   (const std::initializer_list<size_t>&) const ;

        int8_t       Int8ValueAtPixel    (const std::initializer_list<size_t>& k) const {return ByteValueAtPixel(k);}
        int8_t       ByteValueAtPixel    (const std::initializer_list<size_t>&) const ;

        uint16_t     UInt16ValueAtPixel  (const std::initializer_list<size_t>& k) const {return UShortValueAtPixel(k);}
        uint16_t     UShortValueAtPixel  (const std::initializer_list<size_t>&) const ;

        int16_t      Int16ValueAtPixel   (const std::initializer_list<size_t>& k) const {return ShortValueAtPixel(k);}
        int16_t      ShortValueAtPixel   (const std::initializer_list<size_t>&) const ;

        uint32_t     UInt32ValueAtPixel  (const std::initializer_list<size_t>& k) const {return ULongValueAtPixel(k);}
        uint32_t     ULongValueAtPixel   (const std::initializer_list<size_t>&) const ;

        int32_t      Int32ValueAtPixel   (const std::initializer_list<size_t>& k) const {return LongValueAtPixel(k);}
        int32_t      LongValueAtPixel    (const std::initializer_list<size_t>&) const ;

        int64_t      Int64ValueAtPixel   (const std::initializer_list<size_t>& k) const {return LongLongValueAtPixel(k);}
        int64_t      LongLongValueAtPixel(const std::initializer_list<size_t>&) const ;

        uint64_t      UInt64ValueAtPixel   (const std::initializer_list<size_t>& k) const {return ULongLongValueAtPixel(k);}
        uint64_t      ULongLongValueAtPixel(const std::initializer_list<size_t>&) const ;


        float        FloatValueAtPixel   (const std::initializer_list<size_t>&) const ;
        double       DoubleValueAtPixel  (const std::initializer_list<size_t>&) const ;

#pragma region -- Operator +=
        template<typename S>
        void operator+= (const S&);///< Apply positive offset to data

        void operator+=(const  uint8_t& val){return operator+=< uint8_t>(val);}
        void operator+=(const   int8_t& val){return operator+=<  int8_t>(val);}
        void operator+=(const uint16_t& val){return operator+=<uint16_t>(val);}
        void operator+=(const  int16_t& val){return operator+=< int16_t>(val);}
        void operator+=(const uint32_t& val){return operator+=<uint32_t>(val);}
        void operator+=(const  int32_t& val){return operator+=< int32_t>(val);}
        void operator+=(const uint64_t& val){return operator+=<uint64_t>(val);}
        void operator+=(const  int64_t& val){return operator+=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator+=(const   size_t& val){return operator+=<  size_t>(val);}
#endif
        void operator+=(const    float& val){return operator+=<   float>(val);}
        void operator+=(const   double& val){return operator+=<  double>(val);}

        template<typename S>
        void operator+= (const std::valarray<S>&);///< Scale up data

        void operator+=(const std::valarray< uint8_t>& val){return operator+=< uint8_t>(val);}
        void operator+=(const std::valarray<  int8_t>& val){return operator+=<  int8_t>(val);}
        void operator+=(const std::valarray<uint16_t>& val){return operator+=<uint16_t>(val);}
        void operator+=(const std::valarray< int16_t>& val){return operator+=< int16_t>(val);}
        void operator+=(const std::valarray<uint32_t>& val){return operator+=<uint32_t>(val);}
        void operator+=(const std::valarray< int32_t>& val){return operator+=< int32_t>(val);}
        void operator+=(const std::valarray<uint64_t>& val){return operator+=<uint64_t>(val);}
        void operator+=(const std::valarray< int64_t>& val){return operator+=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator+=(const std::valarray<  size_t>& val){return operator+=<  size_t>(val);}
#endif
        void operator+=(const std::valarray<   float>& val){return operator+=<   float>(val);}
        void operator+=(const std::valarray<  double>& val){return operator+=<  double>(val);}

        template<typename S>
        void operator+= (const FITSimg<S>&);///< Multiply images

        void operator+=(const FITScube&);

#pragma endregion
#pragma region -- Operator -=
        template<typename S>
        void operator-= (const S&);///< Apply negative offset to data

        void operator-=(const  uint8_t& val){return operator-=< uint8_t>(val);}
        void operator-=(const   int8_t& val){return operator-=<  int8_t>(val);}
        void operator-=(const uint16_t& val){return operator-=<uint16_t>(val);}
        void operator-=(const  int16_t& val){return operator-=< int16_t>(val);}
        void operator-=(const uint32_t& val){return operator-=<uint32_t>(val);}
        void operator-=(const  int32_t& val){return operator-=< int32_t>(val);}
        void operator-=(const uint64_t& val){return operator-=<uint64_t>(val);}
        void operator-=(const  int64_t& val){return operator-=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator-=(const   size_t& val){return operator-=<  size_t>(val);}
#endif
        void operator-=(const    float& val){return operator-=<   float>(val);}
        void operator-=(const   double& val){return operator-=<  double>(val);}

        template<typename S>
        void operator-= (const std::valarray<S>&);///< Scale up data

        void operator-=(const std::valarray< uint8_t>& val){return operator-=< uint8_t>(val);}
        void operator-=(const std::valarray<  int8_t>& val){return operator-=<  int8_t>(val);}
        void operator-=(const std::valarray<uint16_t>& val){return operator-=<uint16_t>(val);}
        void operator-=(const std::valarray< int16_t>& val){return operator-=< int16_t>(val);}
        void operator-=(const std::valarray<uint32_t>& val){return operator-=<uint32_t>(val);}
        void operator-=(const std::valarray< int32_t>& val){return operator-=< int32_t>(val);}
        void operator-=(const std::valarray<uint64_t>& val){return operator-=<uint64_t>(val);}
        void operator-=(const std::valarray< int64_t>& val){return operator-=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator-=(const std::valarray<  size_t>& val){return operator-=<  size_t>(val);}
#endif
        void operator-=(const std::valarray<   float>& val){return operator-=<   float>(val);}
        void operator-=(const std::valarray<  double>& val){return operator-=<  double>(val);}

        template<typename S>
        void operator-= (const FITSimg<S>&);///< Multiply images

        void operator-=(const FITScube&);

#pragma endregion
#pragma region -- Operator *=
        template<typename S>
        void operator*= (const S&);///< Scale up data

        void operator*=(const  uint8_t& val){return operator*=< uint8_t>(val);}
        void operator*=(const   int8_t& val){return operator*=<  int8_t>(val);}
        void operator*=(const uint16_t& val){return operator*=<uint16_t>(val);}
        void operator*=(const  int16_t& val){return operator*=< int16_t>(val);}
        void operator*=(const uint32_t& val){return operator*=<uint32_t>(val);}
        void operator*=(const  int32_t& val){return operator*=< int32_t>(val);}
        void operator*=(const uint64_t& val){return operator*=<uint64_t>(val);}
        void operator*=(const  int64_t& val){return operator*=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator*=(const   size_t& val){return operator*=<  size_t>(val);}
#endif
        void operator*=(const    float& val){return operator*=<   float>(val);}
        void operator*=(const   double& val){return operator*=<  double>(val);}

        template<typename S>
        void operator*= (const std::valarray<S>&);///< Scale up data

        void operator*=(const std::valarray< uint8_t>& val){return operator*=< uint8_t>(val);}
        void operator*=(const std::valarray<  int8_t>& val){return operator*=<  int8_t>(val);}
        void operator*=(const std::valarray<uint16_t>& val){return operator*=<uint16_t>(val);}
        void operator*=(const std::valarray< int16_t>& val){return operator*=< int16_t>(val);}
        void operator*=(const std::valarray<uint32_t>& val){return operator*=<uint32_t>(val);}
        void operator*=(const std::valarray< int32_t>& val){return operator*=< int32_t>(val);}
        void operator*=(const std::valarray<uint64_t>& val){return operator*=<uint64_t>(val);}
        void operator*=(const std::valarray< int64_t>& val){return operator*=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator*=(const std::valarray<  size_t>& val){return operator*=<  size_t>(val);}
#endif
        void operator*=(const std::valarray<   float>& val){return operator*=<   float>(val);}
        void operator*=(const std::valarray<  double>& val){return operator*=<  double>(val);}

        template<typename S>
        void operator*= (const FITSimg<S>&);///< Multiply images

        void operator*=(const FITScube&);

#pragma endregion
#pragma region -- Operator /=
        template<typename S>
        void operator/= (const S&);///< Scale down data

        void operator/=(const  uint8_t& val){return operator/=< uint8_t>(val);};
        void operator/=(const   int8_t& val){return operator/=<  int8_t>(val);};
        void operator/=(const uint16_t& val){return operator/=<uint16_t>(val);};
        void operator/=(const  int16_t& val){return operator/=< int16_t>(val);};
        void operator/=(const uint32_t& val){return operator/=<uint32_t>(val);};
        void operator/=(const  int32_t& val){return operator/=< int32_t>(val);};
        void operator/=(const uint64_t& val){return operator/=<uint64_t>(val);};
        void operator/=(const  int64_t& val){return operator/=< int64_t>(val);};
#ifdef Darwinx86_64
        void operator/=(const   size_t& val){return operator/=<  size_t>(val);};
#endif
        void operator/=(const    float& val){return operator/=<   float>(val);};
        void operator/=(const   double& val){return operator/=<  double>(val);};

        template<typename S>
        void operator/= (const std::valarray<S>&);///< Scale up data

        void operator/=(const std::valarray< uint8_t>& val){return operator/=< uint8_t>(val);}
        void operator/=(const std::valarray<  int8_t>& val){return operator/=<  int8_t>(val);}
        void operator/=(const std::valarray<uint16_t>& val){return operator/=<uint16_t>(val);}
        void operator/=(const std::valarray< int16_t>& val){return operator/=< int16_t>(val);}
        void operator/=(const std::valarray<uint32_t>& val){return operator/=<uint32_t>(val);}
        void operator/=(const std::valarray< int32_t>& val){return operator/=< int32_t>(val);}
        void operator/=(const std::valarray<uint64_t>& val){return operator/=<uint64_t>(val);}
        void operator/=(const std::valarray< int64_t>& val){return operator/=< int64_t>(val);}
#ifdef Darwinx86_64
        void operator/=(const std::valarray<  size_t>& val){return operator/=<  size_t>(val);}
#endif
        void operator/=(const std::valarray<   float>& val){return operator/=<   float>(val);}
        void operator/=(const std::valarray<  double>& val){return operator/=<  double>(val);}

        template<typename S>
        void operator/=(const FITSimg<S>&);///< Multiply images

        void operator/=(const FITScube&);

#pragma endregion

        void SetPixelValue(const  uint8_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue< uint8_t>(val,vPx);}
        void SetPixelValue(const   int8_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<  int8_t>(val,vPx);}
        void SetPixelValue(const uint16_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<uint16_t>(val,vPx);}
        void SetPixelValue(const  int16_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue< int16_t>(val,vPx);}
        void SetPixelValue(const uint32_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<uint32_t>(val,vPx);}
        void SetPixelValue(const  int32_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue< int32_t>(val,vPx);}
        void SetPixelValue(const uint64_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<uint64_t>(val,vPx);}
        void SetPixelValue(const  int64_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue< int64_t>(val,vPx);}
#ifdef Darwinx86_64
        void SetPixelValue(const   size_t& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<  size_t>(val,vPx);}
#endif
        void SetPixelValue(const    float& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<   float>(val,vPx);}
        void SetPixelValue(const   double& val, const std::vector<size_t>& vPx) {this->template SetPixelValue<  double>(val,vPx);}
        void SetPixelValue(const  uint8_t& val, const size_t& iPx)              {this->template SetPixelValue< uint8_t>(val,iPx);}
        void SetPixelValue(const   int8_t& val, const size_t& iPx)              {this->template SetPixelValue<  int8_t>(val,iPx);}
        void SetPixelValue(const uint16_t& val, const size_t& iPx)              {this->template SetPixelValue<uint16_t>(val,iPx);}
        void SetPixelValue(const  int16_t& val, const size_t& iPx)              {this->template SetPixelValue< int16_t>(val,iPx);}
        void SetPixelValue(const uint32_t& val, const size_t& iPx)              {this->template SetPixelValue<uint32_t>(val,iPx);}
        void SetPixelValue(const  int32_t& val, const size_t& iPx)              {this->template SetPixelValue< int32_t>(val,iPx);}
        void SetPixelValue(const uint64_t& val, const size_t& iPx)              {this->template SetPixelValue<uint64_t>(val,iPx);}
        void SetPixelValue(const  int64_t& val, const size_t& iPx)              {this->template SetPixelValue< int64_t>(val,iPx);}
#ifdef Darwinx86_64
        void SetPixelValue(const   size_t& val, const size_t& iPx)              {this->template SetPixelValue<  size_t>(val,iPx);}
#endif
        void SetPixelValue(const    float& val, const size_t& iPx)              {this->template SetPixelValue<   float>(val,iPx);}
        void SetPixelValue(const   double& val, const size_t& iPx)              {this->template SetPixelValue<  double>(val,iPx);}
        
        template<typename S>
        void SetPixelValue(const S&, const std::vector<size_t>&);

        template<typename S>
        void SetPixelValue(const S&, const size_t&);

#pragma endregion
#pragma region * Accessor

        const T& ReadBscale() const {return BSCALE;}
        const T& ReadBzero () const {return BZERO ;}
        const T& ReadBlank () const {return BLANK ;}
    
#pragma endregion
#pragma region * Display methods

#pragma endregion
#pragma region * Data export methods
        
#pragma endregion
#pragma region * Extraction method
        std::shared_ptr<FITScube> Layer(const size_t&) const;
        std::shared_ptr<FITScube> Window (size_t, size_t, size_t, size_t) const;
        std::shared_ptr<FITScube> Rebin  (const std::vector<size_t>&, bool doMean=false ) const;
        
        void Resize(const size_t&, const size_t&, const size_t&, const size_t&);
        
        void Print() const;
#pragma endregion
    };
    
#pragma endregion
#pragma region - FITSimg class implementation
    
#pragma region * Initialization

    template< typename T >
    void FITSimg<T>::template_init()
    {
        BSCALE = static_cast<T>(1);
        BZERO  = static_cast<T>(0);
        
        if constexpr (std::is_floating_point_v<T>)
            BLANK = std::numeric_limits<T>::quiet_NaN();
        else
            BLANK = std::numeric_limits<T>::min();

        // type-specific FITS parameters
        if constexpr (std::is_same_v<T, uint8_t>)
        {
            BitPerPixel(BYTE_IMG);   // unsigned 8 uses BITPIX=8
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else if constexpr (std::is_same_v<T, int8_t>)
        {
            BitPerPixel(8,SBYTE_IMG); // signed 8 uses BITPIX=8, equiv 10
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(-128));
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            BitPerPixel(SHORT_IMG); // signed 16 uses BITPIX=16
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            BitPerPixel(16,USHORT_IMG); // unsigned 16 uses BITPIX=16, equiv 20
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(32768));
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            BitPerPixel(LONG_IMG);     // signed 32 uses BITPIX=32
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            BitPerPixel(32, ULONG_IMG); // unsigned 32 uses BITPIX=32, equiv 40
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>((T)2147483648));
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            BitPerPixel(LONGLONG_IMG); // signed 64 uses BITPIX=64
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, size_t>)
        {
            BitPerPixel(64, ULONGLONG_IMG); // unsigned 64 uses equiv 80
            Bscale((1));
            //Bzero (((T)9223372036854775808));
            Bzero(static_cast<T>(static_cast<uint64_t>(1) << 63));
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            BitPerPixel(FLOAT_IMG);
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            BitPerPixel(DOUBLE_IMG);
            Bscale(static_cast<T>(1));
            Bzero (static_cast<T>(0));
        }
        else
        {
            // fallback: keep defaults but mark BITPIX neutral (0)
            BitPerPixel(0);
        }
    }
    
    /**
     * @details Initialize private variable to decent default value.
     */
    template< typename T >
    void FITSimg<T>::img_init()
    {
        // create typed storage via polymorphic wrapper
        this->data = std::make_unique< DSL::FitsArray<T> >( Nelements() );
        this->mask = pxMask( Nelements() );
    }
    
#pragma endregion
#pragma region * I/O

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
            
        // * GET THE WHOLE IMAGE DATA
        //    - GET PIXEL DIMENSION
        
        if((verbose&verboseLevel::VERBOSE_HDU)==verboseLevel::VERBOSE_HDU)
            hdu.Dump(std::cout);
        
        const long      num_axis    = static_cast<const long>( Naxis.size() );
        const LONGLONG array_size  = static_cast<const long long>( Nelements() );
        
        //    - ALLOCATE BUFFER MEMORY
        int any_null = 0;
        img_status   = 0;
        
        std::vector<LONGLONG> fpixel(static_cast<size_t>(num_axis), 1);
        std::vector<S>        array(static_cast<size_t>(array_size));
        std::vector<char>     null_array(static_cast<size_t>(array_size), 0);
         
        int TTYPE = 0;
        switch (BITPIX)
        {
            case BYTE_IMG:
            case SBYTE_IMG:
                if (eqBITPIX == SBYTE_IMG)
                    TTYPE = TSBYTE;
                else if (eqBITPIX == FLOAT_IMG)
                    TTYPE = TFLOAT;
                else if (eqBITPIX == DOUBLE_IMG)
                    TTYPE = TDOUBLE;
                else
                    TTYPE = TBYTE;
                break;
            
            case SHORT_IMG:
            case USHORT_IMG:
                if (eqBITPIX == USHORT_IMG)
                    TTYPE = TUSHORT;
                else if (eqBITPIX == FLOAT_IMG)
                    TTYPE = TFLOAT;
                else if (eqBITPIX == DOUBLE_IMG)
                    TTYPE = TDOUBLE;
                else
                    TTYPE = TSHORT;
                break;
                
            case LONG_IMG:
            case ULONG_IMG:
                if (eqBITPIX == ULONG_IMG)
                    TTYPE = TUINT;
                else if (eqBITPIX == FLOAT_IMG)
                    TTYPE = TFLOAT;
                else if (eqBITPIX == DOUBLE_IMG)
                    TTYPE = TDOUBLE;
                else
                    TTYPE = TINT;
                break;
                
            case LONGLONG_IMG:
            case ULONGLONG_IMG:
                if (eqBITPIX == ULONGLONG_IMG)
                    TTYPE = TULONGLONG;
                else if (eqBITPIX == FLOAT_IMG)
                    TTYPE = TFLOAT;
                else if (eqBITPIX == DOUBLE_IMG)
                    TTYPE = TDOUBLE;
                else
                    TTYPE = TLONGLONG;
                break;
                
            case FLOAT_IMG:
                TTYPE = TFLOAT;
                break;
                
            case DOUBLE_IMG:
                TTYPE = TDOUBLE;
                break;
                
            default:
                img_status=BAD_BITPIX;
                throw FITSexception(img_status,"FITSimg","ReadArray","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN.");
        }

        if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
        {
            std::cout<<"        \033[32m`-BUFFER DATA TYPE = \033[0m"<<TTYPE<<"  \033[33m["<<TTYPE<<"]\033[0m"<<std::endl;
        }
        
        //    - EXTRACT BINARY DATA AND CONVERT TO NUMERICAL VALUE
        
        try
        {
            if( fits_read_pixnullll(fptr.get(), TTYPE, fpixel.data(), array_size, array.data(), null_array.data(), &any_null, &img_status  ) )
                 throw FITSexception(img_status,"FITSimg<T>","ReadArray");
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            switch (img_status)
            {
                case 410:
                    std::cerr << "runtime type: " << demangle(typeid(S).name()) <<std::endl;
                    std::cerr<<"    |- IMG DATA TYPE : "<<TTYPE<<std::endl
                             <<"    |- this DATA TYPE: "<<BITPIX<<std::endl
                             <<"    `- EQUIV. BITPIX : "<<eqBITPIX<<std::endl;
                    throw;
                    break;
                    
                case 854:
                    std::cerr << "runtime type: " << demangle(typeid(S).name()) <<std::endl;
                    std::cerr<<"    |- IMG PIXEL INDEX IS OUT OF RANGE"<<std::endl;
                    throw;
                    break;
               
                default:
                    break;
            }
            
            std::cerr << "runtime type: " << demangle(typeid(S).name()) <<std::endl;
            std::cerr<<"    |- IMG DATA TYPE : "<<TTYPE<<std::endl
                     <<"    |- BITPIX        : "<<BITPIX<<std::endl
                     <<"    |- EQUIV. BITPIX : "<<eqBITPIX<<std::endl
                     <<"    |- NAXIS         : "<<num_axis<<std::endl;
            
            for(size_t i=0; i < Naxis.size()-1; i++)
            {
                std::cerr<<"    |    |- NAXIS["<<i<<"] : "<<Naxis[i]<<std::endl;
                std::cerr<<"    |    `- NAXIS["<<Naxis.size()-1<<"] : "<<Naxis[Naxis.size()-1]<<std::endl
                         <<"    |- START COO     : "<<std::endl;
            }
            
            for(long i=0; i < num_axis-1; i++)
            {
                std::cerr<<"    |   |- NAXIS"<<i<<"[0] : "<<fpixel[i]<<std::endl;
                std::cerr<<"    |   `- NAXIS"<<num_axis-1<<"[0] : "<<fpixel[num_axis-1]<<std::endl
                         <<"    |- DATA SIZE     : "<<array_size<<std::endl
                         <<"    |- ARRAY[0]      : "<<array[0]<<std::endl
                         <<"    |- null_array[0] : "<<null_array[0]<<std::endl
                         <<"    `- HAS NULL      : "<<any_null;
            }
            
            std::cerr<<"\033[0m"<<std::endl;
            throw;
        }
        
        // typed storage (non-null)
        std::valarray<T>* typed = this->template GetData<T>();
        if(!typed)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>","ReadArray","typed data missing");

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            for(size_t i = 0; i < static_cast<size_t>(array_size); i++)
            {
                if(null_array[i])
                {
                    arr[i] = static_cast<T>( std::numeric_limits<T>::quiet_NaN() );
                    mask[i] = true;
                }
                else
                {
                    arr[i] = static_cast<T>( array[i] );
                    mask[i] = false;
                }
            }
        });   

        return;
    }
    
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
        
        switch (eqBITPIX)
        {
            case BYTE_IMG:
            case SBYTE_IMG:
                if (eqBITPIX == SBYTE_IMG)
                    WriteData<int8_t>(fptr, TSBYTE);
                else
                    WriteData<uint8_t>(fptr, TBYTE);
                break;
                
            case SHORT_IMG:
            case USHORT_IMG:
                if(eqBITPIX == USHORT_IMG)
                    WriteData<uint16_t>(fptr, TUSHORT);
                else
                    WriteData<int16_t>(fptr, TSHORT);
                break;
                
            case LONG_IMG:
            case ULONG_IMG:
                if(eqBITPIX == ULONG_IMG)
                    WriteData<uint32_t>(fptr, TUINT);
                else
                    WriteData<int32_t>(fptr, TINT);
                break;
                
            case LONGLONG_IMG:
            case ULONGLONG_IMG:
                if(eqBITPIX == ULONGLONG_IMG)
                    WriteData<uint64_t>(fptr, TULONGLONG);
                else
                    WriteData<int64_t>(fptr, TLONGLONG);
                break;
                
            case FLOAT_IMG:
                WriteData<float>(fptr, TFLOAT);
                break;
                
            case DOUBLE_IMG:
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

        if (mask.sum() > 0 && !(std::is_floating_point_v<T> ? std::isnan(static_cast<long double>(BLANK)) : false))
        {
            hdu.ValueForKey("BLANK", BLANK);
        }
        
        const long long    num_axis = static_cast<const long long>( Naxis.size() );
        const long long array_size  = static_cast<const long long>( Nelements() );
    
        std::vector<long long> fpixel(static_cast<size_t>(num_axis));
        for(long long i=0; i < num_axis; ++i) fpixel[i] = static_cast<long long>(Naxis[i]); //1;

        // prepare contiguous output buffer using std::vector (safer than new/delete[])
        std::vector<S> outbuf(static_cast<size_t>(array_size));

        // try to get typed storage pointer (zero-copy fast path)
        std::valarray<T>* _data = this->template GetData<T>();
        
        if((verbose & verboseLevel::VERBOSE_DETAIL) == verboseLevel::VERBOSE_DETAIL||
           (verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
        {
            std::cout<<"\033[31m[FITSimg::WriteDataCube]\033[0m"<<std::endl
                     <<"        \033[31m|- BITPIX       : \033[0m"<<BITPIX<<std::endl
                     <<"        \033[31m|- EQV BITPIX   : \033[0m"<<eqBITPIX<<std::endl
                     <<"        \033[31m|- DATA_TYPE    : \033[0m"<<DATA_TYPE<<std::endl
                     <<"        \033[31m|- BLANK        : \033[0m"<<BLANK<<std::endl
                     <<"        \033[31m|- Number of pix: \033[0m"<<Nelements()<<std::endl
                     <<"        \033[31m|- this C type: \033[0m"<<demangle(typeid(T).name())<<"["<<sizeof(T)<<"]"<<std::endl
                     <<"        \033[31m`-  out C type: \033[0m"<<demangle(typeid(S).name())<<"["<<sizeof(S)<<"]"<<std::endl;

            for(size_t i = 0; i < Naxis.size()-1; i++)
                std::cout<<"             \033[34m|- Axis "<<i<<"    : \033[0m"<<Naxis[i]<<std::endl;
            std::cout<<"             \033[34m`- Axis "<<Naxis.size()-1<<"    : \033[0m"<<Naxis[Naxis.size()-1]<<std::endl;
        }

        if (_data) // fast path: we have a typed valarray<T>
        {
            for(long long i = 0; i < array_size; ++i)
            {
                // apply BZERO/BSCALE and convert to output type
                outbuf[static_cast<size_t>(i)] = static_cast<S> ((*_data)[static_cast<size_t>(i)]);
            }
        }
        else // fallback: build buffer from polymorphic accessor data->get(i)
        {
            if (!data)
                throw std::runtime_error("\033[31m[FITSimg::WriteDataCube]\033[0m no data to write (polymorphic storage missing)");

            for(long long i = 0; i < array_size; ++i)
            {
                double val = static_cast<double>( data->get(static_cast<size_t>(i)) ); // generic value
                outbuf[static_cast<size_t>(i)] = static_cast<S>( val );
            }
        }

        if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
        {
            for(size_t i=0; i < outbuf.size(); ++i) std::cout<<"["<<i<<"]"<<outbuf[i]<<"   ";
            std::cout<<std::endl;
        }

        //if( fits_write_pixll(fptr.get(), DATA_TYPE, fpixel.data(), array_size, outbuf.data(), &img_status) )
        if( fits_write_img(fptr.get(), DATA_TYPE, (LONGLONG)1, array_size, outbuf.data(), &img_status) )
        {
            throw FITSexception(img_status,"FITSimg","WriteDataCube");
        }
        
    }

#pragma endregion
#pragma region * ctor/dtor
    /**
     *  @details Read current HDU of the fitsfile to extract a 2D images
     *  @param fptr: Pointer to the fitfile
     */
    template< typename T >
    FITSimg<T>::FITSimg(const std::shared_ptr<fitsfile>& fptr): FITScube(fptr)
    {
        img_init();

        if(fptr == nullptr || fptr.use_count() < 1)
            throw FITSexception(NOT_IMAGE,"FITSimg<T>","ctor"," received nullptr");

        if(Nelements() != data->size())
            throw FITSexception(BAD_DIMEN,"FITSimg<T>","ctor"," array size missmatch");

        if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
        {
            std::cout<<"\033[32m[FITSimg::ctor]\033[0m Reading image data from FITS file ..."<<std::endl
                        <<"        \033[32m|- NAXIS        : \033[0m"<<Naxis.size()<<std::endl;
            for(size_t i = 0; i < Naxis.size()-1; i++)
                std::cout<<"        \033[32m|    |- NAXIS"<<i<<"  : \033[0m"<<Naxis[i]<<std::endl;
            std::cout<<"        \033[32m|    `- NAXIS"<<Naxis.size()-1<<"  : \033[0m"<<Naxis[Naxis.size()-1]<<std::endl;
            std::cout<<"        \033[32m|- Nelements    : \033[0m"<<Nelements()<<std::endl;
            std::cout<<"        \033[32m|- Array        : \033[0m"<<data->size()<<std::endl;
            std::cout<<"        \033[32m|- Data type    : \033[0m"<<demangle(typeid(T).name())<<"  \033[33m["<<eqBITPIX<<"]\033[0m"<<std::endl;
        }
        
        if(img_status)
            return;
        
        //  * GET BSCALE
        BSCALE = static_cast<T>((hdu.Exists("BSCALE"))? hdu.GetDoubleValueForKey("BSCALE"):1.);
        
        //  * GET BZERO
        BZERO = static_cast<T>((hdu.Exists("BZERO"))? hdu.GetDoubleValueForKey("BZERO"):0.);
        
        //  * GET BLANCK
        BLANK = (hdu.Exists("BLANK"))? static_cast<T>(hdu.GetUInt16ValueForKey("BLANK")):std::numeric_limits<uint16_t>::quiet_NaN();
        
        // * GET THE WHOLE IMAGE DATA
        switch (eqBITPIX)
        {
            case SBYTE_IMG:
                ReadArray<int8_t>(fptr);
                break;

            case BYTE_IMG:
                ReadArray<uint8_t>(fptr);
                break;
                
            case SHORT_IMG:
                ReadArray<int16_t>(fptr);
                break;
            
            case USHORT_IMG:
                ReadArray<uint16_t>(fptr);
                break;
                
            case LONG_IMG:
                ReadArray<int32_t>(fptr);
                break;
            
            case ULONG_IMG:
                ReadArray<uint32_t>(fptr);
                break;
                
            case LONGLONG_IMG:
                ReadArray<int64_t>(fptr);
                break;

            case ULONGLONG_IMG:
                ReadArray<uint64_t>(fptr);
                break;
                
            case FLOAT_IMG:
                ReadArray<float>(fptr);
                break;
                
            case DOUBLE_IMG:
                ReadArray<double>(fptr);
                break;
                
            default:
                throw FITSexception(BAD_BITPIX,"FITSimg","ctor","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN.");
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
        // build a typed copy of underlying array (use pointer accessor to avoid unnecessary copies)
        if(img.data)
        {
            auto p = img.template GetData<T>();
            if(p)
                this->data = std::make_unique< DSL::FitsArray<T> >(*p); // copy underlying valarray<T>
            else
                this->data.reset();
        }
        else
        {
            this->data.reset();
        }
        
        // copy mask
        mask = img.GetMask();
    }
    
    /**
     *  @details Create new empty image of finite dimension.
     *  @param _naxis: Number of dimenssion of the new image
     *  @param _iaxis: Number of pixel in the \f$i^{\rm ieme}\f$ dimension
     */
    template< typename T>
    FITSimg<T>::FITSimg(const size_t& ndim, const std::initializer_list<size_t>& _iaxis) : FITScube()
    {        
        std::vector<size_t> axis = Build_axis(ndim, _iaxis);
        
	    for(size_t i = 0; i < axis.size(); i++)
            Naxis.push_back(axis[i]);

        mask = pxMask(static_cast<size_t>( Nelements() ));

        img_init();

        hdu = FITShdu();
        hdu.ValueForKey("NAXIS",static_cast<int>(axis.size() ));
        for(size_t i = 0; i < Naxis.size(); i++)
	    {
            hdu.ValueForKey("NAXIS"+std::to_string(i+1),Naxis[i]);
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
        Naxis.clear();
        for(size_t i = 0; i < _axis.size(); i++)
            Naxis.push_back(_axis[i]);
        
        mask = pxMask(Nelements());
        img_init();
        
        hdu = FITShdu();
        hdu.ValueForKey("NAXIS",static_cast<int>(_axis.size() ));
        for(size_t i = 0; i < Naxis.size(); i++)
        {
            hdu.ValueForKey("NAXIS"+std::to_string(i+1),Naxis[i]);
        }
        
        template_init();
    }
    
    /**
     *  @details Free memory
     */
    template< typename T >
    FITSimg<T>::~FITSimg()
    {
        // release polymorphic storage
        data.reset();
    }
    
#pragma endregion
#pragma region * modifier

#pragma region -- Bscale
    template< typename T >
    void FITSimg<T>::Bscale(const uint8_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const int8_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const uint16_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const int16_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const uint32_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const int32_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const uint64_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const int64_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

#ifdef Darwinx86_64
    template< typename T >
    void FITSimg<T>::Bscale(const size_t& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }
#endif

    template< typename T >
    void FITSimg<T>::Bscale(const float& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

    template< typename T >
    void FITSimg<T>::Bscale(const double& _bs )
    {
        BSCALE = _bs;
        hdu.ValueForKey("BSCALE",BSCALE);
    }

#pragma endregion
#pragma region -- Bzero
    template< typename T >
    void FITSimg<T>::Bzero(const uint8_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const int8_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const uint16_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const int16_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const uint32_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const int32_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const uint64_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const int64_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

#ifdef Darwinx86_64
    template< typename T >
    void FITSimg<T>::Bzero(const size_t& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }
#endif

    template< typename T >
    void FITSimg<T>::Bzero(const float& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }

    template< typename T >
    void FITSimg<T>::Bzero(const double& _bs )
    {
        BZERO = _bs;
        hdu.ValueForKey("BZERO", BZERO);
    }
#pragma endregion

#pragma region -- Blank

    template< typename T >
    void FITSimg<T>::Blank(const uint8_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const int8_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const uint16_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const int16_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const uint32_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const int32_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const uint64_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const int64_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

#ifdef Darwinx86_64
    template< typename T >
    void FITSimg<T>::Blank(const size_t& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }
#endif

    template< typename T >
    void FITSimg<T>::Blank(const float& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

    template< typename T >
    void FITSimg<T>::Blank(const double& nanVal)
    {
        BLANK = nanVal;
        hdu.ValueForKey("BLANK",BLANK);
    }

#pragma endregion
#pragma endregion
    
    template< typename T >
    void FITSimg<T>::AddLayer(const FITSimg<T>& iLayer)
    {
        // Check the input image is a 2D or 3D image
        if(iLayer.Naxis.size() < 2 ||
           iLayer.Naxis.size() > 3)
            throw FITSexception(BAD_DIMEN,"FITScube","AddLayer","Can't add 1D plot or +4D data-cube to a 2D images");
    
        // Check the two images have the same X and Y dimension
        if((Naxis[0] != iLayer.Naxis[0]) ||
           (Naxis[1] != iLayer.Naxis[1]) )
            throw FITSexception(BAD_DIMEN,"FITScube","AddLayer","The two images didn't have same x and y dimensions");
    
        // Check the data are of the same type
        if(eqBITPIX != iLayer.eqBITPIX)
           throw FITSexception(BAD_DIMEN,"FITScube","AddLayer","The two images have't the same data type");
        
        // Increment the number of dimenssion to account for the new Layer
        if(Naxis.size() < 3)
        {
            Naxis.push_back(1);
            hdu.ValueForKey("NAXIS",Naxis.size());
        }
        size_t nLayer = Naxis[2];
        
        if(iLayer.Naxis.size() == 3)
            Naxis[2] += iLayer.Naxis[2];
        else
            Naxis[2] ++;
        
        hdu.ValueForKey("NAXIS3",Naxis[2]);
        
        // Copy data & mask array to avoid lost any data.
        const std::valarray<T>* myData    = this->template GetData<T>();
        const std::valarray<T>* otherData = iLayer.template GetData<T>();
        
        if(!myData || !otherData)
            throw FITSexception(SHARED_BADARG,"FITScube","AddLayer","Missing typed data pointer");
        
        std::valarray<T> cpy_data = std::valarray<T>(*myData);
        
        pxMask cpy_mask = pxMask(mask);
        size_t data_size = myData->size();
        size_t other_size= otherData->size();
        
        WithTypedData<T>([&](std::valarray<T>& arr){arr.resize(data_size+other_size);});
        mask.resize(data_size+other_size);
       
        if(nLayer > 1)
        {
            WithTypedData<T>([&](std::valarray<T>& arr){arr[std::gslice(0,{nLayer,Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] += cpy_data;});
            //data[std::gslice(0,{nLayer,Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] += cpy_data;
            mask[std::gslice(0,{nLayer,Naxis[1],Naxis[0]},{Naxis[1]*Naxis[0],Naxis[0],1})] |= cpy_mask;
        }
        else
        {
            WithTypedData<T>([&](std::valarray<T>& arr){arr[std::gslice(0,{Naxis[1],Naxis[0]},{Naxis[0],1})] += cpy_data;});
            //data[std::gslice(0,{Naxis[1],Naxis[0]},{Naxis[0],1})] += cpy_data;
            mask[std::gslice(0,{Naxis[1],Naxis[0]},{Naxis[0],1})] |= cpy_mask;
        }
        
        if(iLayer.Naxis.size() > 2)
        {
            // use the typed pointer we already validated
            WithTypedData<T>([&](std::valarray<T>& arr){arr[std::gslice(nLayer*Naxis[1]*Naxis[0],{ iLayer.Naxis[2], Naxis[1], Naxis[0] },{ Naxis[1]*Naxis[0], Naxis[0], 1 })] += *otherData;});
            mask[std::gslice(nLayer*Naxis[1]*Naxis[0],{ iLayer.Naxis[2], Naxis[1], Naxis[0] },{ Naxis[1]*Naxis[0], Naxis[0], 1 })] |= iLayer.mask;
        }
        else
        {
            WithTypedData<T>([&](std::valarray<T>& arr){arr[std::gslice(nLayer*Naxis[1]*Naxis[0],{ Naxis[1], Naxis[0] },{ Naxis[0], 1 })] += *otherData;});
            mask[std::gslice(nLayer*Naxis[1]*Naxis[0],{ Naxis[1], Naxis[0] },{ Naxis[0], 1 })] |= iLayer.mask;
        }
        
        cpy_mask.resize(0);
        cpy_data.resize(0);
        
        
    }
    
#pragma endregion
#pragma region * Image Statistic

/**
     *  @details Compute the sum of all unmasked pixel values
     *  @return Sum of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetSum() const
    {
        if(!data)
            return 0.0;
    
        double sum = 0.0;
        bool handled = false;

        // fast typed paths (add the types you commonly use)
        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(!mask[i])
                        sum += static_cast<double>(arr[i]);
                }
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetSum]\033[0m unsupported data type " + demangle(typeid(T).name()));

        return sum;
    }

    /**
     *  @details Compute the mean of all unmasked pixel values
     *  @return Average of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetMean() const
    {
        if(!data)
            return 0.0;
    
        double sum = GetSum();
        size_t nPix = mask[!mask].size();

        return (nPix > 0) ? sum /= static_cast<double>(nPix) : 0;
    }

    /**
     *  @details Compute the quadratic mean of all unmasked pixel values
     *  @return Quadratic mean of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetQuadraticMean() const
    {
        if(!data)
            return 0.0;
    
        size_t nPix = mask[!mask].size();
        double sum=0;

        bool handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(!mask[i])
                        sum += static_cast<double>(arr[i]*arr[i]);
                }
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetQuadraticMean]\033[0m unsupported data type " + demangle(typeid(T).name()));

        return (nPix > 0) ? std::sqrt(sum /= static_cast<double>(nPix)) : 0;
    }

    /**
     *  @details Compute the variance of all unmasked pixel values
     *  @return Variance of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetVariance() const
    {
        if(!data)
            return 0.0;
    
        size_t nPix = mask[!mask].size();
        double mean = GetMean();
        double var  = 0.0;
        bool handled = false;

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(!mask[i])
                        var += (static_cast<double>(arr[i]) - mean) * (static_cast<double>(arr[i]) - mean);
                }
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetVariance]\033[0m unsupported data type " + demangle(typeid(T).name()));

        return (nPix > 1) ? var /= static_cast<double>(nPix-1) : 0;
    }

    /**
     *  @details Compute the standard deviation of all unmasked pixel values
     *  @return Standard deviation of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetStdDev() const
    {
        if(!data)
            return 0.0;

        return std::sqrt( GetVariance() );
    }

    /**
     *  @details Root Mean Square all unmasked pixel values
     *  @return RMS of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetRMS() const
    {
        return GetQuadraticMean();
    }

    /**
     *  @details Compute Root Mean Square Error of all unmasked pixel values
     *  @return RMSE of all unmasked pixel values
     */
    template< typename T >
    double FITSimg<T>::GetRMSE() const
    {
        if(!data)
            return 0.0;

        double var = GetVariance();
        size_t nPix = mask[!mask].size();
        
        return sqrt(var*static_cast<double>(nPix-1)/static_cast<double>(nPix));
    }

    template< typename T >
    double FITSimg<T>::Getpercentil(const double& fpp) const
    {
        if(fpp < 0. || fpp > 1.)
            throw FITSexception(BAD_OPTION,"FITScube","Getpercentil","fpp should be in the range [0,1]");

        if(!data)
            return 0.0;

        std::vector<double> sorted;
        bool handled = false;

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(!mask[i])
                        sorted.push_back( static_cast<double>(arr[i]) );
                }
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::Getpercentil]\033[0m unsupported data type " + demangle(typeid(T).name()));

        std::sort(sorted.begin(), sorted.end());
        double pos = fpp * static_cast<double>(sorted.size()-1);
        size_t idx = static_cast<size_t>( pos + 0.5 );
        double dpos= pos - static_cast<double>(idx);

        double pp_value = 0.0;

        if(idx +1 < sorted.size())
            pp_value = sorted[idx] + dpos * (sorted[idx+1] - sorted[idx]);
        else
            pp_value = sorted[idx];

        return pp_value;
    }

    template< typename T >
    double FITSimg<T>::GetMedian() const
    {
        return Getpercentil(0.5);
    }

    template< typename T >
    double FITSimg<T>::Get5thpercentil() const
    {
        return Getpercentil(0.05);
    }

    template< typename T >
    double FITSimg<T>::Get25thpercentil() const
    {
        return Getpercentil(0.25);
    }

    template< typename T >
    double FITSimg<T>::Get75thpercentil() const
    {
        return Getpercentil(0.75);
    }

    template< typename T >
    double FITSimg<T>::Get95thpercentil() const
    {
        return Getpercentil(0.95);
    }

    template< typename T >
    double FITSimg<T>::GetKurtosis() const
    {
        if(!data)
            return 0.0;
    
        size_t nPix = mask[!mask].size();
        double mean = GetMean();
        double var  = GetVariance();
        double kurt = 0.0;
        bool handled = false;

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                double m4 = 0.0;

                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(mask[i])
                        continue;
                    
                    double d = static_cast<double>(arr[i]) - mean;
                    double d2 = d*d;
                    m4 += d2*d2;
                }
                
                double s4 = var*var;
                double n = static_cast<double>(nPix);

                double term1 = (n*(n+1)*m4)/( (n-1)*(n-2)*(n-3)*s4 );
                double term2 = (3.*(n-1)*(n-1))/((n-2)*(n-3));
                    
                kurt = term1 - term2;
                
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetKurtosis]\033[0m unsupported data type " + demangle(typeid(T).name()));

        return kurt;
    }

    template< typename T >
    double FITSimg<T>::GetSkewness() const
    {
        if(!data)
            return 0.0;

        size_t nPix = mask[!mask].size();
        double mean = GetMean();
        double var  = GetVariance();
        double skew = 0.0;
        bool handled = false;

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                for(size_t i = 0; i < arr.size(); i++)
                {
                    if(!mask[i])
                        skew += (static_cast<double>(arr[i]) - mean) * (static_cast<double>(arr[i]) - mean) * (static_cast<double>(arr[i]) - mean);
                }
            });
        if(!handled)
            throw std::runtime_error("\033[31m[FITSimg::GetSkewness]\033[0m unsupported data type " + demangle(typeid(T).name()));

        return (nPix > 2 && var > 0.) ? (skew /= static_cast<double>(nPix) ) / (var * std::sqrt(var)) : 0;
    }

    template< typename T >
    double FITSimg<T>::GetMinimum() const
    {
        if(!data)
            return 0.0;
        
        bool handled = false;
        double minVal = std::numeric_limits<double>::max();

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                minVal = static_cast<double>( ((typename std::valarray<T>) arr[!mask]).min() );
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetMin]\033[0m unsupported data type " + demangle(typeid(T).name()));
        
        return minVal;
    }

    template< typename T >
    double FITSimg<T>::GetMaximum() const
    {
        if(!data)
            return 0.0;
        
        bool handled = false;
        double maxVal = -1*std::numeric_limits<double>::max();

        handled = WithTypedData<T>([&](const std::valarray<T>& arr)
            {
                maxVal = static_cast<double>( ((typename std::valarray<T>) arr[!mask]).max() );
            });
        if(!handled) 
            throw std::runtime_error("\033[31m[FITSimg::GetMin]\033[0m unsupported data type " + demangle(typeid(T).name()));
        
        return maxVal;
    }


#pragma endregion
#pragma region * data operation
    
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
        
        // copy base FITScube members
        this->hdu        = img.hdu;
        this->Naxis      = img.Naxis;
        this->BITPIX     = img.BITPIX;
        this->eqBITPIX   = img.eqBITPIX;
        this->name       = img.name;
        this->img_status = img.img_status;
        
        // copy FITSimg-specific members
        this->BSCALE = img.BSCALE;
        this->BZERO  = img.BZERO;
        this->BLANK  = img.BLANK;
        
        // copy mask
        this->mask = img.GetMask();
        
        // copy data (polymorphic). Try typed fast path first; if not available, fallback to per-element copy.
        if(img.data)
        {
            // try to get a typed valarray<T> from source
            const std::valarray<T>* src_arr = img.template GetData<T>();
            if(src_arr)
            {
                // direct copy of underlying typed array
                this->data = std::make_unique< FitsArray<T> >(*src_arr);
            }
            else
            {
                // fallback: allocate and fill using generic get()
                size_t n = img.data->size();
                auto dst = std::make_unique< FitsArray<T> >(n);
                for(size_t i = 0; i < n; ++i)
                    dst->ref()[i] = static_cast<T>( img.data->get(i) );
                this->data = std::move(dst);
            }
        }
        else
        {
            this->data.reset();
        }
        
        return *this;
    }
    
    template< typename T >
    uint8_t FITSimg<T>::UByteValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","UByteValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<uint8_t>(val);
    }
    
    template< typename T >
    uint8_t FITSimg<T>::UByteValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return UByteValueAtPixel(iPx);
    }
    
    template< typename T >
    int8_t FITSimg<T>::ByteValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","ByteValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<int8_t>(val);
    }
    
    template< typename T >
    int8_t FITSimg<T>::ByteValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return ByteValueAtPixel(iPx);
    }
    
    template< typename T >
    uint16_t FITSimg<T>::UShortValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","UShortValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<uint16_t>(val);
    }
    
    template< typename T >
    uint16_t FITSimg<T>::UShortValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return UShortValueAtPixel(iPx);
    }
    
    template< typename T >
    int16_t FITSimg<T>::ShortValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","ShortValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<int16_t>(val);
    }
    
    template< typename T >
    int16_t FITSimg<T>::ShortValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);
        
        return ShortValueAtPixel(iPx);
    }
    
    
    template< typename T >
    uint32_t FITSimg<T>::ULongValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","ULongValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<uint32_t>(val);
    }
    
    template< typename T >
    uint32_t FITSimg<T>::ULongValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return ULongValueAtPixel(iPx);
    }
    
    template< typename T >
    int32_t FITSimg<T>::LongValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","LongValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<int32_t>(val);
    }
    
    template< typename T >
    int32_t FITSimg<T>::LongValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return LongValueAtPixel(iPx);
    }
    
    template< typename T >
    int64_t FITSimg<T>::LongLongValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","LongLongValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<int64_t>(val);
    }
    
    template< typename T >
    int64_t FITSimg<T>::LongLongValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return LongLongValueAtPixel(iPx);
    }

    template< typename T >
    uint64_t FITSimg<T>::ULongLongValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","LongLongValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<uint64_t>(val);
    }
    
    template< typename T >
    uint64_t FITSimg<T>::ULongLongValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return ULongLongValueAtPixel(iPx);
    }
    
    template< typename T >
    float FITSimg<T>::FloatValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","FloatValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<float>(val);
    }
    
    template< typename T >
    float FITSimg<T>::FloatValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);

        return FloatValueAtPixel(iPx);
    }

    
    template< typename T >
    double FITSimg<T>::DoubleValueAtPixel(const size_t& iPx) const
    {
        T val=0;

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            if(iPx >= arr.size())
                throw FITSexception(BAD_DIMEN,"FITSimg","DoubleValueAtPixel","pixel index out of range");
            val = arr[iPx];
        });

        return static_cast<float>(val);
    }
    
    template< typename T >
    double FITSimg<T>::DoubleValueAtPixel(const std::initializer_list<size_t>& iCoo) const
    {
        size_t iPx = this->PixelIndex(iCoo);
        
        return DoubleValueAtPixel(iPx);
    }
    
    /**
     *  @details Optain the physical value of a pixel in the image.
     *  @param iPx: pixel of the image. Pixel is numbered for Naxis[0] to Naxis[n].
     *  @return Content of the pixel or 0 if the pixel is masked.
     */
    template< typename T >
    const T FITSimg<T>::operator [](const size_t& iPx) const
    {
        if(data == nullptr)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator[] const","missing data");
        
        if(iPx >= Nelements())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator[]","pixel index out of range");

        const std::valarray<T>* p = this->template GetData<T>();
        return ( mask[iPx])? 0. : (*p)[iPx];
    }
    
    /**
     *  @details Optain the physical value of a pixel in the image.
     *  @param iPx: pixel of the image. Pixel is numbered for Naxis[0] to Naxis[n].
     *  @return Content of the pixel or 0 if the pixel is masked.
     */
    template< typename T >
    T FITSimg<T>::operator [](const size_t& iPx)
    {
        if(data == nullptr)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator[] const","missing data");
        
        if(iPx >= Nelements())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator[]","pixel index out of range");

        const std::valarray<T>* p = this->template GetData<T>();
        return ( mask[iPx])? 0. : (*p)[iPx];
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator*=(const S& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(!safe_cast_check_scalar<S, T>(val))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator*=", "unsafe conversion from scalar to storage type");

        T v=0;
        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
            v += static_cast<T>(val+0.5);
        else
            v += static_cast<T>(val);

        WithTypedData<T>([&](std::valarray<T>& arr)
            {
                const size_t n = arr.size();
                if(n == 0)
                    return;
                if(mask.size() != n)
                    throw FITSexception(SHARED_BADARG,"FITScube::operator*=","mask/data size mismatch");
                
                std::valarray<bool> keep = !mask; 
                arr[keep] = ((std::valarray<T>)arr[keep]) * v; // branchless version
            });
    }
    
    /**
     *  @details Scale down the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator/= (const S& val)
    {

        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(!safe_cast_check_scalar<S, T>(val))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator/=", "unsafe conversion from scalar to storage type");

        if (val == static_cast<S>(0))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator/=", "division by zero");

        T v=0;
        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
            v += static_cast<T>(val+0.5);
        else
            v += static_cast<T>(val);

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            if(n == 0)
                return;
            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","mask/data size mismatch");

            std::valarray<bool> keep = !mask; 
            arr[keep] = ((std::valarray<T>)arr[keep]) / v; // branchless version
        });
    }
    
    
    /**
     *  @details Offset uppward the whole image by a value of val.
     *  @param val: upward offset.
     *  @note masked pixel aren't affected by the offset.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator+= (const S& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(!safe_cast_check_scalar<S, T>(val))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator+=", "unsafe conversion from scalar to storage type");
        
        T v=0;
        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
            v += static_cast<T>(val+0.5);
        else
            v += static_cast<T>(val);

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            if(n == 0)
                return;
            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator+=","mask/data size mismatch");

            std::valarray<bool> keep = !mask; 
            arr[keep] = ((std::valarray<T>)arr[keep]) + v; // branchless version
        });
    }
    
    
    /**
     *  @details Offset downward the whole image by a value of val.
     *  @param val: downward offset.
     *  @note masked pixel aren't affected by the offset.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator-= (const S& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(!safe_cast_check_scalar<S, T>(val))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator-=", "unsafe conversion from scalar to storage type");

        T v=0;
        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
            v += static_cast<T>(val+0.5);
        else
            v += static_cast<T>(val);
    
        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            if(n == 0)
                return;
            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator-=","mask/data size mismatch");

            std::valarray<bool> keep = !mask; 
            arr[keep] = ((std::valarray<T>)arr[keep]) - v; // branchless version
        });
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator*=(const std::valarray<S>& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            const size_t m = val.size();
            if(n == 0 || m == 0 || n != m)
                throw FITSexception(SHARED_BADARG,"FITScube::operator*=","array size mismatch");

            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator*=","mask/data size mismatch");

            std::valarray<bool> keep = !mask;
            for (size_t i = 0; i < n; ++i)
            {
                if(keep[i])
                    arr[i] = arr[i] * static_cast<T>(val[i]); // safe conversion
            }
        });
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator/=(const std::valarray<S>& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            const size_t m = val.size();
            if(n == 0 || m == 0 || n != m)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","array size mismatch");

            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","mask/data size mismatch");
            
            std::valarray<bool> keep = !mask; 

            for (size_t i = 0; i < n; ++i)
            {
                if(keep[i])
                    arr[i] = arr[i] / static_cast<T>(val[i]); // safe conversion
            }
        });
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator+=(const std::valarray<S>& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            const size_t m = val.size();
            if(n == 0 || m == 0 || n != m)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","array size mismatch");

            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","mask/data size mismatch");

            std::valarray<bool> keep = !mask;
            for (size_t i = 0; i < n; ++i)
            {
                if(keep[i])
                    arr[i] = arr[i] + static_cast<T>(val[i]); // safe conversion
            }
        });
    }
    
    /**
     *  @details Scale up the whole image by a factor val.
     *  @param val: Scaling factor.
     *  @note masked pixel aren't scaled up.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator-=(const std::valarray<S>& val)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            const size_t n = arr.size();
            const size_t m = val.size();
            if(n == 0 || m == 0 || n != m)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","array size mismatch");

            if(mask.size() != n)
                throw FITSexception(SHARED_BADARG,"FITScube::operator/=","mask/data size mismatch");

            std::valarray<bool> keep = !mask; 
            for (size_t i = 0; i < n; ++i)
            {
                if(keep[i])
                    arr[i] = arr[i] - static_cast<T>(val[i]); // safe conversion
            }
        });
    }


    /**
     *  @details Multiply, pixel by pixel, two images.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator*= (const FITSimg<S>& img)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(data == nullptr || img.template GetData<S>() == nullptr || mask.size() == 0 || img.GetMask().size() == 0)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator*=","missing data");

        if(mask.size() != img.GetMask().size() || data->size() != img.template GetData<S>()->size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator*=","mask/data size mismatch");

        const std::valarray<S>* otherData = img.template GetData<S>();

        if(otherData)
        {
            mask |= img.GetMask();
            this->template operator*= (*otherData);
            return;
        }
    }
    
    /**
     *  @details Divide, pixel by pixel, two images.
     *  @param img: Image to divide to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator/= (const FITSimg<S>& img)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(data == nullptr || img.template GetData<S>() == nullptr || mask.size() == 0 || img.GetMask().size() == 0)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator/=","missing data");

        if(mask.size() != img.GetMask().size() || data->size() != img.template GetData<S>()->size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator/=","mask/data size mismatch");

        const std::valarray<S>* otherData = img.template GetData<S>();

        if(otherData)
        {
            mask |= img.GetMask();
            mask |= ( *otherData == static_cast<S>(0) ); // also mask where divisor is zero
            this->template operator/= (*otherData);
            return;
        }
    }
    
    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator+= (const FITSimg<S>& img)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(data == nullptr || img.template GetData<S>() == nullptr || mask.size() == 0 || img.GetMask().size() == 0)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator+=","missing data");

        if(mask.size() != img.GetMask().size() || data->size() != img.template GetData<S>()->size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator+=","mask/data size mismatch");

        const std::valarray<S>* otherData = img.template GetData<S>();

        if(otherData)
        {
            mask |= img.GetMask();
            this->template operator+= (*otherData);
            return;
        }
    }
    
    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    template< typename S >
    void FITSimg<T>::operator-= (const FITSimg<S>& img)
    {
        // compile-time quick check (gives clearer diagnostics)
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(data == nullptr || img.template GetData<S>() == nullptr || mask.size() == 0 || img.GetMask().size() == 0)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::operator-=","missing data");

        if(mask.size() != img.GetMask().size() || data->size() != img.template GetData<S>()->size())
            throw FITSexception(SHARED_BADARG,"FITSimg<T>::operator-=","mask/data size mismatch");

        const std::valarray<S>* otherData = img.template GetData<S>();

        if(otherData)
        {
            mask |= img.GetMask();
            this->template operator-= (*otherData);
            return;
        }
    }

    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator*= (const FITScube& img)
    {
        // try typed-paths, invoke templated operator*= for the matching concrete FITSimg<U>
        if (const auto *p_u8  = dynamic_cast<const FITSimg< uint8_t> *>(&img)) { this->template operator*=(*p_u8);  return; }
        if (const auto *p_i8  = dynamic_cast<const FITSimg<  int8_t> *>(&img)) { this->template operator*=(*p_i8);  return; }
        if (const auto *p_u16 = dynamic_cast<const FITSimg<uint16_t> *>(&img)) { this->template operator*=(*p_u16); return; }
        if (const auto *p_i16 = dynamic_cast<const FITSimg< int16_t> *>(&img)) { this->template operator*=(*p_i16); return; }
        if (const auto *p_u32 = dynamic_cast<const FITSimg<uint32_t> *>(&img)) { this->template operator*=(*p_u32); return; }
        if (const auto *p_i32 = dynamic_cast<const FITSimg< int32_t> *>(&img)) { this->template operator*=(*p_i32); return; }
        if (const auto *p_u64 = dynamic_cast<const FITSimg<uint64_t> *>(&img)) { this->template operator*=(*p_u64); return; }
        if (const auto *p_i64 = dynamic_cast<const FITSimg< int64_t> *>(&img)) { this->template operator*=(*p_i64); return; }
#ifdef Darwinx86_64
        if (const auto *p_sz  = dynamic_cast<const FITSimg<  size_t> *>(&img)) { this->template operator*=(*p_sz);  return; }
#endif
        if (const auto *p_f   = dynamic_cast<const FITSimg<   float> *>(&img)) { this->template operator*=(*p_f);   return; }
        if (const auto *p_d   = dynamic_cast<const FITSimg<  double> *>(&img)) { this->template operator*=(*p_d);   return; }

        // unsupported/unknown concrete type
        throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator*=", "unsupported FITScube concrete type");
    }

    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator/= (const FITScube& img)
    {
        // try typed-paths, invoke templated operator*= for the matching concrete FITSimg<U>
        if (const auto *p_u8  = dynamic_cast<const FITSimg< uint8_t> *>(&img)) { this->template operator/=(*p_u8);  return; }
        if (const auto *p_i8  = dynamic_cast<const FITSimg<  int8_t> *>(&img)) { this->template operator/=(*p_i8);  return; }
        if (const auto *p_u16 = dynamic_cast<const FITSimg<uint16_t> *>(&img)) { this->template operator/=(*p_u16); return; }
        if (const auto *p_i16 = dynamic_cast<const FITSimg< int16_t> *>(&img)) { this->template operator/=(*p_i16); return; }
        if (const auto *p_u32 = dynamic_cast<const FITSimg<uint32_t> *>(&img)) { this->template operator/=(*p_u32); return; }
        if (const auto *p_i32 = dynamic_cast<const FITSimg< int32_t> *>(&img)) { this->template operator/=(*p_i32); return; }
        if (const auto *p_u64 = dynamic_cast<const FITSimg<uint64_t> *>(&img)) { this->template operator/=(*p_u64); return; }
        if (const auto *p_i64 = dynamic_cast<const FITSimg< int64_t> *>(&img)) { this->template operator/=(*p_i64); return; }
#ifdef Darwinx86_64
        if (const auto *p_sz  = dynamic_cast<const FITSimg<  size_t> *>(&img)) { this->template operator/=(*p_sz);  return; }
#endif
        if (const auto *p_f   = dynamic_cast<const FITSimg<   float> *>(&img)) { this->template operator/=(*p_f);   return; }
        if (const auto *p_d   = dynamic_cast<const FITSimg<  double> *>(&img)) { this->template operator/=(*p_d);   return; }

        // unsupported/unknown concrete type
        throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator*=", "unsupported FITScube concrete type");
    }

    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator+= (const FITScube& img)
    {
        // try typed-paths, invoke templated operator*= for the matching concrete FITSimg<U>
        if (const auto *p_u8  = dynamic_cast<const FITSimg< uint8_t> *>(&img)) { this->template operator+=(*p_u8);  return; }
        if (const auto *p_i8  = dynamic_cast<const FITSimg<  int8_t> *>(&img)) { this->template operator+=(*p_i8);  return; }
        if (const auto *p_u16 = dynamic_cast<const FITSimg<uint16_t> *>(&img)) { this->template operator+=(*p_u16); return; }
        if (const auto *p_i16 = dynamic_cast<const FITSimg< int16_t> *>(&img)) { this->template operator+=(*p_i16); return; }
        if (const auto *p_u32 = dynamic_cast<const FITSimg<uint32_t> *>(&img)) { this->template operator+=(*p_u32); return; }
        if (const auto *p_i32 = dynamic_cast<const FITSimg< int32_t> *>(&img)) { this->template operator+=(*p_i32); return; }
        if (const auto *p_u64 = dynamic_cast<const FITSimg<uint64_t> *>(&img)) { this->template operator+=(*p_u64); return; }
        if (const auto *p_i64 = dynamic_cast<const FITSimg< int64_t> *>(&img)) { this->template operator+=(*p_i64); return; }
#ifdef Darwinx86_64
        if (const auto *p_sz  = dynamic_cast<const FITSimg<  size_t> *>(&img)) { this->template operator+=(*p_sz);  return; }
#endif
        if (const auto *p_f   = dynamic_cast<const FITSimg<   float> *>(&img)) { this->template operator+=(*p_f);   return; }
        if (const auto *p_d   = dynamic_cast<const FITSimg<  double> *>(&img)) { this->template operator+=(*p_d);   return; }

        // unsupported/unknown concrete type
        throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator*=", "unsupported FITScube concrete type");
    }

    /**
     *  @details shift, pixel by pixel, this images by the content of img.
     *  @param img: Image to multiply to this.
     *  @note this\f$\rightarrow\f$mask is updated with the img\f$\rightarrow\f$mask.\n If the two images haven't the same dimenssion, the behaviour is undefined.
     */
    template< typename T >
    void FITSimg<T>::operator-= (const FITScube& img)
    {
        // try typed-paths, invoke templated operator*= for the matching concrete FITSimg<U>
        if (const auto *p_u8  = dynamic_cast<const FITSimg< uint8_t> *>(&img)) { this->template operator-=(*p_u8);  return; }
        if (const auto *p_i8  = dynamic_cast<const FITSimg<  int8_t> *>(&img)) { this->template operator-=(*p_i8);  return; }
        if (const auto *p_u16 = dynamic_cast<const FITSimg<uint16_t> *>(&img)) { this->template operator-=(*p_u16); return; }
        if (const auto *p_i16 = dynamic_cast<const FITSimg< int16_t> *>(&img)) { this->template operator-=(*p_i16); return; }
        if (const auto *p_u32 = dynamic_cast<const FITSimg<uint32_t> *>(&img)) { this->template operator-=(*p_u32); return; }
        if (const auto *p_i32 = dynamic_cast<const FITSimg< int32_t> *>(&img)) { this->template operator-=(*p_i32); return; }
        if (const auto *p_u64 = dynamic_cast<const FITSimg<uint64_t> *>(&img)) { this->template operator-=(*p_u64); return; }
        if (const auto *p_i64 = dynamic_cast<const FITSimg< int64_t> *>(&img)) { this->template operator-=(*p_i64); return; }
#ifdef Darwinx86_64
        if (const auto *p_sz  = dynamic_cast<const FITSimg<  size_t> *>(&img)) { this->template operator-=(*p_sz);  return; }
#endif
        if (const auto *p_f   = dynamic_cast<const FITSimg<   float> *>(&img)) { this->template operator-=(*p_f);   return; }
        if (const auto *p_d   = dynamic_cast<const FITSimg<  double> *>(&img)) { this->template operator-=(*p_d);   return; }

        // unsupported/unknown concrete type
        throw FITSexception(SHARED_BADARG, "FITSimg<T>::operator*=", "unsupported FITScube concrete type");
    }
    
    template< typename T >
    template< typename S >
    void FITSimg<T>::SetPixelValue(const S& val, const std::vector<size_t>& iPx)
    {        
        img_status = 0;
        size_t index = PixelIndex(iPx);
        
        if(index >= Nelements())
            return;
        
        size_t sum = 0;
        
        for(size_t i = 0 ; i < iPx.size(); i++)
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
        
        SetPixelValue<S>(val, index);
    }
    
    template< typename T >
    template< typename S >
    void FITSimg<T>::SetPixelValue(const S& val, const size_t& index)
    {
        static_assert(std::is_convertible_v<S, T> || std::is_same_v<S,T>, "Scalar type S is not convertible to storage type T");

        if(!safe_cast_check_scalar<S, T>(val))
            throw FITSexception(SHARED_BADARG, "FITSimg<T>::SetPixelValue", "unsafe conversion from scalar to storage type");

        if(index >= data->size())
        {
                img_status = BAD_DIMEN;
                throw FITSexception(img_status,"FITScube","SetPixelValue","Pixel ["+std::to_string(index)+"] is out of range");
        }

        T v =0;
        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
            v = static_cast<T>(val+0.5);
        else
            v = static_cast<T>(val);
        
        WithTypedData<T>([&](std::valarray<T>& arr)
        {
            arr[index] = v;
        });
    }
    
#pragma endregion
#pragma region * Accessor    
    
#pragma endregion

#pragma region * Data export methods

#pragma endregion
#pragma endregion
#pragma region * Extraction method
    
    /**
     *  Extract layer from a 3D FITS cube.
     *  @param iLayer: The layer one want to extract in single FITSimg.
     *  @return Return pointer to a 2D FITScube for the selected layer. Header information are updated to account for new image dimenssion.
     */
    template< typename T >
    std::shared_ptr<FITScube> FITSimg<T>::Layer(const size_t& iLayer) const
    {
        // create copy (copy constructor duplicates header, data and mask)
        FITSimg<T> *copy = new FITSimg<T>(*this);


        if(Naxis.size() <= 2)
            throw FITSexception(BAD_DIMEN,"FITScube","Layer","Image does not contain layers. Copy of this will be returned.");

        if(Naxis.size() > 3)
            throw FITSexception(BAD_DIMEN,"FITScube","Layer","Can't extract layer of nD data cube with n > 3.");

        if(iLayer >= Naxis[2])
            throw FITSexception(BAD_DIMEN,"FITScube","Layer","Image only contains "+std::to_string(Naxis[2])+" layers.");

        const size_t nx = Size(1);
        const size_t ny = Size(2);
        const size_t n_elements = nx * ny;

        // bounds for scanning: include a small margin but avoid unsigned underflow
        const size_t chunkStart = n_elements * iLayer;
        const size_t chunkEnd   = n_elements * (iLayer + 1);

        const size_t margin = 100;
        const size_t iBegin = (chunkStart > margin) ? (chunkStart - margin) : 0;
        const size_t iEnd   = std::min(chunkEnd + margin, this->data->size());

        // prepare destination storage
        copy->GetData<T>()->resize(n_elements);
        copy->mask.resize(n_elements);

        // try fast typed-path for source data
        const std::valarray<T>* src = this->template GetData<T>();
        if(!src)
        {
            // fall back to polymorphic accessor if typed storage not available
            size_t dst_i = 0;
            copy->WithTypedData<T>([&](std::valarray<T>& arr){
                for(size_t k = iBegin; k < iEnd && dst_i < n_elements; ++k)
                {
                    const size_t layer_of_k = k / n_elements;
                    if(layer_of_k != iLayer) continue;

                    // use generic accessor (may be slower)
                    arr[dst_i] = this->data->get(k);
                    copy->mask[dst_i] = this->mask[k];
                    ++dst_i;
                }
                // optional: if dst_i != n_elements, you may want to resize or warn
            });
        }
        else
        {
            // fast path: direct index into typed source valarray
            size_t dst_i = 0;
            copy->WithTypedData<T>([&](std::valarray<T>& arr){
                for(size_t k = iBegin; k < iEnd && dst_i < n_elements; ++k)
                {
                    const size_t layer_of_k = k / n_elements;
                    if(layer_of_k != iLayer) continue;

                    arr[dst_i] = (*src)[k];
                    copy->mask[dst_i] = this->mask[k];
                    ++dst_i;
                }
                // sanity: if dst_i != n_elements, you can throw/warn or adjust
            });
        }

        if((verbose & verboseLevel::VERBOSE_IMG) == verboseLevel::VERBOSE_IMG)
            copy->hdu.Dump(std::cout);

        copy->DeleteLastAxis();

        if(copy->GetData<T>()->size() != copy->hdu.GetDimension() )
            throw FITSwarning("FITScube","Layer","DIMENSION MISSMATCH : "+std::to_string(copy->GetData<T>()->size())+" != "+std::to_string(copy->hdu.GetDimension()));

        return std::shared_ptr<FITScube>(copy);
    }
    
    /**
     *  Resize the current data array.
     *  @param xMin, yMin: The bottom left corner of the window used for the extraction
     *  @param xSize, ySize: The pixel size, along the x and y axis, of the windows
     *  @note Only work for two dimensional array
     */
    template< typename T >
    void FITSimg<T>::Resize (const size_t& xMin, const size_t& yMin, const size_t& xSize, const size_t& ySize)
    {        
        if(data == nullptr)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>","Resize","missing data");

        size_t _xSize = (xMin + xSize < Naxis[0])?xSize : Naxis[0]-xMin;
        size_t _ySize = (yMin + ySize < Naxis[1])?ySize : Naxis[1]-yMin;

        
        std::vector<size_t> pixels;
        
        pixels.push_back(xMin);
        pixels.push_back(yMin);
        
        for(std::vector<size_t>::const_iterator it = Naxis.cbegin(); it != Naxis.cend(); it++)
            pixels.push_back(0);
        
        size_t i_px = PixelIndex(pixels);
        pixels.clear();
        
        if(i_px >= Nelements())
            return;
        
        std::valarray<size_t> size  (Naxis.size());
        std::valarray<size_t> stride(Naxis.size());
        
        stride  += 1;
        
        for(size_t k = 0; k < Naxis.size(); k++)
        {
            if(k == Naxis.size()-1)
                size[k] = _xSize;
            else if(k==Naxis.size()-2)
                size[k] = _ySize;
            else
                size[k] = Naxis[Naxis.size() - 1 - k];
            
            for(size_t l = 0; l < k; l++)
                stride[Naxis.size() - 1 - k] *= Naxis[l];
        }
        
        WithTypedData<T>([&](std::valarray<T>& arr){
            
            std::valarray<T>     cpy = arr[std::gslice(i_px,size,stride)];
            std::valarray<bool> mcpy = mask[std::gslice(i_px,size,stride)];
        
            arr.resize(cpy.size());
            mask.resize(mcpy.size());
        
            arr = cpy;
            mask= mcpy;
        });

        size.resize(0);
        stride.resize(0);
        
        Naxis[0] = static_cast<size_t>( _xSize );
        Naxis[1] = static_cast<size_t>( _ySize );
        
        hdu.ValueForKey("NAXIS1",Naxis[0]);
        hdu.ValueForKey("NAXIS2",Naxis[1]);
        
        if(getNumberOfWCS() < 1)
            return;

        // update WCS info
        for(size_t idx = 0; idx < getNumberOfWCS(); idx++)
        {
            try
            {
                std::vector<double> crpx;
                for(size_t iAxes = 0; iAxes < Naxis.size(); iAxes++)
                {
                    if(iAxes == 0)
                        crpx.push_back( static_cast<double>(xMin) ); // FITS pixel start at 1
                    else if(iAxes == 1)
                        crpx.push_back( static_cast<double>(yMin) );
                    else
                    {
                        crpx.push_back(fwcs.CRPIX(idx,iAxes+1));
                    }
                }

                FITSwcs tmp(fwcs,idx,crpx);
                FITShdu tmp_hdu = tmp.asFITShdu();

                for(FITSDictionary::const_iterator it = tmp_hdu.begin(); it != tmp_hdu.end(); it++)
                {
                    hdu.ValueForKey(it->first,it->second.value(),it->second.type(),it->second.comment());
                }

                FITSwcs::swap(fwcs,tmp);
            }
            catch(WCSexception& e)
            {
                std::cerr<<e.what()<<std::flush;

                FITSwcs tmp(fwcs,idx);
                FITShdu tmp_hdu = tmp.asFITShdu();

                for(FITSDictionary::const_iterator it = tmp_hdu.begin(); it != tmp_hdu.end(); it++)
                {
                    hdu.DeleteKey(it->first);
                }
            }

        }
    }
    
    /**
     *  Extract part of an image based on a cropping windows of the primary 2D slice of a N dimensional datacube.
     *  @param xMin, yMin: The bottom left corner of the window used for the extraction
     *  @param xSize, ySize: The pixel size, along the x and y axis, of the windows
     *  @return Return the FITScube for which x and y pixel coordinate are encapsulated in the specified window size.  Header information are updated to account for new image dimenssion.
     */
    template< typename T >
    std::shared_ptr<FITScube> FITSimg<T>::Window (size_t xMin, size_t yMin, size_t xSize, size_t ySize) const
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
        
        if((verbose & verboseLevel::VERBOSE_DETAIL) == verboseLevel::VERBOSE_DETAIL)
            std::cout<<"\033[31m[FITScube::Window]\033[0m"<<std::endl
                     << "    \033[31m|- NEW IMAGE SIZE :\033[0m "<<n_elements<<std::endl
                     << "    \033[31m|- WINDOW         :\033[0m ("<<xMin<<","<<yMin<<")\033[34m -> \033[0m("<<xMax<<","<<yMax<<") [pix]"<<std::endl
                     << "    \033[31m`- PIXEL OFFSET   :\033[0m ("<<PixelCoordinates(0)[0]<<","<<PixelCoordinates(0)[1]<<")"<<std::endl;
        
        copy->Resize(xMin, yMin, xSize, ySize);
        
        return  std::shared_ptr<FITScube>(copy);
    }
    
    /**
     *  Rebin image to smaller resolution.
     *  @param nbin Number of pixel to merge for each axis. If nbin[i]=0 or note defined, the i axis isn't rebined. Note that the number of pixel to be merged shall be a multiple of the total number of the pixel of that axis.
     *  @return Return rebined image as a new FITScube data cube.
     */
    template< typename T >
    std::shared_ptr<FITScube> FITSimg<T>::Rebin (const std::vector<size_t>& nbin, bool doMean) const
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
        
        for(size_t k = 1; k <= this->GetDimension(); k++)
        {
            if(hdu.Exists("CDELT"+std::to_string(k)))
                copy->HDU().ValueForKey("CDELT"+std::to_string(k),
                                         this->HDU().GetDoubleValueForKey("CDELT"+std::to_string(k))
                                         * static_cast<double>(Size(k))/static_cast<double>(copy->Size(k)));
            else
                copy->HDU().ValueForKey("CDELT"+std::to_string(k),
                                         static_cast<double>(Size(k))/static_cast<double>(copy->Size(k)),"");

            if(hdu.Exists("CRVAL"+std::to_string(k)))
                copy->HDU().ValueForKey("CRVAL"+std::to_string(k),
                                         this->HDU().GetDoubleValueForKey("CRVAL"+std::to_string(k))
                                         + static_cast<double>(Size(k))/static_cast<double>(copy->Size(k))/2.);
            else
                copy->HDU().ValueForKey("CRVAL"+std::to_string(k),
                                         static_cast<double>(Size(k))/static_cast<double>(copy->Size(k))/2.);
        }

        if((verbose & verboseLevel::VERBOSE_DETAIL) == verboseLevel::VERBOSE_DETAIL)
            std::cout<<" Rebining :";

        bool handle=WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            size_t pos = 0;
            for(size_t n = 0; n < copy->Size(); n++)
            {
                if(!(((n+1)/copy->Size(0)*100) % 10 ) &&
                    ((n+1)/copy->Size(0)*100) >= 1 &&
                    (verbose & verboseLevel::VERBOSE_DETAIL) == verboseLevel::VERBOSE_DETAIL)
                    std::cout<<"."<<std::flush;
                
                std::valarray<T> val = arr[std::gslice(pos,
                                                        std::valarray<size_t>(size.data(), size.size()),
                                                        std::valarray<size_t>(stride.data(), stride.size()))];
                msk[std::gslice(pos,
                            std::valarray<size_t>(size.data(), size.size()),
                            std::valarray<size_t>(stride.data(), stride.size()))] = true;

                

                copy->SetPixelValue(val.sum()/((doMean)?bin_size:1.), n);

                while(pos < msk.size() && msk[pos])
                    pos++;

                if( pos 
                    >= Nelements())
                    break;
            }
        });
        if(!handle)
            throw FITSexception(SHARED_NULPTR,"FITSimg<T>::Rebin","missing data");

        if((verbose & verboseLevel::VERBOSE_DETAIL) == verboseLevel::VERBOSE_DETAIL)
            std::cout<<" \033[33m DONE\033[0m"<<std::endl;
        
        naxis.clear();
        stride.clear();
        size.clear();
        msk.resize(0);
        
        return  std::shared_ptr<FITScube>(copy);
    }
    
    template < typename T >
    void FITSimg<T>::Print() const
    {

        WithTypedData<T>([&](const std::valarray<T>& arr)
        {
            for (size_t k = 0; k < arr.size(); k++)
            {
                std::cout<<arr[k]<<" ";

                for(size_t i = 0; i < Naxis.size(); i++)
                    std::cout<<WorldCoordinates(k)[i]<<"    ";
                
                std::cout<<arr[k]<<"   "<<mask[k]<<std::endl;
            };
        });
    }
    
    
#pragma endregion
#pragma endregion
#pragma region - External opperators    
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) == (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) <= (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) < (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) >= (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) > (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) != (*(img2.template GetData<T>())) );
        test[img1.GetMask() || img2.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) == val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) <= val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) < val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) >= val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) > val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( (*(img1.template GetData<T>())) != val );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( val ==  (*(img1.template GetData<T>())) );
        test[img1.GetMask()] = false;

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
        std::valarray<bool> test = ( val <=  (*(img1.template GetData<T>())) );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( val <  (*(img1.template GetData<T>())) );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( val >=  (*(img1.template GetData<T>())) );
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( val >  (*(img1.template GetData<T>())));
        test[img1.GetMask()] = false;
        
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
        std::valarray<bool> test = ( val !=  (*(img1.template GetData<T>())) );
        test[img1.GetMask()] = false;
        
        return test;
    }
    
#pragma endregion

}

#endif

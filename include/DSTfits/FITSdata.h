//
//  FITSdata.h
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/
//

#ifndef _DSL_FITSdata_
#define _DSL_FITSdata_

#include <fitsio.h>
#include <cmath>
#include <limits>
#include <typeinfo>
#include <cxxabi.h>
#include <string>

namespace DSL
{
    
    typedef std::vector<double> pixelCoords;
    typedef std::vector<double> worldCoords;

    typedef std::vector<pixelCoords> pixelVectors;
    typedef std::vector<worldCoords> worldVectors;
    
#ifdef Darwinx86_64
    enum class verboseLevel: uint8_t
#else
    enum verboseLevel
#endif
    {
        VERBOSE_NONE    = 0x00,
        VERBOSE_BASIC   = 0x01,
        VERBOSE_DETAIL  = 0xF0,
        VERBOSE_ALL     = 0x0F,
        VERBOSE_HDU     = 0x02,
        VERBOSE_IMG     = 0x04,
        VERBOSE_TBL     = 0x08,
        VERBOSE_WCS     = 0x0A,
        VERBOSE_DEBUG   = 0xFF
    };

    inline verboseLevel verbose = verboseLevel::VERBOSE_NONE;

    verboseLevel operator|(verboseLevel a, verboseLevel b);
    verboseLevel operator&(verboseLevel a, verboseLevel b);
    verboseLevel& operator|=(verboseLevel& a, verboseLevel b);
    verboseLevel& operator&=(verboseLevel& a, verboseLevel b);
    verboseLevel operator~(verboseLevel a);
    std::ostream& operator<<(std::ostream& os, verboseLevel v);

    enum ttype
    {
        /**
         * @enum Possible type for FITS HDU bloc
         */
        tascii = ASCII_TBL, //!< FITS ASCII table
        tbinary= BINARY_TBL, //!< FITS binary table
        timg   = IMAGE_HDU, //!< FITS image or datacube
        tany   = ANY_HDU,  //!< Any of those listed above
    };
    
    enum dtype
    {
        /**
         * @enum Possible type for FITS embeded data
         */
        tsbyte     = TSBYTE,
        tshort     = TSHORT,    // int16_t
        tushort    = TUSHORT,   // uint16_t
        tint       = TINT,      // int32_t
        tuint      = TUINT,     // uint32_t
        tlong      = TLONG,     // int32_t
        tulong     = TULONG,    // uint32_t
        tlonglong  = TLONGLONG, // int64_t
        tulonglong = TULONGLONG,// uint64_t
        tfloat     = TFLOAT,
        tdouble    = TDOUBLE,
        tstring    = TSTRING,
        tlogical   = TLOGICAL,
        tbit       = TBIT,
        tbyte      = TBYTE,
        tcplx      = TCOMPLEX,
        tdbcplx    = TDBLCOMPLEX,
        tnone      = 666,
    };

    enum key_type
    {
        /*!
         *  @enum Possible type for FITS keywords
         *  @details key_type defines the base type of the FITS KEYWORD value.
         */
        fChar     = TSTRING,       //!< string
        fShort    = TSHORT ,       //!< int8_t
        fUShort   = TUSHORT ,      //!< uint8_t
        fInt      = TINT   ,       //!< int16_t
        fUInt     = TUINT   ,      //!< uint16_t
        fLong     = TLONG  ,       //!< int32_t
        fULong    = TULONG  ,      //!< uint32_t
        fLongLong = TLONGLONG,     //!< int64_t
        fULongLong= TULONGLONG,        //!< uint64_t
        fBool     = TLOGICAL,      //!< boolean
        fFloat    = TFLOAT,        //!< floiting poind
        fDouble   = TDOUBLE,       //!< double floiting point
        fByte     = TBYTE,
        fUndef,                   //!< undefined
    };

}
#endif

#ifndef DSL_DATAARRAY_H
#define DSL_DATAARRAY_H

#include <valarray>
#include <memory>
#include <functional>
#include <typeindex>
#include <cstddef>
#include <type_traits>

namespace DSL {

    // forward declaration of template derived class used by applyIfType
    template<typename T>
    struct FitsArray;


    struct FitsArrayBase
    {
        virtual ~FitsArrayBase() = default;
        virtual size_t size() const = 0;
        virtual double get(const size_t& idx) const = 0;
        virtual void   set(const size_t& idx, double v) = 0;
        virtual std::type_index type() const = 0;
        
        // attempt to execute fn with the typed valarray<T>&; returns true if successful
        template<typename T, typename Fn>
        bool applyIfType(Fn &&fn)
        {
            if(type() != std::type_index(typeid(T))) return false;
            auto *d = static_cast< FitsArray<T>* >(this);
            fn(d->ref());
            return true;
        }
    };

    template<typename T>
    struct FitsArray : FitsArrayBase
    {
        std::valarray<T> arr;
        FitsArray() = default;
        FitsArray(size_t n) : arr(static_cast<std::size_t>(n)) {}
        FitsArray(const std::valarray<T>& v) : arr(v) {}
        
        size_t size() const override { return arr.size(); }
        double get(const size_t& idx) const override { return static_cast<double>( arr[ idx ] ); }
        void   set(const size_t& idx, double v) override { arr[ idx ] = static_cast<T>( v ); }
        std::type_index type() const override { return std::type_index(typeid(T)); }
        std::valarray<T>& ref() { return arr; }
        const std::valarray<T>& ref() const { return arr; }
    };

    template<typename U>
    constexpr bool is_allowed_storage_type_v =
        std::is_same_v<U,uint8_t> || std::is_same_v<U,int8_t> ||
        std::is_same_v<U,uint16_t> || std::is_same_v<U,int16_t> ||
        std::is_same_v<U,uint32_t> || std::is_same_v<U,int32_t> ||
        std::is_same_v<U,uint64_t> || std::is_same_v<U,int64_t> ||
        std::is_same_v<U,size_t>  ||
        std::is_same_v<U,float>   || std::is_same_v<U,double> ;

    template<typename S, typename T>
    inline bool safe_cast_check_scalar(const S &v) noexcept
    {

        if(!is_allowed_storage_type_v<T> || !is_allowed_storage_type_v<S>)
            return false;

        if constexpr (!std::is_convertible_v<S, T>)
            return false;

        if constexpr (std::is_floating_point_v<S> && std::is_integral_v<T>)
        {
            // no NaN/inf, in-range and integer-valued
            if (!std::isfinite(static_cast<long double>(v)))
                return false;
                
            long double ld = static_cast<long double>(v);

            if (ld < static_cast<long double>(std::numeric_limits<T>::lowest()) ||
                ld > static_cast<long double>(std::numeric_limits<T>::max()))
                return false;
            
            // reject fractional part (avoid silent truncation)
            return true;
        }
        else if constexpr (std::is_integral_v<S> && std::is_integral_v<T>)
        {
            // check range (signed/unsigned differences handled by numeric_limits)
            long double ld = static_cast<long double>(v);
            return (ld >= static_cast<long double>(std::numeric_limits<T>::lowest()) &&
                    ld <= static_cast<long double>(std::numeric_limits<T>::max()));
        }
        else if constexpr (std::is_floating_point_v<S> && std::is_floating_point_v<T>)
        {
            // only check for finite values
            return std::isfinite(static_cast<long double>(v));
        } else
        {
            // other conversions (e.g. integral->float) are generally safe
            return true;
        }

        return true;
    }

    template<typename> inline constexpr bool always_false_v = false;

#ifdef Darwinx86_64
    auto demangle = [](const char* name) -> std::string
    {
        int status = 0;
        char* dem = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string s = (status == 0 && dem) ? dem : name;
        free(dem);
        return s;
    };
#else
    inline std::string demangle(const char* name)
    {
        int status = 0;
        char* dem = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string s = (status == 0 && dem) ? dem : name;
        free(dem);
        return s;
    }
#endif
     

} // namespace DSL

#endif // DSL_DATAARRAY_H

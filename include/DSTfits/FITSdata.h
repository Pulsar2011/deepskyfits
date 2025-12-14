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
#include <array>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <memory>

namespace DSL
{
    /** @brief Global CFITSIO mutex (serialize all CFITSIO calls across threads). */
    extern std::recursive_timed_mutex g_cfitsio_mutex;

    /**
     * @brief RAII guard for global CFITSIO critical section.
     * @details Locks the global recursive mutex for the lifetime of the guard.
     */
    struct CFITSIOGuard
    {
        std::unique_lock<std::recursive_timed_mutex> lk;
        CFITSIOGuard(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000))
        {
            // Attempt to acquire the global CFITSIO lock with a timeout to avoid hangs
            if(!g_cfitsio_mutex.try_lock_for(timeout))
            {
                // Construct a minimal exception explaining lock contention
                throw std::runtime_error("CFITSIOGuard timeout: possible CFITSIO deadlock");
            }
            lk = std::unique_lock<std::recursive_timed_mutex>(g_cfitsio_mutex, std::adopt_lock);
        }
    };

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
        fULongLong= TULONGLONG,    //!< uint64_t
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

    /*!
     * \brief Polymorphic base for typed FITS array storage.
     *
     * Provides a uniform interface for accessing and modifying array elements
     * as doubles, querying size, and retrieving the concrete storage type via
     * std::type_index. Derived classes wrap std::valarray<T>.
     *
     * \note Only interface is declared here; inline helper applyIfType is implemented below.
     */
    struct FitsArrayBase
    {
        virtual ~FitsArrayBase() = default;
        /*!
         * \brief Get number of elements in the array.
         * \return Element count.
         */
        virtual size_t size() const = 0;
        /*!
         * \brief Read an element as double.
         * \param idx Zero-based index.
         * \return Value converted to double.
         */
        virtual double get(const size_t& idx) const = 0;
        /*!
         * \brief Write an element from double.
         * \param idx Zero-based index.
         * \param v Value to store (cast to underlying type).
         */
        virtual void   set(const size_t& idx, double v) = 0;
        /*!
         * \brief Return the concrete storage type of the array.
         * \return std::type_index of the underlying T.
         */
        virtual std::type_index type() const = 0;
        
        /*!
         * \brief Apply a functor if the underlying type matches T.
         *
         * Attempts a downcast to FitsArray<T> and, if successful, invokes the
         * provided callable with a reference to the internal std::valarray<T>.
         *
         * \tparam T Expected element type.
         * \tparam Fn Callable taking std::valarray<T>&.
         * \param fn Function to execute on the typed storage.
         * \return true if the type matched and fn was invoked; false otherwise.
         */
        template<typename T, typename Fn>
        bool applyIfType(Fn &&fn)
        {
            if(type() != std::type_index(typeid(T))) return false;
            auto *d = static_cast< FitsArray<T>* >(this);
            fn(d->ref());
            return true;
        }
    };

    /*!
     * \brief Typed FITS array wrapper.
     *
     * Holds a std::valarray<T> and provides virtual overrides for size/get/set/type,
     * along with direct accessors to the underlying valarray via ref().
     *
     * \tparam T Element type of the storage.
     */
    template<typename T>
    struct FitsArray : FitsArrayBase
    {
        std::valarray<T> arr;
        /*!
         * \brief Default-construct an empty array.
         */
        FitsArray() = default;
        /*!
         * \brief Construct with a fixed size.
         * \param n Number of elements.
         */
        FitsArray(size_t n) : arr(static_cast<std::size_t>(n)) {}
        /*!
         * \brief Construct from an existing valarray.
         * \param v Source valarray to copy.
         */
        FitsArray(const std::valarray<T>& v) : arr(v) {}
        
        /*!
         * \brief Number of elements.
         * \return Size of valarray.
         */
        size_t size() const override { return arr.size(); }
        /*!
         * \brief Read element at index as double.
         * \param idx Zero-based index.
         * \return Value converted to double.
         */
        double get(const size_t& idx) const override { return static_cast<double>( arr[ idx ] ); }
        /*!
         * \brief Write element at index from double.
         * \param idx Zero-based index.
         * \param v Value to assign after casting to T.
         */
        void   set(const size_t& idx, double v) override { arr[ idx ] = static_cast<T>( v ); }
        /*!
         * \brief Report the underlying storage type.
         * \return std::type_index of T.
         */
        std::type_index type() const override { return std::type_index(typeid(T)); }
        /*!
         * \brief Access the underlying valarray.
         * \return Mutable reference to std::valarray<T>.
         */
        std::valarray<T>& ref() { return arr; }
        /*!
         * \brief Access the underlying valarray.
         * \return Const reference to std::valarray<T>.
         */
        const std::valarray<T>& ref() const { return arr; }
    };

    /*!
     * \brief Trait to restrict supported storage types.
     *
     * Evaluates to true for numeric types allowed in FITS storage:
     * various integers, size_t, float, and double.
     *
     * \tparam U Type to check.
     */
    template<typename U>
    constexpr bool is_allowed_storage_type_v =
        std::is_same_v<U,uint8_t> || std::is_same_v<U,int8_t> ||
        std::is_same_v<U,uint16_t> || std::is_same_v<U,int16_t> ||
        std::is_same_v<U,uint32_t> || std::is_same_v<U,int32_t> ||
        std::is_same_v<U,uint64_t> || std::is_same_v<U,int64_t> ||
        std::is_same_v<U,size_t>  ||
        std::is_same_v<U,float>   || std::is_same_v<U,double> ;

    /*!
     * \brief Validate that a scalar value can be safely cast from S to T.
     *
     * Performs type- and range-checks to avoid undefined behavior or silent
     * truncation when converting numeric types. Checks include:
     * - Allowed storage types (is_allowed_storage_type_v)
     * - Convertibility (std::is_convertible_v)
     * - Finite checks for floating points
     * - Range checks for integral targets
     *
     * \tparam S Source type.
     * \tparam T Target type.
     * \param v Value to validate.
     * \return true if the cast is considered safe; false otherwise.
     */
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

    /*!
     * \brief Helper variable template for static_assert in templates.
     *
     * Always false, used to trigger compile-time errors in dependent contexts.
     */
    template<typename> inline constexpr bool always_false_v = false;

#ifdef Darwinx86_64
    /*!
     * \brief Demangle a C++ type or symbol name (Darwin x86_64 variant).
     *
     * Uses abi::__cxa_demangle to obtain a human-readable C++ name.
     * Falls back to the input name on failure.
     *
     * \param name Mangled symbol name (e.g., typeid(T).name()).
     * \return Demangled name string if available; otherwise original.
     */
    auto demangle = [](const char* name) -> std::string
    {
        int status = 0;
        char* dem = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string s = (status == 0 && dem) ? dem : name;
        free(dem);
        return s;
    };
#else
    /*!
     * \brief Demangle a C++ type or symbol name.
     *
     * Uses abi::__cxa_demangle to convert a mangled name to a readable one.
     * Returns the input name if demangling fails.
     *
     * \param name Mangled symbol name (e.g., typeid(T).name()).
     * \return Demangled name string if available; otherwise original.
     */
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

//
//  FITStable.h
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


#ifndef _DSL_FITStable_
#define _DSL_FITStable_

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <any>
#include <memory>
#include <vector>
#include <valarray>
#include <limits>
#include <stdexcept>
#include <functional>
#include <iomanip>
#include <typeindex> 
#include <numeric>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <type_traits>


#include "FITShdu.h"
#include "FITSdata.h"
#include "FITSexception.h"

#include <fitsio.h>
#include <mutex>

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
        CFITSIOGuard() : lk(g_cfitsio_mutex) {}
    };

#if __cplusplus < 201703L
    /**
     * @brief Fallback replacement for std::bad_any_cast on pre-C++17 compilers.
     */
    struct bad_any_cast : std::bad_cast {};

    /**
     * @brief Lightweight std::any-like container for pre-C++17.
     * @details Stores a heap-allocated type-erased holder with cloning support.
     */
    class any_type {
        struct base
        {
            virtual ~base(){} virtual base* clone() const = 0;
        };
      
        template<typename U> struct holder : base
        {
            U value;
            explicit holder(U v): value(std::move(v)) {}
            base* clone() const override { return new holder<U>(value); }
        };
        
        base* ptr;
        public:
            any_type():ptr(nullptr){}
            
            template<typename U>
            any_type(U v):ptr(new holder<U>(std::move(v))){}
            
            any_type(const any_type& o):ptr(o.ptr?o.ptr->clone():nullptr){}
            
            any_type& operator=(any_type o){ std::swap(ptr,o.ptr); return *this; }
            
            ~any_type(){ delete ptr; }
        template<typename U>
        U& get()
        {
            auto h = dynamic_cast<holder<U>*>(ptr);
            if(!h) throw bad_any_cast();
            return h->value;
        }
    };
    /**
     * @brief Helper to extract a value from any_type (pre-C++17 helper).
     * @tparam U Value type.
     * @param a Source any_type.
     * @return Extracted value of type U.
     * @throws bad_any_cast if type mismatches.
     */
    template<typename U>
    inline U any_cast_any(const any_type& a){ return const_cast<any_type&>(a).get<U>(); }

#endif

    class FITStable;
    class FITSform;
    class RowSet;

    template<typename T>
    class RowSetBuilder;

    template<typename T>
    class ColumnView;
    
    class ColumnHandle;

    template<typename T>
    class ColumnFilterExpr;

    namespace TForm
    {
        template<typename T>
        struct columnData;

        struct columnDataBase
        {
            virtual ~columnDataBase() = default;
            virtual size_t size() const = 0;
            virtual std::type_index type() const = 0;
        };

        /**
         * @brief Typed storage for a column's in-memory data.
         * @tparam T scalar or vector payload type held by a column.
         */
        template<typename T>
        struct columnData : columnDataBase
        {
            std::vector<T> arr;
            columnData() = default;
            /*!
             * \brief Construct a storage with a given size.
             * \param n Number of elements to allocate.
             */
            columnData(size_t n) : arr(static_cast<std::size_t>(n)) {}
            /*!
             * \brief Construct a storage from an existing vector.
             * \param v Values to initialize from.
             */
            columnData(const std::vector<T>& v) : arr(v) {}

            /*!
             * \brief Number of stored elements.
             * \return Vector size.
             */
            size_t size() const override { return arr.size(); }
            /*!
             * \brief Report the payload type held by this storage.
             * \return std::type_index of T.
             */
            std::type_index type() const override { return std::type_index(typeid(T)); }
            /*!
             * \brief Mutable access to the underlying data vector.
             */
            std::vector<T>&       ref()       { return arr; }
            /*!
             * \brief Const access to the underlying data vector.
             */
            const std::vector<T>& ref() const { return arr; }
        };
    }

    using namespace TForm;
    
#pragma region - FITSform class definition
    /**
     * @class FITSform
     * @brief FITS table's column descriptor and storage proxy.
     * @author GILLARD William
     * @details
     *  Encapsulates column metadata (name, dtype, units, repeats, width, scale/zero)
     *  and provides typed access to the in-memory data buffer through values<T>().
     */
    class FITSform
    {
    private:
#pragma region -- private member
        const std::string fname;
        const dtype ftype;
        std::string funit;
        
        mutable double fscale;
        mutable double fzero;
        
        int64_t frepeat;
        int64_t fwidth;

        size_t fpos;

        std::unique_ptr<columnDataBase> fdata;

    protected:

        template<typename T>
        columnData<T>* storage()
        {
            return (fdata && fdata->type() == std::type_index(typeid(T)))
                   ? static_cast<columnData<T>*>(fdata.get()) : nullptr;
        }

        template<typename T>
        const columnData<T>* storage() const
        {
            return (fdata && fdata->type() == std::type_index(typeid(T)))
                   ? static_cast<const columnData<T>*>(fdata.get()) : nullptr;
        }
        
        template<typename T>
        void allocateStorageIfNeeded()
        {
            if(storage<T>()) return;
            fdata = std::make_unique< columnData<T> >();
        }
        /*!
         * \brief Allocate typed storage with a specific size.
         * \tparam T Payload type.
         * \param n Number of elements to pre-allocate.
         */
        template<typename T>
        void allocateStorageWithSize(size_t n)
        {
            fdata = std::make_unique< columnData<T> >(n);
        }

#pragma endregion
#pragma region -- protected member function
        inline FITSform():fname(),ftype(dtype::tnone),funit(),fscale(1),fzero(0),frepeat(1),fwidth(0),fpos(0) {};
        
        inline void setNelem(const int64_t& n) {frepeat = n;}
        inline void setWidth(const int64_t& w) {fwidth  = w;}

        /**
         * @brief Initialize scale/zero defaults for pseudo-unsigned types.
         */
        void initWithType()
        {
            switch(ftype)
            {
                case tsbyte:
                    fscale = 1.0;
                    fzero  = -128;
                    break;
                
                case tushort:
                    fscale = 1.0;
                    fzero  = 32768;
                    break;

                case tuint:
                case tulong:
                    fscale = 1.0;
                    fzero  = 2147483648;
                    break;
                
                case tulonglong:
                    fscale = 1.0;
                    fzero = static_cast<double>(1ULL << 63);
                    break;
                
                default:
                    fscale = 1.0;
                    fzero  = 0.0;
                    break;
            }
        }
        
    public:
#pragma endregion
#pragma region -- definition
        typedef std::pair<float,float>   complex;
        typedef std::pair<double,double> dblcomplex;

        typedef std::vector< int8_t >                   int8Vector;
        typedef std::vector< uint8_t >                  uint8Vector;
        typedef std::vector< int16_t >                  int16Vector;
        typedef std::vector< uint16_t >                 uint16Vector;
        typedef std::vector< int32_t >                  int32Vector;
        typedef std::vector< uint32_t >                 uint32Vector;
        typedef std::vector< int64_t >                  int64Vector;
        typedef std::vector< uint64_t >                 uint64Vector;
        typedef std::vector< float >                    floatVector;
        typedef std::vector< double >                   doubleVector;
        typedef std::vector< bool  >                    boolVector;
        typedef std::vector< std::string >              stringVector;
        typedef std::vector< std::pair<float,float> >   complexVector;
        typedef std::vector< std::pair<double,double> > dblcomplexVector;

        /*!
         * \brief Convert 8 bits into a boolean vector (MSB-first).
         * \param v Output vector cleared and filled with 8 booleans.
         * \param bits Source 8-bit mask.
         */
        static void toBoolVector(boolVector* v, const uint8_t& bits)
        {
            v->clear();
            size_t i = 8;
            while (i--)
            {
                const size_t bit = i; // 7..0
                v->push_back( ((bits >> bit) & 0x1u) != 0 );
            }
        }

        /*!
         * \brief Convert 16 bits into a boolean vector (MSB-first).
         * \param v Output vector cleared and filled with 16 booleans.
         * \param bits Source 16-bit mask.
         */
        static void toBoolVector(boolVector* v, const uint16_t& bits)
        {
            v->clear();
            size_t i = 16;
            while (i--)
            {
                const size_t bit = i; // 15..0
                v->push_back( ((bits >> bit) & 0x1u) != 0 );
            }
        }

        /*!
         * \brief Convert 32 bits into a boolean vector (MSB-first).
         * \param v Output vector cleared and filled with 32 booleans.
         * \param bits Source 32-bit mask.
         */
        static void toBoolVector(boolVector* v, const uint32_t& bits)
        {
            v->clear();
            size_t i = 32;
            while (i--)
            {
                const size_t bit = i; // 31..0
                v->push_back( ((bits >> bit) & 0x1u) != 0 );
            }
        }

        /*!
         * \brief Convert 64 bits into a boolean vector (MSB-first).
         * \param v Output vector cleared and filled with 64 booleans.
         * \param bits Source 64-bit mask.
         */
        static void toBoolVector(boolVector* v, const uint64_t& bits)
        {
            v->clear();
            size_t i = 64;
            while (i--)
            {
                const size_t bit = i; // 63..0
                v->push_back( ((bits >> bit) & 0x1ull) != 0 );
            }
        }

        /*!
         * \brief Pack up to 8 booleans into a byte (MSB-first).
         * \param v Source booleans; only first 8 are used.
         * \param bits Output bitfield.
         */
        static void fromBoolVector(const boolVector& v, uint8_t* bits)
        {
            (*bits) = 0;
            const size_t N = std::min(v.size(), static_cast<size_t>(8));
            for (size_t i = 0; i < N; ++i)
            {
                if (v[i])
                    (*bits) |= static_cast<uint8_t>(0x1u << (7 - i));
            }
        }

        /*!
         * \brief Pack up to 16 booleans into a 16-bit value (MSB-first).
         * \param v Source booleans; only first 16 are used.
         * \param bits Output bitfield.
         */
        static void fromBoolVector(const boolVector& v, uint16_t* bits)
        {
            (*bits) = 0;
            const size_t N = std::min(v.size(), static_cast<size_t>(16));
            for (size_t i = 0; i < N; ++i)
            {
                if (v[i])
                    (*bits) |= static_cast<uint16_t>(0x1u << (15 - i));
            }
        }

        /*!
         * \brief Pack up to 32 booleans into a 32-bit value (MSB-first).
         * \param v Source booleans; only first 32 are used.
         * \param bits Output bitfield.
         */
        static void fromBoolVector(const boolVector& v, uint32_t* bits)
        {
            (*bits) = 0;
            const size_t N = std::min(v.size(), static_cast<size_t>(32));
            for (size_t i = 0; i < N; ++i)
            {
                if (v[i])
                    (*bits) |= static_cast<uint32_t>(0x1u << (31 - i));
            }
        }

        /*!
         * \brief Pack up to 64 booleans into a 64-bit value (MSB-first).
         * \param v Source booleans; only first 64 are used.
         * \param bits Output bitfield.
         */
        static void fromBoolVector(const boolVector& v, uint64_t* bits)
        {
            (*bits) = 0;
            const size_t N = std::min(v.size(), static_cast<size_t>(64));
            for (size_t i = 0; i < N; ++i)
            {
                if (v[i])
                    (*bits) |= static_cast<uint64_t>(0x1ull << (63 - i));
            }
        }
        
        
#pragma endregion
#pragma region -- static member function
        static const std::string getDataType(const dtype &);
        static const dtype getDataTypeID(const std::string &);
        
        const std::string getTTYPE() const;
        
#pragma endregion
#pragma region -- ctor/dtor
        FITSform(const size_t&, const std::string&, const dtype&, const std::string unit="");
        FITSform(const size_t&, const std::string&, const dtype&, const double&, const double&, const std::string unit="");
        
        FITSform(const size_t&, const std::string&, const dtype&, const int64_t&, const int64_t&, const std::string unit="");
        FITSform(const size_t&, const std::string&, const dtype&, const int64_t&, const int64_t&, const double&, const double&, const std::string unit="");
        
        FITSform(const FITSform&);
        
        virtual ~FITSform(){};
        
#pragma endregion
#pragma region -- acessor
        inline const std::string& getName() const {return fname;}
        inline const dtype&       getType() const {return ftype;}
        inline const std::string& getUnit() const {return funit;}
        inline const double&      getScale()const {return fscale;}
        inline const double&      getZero() const {return fzero;}
        inline const size_t&      getPosition() const {return fpos;}
        
        inline const int64_t&    getNelem()  const {return frepeat;}
        inline const int64_t&    getWidth()  const {return fwidth;}

        /**
         * @brief CFITSIO storage code to use when writing this column.
         * @details
         *  Maps pseudo-unsigned dtypes to signed storage types and relies on
         *  BSCALE/BZERO reported by getTTYPE() to preserve values.
         * @return CFITSIO TFORM code (as dtype) that matches the storage.
         */
        inline int getCFITSIOStorageType() const
        {
            switch(ftype)
            {
                case tushort:       return static_cast<int>(tshort);     // store as SHORT with BZERO=32768
                case tuint:
                case tulong:        return static_cast<int>(tlong);      // store as LONG with BZERO=2147483648
                case tsbyte:        return static_cast<int>(tbyte);      // store as BYTE with BZERO=-128
                default:            return static_cast<int>(ftype);      // other types as-is
            }
        }

        /**
         * @brief Typed const access to column's in-memory values.
         * @tparam T Expected payload type for this column.
         * @return Reference to a const vector of values; empty if storage type differs.
         */
        template<typename T>
        const std::vector<T>& values() const
        {
            static const std::vector<T> empty;
            const auto* s = storage<T>();
            if(!s) return empty;
            return s->ref();
        }

        /*!
         * \brief Mutable access to column values of type T.
         * \tparam T Expected payload type.
         * \return Reference to the mutable vector of values.
         * \throws std::bad_cast if internal storage is not T.
         */
        template<typename T>
        std::vector<T>& values()
        {
            auto* s = storage<T>();
            if(!s)
                throw std::bad_cast();
            return s->ref();
        }

#pragma endregion

#pragma region -- pur virtual function

        /**
         * @brief Number of rows (cells) stored in this column.
         */
        virtual size_t size() const =0;

        /**
         * @brief Append one value to this column (type-erased).
         * @param value std::any wrapping a T value.
         * @throws std::bad_any_cast if value type mismatches.
         */
        virtual void push_back(const std::any& value) = 0;
        virtual std::unique_ptr<FITSform> clone() const = 0; // add
        virtual void sortOn(const std::vector<size_t>& order) = 0;

#pragma endregion

#pragma region -- modifier
        inline void setUnit    (const std::string& s){funit.clear(); funit+=s;}
        inline void setPosition(const size_t& p)    {fpos = p;}
        
#pragma endregion
#pragma region -- Saving & printing
        /**
         * @brief Emit a formatted description of this column to a stream.
         */
        virtual void Dump( std::ostream& ) const;

        /**
         * @brief Write this column's data to an opened CFITSIO table.
         * @param fptr Shared pointer to CFITSIO handle.
         * @param first_row 1-based first row to write.
         * @throws FITSexception on CFITSIO error or invalid state.
         */
        virtual void write(const std::shared_ptr<fitsfile>&, const int64_t&)=0;
#pragma endregion
    };
#pragma endregion

#pragma region - FITScolumn class definition
    /**
     * @class FITScolumn
     * @brief Typed column container for FITS table data.
     * @details
     *  Specializes FITSform with typed in-memory storage and helpers to update
     *  TFORM-related metadata (repeat, width) when values are appended.
     * @tparam T Scalar payload or std::vector<scalar> payload type.
     */
    template<typename T>
    class FITScolumn: public FITSform
    {
        /**
         * @class FITScolumn  FITStable.h "fitsExtractor/FITStable.h"
         * @author GILLARD William
         * @date 08/08/2017
         * @brief Helper class to read and write FITS columns data
         * @details This class provide methods to intercats with the data contained into columns of FITS ASCII and BINARY table. Once modified, it does not alter the fitsfile content. In order the modification are propagated to the fitsfile, the FITScolumn instance have to be passed to the FITStable instance. The later is directly responsible to managing the FITStable within the fitsfile.
         **/
    protected:
        typedef std::vector<T> col_map;
        //col_map fdata;

    private:

#pragma region -- private member function
        FITScolumn();
        
        void Update(const std::string&);
        void Update(const std::vector<std::string>&);
        
        template<typename U>
        void Update(const U&);
        
        template<typename U>
        void Update(const std::vector<U>&);
        
        template<typename U>
        void Update(const std::pair<U,U>&);
        
        template<typename U>
        void Update(const std::vector<std::pair<U,U>>&);

        template<typename InputIt>
        void Update(InputIt, InputIt);
        
        template<typename U>
        void dump( std::ostream&, const U& ) const;
        
        template<typename P>
        void dump( std::ostream&, const std::pair<P,P>& ) const;

        template<typename Q>
        void dump( std::ostream&, const std::vector<Q>& ) const;
        
        template< typename L >
        void dump( std::ostream&, const std::vector< std::pair<L,L> >&) const;
        
    public:
        
#pragma endregion
#pragma region -- definition

        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
        
        
#pragma endregion
#pragma region -- ctor/dtor
        FITScolumn(const std::string&, const dtype&, const std::string unit="", const size_t pos=0);
        FITScolumn(const std::string&, const dtype&, const double&, const double&, const std::string unit="", const size_t pos=0);
        FITScolumn(const std::string&, const dtype&, const int64_t&, const int64_t&, const std::string unit="", const size_t pos=0);
        FITScolumn(const std::string&, const dtype&, const int64_t&, const int64_t&, const double&, const double&, const std::string unit="", const size_t pos=0);
        FITScolumn(const FITScolumn&);
        
        virtual ~FITScolumn();
        
#pragma endregion
#pragma region -- accessor

        /**
         * @brief Number of rows (cells) stored in this column.
         */
        size_t size() const override { return this->values<T>().size(); }


        /**
         * @brief Append one value to this column (type-erased).
         * @param value std::any wrapping a T value.
         * @throws std::bad_any_cast if value type mismatches.
         */
        void push_back(const std::any& value) override
        {
#if __cplusplus < 201703L
            const T& v = any_cast_any<T>(value);
            Update(v);
            allocateStorageIfNeeded<T>();
            FITSform::values<T>().push_back(v);
#else
            if (const T* pv = std::any_cast<T>(&value))
            {
                Update(*pv);
                allocateStorageIfNeeded<T>();
                FITSform::values<T>().push_back(*pv);
            }
            else
            {
                throw std::bad_any_cast();
            }
#endif
        }

        /**
         * @brief Reorder the rows according to a permutation.
         * @param order New-to-old index mapping; order.size() must equal size().
         * @throws std::logic_error on mismatch or invalid indices.
         */
        void sortOn(const std::vector<size_t>& order) override
        {
            auto& data = this->values<T>();
            if(order.size() != data.size())
            {
                if(order.empty() && data.empty())
                    return;
                throw std::logic_error("FITScolumn::reorderRows size mismatch");
            }

            std::vector<T> reordered(order.size());
            for(size_t newPos = 0; newPos < order.size(); ++newPos)
            {
                const size_t src = order[newPos];
                if(src >= data.size())
                    throw std::logic_error("FITScolumn::reorderRows invalid index");
                reordered[newPos] = std::move(data[src]);
            }
            data = std::move(reordered);
        }

        // Read-only accessor for testing
        //inline const col_map& values() const { return data; }
        
#pragma endregion
#pragma region -- diagnoze
        /**
         * @brief Print column descriptor and a preview of data to the stream.
         */
        virtual void Dump( std::ostream& ) const override;

        /**
         * @brief Polymorphic clone.
         * @return Newly allocated FITScolumn<T> copy.
         */
        std::unique_ptr<FITSform> clone() const override
        {
            return std::unique_ptr<FITSform>( new FITScolumn<T>(*this) );
        }
#pragma endregion
#pragma region -- Saving
        /**
         * @brief Write column data to CFITSIO (generic scalar version).
         * @param fptr Shared pointer to CFITSIO handle.
         * @param first_row 1-based first row to write.
         * @throws FITSexception on CFITSIO error or invalid state.
         */
        virtual void write(const std::shared_ptr<fitsfile>&, const int64_t&) override;
#pragma endregion
    };

// Scalar type specializations
template<> void FITScolumn<uint32_t>                  ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::complex>         ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::dblcomplex>      ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<std::string>               ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<bool>                      ::write(const std::shared_ptr<fitsfile>&, const int64_t&);

// Vector type specializations
template<> void FITScolumn<FITSform::int8Vector>      ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::uint8Vector>     ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::int16Vector>     ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::uint16Vector>    ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::int32Vector>     ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::uint32Vector>    ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::int64Vector>     ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::uint64Vector>    ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::floatVector>     ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::doubleVector>    ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::complexVector>   ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::dblcomplexVector>::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::stringVector>    ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::boolVector>      ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
    
#pragma endregion
#pragma region - FITStable class definition
    /**
     * @class FITStable
     * @brief Abstraction over a FITS ASCII/BINARY table HDU.
     * @author GI::ARD William
     * @details
     *  Wraps CFITSIO HDU navigation, column discovery/loading, typed access,
     *  in-memory mutation, and writing back to a (new or existing) table HDU.
     */
    class FITStable
    {
    public:
        typedef std::vector< std::unique_ptr<FITSform> > columns_list;
        typedef std::vector< std::vector<std::string> > clist;

    protected:
        columns_list fcolumns;

        template<typename T>
        ColumnFilterExpr<T> operator()(const std::string& columnName);

    private:
#pragma endregion
#pragma region -- private member
        const volatile ttype ftbl_type;
        FITShdu hdu;                              //!< Header of the table
        mutable size_t nrows_cache;          //!< Cache for number of rows

#pragma endregion
#pragma region -- private member function
        void load(const std::shared_ptr<fitsfile>&, const size_t& start=1);
        std::unique_ptr<FITSform> readColumn(const std::shared_ptr<fitsfile>&, const size_t&, const size_t&);
    

        //FITSform* readArray(const FITSform& , const int64_t&);
        
        template <typename T>
        void read(FITScolumn<T>*, const std::shared_ptr<fitsfile>& fptr, const size_t& row=1);
        
        template <typename T>
        void readVector(FITScolumn< std::vector<T> >*, const std::shared_ptr<fitsfile>& fptr, const size_t& row=1);
        
    public:

#pragma endregion
#pragma region -- ctor/dtor
        FITStable();
        FITStable(const std::string&);
        FITStable(const std::shared_ptr<fitsfile>&, const int&);
        FITStable(const std::shared_ptr<fitsfile>&, const std::string&);
        
        FITStable(const FITStable& tbl): ftbl_type(tbl.ftbl_type), hdu(tbl.hdu)
        {
            for(const auto& c : tbl.fcolumns)
            {
                fcolumns.push_back( c->clone() );
            }
        }
        
        ~FITStable();
        
#pragma endregion
#pragma region -- Properties
        /**
         * @brief Number of rows (max over columns).
         * @return Row count.
         */
        size_t nrows() const;

        /**
         * @brief Number of columns.
         */
        size_t ncols() const;

        /**
         * @brief List column metadata as {name, TFORM, unit} triples.
         */
        const clist    listColumns();

        /**
         * @brief Access column descriptor by name.
         * @param cname Column name.
         * @throws std::out_of_range if not found.
         */
        const std::unique_ptr<FITSform>& getColumn(const std::string& cname) const;

        /**
         * @brief Access column descriptor by 1-based position.
         * @param cindex Column position (TFIELD index).
         * @throws std::out_of_range if not found.
         */
        const std::unique_ptr<FITSform>& getColumn(const size_t& cindex) const;

        /**
         * @brief Retrive the name of the table extension
         * @return The extension name of \c this table or an empty string if the EXTNAME keyword wasn't found into the HDU.
         */
        inline const std::string GetName() const {if(hdu.Exists("EXTNAME")) return hdu.GetValueForKey("EXTNAME"); else return std::string("NO NAME");}       //!< Get the name of the image      

#pragma endregion
#pragma region -- Accessor
        inline const FITShdu& HDU() const {return hdu;}
        inline       FITShdu& HDU() {return hdu;}
        inline const ttype getTableType() const {return ftbl_type;}
        
         /**
         * @brief Begin a row set selection on a given column.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return RowSetBuilder<T> to build a row set selection.
         * @throws std::out_of_range if column not found.
         */
        template<typename T>
        RowSetBuilder<T> select(const std::string& columnName);

        /**
         * @brief Begin a row set selection on a given column.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return RowSetBuilder<T> to build a row set selection.
         * @throws std::out_of_range if column not found.
         */
        template<typename T>
        RowSetBuilder<T> select(const std::string& columnName) const;

        /**
         * @brief Access a column's data as a ColumnView<T>.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return ColumnView<T> for typed access.
         * @throws std::out_of_range if column not found.
         * @throws std::bad_cast if column type mismatches T.
         */
        template<typename T>
        ColumnView<T> column(const std::string& columnName);

        /**
         * @brief Access a column's data as a ColumnView<T>.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return ColumnView<T> for typed access.
         * @throws std::out_of_range if column not found.
         * @throws std::bad_cast if column type mismatches T.
         */
        template<typename T>
        ColumnView<T> column(const std::string& columnName) const;

        /**
         * @brief Access a column's data through a ColumnHandle.
         * @param columnName Column name.
         * @return ColumnHandle for type-erased access.
         * @throws std::out_of_range if column not found.
         */
        ColumnHandle operator[](const std::string& columnName);

        /**
         * @brief Access a column's data through a ColumnHandle.
         * @param columnName Column name.
         * @return ColumnHandle for type-erased access.
         * @throws std::out_of_range if column not found.
         */
        ColumnHandle operator[](const std::string& columnName) const;

        /**
         * @brief Begin a filter expression on a given column.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return ColumnFilterExpr<T> to build a filter expression.
         * @throws std::out_of_range if column not found.
         */
        template<typename T>
        ColumnFilterExpr<T> filter(const std::string& columnName)
        {
            (void)getColumn(columnName);      // ensure it exists
            return ColumnFilterExpr<T>(*this, columnName);
        }

        /**
         * @brief Begin a filter expression on a given column.
         * @tparam T Payload type of the column.
         * @param columnName Column name.
         * @return ColumnFilterExpr<T> to build a filter expression.
         * @throws std::out_of_range if column not found.
         */
        template<typename T>
        ColumnFilterExpr<T> filter(const std::string& columnName) const
        {
            (void)getColumn(columnName);      // ensure it exists
            return ColumnFilterExpr<T>(const_cast<FITStable&>(*this), columnName);
        }

        /**
         * @brief Apply a global permutation to reorder all columns consistently.
         * @param order New-to-old index mapping; must be a permutation of [0..n-1].
         * @throws std::logic_error on size mismatch, duplicates, or out-of-range.
         */
        void reorderRows(const std::vector<size_t>& order);

#pragma endregion        

#pragma region -- Modifier
        inline void setName(const std::string& tname) {hdu.ValueForKey("EXTNAME", tname, fChar);}
#pragma endregion
#pragma region -- Inserting/Updating data to column

#pragma region 1- Inseting new column
        
        /**
         * @brief Append a new empty column with a given dtype and TUNIT.
         * @param cname Column name.
         * @param type dtype to append.
         * @param tunit Unit string.
         * @throws FITSexception on unsupported dtype.
         */
        void InsertColumn(const std::string& cname, const dtype& type, const std::string& tunit);

        /**
         * @brief Append a populated column; validates row count consistency.
         * @param col New column to append.
         * @throws FITSexception on dimension mismatch.
         */
        void InsertColumn( std::shared_ptr<FITSform> col );
        
#pragma endregion
#pragma region 2- Inseting value to an existing column
        
#pragma endregion
#pragma region 3- Updating value from an existing column
        
#pragma endregion
#pragma endregion
#pragma region -- Diagnoze
        void Dump( std::ostream& );
#pragma endregion

#pragma region -- Saving to file
        /**
         * @brief Write the in-memory table to a CFITSIO handle, creating or moving to the HDU.
         * @param fptr Shared pointer to an opened fitsfile.
         * @param start 1-based first row where data will be written (append if start==nrows+1).
         * @throws FITSexception on CFITSIO error or invalid row number.
         */
        void write(const std::shared_ptr<fitsfile>& fptr, const int64_t&);
        /**
         * @brief Create (or replace) a FITS file and write this table into it.
         * @param filename Target FITS path; prepend '!' if replace==true.
         * @param start 1-based first row to write.
         * @param replace Overwrite file if true.
         * @throws FITSexception on CFITSIO error.
         */
        void write(const std::string&, const int64_t&, bool replace=false);
#pragma endregion

    };
#pragma endregion
#pragma region - FITStable Template implementation
    
    /**
     @brief Read column content
     @details Private methods used to read column the content of FITS scalar array. The method access the FITS column data and fill up a FITScolumn<T> instance with the corresponding data.

     @param tform address of a FITSform property container to be read
     @param row First row to be read
     @return FITScolumn<T> instance that contains FITS column prperties as well as std::map<size_t,T> container with the requested data.
     */
    template< typename T>
    void FITStable::read(FITScolumn<T>* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {      
        long nrows = 0; 
        int tbl_status = 0;

        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<T>");

        nrows_cache = static_cast<size_t>(nrows);
        int64_t nelem  = ( static_cast<int64_t>( nrows )-(row-1) );
        
        char* nullarray = new char[nelem];
        int   anynull   = 0;

        if(data->getType() == tshort)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            short* uarray = new short[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<short>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tushort)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned short* uarray = new unsigned short[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tint)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            int* uarray = new int[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<int>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tuint)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned int* uarray = new unsigned int[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned int>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tlong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            long* uarray = new long[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<long>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tulong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned long* uarray = new unsigned long[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned long>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tlonglong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            LONGLONG* uarray = new LONGLONG [nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<lonlong>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else if(data->getType() == tulonglong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            ULONGLONG* uarray = new ULONGLONG [nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned longlong>");
            }
            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(uarray[k]));
            }
            delete[] uarray;
        }
        else
        {
            T*    array  = new T   [nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>    (data->getPosition()), row, 1, nelem, array, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] array;

                throw FITSexception(tbl_status,"FITStable","read<T>");
            }

            for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
            {
                data->push_back(static_cast<T>(array[k]));
            }

            delete[] array;
        }

        return;
    }
    
    /**
     @brief Read column content
     @details Private methods used to read  the content of FITS vector array. The method access the FITS column data and fill up a FITScolumn<std::vector<T>> instance with the corresponding data.
     
     @param tform address of a FITSform property container to be read
     @param row First row to be read
     @return FITScolumn<std::vector<T>> instance that contains FITS column prperties as well as std::map<size_t,std::vector<T>> container with the requested data.
     */
    template< typename T>
    void FITStable::readVector(FITScolumn< std::vector<T> >* data,  const std::shared_ptr<fitsfile>& fptr,  const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complex>");

        nrows_cache = static_cast<size_t>(nrows);
        int64_t nelem = ( static_cast<int64_t>(nrows)-static_cast<int64_t>(row-1) )* static_cast<int64_t>(data->getNelem());
        
        char* nullarray = new char[nelem];
        int   anynull   = 0;

        if(data->getType() == tshort)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            short* uarray = new short[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
            return;
        }
        else if(data->getType() == tushort)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned short* uarray = new unsigned short[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tint)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            int* uarray = new int[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tuint)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned int* uarray = new unsigned int[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned int>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tlong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            long* uarray = new long[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tulong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            unsigned long* uarray = new unsigned long[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned long>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tlonglong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            LONGLONG* uarray = new LONGLONG[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else if(data->getType() == tulonglong)
        {
            // Special case for signed int32_t stored as unsigned in FITS (with BZERO/BSCALE)
            ULONGLONG* uarray = new ULONGLONG[nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, uarray, nullarray, &anynull, &tbl_status))
            {
                delete[] nullarray;
                delete[] uarray;
                
                throw FITSexception(tbl_status,"FITStable","read<unsigned short>");
            }
            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());
                
                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    tmpv[l] = static_cast<T>(uarray[k+l]);
                }
                
                data->push_back(tmpv);
            }
            delete[] uarray;
            delete[] nullarray;
        }
        else
        {
            T* array = new T [nelem];

            if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem , array, nullarray, &anynull, &tbl_status))
            {
                delete[] array;
                delete[] nullarray;

                throw FITSexception(tbl_status,"FITStable","read<T>");
            }

            for(int64_t k = 0; k < nelem; k += data->getNelem())
            {
                std::vector<T> tmpv = std::vector<T>(data->getNelem());

                for(int64_t l = 0; l < data->getNelem(); l++ )
                {
                    if( k+l >= nelem*data->getNelem())
                    {
                        std::cout<<l<<" + "<<k<<" = "<<l+k<<" % "<<nelem * static_cast<int64_t>(data->getNelem())<<std::endl;
                        throw FITSwarning("FITStable","readVector","Out of range : "+std::to_string(k+l)+" <-> "+std::to_string((static_cast<int64_t>(nrows)    -static_cast<int64_t>(row))*data->getNelem()));
                    }
                    tmpv[l] = static_cast<T>(array[k+l]);
                }

                data->push_back(tmpv);
            }

            delete[]  array;
            delete[]  nullarray;
        }
    }
    
#pragma endregion
#pragma region - FITScolumn class implementation

#pragma region --saving
    template< typename T >
    void FITScolumn<T>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        const auto& data = this->values<T>();
        if(fptr == nullptr)
        {
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<T>","write");
        }

        if(data.size() < 1)
        {
            throw FITSexception(NOT_TABLE,"FITScolumn<T>","write");
        }

        int64_t n = 0;
        int tbl_status  = 0;

        for(typename col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            if(n >= (first_row-1))
            {
                T array = (*it);

                if(ffpcl(fptr.get(),static_cast<int>(getType()), static_cast<int>(getPosition()), n+1, 1, getNelem(),  &array, &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITStable","write<T>");
                }
            }
            n++;
        }
    }
    
#pragma endregion
#pragma region -- ctor/dtor
    
    /**
     @brief Empty constructor
     @note Empty constructor is made private because \c this->fname and \c this->ftype are required.
     */
    template< typename T >
    FITScolumn<T>::FITScolumn():FITSform()
    {
        allocateStorageIfNeeded<T>();
    }
    
    /**
     @brief Default constructor
     @param name FITS column name
     @param type FITS column type
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name , const dtype& type, const std::string unit, const size_t pos):FITSform(pos,name,type,unit)
    {
        allocateStorageIfNeeded<T>();
    }

    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name, const dtype& type, const int64_t& r, const int64_t& w, const std::string unit, const size_t pos):FITSform(pos,name,type,r,w,unit)
    {
        allocateStorageIfNeeded<T>();
    }
    
    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name, const dtype& type, const int64_t& r, const int64_t& w, const double& s, const double& z, const std::string unit, const size_t pos):FITSform(pos,name,type,r,w,s,z,unit)
    {
        allocateStorageIfNeeded<T>();
    }
    
    /**
     Constructor

     @param name Coulumn name
     @param type Column data type
     @param scale data scaling factor
     @param zero data offset
     @param unit Column data units
     @param pos Column position in the FITStable
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name, const dtype& type, const double& scale, const double& zero, const std::string unit, const size_t pos):FITSform(pos,name,type,scale,zero,unit)
    {
        allocateStorageIfNeeded<T>();
    }
    
    /**
     @brief Copy constructor
     @param col FITS column to be copyed to this
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const FITScolumn<T>& col):FITSform(col)
    {
        // Avoid calling protected storage<T>() on another instance.
        // Use the public const accessor which is legal and equivalent.
        const auto& v = col.values<T>();
        if (!v.empty())
        {
            allocateStorageWithSize<T>(v.size());
            this->values<T>() = v;
        }
        else
        {
            allocateStorageIfNeeded<T>();
        }
    }
    
    template< typename T >
    FITScolumn<T>::~FITScolumn()
    {}
#pragma endregion

    template< typename T >
    template<typename InputIt>
    void FITScolumn<T>::Update(InputIt first, InputIt last)
    {
        using V = typename std::iterator_traits<InputIt>::value_type;

#if __cplusplus >= 201103L
        if(std::is_arithmetic<V>::value)
        {
            FITSform::setWidth(sizeof(V));
            int64_t cnt = 0;
            for(auto it = first; it != last; ++it) ++cnt;
            if(cnt > getNelem()) FITSform::setNelem(cnt);
            return;
        }
#endif

        std::vector<V> buffer;
#if __cplusplus >= 201103L
        using Cat = typename std::iterator_traits<InputIt>::iterator_category;
        if(std::is_base_of<std::random_access_iterator_tag, Cat>::value)
            buffer.reserve(static_cast<size_t>(std::distance(first,last)));
#endif
        for(; first != last; ++first)
            buffer.push_back(*first);

        Update(buffer); 
    }
    
    template< typename T >
    void FITScolumn<T>::Update(const std::string& str)
    {
        int64_t maxSize = getWidth();
        maxSize = (static_cast<int64_t>(str.size()) >= maxSize)? static_cast<int64_t>(str.size()) : maxSize;
        
        FITSform::setWidth(maxSize);
        FITSform::setNelem(1);
    }
    
    template< typename T >
    void FITScolumn<T>::Update(const std::vector<std::string>& vstr)
    {
        int64_t maxSize  = 0;
        int64_t maxWidth = 0;
        int64_t nelem   = getNelem();
        int64_t width   = getWidth();
        
        for(size_t i = 0; i < vstr.size(); i++)
        {
            maxSize  = static_cast<int64_t>(vstr.size());
            maxWidth = (static_cast<int64_t>(vstr[i].size()) > maxWidth) ? static_cast<int64_t>(vstr[i].size()) : maxWidth;
        }
        
        nelem = (maxSize > nelem) ? maxSize : nelem;
        width = (maxWidth > width) ? maxWidth : width;
        
        FITSform::setWidth(width);
        FITSform::setNelem(nelem);
    }
    
    template< typename T >
    template< typename U >
    void FITScolumn<T>::Update(const U& i)
    {
        setWidth(sizeof(U));
    }
    
    template< typename T >
    template< typename U >
    void FITScolumn<T>::Update(const std::vector<U>& i)
    {
        setWidth(sizeof(U));
        
        int64_t nelem = getNelem();
        nelem = (static_cast<int64_t>(i.size()) > nelem)? i.size() : nelem;
        setNelem(nelem);
    }
    
    template< typename T >
    template< typename U >
    void FITScolumn<T>::Update(const std::pair<U,U>& i)
    {
        setWidth(sizeof(U)*2);
    }
    
    template< typename T >
    template< typename U >
    void FITScolumn<T>::Update(const std::vector<std::pair<U,U>>& i)
    {
        setWidth(sizeof(U)*2);
        
        int64_t nelem = getNelem();
        nelem = (static_cast<int64_t>(i.size()) > nelem)? static_cast<int64_t>(i.size()) : nelem;
        setNelem(nelem);
    }
    
    
#pragma endregion
#pragma region -- diagnose
    template< typename T >
    void FITScolumn<T>::Dump( std::ostream& fout) const
    {
        FITSform::Dump(fout);
        const auto& data = this->values<T>();
        size_t n=0;
        for(typename col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            fout<<"\033[32m   |       \033[31m[\033[0m"<<n<<"\033[31m]\033[0m   "<<std::flush;
            dump(fout,*it);
            fout<<std::endl;
            n++;
        }
    }

    // --- Added specialized Dump implementations for byte/short columns ---

    /*!
     * \brief Specialized Dump for int8_t columns.
     * \details Prints values as decimal for tshort, otherwise as hex bytes.
     */
    template<>
    inline void DSL::FITScolumn<int8_t>::Dump(std::ostream& fout) const
    {
        FITSform::Dump(fout);
        const auto& data = this->values<int8_t>();
        size_t n = 0;
        for(const auto& v : data)
        {
            switch (getType())
            {
                case tshort:
                        fout<<"\033[32m   |       \033[31m[\033[0m"<<n<<"\033[31m]\033[0m   "
                         << static_cast<int16_t>(v)
                         << std::endl;
                    break;
                
                default:
                        fout<<"\033[32m   |       \033[31m[\033[0m"<<n<<"\033[31m]\033[0m   0x"
                         << std::uppercase << std::hex
                         << std::setw(2) << std::setfill('0')
                         << static_cast<unsigned int>(static_cast<uint8_t>(v))
                         << std::dec << std::nouppercase << std::endl;
                    break;
            }
            ++n;
        }
    }

    /*!
     * \brief Specialized Dump for uint8_t columns.
     * \details Prints values as decimal for tushort, otherwise as hex bytes.
     */
    template<>
    inline void DSL::FITScolumn<uint8_t>::Dump(std::ostream& fout) const
    {
        FITSform::Dump(fout);
        const auto& data = this->values<uint8_t>();
        size_t n = 0;
        for(const auto& v : data)
        {
            switch (getType())
            {
                case tushort:
                        fout<<"\033[32m   |       \033[31m[\033[0m"<<n<<"\033[31m]\033[0m   "
                         << static_cast<uint16_t>(v)
                         << std::endl;
                    break;

                default:
                        fout<<"\033[32m   |       \033[31m[\033[0m"<<n<<"\033[31m]\033[0m   0x"
                         << std::uppercase << std::hex
                         << std::setw(2) << std::setfill('0')
                         << static_cast<unsigned int>(static_cast<uint8_t>(v))
                         << std::dec << std::nouppercase << std::endl;
                    break;
            }
            ++n;
        }
    }

    /*!
     * \brief Specialized Dump for vector<uint8_t> columns.
     * \details Prints row size and preview, then each element in readable form.
     */
    template<>
    inline void DSL::FITScolumn< std::vector<uint8_t> >::Dump(std::ostream& fout) const
    {
        FITSform::Dump(fout);
        const auto& data = this->values< std::vector<uint8_t> >();
        size_t row = 0;

        for(const auto& vec : data)
        {
            fout<<"\033[32m   |       \033[31m[\033[0m"<<row<<"\033[31m]\033[0m   size="<<vec.size();
            if(!vec.empty())
            {
                switch (getType())
                {
                    case tushort:
                        fout << " first=" << static_cast<uint16_t>(vec.front())
                             << " last="  << static_cast<uint16_t>(vec.back());
                        break;
                    default:
                        fout << " first=0x"
                             << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                             << static_cast<unsigned int>(vec.front())
                             << " last=0x"
                             << std::setw(2) << std::setfill('0')
                             << static_cast<unsigned int>(vec.back())
                             << std::dec << std::nouppercase;
                        break;
                }
            }
            fout << std::endl;
            size_t idx = 0;
            for(const auto& b : vec)
            {
                switch (getType())
                {
                    case tushort:
                        fout << "   \033[32m|\033[0m      ("<<idx<<")   "
                             << static_cast<uint16_t>(b)
                             << std::endl;
                        break;
                    default:
                        fout << "   \033[32m|\033[0m      ("<<idx<<")  0x"
                             << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                             << static_cast<unsigned int>(b)
                             << std::dec << std::nouppercase << std::endl;
                        break;
                }
                ++idx;
            }
            ++row;
        }
    }

    /*!
     * \brief Specialized Dump for vector<int8_t> columns.
     * \details Prints row size and per-element hex values.
     */
    template<>
    inline void DSL::FITScolumn< std::vector<int8_t> >::Dump(std::ostream& fout) const
    {
        FITSform::Dump(fout);
        const auto& data = this->values< std::vector<int8_t> >();
        size_t row = 0;
        for(const auto& vec : data)
        {
            fout<<"\033[32m   \033[32m|\033[0m       \033[31m[\033[0m"<<row<<"\033[31m]\033[0m   size="<<vec.size();
            if(!vec.empty())
            {
                fout << " first=0x"
                     << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                     << static_cast<unsigned int>(vec.front())
                     << " last=0x"
                     << std::setw(2) << std::setfill('0')
                     << static_cast<int>(vec.back())
                     << std::dec << std::nouppercase;
            }
            fout << std::endl;
            size_t idx = 0;
            for(const auto& b : vec)
            {
                fout << "   \033[32m|\033[0m      ("<<idx<<")  0x"
                     << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                     << static_cast<int>(b)
                     << std::dec << std::nouppercase << std::endl;
                ++idx;
            }
            ++row;
        }
    }
    
    // Generic dump helpers
    /*!
     * \brief Dump a scalar value.
     * \tparam U Scalar type printable to ostream.
     */
    template< typename T>
    template< typename U >
    void FITScolumn<T>::dump( std::ostream& fout, const U& val) const
    {
        fout<<val<<std::flush;
    }
    /*!
     * \brief Dump a pair of scalar values.
     * \tparam P Scalar type.
     */
    template< typename T>
    template< typename P >
    void FITScolumn<T>::dump( std::ostream& fout, const std::pair<P,P>& val) const
    {
        fout<<val.first<<" , "<<val.second<<std::flush;
    }
    /*!
     * \brief Dump a vector of scalar values.
     * \tparam Q Scalar type.
     */
    template< typename T>
    template< typename Q >
    void FITScolumn<T>::dump( std::ostream& fout, const std::vector<Q>& val) const
    {
        // Fixed: treat 'data' as a single row vector, not a vector-of-vectors.
        if(val.empty())
            return;
        fout << "size=" << val.size()
             << " first=" << val.front()
             << " last="  << val.back()
             << std::endl;
        size_t idx = 0;
        for(const auto& v : val)
        {
            fout << "   \033[32m|\033[0m      ("<<idx<<")   " << v << std::endl;
            ++idx;
        }
    }
    /*!
     * \brief Dump a vector of pairs.
     * \tparam L Scalar type of pair elements.
     */
    template< typename T>
    template< typename L >
    void FITScolumn<T>::dump( std::ostream& fout, const std::vector< std::pair<L,L> >& val) const
    {
        // Fixed: same logic as above for vector of pairs.
        if(val.empty())
            return;
        fout << "size=" << val.size()
             << " first=" << val.front().first << " , " << val.front().second
             << " last="  << val.back().first  << " , " << val.back().second
             << std::endl;
        size_t idx = 0;
        for(const auto& p : val)
        {
            fout << "         ("<<idx<<")   " << p.first << " , " << p.second << std::endl;
            ++idx;
        }
    }
    
    #pragma region - RowSet / ColumnView helpers
    namespace detail
    {
        template<typename T> struct is_std_vector : std::false_type {};
        
        template<typename U, typename Alloc>
        struct is_std_vector<std::vector<U,Alloc>> : std::true_type {};
    }

    /**
     * @brief Immutable set of row indices to operate on.
     * @details Supports set algebra (&& for intersection, || for union) and iteration.
     */
    class RowSet
    {
        std::vector<size_t> indices_;
        public:
            RowSet() = default;
            /*!
             * \brief Construct from a list of indices.
             * \details Input is sorted and deduplicated.
             */
            explicit RowSet(std::vector<size_t> rows)
                : indices_(std::move(rows))
            {
                std::sort(indices_.begin(), indices_.end());
                indices_.erase(std::unique(indices_.begin(), indices_.end()), indices_.end());
            }
            /*!
             * \brief Number of indices in the set.
             */
            size_t size()  const { return indices_.size(); }
            /*!
             * \brief Whether the set is empty.
             */
            bool   empty() const { return indices_.empty(); }
            /*!
             * \brief Access the internal indices.
             * \return Const reference to indices vector.
             */
            const std::vector<size_t>& indices() const { return indices_; }
            /*!
             * \brief Intersection with another set.
             * \return Resulting RowSet containing common indices.
             */
            RowSet intersected(const RowSet& other) const
            {
                std::vector<size_t> out;
                out.reserve(std::min(indices_.size(), other.indices_.size()));
                std::set_intersection(indices_.begin(), indices_.end(),
                                      other.indices_.begin(), other.indices_.end(),
                                      std::back_inserter(out));
                return RowSet(std::move(out));
            }

            /*!
             * \brief Union with another set.
             * \return Resulting RowSet containing all unique indices.
             */
            RowSet united(const RowSet& other) const
            {
                std::vector<size_t> out;
                out.reserve(indices_.size() + other.indices_.size());
                std::set_union(indices_.begin(), indices_.end(),
                               other.indices_.begin(), other.indices_.end(),
                               std::back_inserter(out));
                return RowSet(std::move(out));
            }

            /*!
             * \brief Set difference (this \ other).
             * \return Indices present in this but not in other.
             */
            RowSet subtracted(const RowSet& other) const
            {
                std::vector<size_t> out;
                out.reserve(indices_.size());
                std::set_difference(indices_.begin(), indices_.end(),
                                    other.indices_.begin(), other.indices_.end(),
                                    std::back_inserter(out));
                return RowSet(std::move(out));
            }

            /*!
             * \brief Iterate over indices and invoke a callable.
             * \tparam Func Callable signature void(size_t).
             * \param fn Function to apply to each index.
             */
            template<typename Func>
            void forEach(Func&& fn) const
            {
                for(size_t idx : indices_) fn(idx);
            }
    };
    /*!
     * \brief Logical AND of two row sets (intersection).
     */
    inline RowSet operator&&(const RowSet& lhs, const RowSet& rhs)
    {
        return lhs.intersected(rhs);
    }
    /*!
     * \brief Logical OR of two row sets (union).
     */
    inline RowSet operator||(const RowSet& lhs, const RowSet& rhs)
    {
        return lhs.united(rhs);
    }

    template<typename T>
    class ColumnFilterExpr
    {
        FITStable& tbl_;
        std::string column_;

        template<typename Fn>
        RowSet buildWith(Fn&& mutator)
        {
            auto& form = *tbl_.getColumn(column_);
            RowSetBuilder<T> builder(form);
            mutator(builder);
            return builder.build();
        }

    public:
        /*!
         * \brief Construct a filter expression bound to a table and column.
         * \param tbl Owning table.
         * \param column Column name.
         */
        ColumnFilterExpr(FITStable& tbl, std::string column)
            : tbl_(tbl), column_(std::move(column)) {}

        /*!
         * \brief Equality filter.
         * \param value Scalar to compare.
         * \return RowSet of matching rows.
         */
        RowSet operator==(const T& value) { return buildWith([&](auto& b){ b.eq(value); }); }
        /*!
         * \brief Inequality filter.
         */
        RowSet operator!=(const T& value) { return buildWith([&](auto& b){ b.ne(value); }); }
        /*!
         * \brief Less-than filter.
         */
        RowSet operator< (const T& value) { return buildWith([&](auto& b){ b.lt(value); }); }
        /*!
         * \brief Less-or-equal filter.
         */
        RowSet operator<=(const T& value) { return buildWith([&](auto& b){ b.le(value); }); }
        /*!
         * \brief Greater-than filter.
         */
        RowSet operator> (const T& value) { return buildWith([&](auto& b){ b.gt(value); }); }
        /*!
         * \brief Greater-or-equal filter.
         */
        RowSet operator>=(const T& value) { return buildWith([&](auto& b){ b.ge(value); }); }
        /*!
         * \brief Range filter [low, high].
         * \param low Lower bound (inclusive).
         * \param high Upper bound (inclusive).
         */
        RowSet between(const T& low, const T& high)
        {
            return buildWith([&](auto& b){ b.between(low, high); });
        }
    };

    /**
     * @brief RowSet builder for scalar columns; supports chaining of predicates.
     * @tparam T Scalar column type.
     */
    template<typename T>
    class RowSetBuilder
    {
            FITSform* column_;
            std::vector<T>* values_;
            std::vector<size_t> indices_;

            void ensureScalar()
            {
                static_assert(!detail::is_std_vector<T>::value,
                              "RowSetBuilder does not support vector payload columns");
                if(column_->getNelem() > 1)
                    throw std::logic_error("RowSetBuilder: repeated/vector columns are not supported");
            }

            template<typename Predicate>
            RowSetBuilder& filter(Predicate&& pred)
            {
                auto* vals = values_;
                auto newEnd = std::remove_if(indices_.begin(), indices_.end(),
                    [&](size_t idx){ return !pred((*vals)[idx], idx); });
                indices_.erase(newEnd, indices_.end());
                return *this;
            }

        public:
            /*!
             * \brief Initialize a builder for a scalar column.
             * \param column Column descriptor; must be scalar (nelem==1).
             * \throws std::logic_error if column repeats or is a vector type.
             */
            explicit RowSetBuilder(FITSform& column)
                : column_(&column)
            {
                auto& vec = column.values<T>(); // throws if wrong scalar type
                values_ = &vec;
                ensureScalar();
                indices_.resize(vec.size());
                std::iota(indices_.begin(), indices_.end(), 0);
            }

            /*!
             * \brief Keep rows equal to value.
             */
            RowSetBuilder& eq(const T& value) { return filter([&](const T& v, size_t){ return v == value; }); }
            /*!
             * \brief Keep rows not equal to value.
             */
            RowSetBuilder& ne(const T& value) { return filter([&](const T& v, size_t){ return v != value; }); }
            /*!
             * \brief Keep rows less than value.
             */
            RowSetBuilder& lt(const T& value) { return filter([&](const T& v, size_t){ return v <  value; }); }
            /*!
             * \brief Keep rows less or equal to value.
             */
            RowSetBuilder& le(const T& value) { return filter([&](const T& v, size_t){ return v <= value; }); }
            /*!
             * \brief Keep rows greater than value.
             */
            RowSetBuilder& gt(const T& value) { return filter([&](const T& v, size_t){ return v >  value; }); }
            /*!
             * \brief Keep rows greater or equal to value.
             */
            RowSetBuilder& ge(const T& value) { return filter([&](const T& v, size_t){ return v >= value; }); }
            /*!
             * \brief Keep rows within [low, high].
             */
            RowSetBuilder& between(const T& low, const T& high)
            {
                return filter([&](const T& v, size_t){ return v >= low && v <= high; });
            }
            /*!
             * \brief Apply a custom predicate to filter rows.
             * \tparam Predicate bool(const T&) or bool(const T&, size_t) callable.
             */
            template<typename Predicate>
            RowSetBuilder& custom(Predicate&& pred)
            {
                // Support both signatures:
                // - bool(const T&, size_t)
                // - bool(const T&)
                if constexpr (std::is_invocable_r_v<bool, Predicate, const T&, size_t>)
                {
                    return filter(std::forward<Predicate>(pred));
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, const T&>)
                {
                    return filter([&](const T& v, size_t){ return pred(v); });
                }
                else
                {
                    static_assert(std::is_invocable_v<Predicate, const T&> || std::is_invocable_v<Predicate, const T&, size_t>,
                                  "Predicate must be callable as bool(const T&) or bool(const T&, size_t)");
                    return *this; 
                }
            }

            /*!
             * \brief Build the final RowSet.
             */
            RowSet build() const { return RowSet(indices_); }
            /*!
             * \brief Implicit conversion to RowSet.
             */
            operator RowSet() const { return build(); }
    };

    template<typename T>
    class ColumnSelection
    {
        ColumnView<T> view_;
    public:
        ColumnSelection(ColumnView<T>&& view, const RowSet& rows)
            : view_(std::move(view))
        {
            view_.on(rows);
        }

        ColumnSelection& add(const T& v) { view_.add(v); return *this; }
        ColumnSelection& sub(const T& v) { view_.sub(v); return *this; }
        ColumnSelection& mul(const T& v) { view_.mul(v); return *this; }
        ColumnSelection& div(const T& v) { view_.div(v); return *this; }
        ColumnSelection& set(const T& v) { view_.set(v); return *this; }

        ColumnSelection& operator+=(const T& v) { return add(v); }
        ColumnSelection& operator-=(const T& v) { return sub(v); }
        ColumnSelection& operator*=(const T& v) { return mul(v); }
        ColumnSelection& operator/=(const T& v) { return div(v); }
    };

    template<typename T>
    class ColumnView
    {
        FITStable* owner_;
        FITSform* column_;
        std::vector<T>* values_;
        mutable std::vector<T> cache_;
        std::vector<size_t> selection_;
        bool hasSelection_ = false;

        void ensureScalar()
        {
            static_assert(!detail::is_std_vector<T>::value,
                          "ColumnView does not support vector payload columns");
            if(column_->getNelem() > 1)
                throw std::logic_error("ColumnView: repeated/vector columns are not supported");
        }

        void ensureOwner() const
        {
            if(!owner_)
                throw std::logic_error("ColumnView requires owning FITStable");
        }

        template<typename Func>
        ColumnView& mutate(Func&& fn)
        {
            auto& vec = *values_;
            if(hasSelection_)
            {
                for(size_t idx : selection_)
                    fn(vec[idx], idx);
            }
            else
            {
                for(size_t idx = 0; idx < vec.size(); ++idx)
                    fn(vec[idx], idx);
            }
            return *this;
        }

        template<typename Comparator>
        ColumnView& sortWithComparator(Comparator cmp)
        {
            ensureOwner();
            auto& vec = *values_;
            if(vec.empty())
            {
                clearSelection();
                return *this;
            }

            std::vector<size_t> order(vec.size());
            std::iota(order.begin(), order.end(), 0);
            std::stable_sort(order.begin(), order.end(),
                             [&](size_t lhs, size_t rhs)
                             {
                                 const T& lv = vec[lhs];
                                 const T& rv = vec[rhs];
                                 if(cmp(lv, rv)) return true;
                                 if(cmp(rv, lv)) return false;
                                 return lhs < rhs;
                             });

            owner_->reorderRows(order);
            clearSelection();
            values_ = &column_->values<T>();
            return *this;
        }

    public:
        /*!
         * \brief Construct a typed view on a scalar column.
         * \param owner Owning FITStable.
         * \param column Column descriptor.
         * \throws std::logic_error if column repeats or is a vector type.
         */
        explicit ColumnView(FITStable& owner, FITSform& column)
            : owner_(&owner),
              column_(&column),
              values_(&column.values<T>())
        {
            ensureScalar();
        }

        /*!
         * \brief Restrict the view to a set of rows.
         * \param rows RowSet of indices.
         * \return Reference to this view.
         */
        ColumnView& on(const RowSet& rows)
        {
            selection_ = rows.indices();
            hasSelection_ = true;
            return *this;
        }

        /*!
         * \brief Clear any active row selection.
         * \return Reference to this view.
         */
        ColumnView& clearSelection()
        {
            selection_.clear();
            hasSelection_ = false;
            return *this;
        }

        /*!
         * \brief Add a value to selected cells.
         */
        ColumnView& add(const T& value)
        {
            return mutate([&](T& cell, size_t){ cell += value; });
        }

        /*!
         * \brief Subtract a value from selected cells.
         */
        ColumnView& sub(const T& value)
        {
            return mutate([&](T& cell, size_t){ cell -= value; });
        }

        /*!
         * \brief Multiply selected cells by a value.
         */
        ColumnView& mul(const T& value)
        {
            return mutate([&](T& cell, size_t){ cell *= value; });
        }

        /*!
         * \brief Divide selected cells by a value.
         */
        ColumnView& div(const T& value)
        {
            return mutate([&](T& cell, size_t){ cell /= value; });
        }

        /*!
         * \brief Assign a constant to selected cells.
         */
        ColumnView& set(const T& value)
        {
            return mutate([&](T& cell, size_t){ cell = value; });
        }

        /*!
         * \brief Apply a custom mutator to each selected cell.
         * \tparam Func Callable with signature void(T&, size_t).
         */
        template<typename Func>
        ColumnView& apply(Func&& fn)
        {
            return mutate(std::forward<Func>(fn));
        }

        /*!
         * \brief Sort rows ascending by the column and reorder all columns.
         * \return Reference to this view (selection cleared).
         */
        ColumnView& sortAscending()
        {
            return sortWithComparator(std::less<T>());
        }

        /*!
         * \brief Sort rows descending by the column and reorder all columns.
         * \return Reference to this view (selection cleared).
         */
        ColumnView& sortDescending()
        {
            return sortWithComparator(std::greater<T>());
        }

        /*!
         * \brief Access the underlying data vector.
         * \return Const reference to values.
         */
        const std::vector<T>& data() const
        {
            if(!hasSelection_) return *values_;
            cache_.clear();
            cache_.reserve(selection_.size());
            for(size_t idx : selection_) cache_.push_back((*values_)[idx]);
            return cache_;
        }

        /*!
         * \brief Create a ColumnSelection from an rvalue view and a RowSet.
         * \param rows Selection of rows.
         * \return ColumnSelection bound to the rows.
         */
        ColumnSelection<T> operator[](const RowSet& rows) &&
        {
            ColumnView<T> tmp(std::move(*this));
            tmp.on(rows);
            return ColumnSelection<T>(std::move(tmp), rows);
        }

        /*!
         * \brief Create a ColumnSelection from a const lvalue view and a RowSet.
         * \param rows Selection of rows.
         * \return ColumnSelection bound to the rows.
         */
        ColumnSelection<T> operator[](const RowSet& rows) const &
        {
            ColumnView<T> tmp(*this);
            tmp.on(rows);
            return ColumnSelection<T>(std::move(tmp), rows);
        }
    };

    /**
     * @brief Entry point to create views and where-chains for a named column.
     * @details Enables fluently building filters and applying updates.
     */
    class ColumnHandle
    {
            FITStable& tbl_;
            std::string target_;

        public:
            ColumnHandle(FITStable& tbl, std::string name)
                : tbl_(tbl), target_(std::move(name)) {}

            /*!
             * \brief Create a typed view over the entire column.
             * \tparam TargetT Scalar type expected for the column.
             * \return ColumnView<TargetT> with no selection.
             */
            template<typename TargetT>
            ColumnView<TargetT> all()
            {
                auto& form = *tbl_.getColumn(target_);
                return ColumnView<TargetT>(tbl_, form);
            }

            /*!
             * \brief Create a typed view restricted to a RowSet.
             * \tparam TargetT Scalar type of the column.
             * \param rows Row selection.
             * \return ColumnView<TargetT> with the selection applied.
             */
            template<typename TargetT>
            ColumnView<TargetT> on(const RowSet& rows)
            {
                auto view = all<TargetT>();
                view.on(rows);
                return view;
            }

            template<typename FilterT>
            class WhereChain
            {
                ColumnHandle& owner_;
                RowSetBuilder<FilterT> builder_;

                RowSet rows() const { return builder_.build(); }

            public:
                WhereChain(ColumnHandle& owner, RowSetBuilder<FilterT>&& builder)
                    : owner_(owner), builder_(std::move(builder)) {}

                WhereChain& eq(const FilterT& value) { builder_.eq(value); return *this; }
                WhereChain& ne(const FilterT& value) { builder_.ne(value); return *this; }
                WhereChain& lt(const FilterT& value) { builder_.lt(value); return *this; }
                WhereChain& le(const FilterT& value) { builder_.le(value); return *this; }
                WhereChain& gt(const FilterT& value) { builder_.gt(value); return *this; }
                WhereChain& ge(const FilterT& value) { builder_.ge(value); return *this; }
                WhereChain& between(const FilterT& low, const FilterT& high) { builder_.between(low, high); return *this; }

                template<typename Predicate>
                WhereChain& custom(Predicate&& pred)
                {
                    builder_.custom(std::forward<Predicate>(pred));
                    return *this;
                }

                template<typename TargetT>
                WhereChain& add(const TargetT& value)
                {
                    owner_.all<TargetT>().on(rows()).add(value);
                    return *this;
                }

                template<typename TargetT>
                WhereChain& sub(const TargetT& value)
                {
                    owner_.all<TargetT>().on(rows()).sub(value);
                    return *this;
                }

                template<typename TargetT>
                WhereChain& mul(const TargetT& value)
                {
                    owner_.all<TargetT>().on(rows()).mul(value);
                    return *this;
                }

                template<typename TargetT>
                WhereChain& div(const TargetT& value)
                {
                    owner_.all<TargetT>().on(rows()).div(value);
                    return *this;
                }

                template<typename TargetT>
                WhereChain& set(const TargetT& value)
                {
                    owner_.all<TargetT>().on(rows()).set(value);
                    return *this;
                }

                template<typename TargetT, typename Func>
                WhereChain& apply(Func&& fn)
                {
                    owner_.all<TargetT>().on(rows()).apply(std::forward<Func>(fn));
                    return *this;
                }

                RowSet toRowSet() const { return rows(); }
            };

            template<typename FilterT>
            WhereChain<FilterT> where(const std::string& filterColumn)
            {
                auto& form = *tbl_.getColumn(filterColumn);
                return WhereChain<FilterT>(*this, RowSetBuilder<FilterT>(form));
            }
    };

    /**
     * @brief Build a RowSet for the named column, filtering by scalar comparisons.
     * @tparam T Scalar type of the target column.
     */
    template<typename T>
    RowSetBuilder<T> FITStable::select(const std::string& columnName)
    {
        auto& form = *getColumn(columnName);
        return RowSetBuilder<T>(form);
    }

    /**
     * @brief Build a RowSet for the named column, filtering by scalar comparisons.
     * @tparam T Scalar type of the target column.
     */
    template<typename T>
    RowSetBuilder<T> FITStable::select(const std::string& columnName) const
    {
        auto& form = *getColumn(columnName);
        return RowSetBuilder<T>(const_cast<FITSform&>(form));
    }

    /*!
     * \brief Get a typed ColumnView for a named column.
     * \tparam T Expected scalar type.
     * \param columnName Column name.
     * \return ColumnView bound to this table and column.
     * \throws std::out_of_range if column not found.
     * \throws std::bad_cast if type mismatches.
     */
    template<typename T>
    ColumnView<T> FITStable::column(const std::string& columnName)
    {
        auto& form = *getColumn(columnName);
        return ColumnView<T>(*this, form);
    }

    /*!
     * \brief Get a typed ColumnView for a named column.
     * \tparam T Expected scalar type.
     * \param columnName Column name.
     * \return ColumnView bound to this table and column.
     * \throws std::out_of_range if column not found.
     * \throws std::bad_cast if type mismatches.
     */
    template<typename T>
    ColumnView<T> FITStable::column(const std::string& columnName) const
    {
        auto& form = *getColumn(columnName);
        return ColumnView<T>(const_cast<FITStable&>(*this), const_cast<FITSform&>(form));
    }

    /*!
     * \brief Get a ColumnHandle to fluently build views and filters.
     * \param columnName Target column name.
     * \return ColumnHandle bound to this table.
     */
    inline ColumnHandle FITStable::operator[](const std::string& columnName)
    {
        return ColumnHandle(*this, columnName);
    }

    /*!
     * \brief Get a ColumnHandle to fluently build views and filters.
     * \param columnName Target column name.
     * \return ColumnHandle bound to this table.
     */
    inline ColumnHandle FITStable::operator[](const std::string& columnName) const
    {
        return ColumnHandle(const_cast<FITStable&>(*this), columnName);
    }

    /*!
     * \brief Reorder rows in all columns according to a global permutation.
     * \param order New-to-old index mapping; must be a permutation of [0..n-1].
     * \throws std::logic_error on mismatch, duplicates, or out-of-range.
     */
    inline void FITStable::reorderRows(const std::vector<size_t>& order)
    {
        if(fcolumns.empty())
            return;

        const size_t expected = fcolumns.front()->size();
        if(order.size() != expected)
            throw std::logic_error("FITStable::reorderRows permutation size mismatch");

        std::vector<uint8_t> seen(expected, 0);
        for(size_t idx : order)
        {
            if(idx >= expected)
                throw std::logic_error("FITStable::reorderRows permutation index out of range");
            if(seen[idx])
                throw std::logic_error("FITStable::reorderRows permutation contains duplicates");
            seen[idx] = 1;
        }

        for(auto& col : fcolumns)
        {
            if(col->size() != expected)
                throw std::logic_error("FITStable::reorderRows column size mismatch");
            col->sortOn(order);
        }
        nrows_cache = expected;
    }

    /*!
     * \brief Begin a filter expression for a named column.
     * \tparam T Scalar type of the filter column.
     * \param columnName Column name; must exist.
     * \return ColumnFilterExpr<T> bound to the column.
     */
    template<typename T>
    ColumnFilterExpr<T> FITStable::operator()(const std::string& columnName)
    {
        // Ensures column exists (throws if not)
        (void)getColumn(columnName);
        return ColumnFilterExpr<T>(*this, columnName);
    }
#pragma endregion
}
#endif
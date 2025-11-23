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

#include "FITShdu.h"
#include "FITSdata.h"
#include "FITSexception.h"

#include <fitsio.h>

namespace DSL
{
    class FITStable;
    class FITSform;
    
#pragma region - FITSform class definition
    class FITSform
    {
        /**
         * @class FITSform  FITStable.h "fitsExtractor/FITStable.h"
         * @author GILLARD William
         * @date 08/08/2017
         * @brief FITS table's column descriptor
         * @details Helper class that allow easy access to the name, type, unit of FITS ASCII and BINARY columns.
         **/
        
    private:
#pragma region -- private member
        const std::string fname;
        const dtype ftype;
        std::string funit;
        
        const double fscale;
        const double fzero;
        
        int64_t frepeat;
        int64_t fwidth;

        size_t fpos;

    protected:
#pragma endregion
#pragma region -- protected member function
        inline FITSform():fname(),ftype(dtype::tnone),funit(),fscale(1),fzero(0),frepeat(1),fwidth(0),fpos(0) {};
        
        inline void setNelem(const int64_t& n) {frepeat = n;}
        inline void setWidth(const int64_t& w) {fwidth  = w;}
        
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
        typedef std::vector< char* >                    charVector;
        typedef std::vector< std::string >              stringVector;
        typedef std::vector< std::pair<float,float> >   complexVector;
        typedef std::vector< std::pair<double,double> > dblcomplexVector;
        
        
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

#pragma endregion

#pragma region -- pur virtual function

        virtual size_t size() const =0;
        virtual void push_back(const std::any& value) = 0;
        virtual std::unique_ptr<FITSform> clone() const = 0; // add

#pragma endregion

#pragma region -- modifier
        inline void setUnit    (const std::string& s){funit.clear(); funit+=s;}
        inline void setPosition(const size_t& p)    {fpos = p;}
        
#pragma endregion
#pragma region -- Saving & printing
        virtual void Dump( std::ostream& ) const;
        virtual void write(const std::shared_ptr<fitsfile>&, const int64_t&)=0;
#pragma endregion
    };
#pragma endregion

#pragma region - FITScolumn class definition
    
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
        col_map data;

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
        FITScolumn(const FITScolumn&);
        
        virtual ~FITScolumn();
        
#pragma endregion
#pragma region -- accessor

        size_t size() const override { return data.size(); }
        void push_back(const std::any& value) override
        {
            data.push_back( std::any_cast<T>(value) );
        }
        
#pragma endregion
#pragma region -- diagnoze
        virtual void Dump( std::ostream& ) const override;
        
        std::unique_ptr<FITSform> clone() const override
        {
            return std::unique_ptr<FITSform>( new FITScolumn<T>(*this) );
        }
#pragma endregion
#pragma region -- Saving
        virtual void write(const std::shared_ptr<fitsfile>&, const int64_t&) override;
#pragma endregion
    };

// Scalar type specializations
template<> void FITScolumn<FITSform::complex>   ::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<FITSform::dblcomplex>::write(const std::shared_ptr<fitsfile>&, const int64_t&);
template<> void FITScolumn<std::string>         ::write(const std::shared_ptr<fitsfile>&, const int64_t&);

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
    
#pragma endregion
#pragma region - FITStable class definition
    class FITStable
    {
        /**
         * @class DSL::FITStable FITStable.h "fitsExtractor/FITStable.h"
         * @author GILLARD William
         * @version 1.0
         * @date 07/08/2017
         * @brief Helper class to work with FITS ASCII and BINARY tables.
         * @details This class provide foreground method to read and write FITS ASCII and BINARY tables. The FITStable class point toward the HDU block containing the requested tables and it is the reponsibility of the User to deleted and close the fits file that contains the ASCII and/or BINARY table.
         **/

    public:
        typedef std::vector< std::unique_ptr<FITSform> > columns_list;
        typedef std::vector< std::vector<std::string> > clist;

    protected:
        columns_list fcolumns;

    private:
#pragma endregion
#pragma region -- private member
        const volatile ttype ftbl_type;
        FITShdu hdu;                              //!< Header of the table

#pragma endregion
#pragma region -- private member function
        void load(const std::shared_ptr<fitsfile>&);
        std::unique_ptr<FITSform> readColumn(const std::shared_ptr<fitsfile>&, const size_t&, const size_t&);
    

        //FITSform* readArray(const FITSform& , const int64_t&);
        
        template <typename T>
        void read(FITScolumn<T>*, const std::shared_ptr<fitsfile>& fptr, const size_t&);
        
        template <typename T>
        void readVector(FITScolumn< std::vector<T> >*, const std::shared_ptr<fitsfile>& fptr, const size_t&);
        
    public:

#pragma endregion
#pragma region -- ctor/dtor
        
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
        size_t nrows() const;
        size_t ncols() const;
        
        const clist    listColumns();
        //const FITSform columnProperties(const std::string&);
        //const FITSform columnProperties(const size_t&);

        /**
            @brief Retrive the name of the table extension
            @return The extension name of \c this table or an empty string if the EXTNAME keyword wasn't found into the HDU.
        */
        inline const std::string GetName() const {if(hdu.Exists("EXTNAME")) return hdu.GetValueForKey("EXTNAME"); else return std::string("NO NAME");}       //!< Get the name of the image      

#pragma endregion
#pragma region -- Accessor
        inline const FITShdu& HDU() const {return hdu;}
        inline       FITShdu& HDU() {return hdu;}
#pragma endregion
#pragma region -- Accessing coulumn data
        //FITSform* readColumn(const std::string&);
        //FITSform* readColumn(const size_t&);
        
        //FITSform* readColumn(const std::string&, const size_t&);
        //FITSform* readColumn(const size_t&, const size_t&);
#pragma endregion
#pragma region -- Inserting/Updating data to column

#pragma region 1- Inseting new column
        
        void InsertColumn(const std::string& cname, const dtype& type, const std::string& tunit);
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
        void writeArray(const std::shared_ptr<fitsfile>& fptr, const int64_t&);
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
        int64_t nelem = (static_cast<int64_t>(nrows())-(row-1));
        
        T*    array     = new T   [nelem];
        char* nullarray = new char[nelem];
        int tbl_status  = 0;
        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem, array, nullarray, NULL, &tbl_status))
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
        {
            if(! std::atoi(&nullarray[k]) )
                data->push_back(array[k]);
        }
        
        delete [] array;
        delete [] nullarray;
        
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
        int64_t nelem = ( static_cast<int64_t>(nrows())-static_cast<int64_t>(row-1) )* static_cast<int64_t>(data->getNelem());
        
        T*   array      = new T   [nelem];
        char* nullarray = new char[nelem];
        int   anynull   = 0;
        int tbl_status  = 0;
        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem , array, nullarray, &anynull, &tbl_status))
        {
            delete[] array;
            delete[] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(int64_t k = 0; k < nelem; k += data->getNelem())
        {
            if(anynull && std::atoi(&nullarray[k]) )
                continue;
            
            //data->insert(typename FITScolumn< std::vector<T> >::col_data(k+1,std::vector<T>(tform.getNelem())));
            
            
            std::vector<T> tmpv = std::vector<T>(data->getNelem());
            
            for(int64_t l = 0; l < data->getNelem(); l++ )
            {
                if( k+l >= nelem*data->getNelem())
                {
                    std::cout<<l<<" + "<<k<<" = "<<l+k<<" % "<<nelem * static_cast<int64_t>(data->getNelem())<<std::endl;
                    throw FITSwarning("FITStable","readVector","Out of range : "+std::to_string(k+l)+" <-> "+std::to_string((static_cast<int64_t>(nrows())-static_cast<int64_t>(row))*data->getNelem()));
                }
                tmpv[l] = array[k+l];
            }
            
            data->push_back(tmpv);
        }
        
        if(array != NULL)
            delete[]  array;
        
        if(nullarray != NULL)
            delete[]  nullarray;
    }
    
#pragma endregion
#pragma region - FITScolumn class implementation

#pragma region --saving
    template< typename T >
    void FITScolumn<T>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
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
                
                if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), n+1, 1, getNelem(),  &array, &tbl_status))
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
    FITScolumn<T>::FITScolumn():FITSform(),data()
    {
    }
    
    /**
     @brief Default constructor
     @param name FITS column name
     @param type FITS column type
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name , const dtype& type, const std::string unit, const size_t pos):FITSform(pos,name,type,unit),data()
    {
        
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
    FITScolumn<T>::FITScolumn(const std::string& name, const dtype& type, const double& scale, const double& zero, const std::string unit, const size_t pos):FITSform(pos,name,type,scale,zero,unit),data()
    {
        
    }
    
    /**
     @brief Copy constructor
     @param col FITS column to be copyed to this
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const FITScolumn<T>& col):FITSform(col),data(col.data)
    {
        
    }
    
    template< typename T >
    FITScolumn<T>::~FITScolumn()
    {
        data.clear();
    }
#pragma endregion
    
    /*
    template< typename T >
    FITScolumn<T>::iterator FITScolumn<T>::insert(FITScolumn<T>::const_iterator pos, const  T& value)
    {
        typename std::vector<T>::iterator p = std::vector<T>::insert(pos, value);
        Update(value);
        
        return p;
    }

    template< typename T >
    FITScolumn<T>::iterator FITScolumn<T>::insert(FITScolumn<T>::const_iterator pos, const  T&& value)
    {
        typename std::vector<T>::iterator p = std::vector<T>::insert(pos, value);
        Update(value);

        int64_t newSize = static_cast<int64_t>(std::vector<T>::size());
        if(getNelem() < newSize) setNelem(newSize);
        
        return p;
    }

    template< typename T >
    FITScolumn<T>::iterator FITScolumn<T>::insert(FITScolumn<T>::const_iterator pos , size_type count, const T& value)
    {
        typename std::vector<T>::iterator p = std::vector<T>::insert(pos, count, value);
        Update(value);

        int64_t newSize = static_cast<int64_t>(std::vector<T>::size());
        if(getNelem() < newSize) setNelem(newSize);
        
        return p;
    }

    template< typename T >
    FITScolumn<T>::iterator FITScolumn<T>::insert(FITScolumn<T>::const_iterator pos, std::initializer_list<T> ilist )
    {
        typename std::vector<T>::iterator p = std::vector<T>::insert(pos, ilist);
        Update(std::vector<T>(ilist));

        int64_t newSize = static_cast<int64_t>(std::vector<T>::size());
        if(getNelem() < newSize) setNelem(newSize);
        
        return p;
    }

    template< typename T >
    template< class InputIt >
    FITScolumn<T>::iterator FITScolumn<T>::insert(FITScolumn<T>::const_iterator pos, InputIt first, InputIt last )
    {
        typename std::vector<T>::iterator p = std::vector<T>::insert(pos, first, last);
        Update(std::vector<T>(ilist));

        int64_t newSize = static_cast<int64_t>(std::vector<T>::size());
        if(getNelem() < newSize) setNelem(newSize);
        
        return p;
    }
    */

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
        
        size_t n=0;
        for(typename col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            fout<<"    ["<<n<<"]   "<<std::flush;
            dump(fout,*it);
            fout<<std::endl;
            n++;
        }
    }
    
    template< typename T>
    template< typename U >
    void FITScolumn<T>::dump( std::ostream& fout, const U& data) const
    {
        fout<<data<<std::flush;
    }
    
    template< typename T>
    template< typename P >
    void FITScolumn<T>::dump( std::ostream& fout, const std::pair<P,P>& data) const
    {
        fout<<data.first<<" , "<<data.second<<std::flush;
    }
    
    template< typename T>
    template< typename Q >
    void FITScolumn<T>::dump( std::ostream& fout, const std::vector<Q>& data) const
    {
        if(data.size() < 1)
            return;
        
        fout<<data[0]<<" ... "<<data[data.size()-1]<<std::endl;
        for(size_t n = 0; n < data.size(); n++)
            fout<<"         ("<<n<<")   "<<data[n]<<std::endl;
    }
    
    template< typename T>
    template< typename L >
    void FITScolumn<T>::dump( std::ostream& fout, const std::vector< std::pair<L,L> >& data) const
    {
        if(data.size() < 1)
            return;

        fout<<data[0].first<<" , "<<data[0].second<<" ... "<<data[data.size()-1].first<<" , "<<data[data.size()-1].second<<std::endl;
        for(size_t n = 0; n < data.size(); n++)
            fout<<"         ("<<n<<")   "<<data[n].first<<" , "<<data[n].second<<std::endl;
        
    }
    
}
#endif


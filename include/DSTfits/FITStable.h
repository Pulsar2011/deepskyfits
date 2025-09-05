//
//  FITStable.h
//  DeepSkyLib
//
//  Created by GILLARD William on 07/08/17.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#ifndef _DSL_FITStable_
#define _DSL_FITStable_

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <map>
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
    
    typedef std::map<size_t,FITSform> clist;
    
#pragma mark - cdescriptor class definition
    class FITSform
    {
        /**
         * @class FITSform  FITStable.h "fitsExtractor/FITStable.h"
         * @author GILLARD William
         * @version 1.0.0
         * @date 08/08/2017
         * @brief FITS table's column descriptor
         * @details Helper class that allow easy access to the name, type, unit of FITS ASCII and BINARY columns.
         **/
        
    private:
#pragma mark • private member
        const std::string fname;
        const dtype ftype;
        std::string funit;
        
        const double fscale;
        const double fzero;
        
        int64_t frepeat;
        int64_t fwidth;
        
        size_t fpos;

    protected:
#pragma mark • protected member function
        inline FITSform():fname(),ftype(dtype::tnone),funit(),fscale(1),fzero(0),frepeat(1),fwidth(0),fpos(0) {};
        
        inline void setNelem(const int64_t& n) {frepeat = n;}
        inline void setWidth(const int64_t& w) {fwidth  = w;}
        
    public:
#pragma mark • definition
        typedef std::pair<float,float>   complex;
        typedef std::pair<double,double> dblcomplex;
        
#pragma mark • static member function
        static const std::string getDataType(const dtype &);
        static const dtype getDataTypeID(const std::string &);
        
        const std::string getTTYPE() const;
        
#pragma mark • ctor/dtor
        FITSform(const size_t, const std::string, const dtype, const std::string unit="");
        FITSform(const size_t, const std::string, const dtype, const double, const double, const std::string unit="");
        
        FITSform(const size_t, const std::string, const dtype, const int64_t, const int64_t, const std::string unit="");
        FITSform(const size_t, const std::string, const dtype, const int64_t, const int64_t, const double, const double, const std::string unit="");
        
        FITSform(const FITSform&);
        
        virtual ~FITSform(){};
        
#pragma mark • acessor
        inline const std::string& getName() const {return fname;}
        inline const dtype&       getType() const {return ftype;}
        inline const std::string& getUnit() const {return funit;}
        inline const double&      getScale()const {return fscale;}
        inline const double&      getZero() const {return fzero;}
        inline const size_t&      getPosition() const {return fpos;}
        
        inline const int64_t&    getNelem()  const {return frepeat;}
        inline const int64_t&    getWidth()  const {return fwidth;}

#pragma mark • modifier
        inline void setUnit    (const std::string& s){funit.clear(); funit+=s;}
        inline void setPosition(const size_t& p){fpos = p;}
        
#pragma mark • modifier
        virtual void Dump( std::ostream& ) const;
    };

#pragma mark - FITScolumn class definition
    
    template<typename T>
    class FITScolumn: public std::map<size_t,T>, public FITSform
    {
        /**
         * @class FITScolumn  FITStable.h "fitsExtractor/FITStable.h"
         * @author GILLARD William
         * @version 1.0.0
         * @date 08/08/2017
         * @brief Helper class to read and write FITS columns data
         * @details This class provide methods to intercats with the data contained into columns of FITS ASCII and BINARY table. Once modified, it does not alter the fitsfile content. In order the modification are propagated to the fitsfile, the FITScolumn instance have to be passed to the FITStable instance. The later is directly responsible to managing the FITStable within the fitsfile.
         **/
        
    private:
#pragma mark • private member function
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
        
        template<typename U>
        void dump( std::ostream&, const U& ) const;
        
        template<typename P>
        void dump( std::ostream&, const std::pair<P,P>& ) const;

        template<typename Q>
        void dump( std::ostream&, const std::vector<Q>& ) const;
        
        template< typename L >
        void dump( std::ostream&, const std::vector< std::pair<L,L> >&) const;
        
    public:
        
#pragma mark • definition
        typedef typename std::vector<T>  vec_data;
        typedef typename std::map<size_t,T>  col_map;
        typedef typename std::pair<size_t,T> col_data;
        typedef typename col_map::iterator iterator;
        typedef typename col_map::const_iterator const_iterator;
        
        std::pair< typename FITScolumn<T>::iterator, bool > insert(const typename FITScolumn<T>::col_data&);
        
#pragma mark • ctor/dtor
        FITScolumn(const std::string&, const dtype&, const std::string unit="", const size_t pos=0);
        FITScolumn(const std::string&, const dtype&, const double&, const double&, const std::string unit="", const size_t pos=0);
        FITScolumn(const FITScolumn&);
        FITScolumn(const FITSform&);
        //FITScolumn(typename std::map<size_t,T>::const_iterator, typename std::map<size_t,T>::const_iterator, const std::string&, const dtype&);
        
        virtual ~FITScolumn();
        
#pragma mark • accessor

        
#pragma mark • diagnoze
        virtual void Dump( std::ostream& ) const;
        
    };
    
#pragma mark - FITStable class definition
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
    private:
#pragma mark • private member
        int tbl_status;
        const volatile ttype ftbl_type;
        const volatile std::string fextname;
        const volatile int fhdu_num;
        
#if __cplusplus < 201103L
        fitsfile* fptr;
#else
        std::shared_ptr<fitsfile> fptr;
        void CloseFile(fitsfile* in);
#endif

#pragma mark • private member function
        void rload();
        const std::string getFileName(fitsfile *fptr) const;
        const std::string getExtName() const;
        const ttype getExtType() const;
        const int getHDUindex() const;
        
        bool isValid() const;
        
        FITSform* readArray(const FITSform& , const int64_t&);
        
        template <typename T>
        FITScolumn<T>* read(const FITSform&, const int64_t&);
        
        template <typename T>
        FITScolumn< std::vector<T> >* readVector(const FITSform&, const size_t&);

        void writeArray(const FITSform*, const int64_t&);
        
        template <typename T>
        void write      (const FITScolumn<T>*, const int64_t&);
        
        template <typename T>
        void writeVector(const FITScolumn<std::vector<T> >*, const int64_t&);
        
    public:

#pragma mark • ctor/dtor
        
#if __cplusplus < 201103L
        FITStable(fitsfile *, const int&);
        FITStable(fitsfile *, const std::string&);
#else
        FITStable(std::shared_ptr<fitsfile>, const int&);
        FITStable(std::shared_ptr<fitsfile>, const std::string&);
#endif
        
        FITStable(const FITStable&);
        
        ~FITStable();
        
#pragma mark • Properties
        size_t getNrows();
        size_t getNcols();
        
        const clist listColumns();
        const FITSform columnProperties(const std::string&);
        const FITSform columnProperties(const size_t&);
        
#pragma mark • Accessing coulumn data
        FITSform* readColumn(const std::string&);
        FITSform* readColumn(const size_t&);
        
        FITSform* readColumn(const std::string&, const size_t&);
        FITSform* readColumn(const size_t&, const size_t&);
        
#pragma mark • Inserting/Updating data to column
#pragma mark 1- Inseting new column
        
        void InsertColumn(const std::string&, const std::string&, const std::string& tunit=std::string(), int* colnum = NULL);
        void InsertColumn(FITSform*);
        
#pragma mark 2- Inseting value to an existing column
        
#pragma mark 3- Updating value from an existing column
        
#pragma mark • Diagnoze
        inline const int Status() const {return tbl_status;}
        void Dump( std::ostream& );

    };
    
#pragma mark - FITStable Template implementation
    
    /**
     @brief Read column content
     @details Private methods used to read column the content of FITS scalar array. The method access the FITS column data and fill up a FITScolumn<T> instance with the corresponding data.

     @param tform address of a FITSform property container to be read
     @param row First row to be read
     @return FITScolumn<T> instance that contains FITS column prperties as well as std::map<size_t,T> container with the requested data.
     */
    template< typename T>
    FITScolumn<T>* FITStable::read(const FITSform& tform, const int64_t& row)
    {
        FITScolumn<T>* data = new FITScolumn<T>(tform);
        
        int64_t nelem = (static_cast<int64_t>(getNrows())-(row-1));
        
        T*    array     = new T   [nelem];
        char* nullarray = new char[nelem];
        
        if(
#if __cplusplus < 201103L
           ffgcf(fptr      , static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem, array, nullarray, NULL, &tbl_status)
#else
           ffgcf(fptr.get(), static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem, array, nullarray, NULL, &tbl_status)
#endif
            )
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; static_cast<int64_t>(k) < nelem; k++)
        {
            if(! std::atoi(&nullarray[k]) )
                data->insert(typename FITScolumn<T>::col_data(data->size()+row,array[k]));
        }
        
        delete [] array;
        delete [] nullarray;
        
        return data;
        
    }
    
    /**
     @brief Read column content
     @details Private methods used to read  the content of FITS vector array. The method access the FITS column data and fill up a FITScolumn<std::vector<T>> instance with the corresponding data.
     
     @param tform address of a FITSform property container to be read
     @param row First row to be read
     @return FITScolumn<std::vector<T>> instance that contains FITS column prperties as well as std::map<size_t,std::vector<T>> container with the requested data.
     */
    template< typename T>
    FITScolumn< std::vector< T > >* FITStable::readVector(const FITSform& tform, const size_t& row)
    {
        FITScolumn< std::vector< T > >* data = new FITScolumn< std::vector<T> >(tform);
        
        int64_t nelem = ( static_cast<int64_t>(getNrows())-static_cast<int64_t>(row-1) )* static_cast<int64_t>(tform.getNelem());
        
        T*   array      = new T   [nelem];
        char* nullarray = new char[nelem];
        int   anynull   = 0;
        
        if(
#if __cplusplus < 201103L
           ffgcf(fptr      , static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, static_cast<LONGLONG>(tform.getNelem()), array, nullarray, &anynull, &tbl_status)
#else
           ffgcf(fptr.get(), static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem , array, nullarray, &anynull, &tbl_status)
#endif
           )
        {
            delete[] array;
            delete[] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(int64_t k = 0; k < nelem; k += tform.getNelem())
        {
            if(anynull && std::atoi(&nullarray[k]) )
                continue;
            
            //data->insert(typename FITScolumn< std::vector<T> >::col_data(k+1,std::vector<T>(tform.getNelem())));
            
            
            std::vector<T> tmpv = std::vector<T>(tform.getNelem());
            
            for(int64_t l = 0; l < tform.getNelem(); l++ )
            {
                if( k+l >= nelem*tform.getNelem())
                {
                    std::cout<<l<<" + "<<k<<" = "<<l+k<<" % "<<nelem * static_cast<int64_t>(tform.getNelem())<<std::endl;
#if __cplusplus < 201103L
                    throw FITSwarning("FITStable","readVector","Out of range : "+std::to_string(static_cast<long long>(k+l))+" <-> "+std::to_string((static_cast<long long>(getNrows())-static_cast<long long>(row))*tform.getNelem()));
#else
                    throw FITSwarning("FITStable","readVector","Out of range : "+std::to_string(k+l)+" <-> "+std::to_string((static_cast<int64_t>(getNrows())-static_cast<int64_t>(row))*tform.getNelem()));
#endif
                }
                tmpv[l] = array[k+l];
            }
            
            data->insert(typename FITScolumn< std::vector<T> >::col_data(data->size()+row,tmpv));
        }
        
        if(array != NULL)
            delete[]  array;
        
        if(nullarray != NULL)
            delete[]  nullarray;
        
        return data;
    }
    
    /**
     @brief Write column content into FITS file
     @details Private methods used to write  the content of FITS vector array into a FITS column. .
     
     @param data The column and data to be written into the FITS table
     @param first_row Row position of the first element to write into the table
     */
    template< typename T>
    void FITStable::write(const FITScolumn<T>* data, const int64_t& first_row)
    {
        if(data->size() < 1)
        {
            throw FITSexception(NOT_TABLE,"FITStable","write<T>");
        }
        
        T   array=0;
        
        int64_t n = 0;
        for(typename FITScolumn<T>::const_iterator it = data->cbegin(); it != data->cend(); it++)
        {
            if(n >= (first_row-1))
            {
                array = it->second;
                
                if(
#if __cplusplus < 201103L
                   ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, data->getNelem(), &array, &tbl_status)
#else
                   ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, data->getNelem(),  &array, &tbl_status)
#endif
                   )
                {
                    throw FITSexception(tbl_status,"FITStable","write<T>");
                }
            }
            n++;
        }
        
        return;
    }
    
    /**
     @brief Read column content
     @details Private methods used to read  the content of FITS vector array. The method access the FITS column data and fill up a FITScolumn<std::vector<T>> instance with the corresponding data.
     
     @param data Pointer to the std::vector containing the data to be writtent to the table
     @param first_row First row to be read
     */
    template< typename T>
    void FITStable::writeVector(const FITScolumn<std::vector<T> >* data, const int64_t& first_row)
    {
        const int64_t nelem = data->getNelem();
        T array[nelem];
        
        int64_t n = 0;
        for(typename FITScolumn< std::vector<T> >::const_iterator it = data->cbegin(); it != data->cend(); it++)
        {
            if( n < (first_row-1))
            {
                n++;
                continue;
            }
            
            for(size_t k =0; k < static_cast<size_t>(nelem); k++)
            {
                if(k < it->second.size())
                    array[k] = it->second.at(k);
                else
                    array[k] = std::numeric_limits<T>::quiet_NaN();
            }
            
            if(
#if __cplusplus < 201103L
               ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data.getPosition()), n+1, 1, nelem, array, &tbl_status)
#else
               ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, nelem, array, &tbl_status)
#endif
               )
            {
                throw FITSexception(tbl_status,"FITStable","writeVector<std::vector<T>>");
            }
            
            n++;
        }

        return;
    }
    
#pragma mark - FITScolumn class implementation
    
    /**
     @brief Empty constructor
     @note Empty constructor is made private because \c this->fname and \c this->ftype are required.
     */
    template< typename T >
    FITScolumn<T>::FITScolumn():std::map<size_t,T>(),FITSform()
    {
        
    }
    
    /**
     @brief Default constructor
     @param name FITS column name
     @param type FITS column type
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const std::string& name , const dtype& type, const std::string unit, const size_t pos):std::map<size_t,T>(),FITSform(pos,name,type,unit)
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
    FITScolumn<T>::FITScolumn(const std::string& name, const dtype& type, const double& scale, const double& zero, const std::string unit, const size_t pos):std::map<size_t,T>(),FITSform(pos,name,type,scale,zero,unit)
    {
        
    }
    
    /**
     @brief Copy constructor
     @param col FITS column to be copyed to this
     */
    template< typename T >
    FITScolumn<T>::FITScolumn(const FITScolumn<T>& col):std::map<size_t,T>(col),FITSform(col)
    {
        
    }
    
    template< typename T >
    FITScolumn<T>::FITScolumn(const FITSform& form):std::map<size_t,T>(),FITSform(form)
    {
        
    }
    
    template< typename T >
    FITScolumn<T>::~FITScolumn()
    {
        dynamic_cast<typename std::map<size_t,T>*>(this)->clear();
    }
    
    template< typename T >
    std::pair< typename FITScolumn<T>::iterator, bool> FITScolumn<T>::insert(const  typename FITScolumn<T>::col_data& d)
    {
        std::pair< typename FITScolumn<T>::iterator, bool> p =  std::map<size_t,T>::insert(std::pair<size_t,T>(d));
        
        const T& val = d.second;
        Update(val);
        
        return p;
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
    
    
#pragma mark • diagnose
    template< typename T >
    void FITScolumn<T>::Dump( std::ostream& fout) const
    {
        FITSform::Dump(fout);
        
        for(typename col_map::const_iterator it = this->begin(); it != this->end(); it++)
        {
            fout<<"    ["<<it->first<<"]   "<<std::flush;
            dump(fout,it->second);
            fout<<std::endl;
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


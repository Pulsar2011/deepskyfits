//
//  FITStable.cxx
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstrea
#include <iomanip>      // std::setw
#include <stdexcept>


#include <limits>
#if __cplusplus < 201103L
#include <memory>
#endif

#include <DSTfits/FITStable.h>
#include <DSTfits/FITSexception.h>

namespace DSL
{
    
#pragma mark - cdescriptor class definition
#pragma mark * ctor/dtor
    
    FITSform::FITSform(const size_t p,const std::string name, const dtype t, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(1),fwidth(1),fpos(p)
    {};
    
    FITSform::FITSform(const size_t p, const std::string name, const dtype t, const double s, const double z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(1),fwidth(1),fpos(p)
    {};
    
    FITSform::FITSform(const size_t p,const std::string name, const dtype t, const int64_t r, const int64_t w, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(r),fwidth(w),fpos(p)
    {};
    
    FITSform::FITSform(const size_t p, const std::string name, const dtype t, const int64_t r, const int64_t w, const double s, const double z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(r),fwidth(w),fpos(p)
    {};
    
    
    FITSform::FITSform(const FITSform& col):fname(col.fname),ftype(col.ftype),funit(col.funit),fscale(col.fscale),fzero(col.fzero),frepeat(col.frepeat),fwidth(col.fwidth),fpos(col.fpos)
    {};
    
#pragma mark * static member function
    const std::string FITSform::getDataType(const dtype & tt)
    {
        std::string ss = std::string();
        
        switch(tt)
        {
            case tsbyte:
                ss += "SBYTE";
                break;
                
            case tshort:
                ss += "SHORT";
                break;
                
            case tushort:
                ss += "USHORT";
                break;
                
            case tint:
                ss += "INT";
                break;
                
            case tuint:
                ss += "UINT";
                break;
                
            case tlong:
                ss += "LONG";
                break;
                
            case tlonglong:
                ss += "LONGLONG";
                break;
                
            case tulong:
                ss += "ULONG";
                break;
                
            case tfloat:
                ss += "FLOAT";
                break;
                
            case tdouble:
                ss += "DOUBLE";
                break;
                
            case tstring:
                ss += "STRING";
                break;
                
            case tlogical:
                ss += "BOOL";
                break;
                
            case tbit:
                ss += "BIT";
                break;
                
            case tbyte:
                ss += "BYTE";
                break;
                
            case tcplx:
                ss += "COMPLEX";
                break;
                
            case tdbcplx:
                ss += "DOUBLE COMPLEX";
                break;
                
            default:
                ss += "UNKNOWN";
        }
        
        return ss;
    }
    
    const std::string FITSform::getTTYPE() const
    {
        std::string ss = std::string();
#if __cplusplus < 201103L
        ss += std::to_string(static_cast<unsigned long long>(getNelem()));
#else
        ss += std::to_string(getNelem());
#endif
        
        switch(ftype)
        {
            case tsbyte:
                ss += "S";
                break;
                
            case tshort:
                ss += "I";
                break;
                
            case tushort:
                ss += "U";
                break;
                
            case tint:
                ss += "I4";
                break;
                
            case tuint:
                ss += "V";
                break;
                
            case tlong:
                ss += "I8";
                break;
                
            case tlonglong:
                ss += "K";
                break;
                
            case tulong:
                ss += "U8";
                break;
                
            case tfloat:
                ss += "E";
                break;
                
            case tdouble:
                ss += "D";
                break;
                
            case tstring:
                ss.clear();
                if(getNelem() == 1)
                {
#if __cplusplus < 201103L
                    ss += std::to_string(static_cast<unsigned long long>(getWidth()));
#else
                    ss += std::to_string(getWidth());
#endif
                    ss += "A";
                }
                else
                {
#if __cplusplus < 201103L
                    ss += std::to_string(static_cast<unsigned long long>(getNelem()*getWidth()));
#else
                    ss += std::to_string(getNelem()*getWidth());
#endif
                    ss += "A";
#if __cplusplus < 201103L
                    ss += std::to_string(static_cast<unsigned long long>(getWidth()));
#else
                    ss += std::to_string(getWidth());
#endif
                }
                break;
                
            case tlogical:
                ss += "L";
                break;
                
            case tbit:
                ss += "X";
                break;
                
            case tbyte:
                ss += "B";
                break;
                
            case tcplx:
                ss += "C";
                break;
                
            case tdbcplx:
                ss += "M";
                break;
                
            default:
                ss += "I";
        }
        
        return ss;
    }
    
    const dtype FITSform::getDataTypeID(const std::string & stype)
    {
        dtype tt = tnone;
        
        std::string ss = std::string(stype);
        std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);
        
        if(!ss.compare("sbyte"))
            tt =tsbyte;
        else if(!ss.compare("short"))
            tt =tshort;
        else if(!ss.compare("ushort"))
            tt =tushort;
        else if(!ss.compare("int"))
            tt =tint;
        else if(!ss.compare("uint"))
            tt =tuint;
        else if(!ss.compare("long"))
            tt =tlong;
        else if(!ss.compare("longlong"))
            tt =tlonglong;
        else if(!ss.compare("ulong"))
            tt =tulong;
        else if(!ss.compare("float"))
            tt =tfloat;
        else if(!ss.compare("double"))
            tt =tdouble;
        else if(!ss.compare("string"))
            tt =tstring;
        else if(!ss.compare("bool"))
            tt =tlogical;
        else if(!ss.compare("bit"))
            tt =tbit;
        else if(!ss.compare("byte"))
            tt =tbyte;
        else if(!ss.compare("complex"))
            tt =tcplx;
        else if(!ss.compare("double complex"))
            tt = tdbcplx;
        else
            tt = tnone;
    
        return tt;
    }
 
    
#pragma mark * modifier
    void FITSform::Dump( std::ostream& out) const
    {
        out<<"\033[31m- COL #\033[0m"<<fpos<<std::endl
           <<"\033[31m   |- NAME  : \033[0m"<<fname<<std::endl
           <<"\033[31m   |- UNIT  : \033[0m"<<funit<<std::endl
           <<"\033[31m   |- TYPE  : \033[0m"<<getDataType(ftype)<<std::endl;
        
        if(frepeat > 1)
            out<<"\033[31m   |- NELEM : \033[0m"<<frepeat<<std::endl;
        if(fwidth > 0)
            out<<"\033[31m   |- WIDTH : \033[0m"<<fwidth<<" bytes/elmts"<<std::endl;
        
        out<<"\033[31m   |- SCALE : \033[0m"<<fscale<<std::endl
           <<"\033[31m   `- ZERO  : \033[0m"<<fzero<<std::endl;
        
        return;
    }
    
    
#pragma mark - FITScolumn template specialization
    
#pragma mark * initialization
    
    template class FITScolumn< FITSform::complex>;
    template class FITScolumn< FITSform::dblcomplex>;
    template class FITScolumn< std::string >;
    template class FITScolumn< std::vector<std::string> >;

    
#pragma mark - FITStable class implementation
#pragma mark * template specialization
    
    template< >
    FITScolumn<FITSform::complex>* FITStable::read(const FITSform& tform, const int64_t& row)
    {
        FITScolumn<FITSform::complex>* data = new FITScolumn<FITSform::complex>(tform);
        
        int64_t nelem = (static_cast<int64_t>(getNrows())-(row-1))*2;
        
        float   *array  = new float [nelem];
        char *nullarray = new char  [nelem];
        
        if(
#if __cplusplus < 201103L
           ffgcf(fptr      , static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, row, 1, nelem/2, array, nullarray, NULL, &tbl_status)
#else
           ffgcf(fptr.get(), static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem/2, array, nullarray, NULL, &tbl_status)
#endif
           )
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(! std::atoi(&nullarray[k]) )
                data->insert(FITScolumn<FITSform::complex>::col_data(data->size()+row,FITSform::complex(array[k],array[k+1])));
        }
        
        delete [] array;
        delete [] nullarray;
        
        return data;
        
    }
    
    template< >
    FITScolumn<FITSform::dblcomplex>* FITStable::read(const FITSform& tform, const int64_t& row)
    {
        FITScolumn<FITSform::dblcomplex>* data = new FITScolumn<FITSform::dblcomplex>(tform);
        
        int64_t nelem = (static_cast<int64_t>(getNrows())-(row-1))*2;
        
        double   *array = new double [nelem];
        char *nullarray = new char   [nelem];
        
        if(
#if __cplusplus < 201103L
           ffgcf(fptr      , static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem/2, array, nullarray, NULL, &tbl_status)
#else
           ffgcf(fptr.get(), static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem/2, array, nullarray, NULL, &tbl_status)
#endif
           )
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(! std::atoi(&nullarray[k]) )
                data->insert(FITScolumn<FITSform::dblcomplex>::col_data(data->size()+row,FITSform::complex(array[k],array[k+1])));
        }
        
        delete [] array;
        delete [] nullarray;
        
        return data;
        
    }
    
    template< >
    FITScolumn<std::string>* FITStable::read(const FITSform& tform, const int64_t& row)
    {
        FITScolumn<std::string>* data = new FITScolumn<std::string>(tform);
        
        int64_t nrow  = ( static_cast<int64_t>( getNrows() )-(row-1) );
        int64_t nelem =   static_cast<int64_t>( tform.getNelem() );
        
        char  **array = new char *[nrow];
        
        for(int64_t k = 0; k < nrow; k++)
            array[k] = new char [nelem];
        
        char  nullarray[5];
        int   anynull = 0;
        
        strcpy(nullarray,"NULL");
            
        if(
#if __cplusplus < 201103L
            ffgcf(fptr      , static_cast<int>(tform.getType()), static_cast<int>(tform.getPosition()), row, 1, nelem, array, nullarray, NULL, &tbl_status)
#else
            ffgcvs(fptr.get(), static_cast<int>(tform.getPosition()), row, 1, nrow, nullarray, array, &anynull, &tbl_status)
#endif
            )
        {
            delete [] array;
            throw FITSexception(tbl_status,"FITStable","read<std::string>");
        }
            
        
        for(int64_t k = 0; k < nrow; k++)
        {
            if(! std::string(array[k]).compare("NULL"))
                continue;
            
            data->insert(FITScolumn<std::string>::col_data(data->size()+row,std::string(array[k])));
        }
        
        for(int64_t k = 0; k < nrow; k++)
            delete [] array[k];
        
        delete [] array;
        
        return data;
        
    }
    
    template< >
    FITScolumn< std::vector<std::string> >* FITStable::readVector(const FITSform& tform, const size_t& row)
    {
        FITScolumn< std::vector<std::string> >* data = new FITScolumn< std::vector<std::string> >(tform);
        
        int64_t nrow  = ( static_cast<int64_t>( getNrows() )-(row-1) );
        int64_t nelem =   static_cast<int64_t>( tform.getNelem() );
        int64_t nstr  = nelem/tform.getWidth();
        
        char  **array = new char* [nelem];
        
        for(int64_t n = 0; n < nelem; n++)
            array[n] = new char[tform.getWidth()];
        
        char  nullarray[5];
        int   anynull = 0;
        
        strcpy(nullarray,"NULL");
        
        if(
#if __cplusplus < 201103L
            ffgcvs(fptr, static_cast<int>(tform.getPosition()), row, 1, nrow*nstr, nullarray, array, &anynull, &tbl_status)
#else
           ffgcvs(fptr.get(), static_cast<int>(tform.getPosition()), row, 1, nrow*nstr, nullarray, array, &anynull, &tbl_status)
#endif
           )
        {
            throw FITSexception(tbl_status,"FITStable","read<std::string>");
        }
        
        std::vector<std::string> str;
        
        size_t s = 1;
        for(int64_t k = 0; k < nelem; k++)
        {
            if( strcmp(array[k], "NULL") && std::string(array[k]).size() > 0)
                str.push_back( std::string(array[k]) );
            
            
            if(! (s%nstr) )
            {
                if(str.size() > 0)
		  data->insert(FITScolumn< std::vector<std::string> >::col_data(data->size()+static_cast<size_t>(row),std::vector<std::string>(str)));
                str.clear();
                s=0;
            }
           
            s++;
        }
               
        return data;
        
    }
    
    template< >
    void FITStable::write<FITSform::complex>(const FITScolumn<FITSform::complex>* data, const int64_t& first_row)
    {
        float   array[2];
        
        int64_t n = 0;
        for(typename FITScolumn<FITSform::complex>::const_iterator it = data->cbegin(); it != data->cend() && static_cast<size_t>(n) < data->size(); it++)
        {
            if(n >= (first_row-1))
            {
                array[0] = it->second.first;
                array[1] = it->second.second;
                
                if(
#if __cplusplus < 201103L
                   ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#else
                   ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#endif
                   )
                {
                    throw FITSexception(tbl_status,"FITStable","write<complex>");
                }
            }
            n++;
        }
        
        return;
    }
    
    template< >
    void FITStable::write<FITSform::dblcomplex>(const FITScolumn<FITSform::dblcomplex>* data, const int64_t& first_row)
    {
        double   array[2];
        
        int64_t n = 0;
        for(typename FITScolumn<FITSform::dblcomplex>::const_iterator it = data->cbegin(); it != data->cend()  && static_cast<size_t>(n) < data->size(); it++)
        {
            if(n >= (first_row-1))
            {
                array[0] = it->second.first;
                array[1] = it->second.second;
                
                if(
#if __cplusplus < 201103L
                   ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#else
                   ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#endif
                   )
                {
                    throw FITSexception(tbl_status,"FITStable","write<complex>");
                }
            }
            n++;
        }
        
        return;
    }
    
    template< >
    void FITStable::write<std::string>(const FITScolumn<std::string>* data, const int64_t& first_row)
    {
        const int64_t nelem    = data->getWidth();
        char   **array = new char* [1];
        array [0]      = new char [nelem];
        
        size_t n = 0;
        for(typename FITScolumn<std::string>::const_iterator it = data->cbegin(); it != data->cend() && n < data->size(); it++)
        {
           
            if(n >= static_cast<size_t>(first_row-1))
            {
                strcpy(array[0],const_cast<char*>(it->second.c_str()));
                
                if(
#if __cplusplus < 201103L
                   ffpcls(fptr      , static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#else
                   ffpcls(fptr.get(), static_cast<int>(data->getPosition()), n+1, 1, 1, array, &tbl_status)
#endif
                   )
                {
                    throw FITSexception(tbl_status,"FITStable","write<std::string>");
                }
            }
            
            n++;
        }
        
        delete [ ] array[0];
        delete [ ] array;
        
        return;
    }
    
    template<>
    void FITStable::writeVector(const FITScolumn<std::vector<FITSform::complex> >* data, const int64_t& first_row)
    {
        const int64_t nelem = data->getNelem();
        // FIX: Use dynamic allocation instead of VLA
        float* array = new float[nelem*2];

        size_t n = 0;
        for(typename FITScolumn< std::vector<FITSform::complex> >::const_iterator it = data->cbegin(); it != data->cend(); it++)
        {
            if( n < static_cast<size_t>(first_row-1))
            {
                n++;
                continue;
            }
            
            int64_t pos = 0;
            for(size_t k =0; k < static_cast<size_t>(data->getNelem()); k++)
            {
                if(k < it->second.size())
                {
                    array[pos  ] = it->second[k].first;
                    array[pos+1] = it->second[k].second;
                }
                else
                {
                    array[pos  ] = std::numeric_limits<float>::quiet_NaN();
                    array[pos+1] = std::numeric_limits<float>::quiet_NaN();
                }
                pos+=2;
            }
            
            if(
#if __cplusplus < 201103L
               ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data.getPosition()), n+1, 1, nelem, array, &tbl_status)
#else
               ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, nelem, array, &tbl_status)
#endif
               )
            {
                delete[] array;
                throw FITSexception(tbl_status,"FITStable","writeVector<std::vector<FITSform::complex>>");
            }
            
            n++;
        }

        delete[] array;
        return;
    }
    
    template<>
    void FITStable::writeVector(const FITScolumn<std::vector<FITSform::dblcomplex> >* data, const int64_t& first_row)
    {
        const int64_t nelem = data->getNelem();
        // FIX: Use dynamic allocation instead of VLA
        double* array = new double[nelem*2];

        size_t n = 0;
        for(typename FITScolumn< std::vector<FITSform::dblcomplex> >::const_iterator it = data->cbegin(); it != data->cend(); it++)
        {
            if( n < static_cast<size_t>(first_row-1))
            {
                n++;
                continue;
            }
            
            int64_t pos = 0;
            for(size_t k =0; k < static_cast<size_t>(data->getNelem()); k++)
            {
                if(k < it->second.size())
                {
                    array[pos  ] = it->second[k].first;
                    array[pos+1] = it->second[k].second;
                }
                else
                {
                    array[pos  ] = std::numeric_limits<float>::quiet_NaN();
                    array[pos+1] = std::numeric_limits<float>::quiet_NaN();
                }
                pos+=2;
            }
            
            if(
#if __cplusplus < 201103L
               ffpcl(fptr      , static_cast<int>(data->getType()), static_cast<int>(data.getPosition()), n+1, 1, nelem, array, &tbl_status)
#else
               ffpcl(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), n+1, 1, nelem, array, &tbl_status)
#endif
               )
            {
                delete[] array;
                throw FITSexception(tbl_status,"FITStable","writeVector<std::vector<FITSform::complex>>");
            }
            
            n++;
        }

        delete[] array;
        return;
    }
    
    template< >
    void FITStable::writeVector<std::string>(const FITScolumn<std::vector<std::string>>* data, const int64_t& first_row)
    {
        const int64_t nelem    = data->getWidth();
        char   **array = new char* [1];
        array[0] = new char [nelem];
        
        size_t n = 0;
        for(typename FITScolumn<std::vector<std::string> >::const_iterator it = data->cbegin(); it != data->cend() && n < data->size(); it++)
        {
            if(n < static_cast<size_t>(first_row-1))
            {
                n++;
                continue;
            }
            
            for(size_t k = 0; k < static_cast<size_t>(data->getNelem()); k++)
            {
                std::string tmpstr = std::string();
                int64_t nchar = 0;
                
                if( k < it->second.size() )
                {
                    nchar = static_cast<int64_t>(it->second[k].size());
                
                    if(it->second[k].size() > 0)
                        tmpstr += it->second[k];
                    else
                        tmpstr += "NULL";
                }
                else
                {
                    tmpstr += "NULL";
                    nchar = static_cast<int64_t>(tmpstr.size());
                }
                
                int64_t setw = 0;
                while(setw < nelem-nchar)
                {
                    tmpstr+=" ";
                    setw++;
                }
                
                strcpy(array[0], const_cast<char*>(tmpstr.c_str()));
                std::cout<<array[0]<<std::endl;
            
                if(
#if __cplusplus < 201103L
                   ffpcls(fptr      , static_cast<int>(data->getPosition()), n+1, static_cast<int>(k)+1, 1, array, &tbl_status)
#else
                   ffpcls(fptr.get(), static_cast<int>(data->getPosition()), n+1, static_cast<int>(k)+1, 1, array, &tbl_status)
#endif
                   )
                {
                    throw FITSexception(tbl_status,"FITStable","write<std::vector<std::string>>");
                }
            }
            
            n++;
        }

        delete [ ] array[0];
        delete [ ] array;
        
        return;
    }
    
#pragma mark * private member function
 
#if __cplusplus >= 201103L
    
    /**
     * @brief Cleanly close the fits file and free memory
     * @param in Pointer to the fitsfile one want to close.
     **/
    void FITStable::CloseFile(fitsfile* in)
    {
        int status = 0;
        if(fits_close_file(in, &status))
            throw FITSexception(status,"FITStable","Close");
        
         std::cout<<"File CLOSED"<<std::endl;
        
        return;
    }
#endif
    
    
    /**
     @brief Check Fitsfile pointer validity
     @return true if fitsfile pointer isn't a null pointer
     @throw std::invalid_argument if fitsfile pointer is null. This may happen if the file wasn't properly openned or if the file have already been closed.
     */
    bool FITStable::isValid() const
    {
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
            throw FITSexception(SHARED_NULPTR,"FITStable","ctor"," received nullptr");
        
        return true;
    }
    
    const std::string FITStable::getFileName(fitsfile *fptr) const
    {
        int status;
        
        isValid();
        
        char ffname[999];
        ffflnm(fptr, ffname, &status);
        
        return std::string(ffname);
    }
    
    /**
     @brief Retrive the name of the current extension
     @return The extension name of the current HDU block or an empty string if the EXTNAME keyword wasn't found into the current HDU.
     */
    const std::string FITStable::getExtName() const
    {
        isValid();
        
        if(getHDUindex() == 1)
            return std::string("PRIMARY");
        
        char ffname[999];
        char ffcomment[999];
        int status = 0;
        
        try
        {
            if(
#if __cplusplus < 201103L
               ffgkey(fptr, "EXTNAME",ffname, ffcomment, &status)
#else
               ffgkey(fptr.get(), "EXTNAME", ffname, ffcomment, &status)
#endif
               )
            {
                throw FITSexception(status,"FITStable","getExtName","FITS keyword EXTNAME wasn't found in the current HDU.");
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return std::string();
        }
        
        std::string ss =  std::string(ffname);
        while(ss.find_first_of("'") != std::string::npos)
            ss.erase(ss.find_first_of("'"), 1);
        
        while(ss.find_first_of(" ") != std::string::npos)
            ss.erase(ss.find_first_of(" "), 1);
        
        return ss;
    }
    
    /**
     @brief Retrive the data type of the current extension
     @return The data type of the current HDU block.
     */
    const ttype FITStable::getExtType() const
    {
        isValid();
        int status = 0;
        
        int tt = ANY_HDU;
        
        try
        {
#if __cplusplus < 201103L
            ffghdt(fptr, &tt, &status);
#else
            ffghdt(fptr.get(), &tt, &status);
#endif
            if(status)
                throw FITSexception(status,"FITStable","getExtType");
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return static_cast<ttype>(ANY_HDU);
        }
        
        return static_cast<ttype>(tt);
    }
    
    /**
     @brief Retrive the data type of the current extension
     @return The data type of the current HDU block.
     */
    const int FITStable::getHDUindex() const
    {
        isValid();
        
        int tt = 0;
        
#if __cplusplus < 201103L
        ffghdn(fptr, &tt);
#else
        ffghdn(fptr.get(), &tt);
#endif
        return tt;
    }
    
    
    /**
     @brief Relaod HDU block
     @details Make sure the fitsfile is still pointing onto the correct HDU block related to this FITStable. If it is not the case, we move back to the corresponding HDU block.
     @note The verification of the HDU block is done by comparing the FITS header extension name of the current HDU to \c this->fextname. If \c this->fextname is empty, the comparison is done thank to the HDU block number and HDU data type. 
     @throw logic_error if the HDU block corresponding to \c this isn't found. This may happen if the corresponding block have been deleted from the fitsfile by an other instance.
     */
    void FITStable::rload()
    {
        tbl_status = 0;
        std::string extName     = std::string(*const_cast<std::string*>(&fextname));
        
        //-1 Check fits file is still valid
        try
        {
            isValid();
        }
        catch (std::exception &e)
        {
            std::cerr<<e.what()<<std::endl;
            throw;
        }
        
        //-2 Check the current HDU extension name correspond to \c this->fextname
        int hdu_type = ANY_HDU;
        
        if (extName.size() == 0)
        {
            if( fhdu_num != getHDUindex() )
            {
#if __cplusplus < 201103L
                fits_movabs_hdu(fptr, fhdu_num, &hdu_type, &tbl_status);
#else
                fits_movabs_hdu(fptr.get(), fhdu_num, &hdu_type, &tbl_status);
#endif
            }
        }
        else if( (extName.compare(getExtName())) )
        {
#if __cplusplus < 201103L
                    fits_movnam_hdu(fptr, ANY_HDU, const_cast<char*>(extName.c_str()), 0, &tbl_status)
#else
                    fits_movnam_hdu(fptr.get(), ANY_HDU, const_cast<char*>(extName.c_str()), 0, &tbl_status);
#endif
        }
        
        extName.clear();
        
        if(tbl_status)
            throw FITSexception(tbl_status, "FITStable","rload");
        
        //-2 Check the current HDU extension name correspond to \c this->fextname
        if( ftbl_type != getExtType() )
        {
            tbl_status = NOT_TABLE;
            throw FITSexception(tbl_status,"FITStable","rload","Current HDU data type has been changed !!!.");
        }
    }
    
    
    /**
     @brief Read FITS columns and fill a FITScolumn container
     @details Reads FITS HDU block and extract all, or part, of the data associated to a TFORM.
     @param tform TForm one wish to read
     @param start Row index of the first element of the row to read
     @return pointer to the extracted FITSform
     */
    FITSform* FITStable::readArray(const FITSform& tform, const int64_t& start)
    {
        if(start < 1 || start > static_cast<int64_t>(getNrows()))
        {
            tbl_status  = BAD_ROW_NUM;
            throw FITSexception(tbl_status,"FITStable","readArray");
        }
        
        rload();
        
        if(tform.getNelem() <= 1)
        {
            switch(tform.getType())
            {
                case tsbyte:
                    return read<int8_t>(tform, start);
                    
                case tshort:
                    return read<short>(tform, start);
                    
                case tushort:
                    return read<unsigned short>(tform, start);
                    
                case tint:
                    return read<int>(tform, start);
                    
                case tuint:
                    return read<unsigned int>(tform, start);
                    
                case tlong:
                    return read<long int>(tform, start);
                    
                case tlonglong:
                    return read<int64_t>(tform, start);
                    
                case tulong:
                    return read<unsigned long int>(tform, start);
                    
                case tfloat:
                    return read<float>(tform, start);
                    
                case tdouble:
                    return read<double>(tform, start);
                    
                case tstring:
                    return read<std::string>(tform, start);
                    
                case tlogical:
                    return read<char*>(tform, start);
                    
                case tbit:
                    return read<char*>(tform, start);
                    
                case tbyte:
                    return read<uint8_t>(tform, start);
                    
                case tcplx:
                    return read<FITSform::complex>(tform, start);
                    
                case tdbcplx:
                    return read<FITSform::dblcomplex>(tform, start);
                    
                default:
                    break;
            }
        }
         else
        {
            switch(tform.getType())
            {
                case tsbyte:
                    return readVector<int8_t>(tform, start);
                    
                case tshort:
                    return readVector<short>(tform, start);
                    
                case tushort:
                    return readVector<unsigned short>(tform, start);
                    
                case tint:
                    return readVector<int>(tform, start);
                    
                case tuint:
                    return readVector<unsigned int>(tform, start);
                    
                case tlong:
                    return readVector<long int>(tform, start);
                    
                case tlonglong:
                    return readVector<int64_t>(tform, start);
                    
                case tulong:
                    return readVector<unsigned long int>(tform, start);
                    
                case tfloat:
                    return readVector<float>(tform, start);
                    
                case tdouble:
                    return readVector<double>(tform, start);
                    
                case tstring:
                    if(tform.getNelem() / tform.getWidth() <= 1)
                        return read<std::string>(tform, start);
                    else
                        return readVector<std::string>(tform, start);
                    break;
                    
                case tlogical:
                    return readVector<char*>(tform, start);
                    
                case tbit:
                    return readVector<char*>(tform, start);
                    
                case tbyte:
                    return readVector<uint8_t>(tform, start);
                    
                case tcplx:
                    return readVector<FITSform::complex>(tform, start);
                    
                case tdbcplx:
                    return readVector<FITSform::dblcomplex>(tform, start);
                    
                default:
                    break;
            }
        }
        
        return new FITSform(tform);
    }
    
    
    
#pragma mark * ctor/dtor
    
    /**
     @brief Constructor
     @details Construct DSL::FITStable instance from specific HDU block of a fitsfile.

     @param fits pointer to a fits file that is already opened.
     @param iHDU index of the HDU block of interest that contains the FITS table
     @throw invalid_argument if the fitsfile haven't been open properly. This may occurs if you failed to open the FITS file.
     @throw invalid_argument if the HDU block you wish to access does not exist.
     @throw logic_error if the HDU block you wish to access neither contains a FITS BINARY nor a FITS ASCII table.
     */
    FITStable::FITStable(
#if __cplusplus < 201103L
                         fitsfile* fits
#else
                         std::shared_ptr<fitsfile> fits
#endif
                         , const int& iHDU):tbl_status(0),ftbl_type(ttype::tbinary), fextname(), fhdu_num(iHDU)
    {
        
        int hdu_type = ANY_HDU;

        //Store adress of the current fits file
#if __cplusplus < 201103L
        fptr = fits;
#else
        fptr = std::shared_ptr<fitsfile>(fits);
#endif
        
        isValid();
        
        //Move to desired HDU block
        if(
#if __cplusplus < 201103L
           fits_movabs_hdu(fptr, fhdu_num, &hdu_type, &tbl_status)
#else
           fits_movabs_hdu(fptr.get(), fhdu_num, &hdu_type, &tbl_status)
#endif
           )
            throw FITSexception(tbl_status, "FITStable","ctor");
        
        ttype* tt = const_cast<ttype*>(&ftbl_type);
        
        switch (hdu_type) {
            case ASCII_TBL:
                *tt = tascii;
                break;
            
            case BINARY_TBL:
                *tt = tbinary;
                break;
                
            default:
                *tt = timg;
                break;
        }
        
        //Check the HDU block i indeed a FITS table and register either it is a binary or an ASCII table
        if(ftbl_type != ttype::tascii &&
           ftbl_type != ttype::tbinary)
        {
            tbl_status = NOT_TABLE;
            throw FITSexception(tbl_status,"FITStable","ctor","Current HDU isn't a BINARY nor a ASCII FITS table.");
        }
        
        //Get the extension name of the HDU block
        std::string *extname = const_cast<std::string*>(&fextname);
        *extname = std::string(getExtName());
    }
    
    
    /**
     @brief Constructor
     @details Construct DSL::FITStable instance from specific HDU block of a fitsfile.
     
     @param fits pointer to a fits file that is already opened.
     @param extname HDU block extension name  that contains the FITS table of interest.
     @throw invalid_argument if the fitsfile haven't been open properly. This may occurs if you failed to open the FITS file.
     @throw invalid_argument if the HDU block you wish to access does not exist.
     @throw logic_error if the HDU block you wish to access neither contains a FITS BINARY nor a FITS ASCII table.
     */
    FITStable::FITStable(
#if __cplusplus < 201103L
                         fitsfile* fits
#else
                         std::shared_ptr<fitsfile> fits
#endif
                         , const std::string& extname):tbl_status(0),ftbl_type(ttype::tbinary), fextname(extname), fhdu_num(0)
    {
        int hdu_type = ANY_HDU;
        
        //Store adress of the current fits file
        
#if __cplusplus < 201103L
        fptr = fits;
#else
        fptr = std::shared_ptr<fitsfile>(fits);
#endif
        
        isValid();
        
        //Move to desired HDU block
        
        if(
#if __cplusplus < 201103L
           fits_movnam_hdu(fptr, ANY_HDU, const_cast<char*>(extname.c_str()), 0, &tbl_status)
#else
           fits_movnam_hdu(fptr.get(), ANY_HDU, const_cast<char*>(extname.c_str()), 0, &tbl_status)
#endif
           )
            throw FITSexception(tbl_status, "FITStable","ctor");
        
        hdu_type = getExtType();
        ttype* tt = const_cast<ttype*>(&ftbl_type);
        
        switch (hdu_type) {
            case ASCII_TBL:
                *tt = tascii;
                break;
                
            case BINARY_TBL:
                *tt = tbinary;
                break;
                
            default:
                *tt = timg;
                break;
        }
        
        //Check the HDU block i indeed a FITS table and register either it is a binary or an ASCII table
        if(ftbl_type != ttype::tascii &&
           ftbl_type != ttype::tbinary)
        {
            tbl_status = NOT_TABLE;
            throw FITSexception(tbl_status,"FITStable","ctor","Current HDU isn't a BINARY nor a ASCII FITS table.");
        }
        
    }
    /**
     * @brief destructor
     * @note C++11 : Take care to properly close the fitsfile if it isn't use anymore by other instance.
     * @note Non C++11 : the pointer to the fitsfile isn't freed and it is the responsibility to the user to close the file and free that pointer properly.
     **/
    FITStable::~FITStable()
    {
#if __cplusplus >= 201103L
        fptr.~shared_ptr();
#endif
    }
    
    
#pragma mark * Properties
    
    /**
     @brief Get the number of rows.
     @return The number of rows in each columns
     */
    size_t FITStable::getNrows()
    {
        try
        {
            rload();
        }
        catch (std::exception &e)
        {
            std::cerr<<e.what()<<std::endl;
            throw;
        }
    
        LONGLONG nrows = 0;
        
        if(
#if __cplusplus < 201103L
           ffgnrwll(fptr, &nrows, &tbl_status)
#else
           ffgnrwll(fptr.get(), &nrows, &tbl_status)
           
#endif
           )
            throw FITSexception(tbl_status,"FITStable","getNrows");
        
        return static_cast<size_t>(nrows);
    }
    
    /**
     @brief Get the number of columns in the FITStable.
     @return The number of columns.
     */
    size_t FITStable::getNcols()
    {
        try
        {
            rload();
        }
        catch (std::exception &e)
        {
            std::cerr<<e.what()<<std::endl;
            throw;
        }
        
        int ncols = 0;
        
        if(
#if __cplusplus < 201103L
           ffgncl(fptr, &ncols, &tbl_status)
#else
           ffgncl(fptr.get(), &ncols, &tbl_status)
#endif
           )
            throw FITSexception(tbl_status,"FITStable","getNcols");
        
        return static_cast<size_t>(ncols);
    }
    
    
    /**
     @brief List all column.
     @details Retrive column name and column datatype of all columns of the FITS table.
     @return The list of column with their index, name and embeded datatype.
     */
    const clist FITStable::FITStable::listColumns()
    {
        size_t ncols;
        try
        {
            ncols = getNcols();
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            throw;
        }
        
        clist list = clist();
        
        for(size_t n = 1; n <= ncols; n++)
        {
            FITSform tform = columnProperties(n);
            list.insert(std::pair<size_t,FITSform>(tform.getPosition(), tform));
        }
        
        return list;
        
    }
    
    
    /**
     @brief Get FITS column properties
     @details Search FITStable header block for a specific column and retrive its intrinsec properties
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @return container that embed the column's properties
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
     */
    const FITSform FITStable::columnProperties(const std::string& name)
    {
        //1- Make sure the current header point toward this table
        rload();
        
        //2- Get column number and column full name for the desired column
        int colnum = 0;
        try
        {
            if(
#if __cplusplus < 201103L
               ffgcno(fptr, CASEINSEN, const_cast<char*>(name.c_str()), &colnum, &tbl_status)
#else
               ffgcno(fptr.get(), CASEINSEN, const_cast<char*>(name.c_str()), &colnum, &tbl_status)
#endif
               )
                throw FITSexception(tbl_status,"FITStable","ColumnsProperties");
        }
        catch (std::exception& e)
        {
            if(tbl_status == COL_NOT_UNIQUE)
                std::cerr<<e.what()<<std::flush;
            else
                throw;
        }
        
        //3- retrive column properties
        return columnProperties(colnum);
    }
    
    
    /**
     @brief Get FITS column properties
     @details Search FITStable header block for a specific column and retrive its intrinsec properties
     @param size_t& Non null unsigned inter number of the desired column
     @return container that embed the column's properties
     */
    const FITSform FITStable::columnProperties(const size_t& n)
    {
        //1- Check column indexing validity
        if(n == 0 || n > getNcols())
        {
            tbl_status = BAD_COL_NUM;
            throw FITSexception(tbl_status,"FITStable","ColumnsProperties");
        }
        
        //2- Make sure the current header point toward this table
        rload();
        
        //3- Retrive column propertie for the desired column
        //3.1- Retrive column name
        
        char TFIELD[100];
        int colnum = 0;
        
        if(
#if __cplusplus < 201103L
           ffgcnn(fptr, CASEINSEN, const_cast<char*>((std::to_string(static_cast<unsigned long long>(n))).c_str()), TFIELD, &colnum, &tbl_status)
#else
           ffgcnn(fptr.get(), CASEINSEN, const_cast<char*>((std::to_string(n)).c_str()), TFIELD, &colnum, &tbl_status)
#endif
           )
            throw FITSexception(tbl_status,"FITStable","listColumns");
        
        //3.2- Retrive column data type
        int data_type = 0;
        LONGLONG trepeat;
        LONGLONG twidth;
        
        if(
#if __cplusplus < 201103L
           ffeqtyll(fptr      , colnum, &data_type, &trepeat, &twidth, &tbl_status)
#else
           ffeqtyll(fptr.get(), colnum, &data_type, &trepeat, &twidth, &tbl_status)
#endif
           )
            throw FITSexception(tbl_status,"FITStable","listColumns");
        
        //3.3- Retrive other column properties
        long   tbcol = 0;
        char   tunit[100];
        double tscale = 0;
        double tzero  = 0;
        
        switch (ftbl_type)
        {
            case tascii:
                if(
#if __cplusplus < 201103L
                   ffgacl(fptr, colnum, NULL, &tbcol, tunit, NULL, &tscale, &tzero, NULL, NULL, &tbl_status)
#else
                   ffgacl(fptr.get(), colnum, NULL, &tbcol, tunit, NULL, &tscale, &tzero, NULL, NULL, &tbl_status)
#endif
                   )
                    throw FITSexception(tbl_status,"FITStable","listColumns");
                break;
                
            case tbinary:
                if(
#if __cplusplus < 201103L
                   ffgbcl(fptr, colnum, NULL, tunit, NULL, NULL, &tscale, &tzero, NULL, NULL, &tbl_status)
#else
                   ffgbcl(fptr.get(), colnum, NULL, tunit, NULL, NULL, &tscale, &tzero, NULL, NULL, &tbl_status)
#endif
                   )
                    throw FITSexception(tbl_status,"FITStable","listColumns");
                break;
                
            default:
                throw FITSwarning("FITStable","listColumns","Current header neither a BINARY nor an ASCII table.");
                break;
        }
        
        //3.4 Fill the FITSform
        FITSform tform = FITSform(colnum,std::string(TFIELD),static_cast<dtype>(data_type),static_cast<int64_t>(trepeat), static_cast<int64_t>(twidth), tscale,tzero,std::string(tunit));
        
        //4 Deliver the container
        return tform;
    }
    
#pragma mark * Accessing coulumn data
    /**
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @return container that embed the column's properties and data
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
     */
    FITSform* FITStable::readColumn(const std::string& name)
    {
        return readColumn(name, 1);
    }
    
    
    /**
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param colnum non-null usinged integer number of the column of interest
     @return container that embed the column's properties and data
     */
    FITSform* FITStable::readColumn(const size_t& colnum)
    {
        return readColumn(colnum, 1);
    }
    
    /**
     @brief Get part of a FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @param start starting row one which to extract the data
     @return container that embed the column's properties and data
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
     */
    FITSform* FITStable::readColumn(const std::string& name, const size_t& start)
    {
        //1- Get columnProperties for the column of interest
        FITSform form = columnProperties(name);
       
        //2- Retrive data
        return readColumn(form.getPosition(), start);
    }
    
    /**
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param colnum non-null usinged integer number of the column of interest
     @param start starting row one which to extract the data
     @return container that embed the column's properties and data
     */
    FITSform* FITStable::readColumn(const size_t& column, const size_t& start)
    {
        //1- Get columnProperties for the column of interest
        FITSform form = columnProperties(column);
        
        return readArray(form, start);
    }
    
#pragma mark * Inserting/Updating data to column
#pragma mark 1- Inseting new column
    
    
    /**
     @brief Append a new column into the FITS file
     @details Create a new empty column into the Fits file. If the column already exist, the column will not be created.

     @param cname The name of the new collumn
     @param dataType The datatype of the new column
     */
    void FITStable::InsertColumn(const std::string& cname, const std::string& tform, const std::string& tunit, int* colnum)
    {
        try
        {            
            if(!
#if __cplusplus < 201103L
                ffgcno(fptr, CASESEN, cname.c_str(), colnum, &tbl_status)
#else
                ffgcno(fptr.get(), CASESEN, const_cast<char*>(cname.c_str()), colnum, &tbl_status)
#endif
               )
            {
                tbl_status = COL_NOT_UNIQUE;
                throw FITSwarning("FITStable","InsertColumn","Column "+cname+" already exists. Please, provide a new name");
            }
            
            if(
#if __cplusplus < 201103L
               fficol(fptr, static_cast<int>( getNcols( ) )+1, const_cast<char*>(cname.c_str()), const_cast<char*>(tform.c_str()), &tbl_status )
#else
               fficol(fptr.get(), static_cast<int>( getNcols( ) )+1, const_cast<char*>(cname.c_str()), const_cast<char*>(tform.c_str()), &tbl_status )
#endif
               )
            {
                throw FITSexception(tbl_status,"FITStable","InsertColumn");
            }
            
            if(
#if __cplusplus < 201103L
               ffgcno(fptr, CASESEN, cname.c_str(), colnum, &tbl_status)
#else
               ffgcno(fptr.get(), CASESEN, const_cast<char*>(cname.c_str()), colnum, &tbl_status)
#endif
               )
            {
                throw FITSexception(tbl_status,"FITStable","InsertColumn","Column "+cname+" not found");
            }
            
            if(tunit.size() > 0)
            {
                if
                (
#if __cplusplus < 201103L
                 long long p = *colnum
                 ffuky(fptr, TSTRING, const_cast<char*>(std::string("TUNIT"+std::to_string(p)).c_str()), const_cast<char*>(tunit.c_str()), "Data unit", &tbl_status )
#else
                 ffuky(fptr.get(), TSTRING, const_cast<char*>(std::string("TUNIT"+std::to_string(*colnum)).c_str()), const_cast<char*>(tunit.c_str()), "Data unit", &tbl_status )
#endif
                )
                {
                    throw FITSexception(tbl_status,"FITStable","InsertColumn");
                }
            }
        }
        catch (std::exception& e)
        {
            if(tbl_status == COL_NOT_UNIQUE)
            {
                std::cerr<<e.what()<<std::endl;
                return;
            }
            else
                throw;
        }
        
        return;
    }
    
    /**
     @brief Append a new column into the FITS file
     @details Insert new column into the FITS file and fill the column rows with the corrsponding value.
     @note Note that if the column already exist, it will reaise an exception.
     
     @param col The column that will be inserted into the table.
     */
    void FITStable::InsertColumn( FITSform* col )
    {
        try
        {
            int colnum = 0;
            InsertColumn(col->getName(), col->getTTYPE(), col->getUnit(), &colnum);
            
            if(tbl_status == COL_NOT_UNIQUE)
                return;
            
            col->setPosition(colnum);
            
            writeArray(col,1);
            
        }
        catch (std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            throw;
        }
    }
    
#pragma mark 2- Inseting value to an existing column
    
    /**
     @brief Write FITS columns to FITS file
     @details Write the content of the FITScolumn to the FITS ASCII or BINARY table referenced by \c this.
     @param tform The data to be written
     @param start The row index to which one whish to start writting data
     */
    void FITStable::writeArray(const FITSform* tform, const int64_t& start)
    {
        rload();
        
        if(start < 1 || start > static_cast<int64_t>(getNrows()+1))
        {
            tbl_status  = BAD_ROW_NUM;
            throw FITSexception(tbl_status,"FITStable","writeArray");
        }
        
        if(tform->getNelem() <= 1)
        {
            switch(tform->getType())
            {
                case tsbyte:
                    return write<int8_t>(dynamic_cast<const FITScolumn<int8_t>*>(tform), start);
                    
                case tshort:
                    return write<short>(dynamic_cast<const FITScolumn<short>*>(tform), start);
                    
                case tushort:
                    return write<unsigned short>(dynamic_cast<const FITScolumn<unsigned short>*>(tform), start);
                    
                case tint:
                    return write<int>(dynamic_cast<const FITScolumn<int>*>(tform), start);
                    
                case tuint:
                    return write<unsigned int>(dynamic_cast<const FITScolumn<unsigned int>*>(tform), start);
                    
                case tlong:
                    return write<long int>(dynamic_cast<const FITScolumn<long int>*>(tform), start);
                    
                case tlonglong:
                    return write<int64_t>(dynamic_cast<const FITScolumn<int64_t>*>(tform), start);
                    
                case tulong:
                    return write<unsigned long int>(dynamic_cast<const FITScolumn<unsigned long int>*>(tform), start);
                    
                case tfloat:
                    return write<float>(dynamic_cast<const FITScolumn<float>*>(tform), start);
                    
                case tdouble:
                    return write<double>(dynamic_cast<const FITScolumn<double>*>(tform), start);
                    
                case tstring:
                    return write<std::string>(dynamic_cast<const FITScolumn<std::string>*>(tform), start);
                    
                case tlogical:
                    return write<char*>(dynamic_cast<const FITScolumn<char*>*>(tform), start);
                    
                case tbit:
                    return write<char*>(dynamic_cast<const FITScolumn<char*>*>(tform), start);
                    
                case tbyte:
                    return write<uint8_t>(dynamic_cast<const FITScolumn<uint8_t>*>(tform), start);
                    
                case tcplx:
                    return write<FITSform::complex>(dynamic_cast<const FITScolumn<FITSform::complex>*>(tform), start);
                    
                case tdbcplx:
                    return write<FITSform::dblcomplex>(dynamic_cast<const FITScolumn<FITSform::dblcomplex>*>(tform), start);
                    
                default:
                    break;
            }
        }
        else
        {
            switch(tform->getType())
            {
                case tsbyte:
                    return writeVector<int8_t>(dynamic_cast<const FITScolumn<std::vector<int8_t>>*>(tform), start);
                    
                case tshort:
                    return writeVector<short>(dynamic_cast<const FITScolumn<std::vector<short>>*>(tform), start);
                    
                case tushort:
                    return writeVector<unsigned short>(dynamic_cast<const FITScolumn<std::vector<unsigned short>>*>(tform), start);
                    
                case tint:
                    return writeVector<int>(dynamic_cast<const FITScolumn<std::vector<int>>*>(tform), start);
                    
                case tuint:
                    return writeVector<unsigned int>(dynamic_cast<const FITScolumn<std::vector<unsigned int>>*>(tform), start);
                    
                case tlong:
                    return writeVector<long int>(dynamic_cast<const FITScolumn<std::vector<long int>>*>(tform), start);
                    
                case tlonglong:
                    return writeVector<int64_t>(dynamic_cast<const FITScolumn<std::vector<int64_t>>*>(tform), start);
                    
                case tulong:
                    return writeVector<unsigned long int>(dynamic_cast<const FITScolumn<std::vector<unsigned long int>>*>(tform), start);
                    
                case tfloat:
                    return writeVector<float>(dynamic_cast<const FITScolumn<std::vector<float>>*>(tform), start);
                    
                case tdouble:
                    return writeVector<double>(dynamic_cast<const FITScolumn<std::vector<double>>*>(tform), start);
                    
                case tstring:
                    return writeVector<std::string>(dynamic_cast<const FITScolumn<std::vector<std::string>>*>(tform), start);
                    
                case tlogical:
                    return writeVector<char*>(dynamic_cast<const FITScolumn<std::vector<char*>>*>(tform), start);
                    
                case tbit:
                    return writeVector<char*>(dynamic_cast<const FITScolumn<std::vector<char*>>*>(tform), start);
                    
                case tbyte:
                    return writeVector<uint8_t>(dynamic_cast<const FITScolumn<std::vector<uint8_t>>*>(tform), start);
                    
                case tcplx:
                    return writeVector<FITSform::complex>(dynamic_cast<const FITScolumn<std::vector<FITSform::complex>>*>(tform), start);
                    
                case tdbcplx:
                    return writeVector<FITSform::dblcomplex>(dynamic_cast<const FITScolumn<std::vector<FITSform::dblcomplex>>*>(tform), start);
                    
                default:
                    break;
            }
        }
        
        return;
    }
    
#pragma mark 3- Updating value from an existing column
    
    
#pragma mark * Diagnoze
    
    /**
     @brief Dumping this FITStable instance.
     @details Print out information related to this FITStable.
     @param out output stream one which to send the information.
     */
    void FITStable::Dump( std::ostream& out)
    {
        try
        {
            rload();
        }
        catch(std::exception& e)
        {
            throw;
        }
        
        out<<"\033[31m --- "<<
#if __cplusplus < 201103L
            getFileName(fptr)
#else
            getFileName(fptr.get())<<" (\033[34m"<<fptr.use_count()<<"\033[31m)"
#endif
        <<" --- \033[0m"<<std::endl;
        
        out<<"\033[31m   |- HDU #    :\033[0m"<<fhdu_num<<" (infile ptr on HDU "<<getHDUindex()<<")"<<std::endl
        <<"\033[31m   |- HDU TITLE:\033[0m"<<*(const_cast<std::string*>(&fextname))<<" (infile ptr on HDU "<<getExtName()<<")"<<std::endl;
        
        switch (ftbl_type)
        {
            case tascii:
                out <<"\033[31m   |- HDU TYPE :\033[0mASCII"<<std::endl;
                break;
                
            case tbinary:
                out <<"\033[31m   |- HDU TYPE :\033[0mBINARY"<<std::endl;
                break;
                
            default:
                break;
        }
        
        out<<"\033[31m   |- N ROWS   :\033[0m"<<getNrows()<<std::endl
           <<"\033[31m   |- N COLS   :\033[0m"<<getNcols()<<std::endl;
        
        clist columns = listColumns();
        for(clist::const_iterator it = columns.cbegin(); it != columns.cend(); it++)
        {
            
            out<<"\033[31m   |- COL #\033[0m"<<it->second.getPosition()<<std::endl
               <<"\033[31m   |   |- NAME  : \033[0m"<<it->second.getName()<<std::endl
               <<"\033[31m   |   |- UNIT  : \033[0m"<<it->second.getUnit()<<std::endl
               <<"\033[31m   |   |- TYPE  : \033[0m"<<FITSform::getDataType(it->second.getType())<<std::endl
               <<"\033[31m   |   |- SCALE : \033[0m"<<it->second.getScale()<<std::endl;
            
            if(it->second.getNelem() > 1)
                out<<"\033[31m   |   |- NELEM : \033[0m"<<it->second.getNelem()<<std::endl;
            if(it->second.getWidth() > 0)
                out<<"\033[31m   |   |- WIDTH : \033[0m"<<it->second.getWidth()<<" bytes/elmts"<<std::endl;
            
            out<<"\033[31m   |   `- ZERO  : \033[0m"<<it->second.getZero()<<std::endl;
        }
        
        out<<"\033[31m   `- \033[34mDONE\033[0m"<<std::endl;
        
    }
}


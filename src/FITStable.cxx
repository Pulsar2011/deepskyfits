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
    
    FITSform::FITSform(const size_t& p,const std::string& name, const dtype& t, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(1),fwidth(1),fpos(p)
    {};
    
    FITSform::FITSform(const size_t& p, const std::string& name, const dtype& t, const double& s, const double& z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(1),fwidth(1),fpos(p)
    {};
    
    FITSform::FITSform(const size_t& p,const std::string& name, const dtype& t, const int64_t& r, const int64_t& w, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(r),fwidth(w),fpos(p)
    {};
    
    FITSform::FITSform(const size_t& p, const std::string& name, const dtype& t, const int64_t& r, const int64_t& w, const double& s, const double& z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(r),fwidth(w),fpos(p)
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
    void FITStable::read(FITScolumn<FITSform::complex>* data,  const std::shared_ptr<fitsfile>& fptr,  const size_t& row)
    {
        int64_t nelem = (static_cast<int64_t>(nrows())-(row-1))*2;
        
        float   *array  = new float [nelem];
        char *nullarray = new char  [nelem];
        int tbl_status  = 0;
        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, NULL, &tbl_status))
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(! std::atoi(&nullarray[k]) )
                data->push_back(FITSform::complex(array[k],array[k+1]));
        }
        
        delete [] array;
        delete [] nullarray;
        
        return;
        
    }
    
    template< >
    void FITStable::read(FITScolumn<FITSform::dblcomplex>* data,  const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        int64_t nelem = (static_cast<int64_t>(nrows())-(row-1))*2;
        
        double   *array = new double [nelem];
        char *nullarray = new char   [nelem];
        int tbl_status  = 0;
        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, NULL, &tbl_status))
        {
            delete [] array;
            delete [] nullarray;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(! std::atoi(&nullarray[k]) )
                data->push_back(FITSform::dblcomplex(array[k],array[k+1]));
        }
        
        delete [] array;
        delete [] nullarray;        
    }
    
    template< >
    void FITStable::read( FITScolumn<std::string>* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        int64_t nrow  = ( static_cast<int64_t>( nrows() )-(row-1) );
        int64_t nelem =   static_cast<int64_t>( data->getNelem() );
        
        char  **array = new char *[nrow];
        
        for(int64_t k = 0; k < nrow; k++)
            array[k] = new char [nelem];
        
        char  nullarray[5];
        int   anynull = 0;
        int tbl_status  = 0;
        
        strcpy(nullarray,"NULL");
            
        if(ffgcvs(fptr.get(), static_cast<int>(data->getPosition()), row, 1, nrow, nullarray, array, &anynull, &tbl_status))
        {
            delete [] array;
            throw FITSexception(tbl_status,"FITStable","read<std::string>");
        }
            
        
        for(int64_t k = 0; k < nrow; k++)
        {
            if(! std::string(array[k]).compare("NULL"))
                continue;
            
            data->push_back(std::string(array[k]));
        }
        
        for(int64_t k = 0; k < nrow; k++)
            delete [] array[k];
        
        delete [] array;        
    }
    
    template< >
    void FITStable::readVector(FITScolumn< std::vector<std::string> >* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        int64_t nrow  = ( static_cast<int64_t>( nrows() )-(row-1) );
        int64_t nelem =   static_cast<int64_t>( data->getNelem() );
        int64_t nstr  = nelem/data->getWidth();
        
        char  **array = new char* [nelem];
        
        for(int64_t n = 0; n < nelem; n++)
            array[n] = new char[data->getWidth()];
        
        char  nullarray[5];
        int   anynull = 0;
        int tbl_status  = 0;
        
        strcpy(nullarray,"NULL");
        
        if(ffgcvs(fptr.get(), static_cast<int>(data->getPosition()), row, 1, nrow*nstr, nullarray, array, &anynull, &tbl_status))
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
		            data->push_back(std::vector<std::string>(str));
                str.clear();
                s=0;
            }
           
            s++;
        }
    }
    
    template< >
    void FITScolumn< FITSform::complex >::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(fptr == nullptr)
        {
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<complex>","write");
        }

        if(data.size() < 1)
        {
            throw FITSexception(NOT_TABLE,"FITScolumn<complex>","write");
        }

        int64_t n = 0;
        int tbl_status = 0;

        for(col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            if(n >= (first_row-1))
            {
                float arr[2] = {it->first, it->second};
                
                if(ffpclc(fptr.get(), static_cast<int>(getPosition()), n+1, 1, 1, arr, &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITStable","write<complex>");
                }
            }
            n++;
        }
    }

    template< >
    void FITScolumn<FITSform::dblcomplex>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(fptr == nullptr)
        {
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<dblcomplex>","write");
        }

        if(data.size() < 1)
        {
            throw FITSexception(NOT_TABLE,"FITScolumn<dblcomplex>","write");
        }

        int64_t n = 0;
        int tbl_status = 0;

        for(col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            if(n >= (first_row-1))
            {
                double arr[2] = {it->first, it->second};
                
                if(ffpclm(fptr.get(), static_cast<int>(getPosition()), n+1, 1, 1, arr, &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITStable","write<dblcomplex>");
                }
            }
            n++;
        }
    }
    
    template< >
    void FITScolumn<std::string>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(fptr == nullptr)
        {
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<std::string>","write");
        }

        if(data.size() < 1)
        {
            throw FITSexception(NOT_TABLE,"FITScolumn<std::string>","write");
        }

        int64_t n = 0;
        int tbl_status = 0;

        for(col_map::const_iterator it = data.cbegin(); it != data.cend(); it++)
        {
            if(n >= (first_row-1))
            {
                char* str = const_cast<char*>(it->c_str());
                
                if(ffpcls(fptr.get(), static_cast<int>(getPosition()), n+1, 1, 1, &str, &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITStable","write<std::string>");
                }
            }
            n++;
        }
    }
    
    template<>
    void FITScolumn<FITSform::int8Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<int8Vector>","write");
        
        if(data.empty())
            throw FITSexception(NOT_TABLE,"FITScolumn<int8Vector>","write");

        const int64_t nelem = getNelem();
        int8_t* buffer = new int8_t[nelem];
        int tbl_status = 0;
        size_t row = 0;

        for(auto it = data.cbegin(); it != data.cend(); ++it, ++row)
        {
            if(row < static_cast<size_t>(first_row-1)) 
                continue;

            for(int64_t i=0;i<nelem;++i)
                buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : int8_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            {
                delete [] buffer;
                throw FITSexception(tbl_status,"FITScolumn<int8Vector>","write");
            }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::uint8Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<uint8Vector>","write");

        if(data.empty())
            throw FITSexception(NOT_TABLE,"FITScolumn<uint8Vector>","write");

        const int64_t nelem = getNelem();
        uint8_t* buffer = new uint8_t[nelem];
        int tbl_status = 0;
        size_t row = 0;

        for(auto it = data.cbegin(); it != data.cend(); ++it, ++row)
        {
            if(row < static_cast<size_t>(first_row-1))
                continue;
            for(int64_t i=0;i<nelem;++i)
                buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : uint8_t(0);
            
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            {
                delete [] buffer;
                throw FITSexception(tbl_status,"FITScolumn<uint8Vector>","write");
            }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::int16Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<int16Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<int16Vector>","write");
        const int64_t nelem = getNelem();
        int16_t* buffer = new int16_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : int16_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<int16Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::uint16Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<uint16Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<uint16Vector>","write");
        const int64_t nelem = getNelem();
        uint16_t* buffer = new uint16_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : uint16_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<uint16Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::int32Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<int32Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<int32Vector>","write");
        const int64_t nelem = getNelem();
        int32_t* buffer = new int32_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : int32_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<int32Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::uint32Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<uint32Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<uint32Vector>","write");
        const int64_t nelem = getNelem();
        uint32_t* buffer = new uint32_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : uint32_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<uint32Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::int64Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<int64Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<int64Vector>","write");
        const int64_t nelem = getNelem();
        int64_t* buffer = new int64_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : int64_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<int64Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::uint64Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<uint64Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<uint64Vector>","write");
        const int64_t nelem = getNelem();
        uint64_t* buffer = new uint64_t[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : uint64_t(0);
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<uint64Vector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::floatVector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<floatVector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<floatVector>","write");
        const int64_t nelem = getNelem();
        float* buffer = new float[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : std::numeric_limits<float>::quiet_NaN();
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<floatVector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::doubleVector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<doubleVector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<doubleVector>","write");
        const int64_t nelem = getNelem();
        double* buffer = new double[nelem];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : std::numeric_limits<double>::quiet_NaN();
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<doubleVector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::complexVector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<complexVector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<complexVector>","write");
        const int64_t nelem = getNelem();
        float* buffer = new float[nelem*2];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            int64_t pos=0;
            for(int64_t i=0;i<nelem;++i)
            {
                if(i < static_cast<int64_t>(it->size()))
                {
                    buffer[pos]   = it->at(static_cast<size_t>(i)).first;
                    buffer[pos+1] = it->at(static_cast<size_t>(i)).second;
                }
                else
                {
                    buffer[pos]   = std::numeric_limits<float>::quiet_NaN();
                    buffer[pos+1] = std::numeric_limits<float>::quiet_NaN();
                }
                pos+=2;
            }
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<complexVector>","write"); }
        }
        delete [] buffer;
    }

    template<>
    void FITScolumn<FITSform::dblcomplexVector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<dblcomplexVector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<dblcomplexVector>","write");
        const int64_t nelem = getNelem();
        double* buffer = new double[nelem*2];
        int tbl_status = 0;
        size_t row=0;
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            int64_t pos=0;
            for(int64_t i=0;i<nelem;++i)
            {
                if(i < static_cast<int64_t>(it->size()))
                {
                    buffer[pos]   = it->at(static_cast<size_t>(i)).first;
                    buffer[pos+1] = it->at(static_cast<size_t>(i)).second;
                }
                else
                {
                    buffer[pos]   = std::numeric_limits<double>::quiet_NaN();
                    buffer[pos+1] = std::numeric_limits<double>::quiet_NaN();
                }
                pos+=2;
            }
            if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
            { delete [] buffer; throw FITSexception(tbl_status,"FITScolumn<dblcomplexVector>","write"); }
        }
        delete [] buffer;
    }
    
    template<>
    void FITScolumn<FITSform::stringVector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(fptr == nullptr)
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<stringVector>","write");
        if(data.size() < 1)
            throw FITSexception(NOT_TABLE,"FITScolumn<stringVector>","write");

        const int64_t width  = getWidth();   // chars per string
        size_t row = 0;
        int tbl_status = 0;

        for(col_map::const_iterator it = data.cbegin(); it != data.cend(); ++it, ++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;

            for(size_t k = 0; k < static_cast<size_t>(getNelem()); ++k)
            {
                std::string tmp = (k < it->size() && !it->at(k).empty()) ? it->at(k) : "NULL";
                tmp.resize(static_cast<size_t>(width), ' ');
                char* buf = const_cast<char*>(tmp.c_str());

                if(ffpcls(fptr.get(), static_cast<int>(getPosition()), static_cast<LONGLONG>(row+1),
                          static_cast<LONGLONG>(k+1), 1, &buf, &tbl_status))
                    throw FITSexception(tbl_status,"FITScolumn<stringVector>","write");
            }
        }
    }

#pragma mark * private member function
    
    /*
     @brief Read FITS columns and fill a FITScolumn container
     @details Reads FITS HDU block and extract all, or part, of the data associated to a TFORM.
     @param tform TForm one wish to read
     @param start Row index of the first element of the row to read
     @return pointer to the extracted FITSform
     
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
    }*/
    
    
    
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
    FITStable::FITStable(const std::shared_ptr<fitsfile>& fits,
                        const int& iHDU):ftbl_type(ttype::tbinary),hdu(FITShdu())
    {
        int tbl_status = 0;

        if(fits == nullptr || fits.use_count() == 0)
        {
            tbl_status = FILE_NOT_OPENED;
            throw FITSexception(tbl_status,"FITStable","ctor","Input fitsfile pointer is null.");
        }

        int hdu_type = ANY_HDU;
        int fhdu_num = 0;        
        
        //Get current HDU position
        if(fits_get_hdu_num(fits.get(), &fhdu_num))
            throw FITSexception(SEEK_ERROR, "FITStable","ctor");

        // Move to the desired HDU if it isn't the current HDU
        if(fhdu_num != iHDU)
        {
            if(fits_movabs_hdu(fits.get(), fhdu_num, &hdu_type, &tbl_status))
                throw FITSexception(tbl_status, "FITStable","ctor");
        }
        
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

        FITShdu tmp(fits);
        hdu.swap(tmp);

        load(fits);
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
    FITStable::FITStable(const std::shared_ptr<fitsfile>& fits,
                         const std::string& extname):ftbl_type(ttype::tbinary),hdu(FITShdu())
    {
        int tbl_status = 0;

        if(fits == nullptr || fits.use_count() == 0)
        {
            tbl_status = FILE_NOT_OPENED;
            throw FITSexception(tbl_status,"FITStable","ctor","Input fitsfile pointer is null.");
        }

        int hdu_type = ANY_HDU;

        //Move to desired HDU block
        if(fits_movnam_hdu(fits.get(), hdu_type, const_cast<char*>(extname.c_str()), 0, &tbl_status))
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

        FITShdu tmp(fits);
        hdu.swap(tmp);
        
        load(fits);
    }
    /**
     * @brief destructor
     * @note C++11 : Take care to properly close the fitsfile if it isn't use anymore by other instance.
     * @note Non C++11 : the pointer to the fitsfile isn't freed and it is the responsibility to the user to close the file and free that pointer properly.
     **/
    FITStable::~FITStable()
    {
        for(columns_list::iterator n = fcolumns.begin(); n != fcolumns.end(); n++)
        {
            n->reset();
        }
        fcolumns.clear();
    }
    
#pragma region -- Load table

    /**
     @brief List all column in the fitsfile.
     @details Retrive column name and column datatype of all columns of the FITS table.
     */
    void FITStable::load(const std::shared_ptr<fitsfile>& fptr)
    {
        int tbl_status = 0;

        if(fptr == nullptr || fptr.use_count() == 0)
        {
            tbl_status = FILE_NOT_OPENED;
            throw FITSexception(tbl_status,"FITStable","load","Input fitsfile pointer is null.");
        }

        int ncols =0 ;

        if(ffgncl(fptr.get(), &ncols, &tbl_status))
            throw FITSexception(tbl_status,"FITStable","getNcols");
        
        columns_list list;
        
        for(size_t n = 1; n <= ncols; n++)
        {
            list.push_back(readColumn(fptr,n,0));
        }
    }

    /** 
     * 
     */
    std::unique_ptr<FITSform> FITStable::readColumn(const std::shared_ptr<fitsfile>& fptr, const size_t& n, const size_t& start)
    {
        int tbl_status = 0;

        if(fptr == nullptr || fptr.use_count() == 0)
        {
            tbl_status = FILE_NOT_OPENED;
            throw FITSexception(tbl_status,"FITStable","readColumn","Input fitsfile pointer is null.");
        }

        //1- Retrive column propertie for the desired column
        //1.1- Retrive column name

        char TFIELD[100];
        int colnum = 0;

        if(ffgcnn(fptr.get(), CASEINSEN, const_cast<char*>((std::to_string(n)).c_str()), TFIELD, &colnum, &tbl_status))
            throw FITSexception(tbl_status,"FITStable","readColumn");
        
        //1.2- Retrive column data type
        int data_type = 0;
        LONGLONG trepeat;
        LONGLONG twidth;

        if(ffeqtyll(fptr.get(), colnum, &data_type, &trepeat, &twidth, &tbl_status))
            throw FITSexception(tbl_status,"FITStable","readColumn");

        //1.3- Retrive other column properties
        long   tbcol = 0;
        char   tunit[100];
        double tscale = 0;
        double tzero  = 0;

        switch (ftbl_type)
        {
            case tascii:
                if(ffgacl(fptr.get(), colnum, NULL, &tbcol, tunit, NULL, &tscale, &tzero, NULL, NULL, &tbl_status))
                    throw FITSexception(tbl_status,"FITStable","listColumns");
                break;
                
            case tbinary:
                if(ffgbcl(fptr.get(), colnum, NULL, tunit, NULL, NULL, &tscale, &tzero, NULL, NULL, &tbl_status))
                    throw FITSexception(tbl_status,"FITStable","listColumns");
                break;
                
            default:
                throw FITSwarning("FITStable","listColumns","Current header neither a BINARY nor an ASCII table.");
                break;
        }

        //2- Create FITSform instance that embed all column properties
        if(trepeat <= 1)
        {
            switch(data_type)
            {
                case tshort:
                case tsbyte:
                    {
                        FITScolumn<int8_t>* tform = new FITScolumn<int8_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<int8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tushort:
                case tbyte:
                    {
                        FITScolumn<uint8_t>* tform = new FITScolumn<uint8_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<uint8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tint:
                    {
                        FITScolumn<int16_t>* tform = new FITScolumn<int16_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<int16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tuint:
                    {
                        FITScolumn<uint16_t>* tform = new FITScolumn<uint16_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<uint16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlong:
                    {
                        FITScolumn<int32_t>* tform = new FITScolumn<int32_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<int32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tulong:
                    {
                        FITScolumn<uint32_t>* tform = new FITScolumn<uint32_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<uint32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlonglong:
                    {
                        FITScolumn<uint64_t>* tform = new FITScolumn<uint64_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<uint64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tfloat:
                    {
                        FITScolumn<float>* tform = new FITScolumn<float>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<float>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdouble:
                    {
                        FITScolumn<double>* tform = new FITScolumn<double>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<double>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tstring:
                    {
                        FITScolumn<std::string>* tform = new FITScolumn<std::string>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<std::string>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                  
                case tbit:
                case tlogical:
                    {
                        FITScolumn<char*>* tform = new FITScolumn<char*>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<char*>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tcplx:
                    {
                        FITScolumn<FITSform::complex>* tform = new FITScolumn<FITSform::complex>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<FITSform::complex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdbcplx:
                    {
                        FITScolumn<FITSform::dblcomplex>* tform = new FITScolumn<FITSform::dblcomplex>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        read<FITSform::dblcomplex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                default:
                    break;
            }
        }
         else
        {
            switch(data_type)
            {
                case tsbyte:
                case tshort:
                    {
                        FITScolumn< FITSform::int8Vector >* tform = new FITScolumn<FITSform::int8Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<int8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tbyte:
                case tushort:
                    {
                        FITScolumn< FITSform::uint8Vector >* tform = new FITScolumn<FITSform::uint8Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<uint8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tint:
                    {
                        FITScolumn< FITSform::int16Vector >* tform = new FITScolumn<FITSform::int16Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<int16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tuint:
                    {
                        FITScolumn< FITSform::uint16Vector >* tform = new FITScolumn<FITSform::uint16Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<uint16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlong:
                    {
                        FITScolumn< FITSform::int32Vector >* tform = new FITScolumn<FITSform::int32Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<int32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tulong:
                    {
                        FITScolumn< FITSform::uint32Vector >* tform = new FITScolumn<FITSform::uint32Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<uint32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlonglong:
                    {
                        FITScolumn< FITSform::int64Vector >* tform = new FITScolumn<FITSform::int64Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<int64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tulonglong:
                    {
                        FITScolumn< FITSform::uint64Vector >* tform = new FITScolumn<FITSform::uint64Vector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<uint64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tfloat:
                    {
                        FITScolumn< FITSform::floatVector >* tform = new FITScolumn<FITSform::floatVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<float>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdouble:
                    {
                        FITScolumn< FITSform::doubleVector >* tform = new FITScolumn<FITSform::doubleVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<double>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tstring:
                    {
                        if(trepeat / twidth <= 1)
                        {
                            FITScolumn< std::string >* tform = new FITScolumn<std::string>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                            read<std::string>(tform, fptr, start);
                            return std::unique_ptr<FITSform>(tform);
                        }
                        else
                        {
                            FITScolumn< FITSform::stringVector >* tform = new FITScolumn<FITSform::stringVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                            readVector<std::string>(tform, fptr, start);
                            return std::unique_ptr<FITSform>(tform);
                        }
                    }
                  
                case tbit:
                case tlogical:
                    {
                        FITScolumn< FITSform::charVector >* tform = new FITScolumn<FITSform::charVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<char*>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tcplx:
                    {
                        FITScolumn< FITSform::complexVector >* tform = new FITScolumn<FITSform::complexVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<FITSform::complex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdbcplx:
                    {
                        FITScolumn< FITSform::dblcomplexVector >* tform = new FITScolumn<FITSform::dblcomplexVector>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),start);
                        readVector<FITSform::dblcomplex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                default:
                    break;
            }
        }
        
        return nullptr;
    }
#pragma endregion
    
#pragma mark * Properties
    
    /**
     @brief Get the number of rows.
     @return The number of rows in each columns
     */
    size_t FITStable::nrows() const
    {
        if(fcolumns.empty()) return 0;
        return fcolumns.front()->size();
    }

    /**
     @brief Get the number of rows.
     @return The number of rows in each columns
     */
    size_t FITStable::ncols() const
    {
        if(fcolumns.empty()) return 0;
        return fcolumns.size();
    }
    
    /**
     @brief List all column.
     @details Retrive column name and column datatype of all columns of the FITS table.
     @return The list of column with their index, name and embeded datatype.
     */
    const FITStable::clist FITStable::listColumns()
    {      
        std::vector< std::vector<std::string> > list;
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            std::vector<std::string> tform({n->get()->getName(),n->get()->getTTYPE(),n->get()->getUnit()});
            list.push_back(tform);
        }
        
        return list;
    }
    
    
    /*
     @brief Get FITS column properties
     @details Search FITStable header block for a specific column and retrive its intrinsec properties
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @return container that embed the column's properties
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
    
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
    */
    
    
    /**
     @brief Get FITS column properties
     @details Search FITStable header block for a specific column and retrive its intrinsec properties
     @param size_t& Non null unsigned inter number of the desired column
     @return container that embed the column's properties
    
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
    */
    
#pragma mark * Accessing coulumn data
    /*
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @return container that embed the column's properties and data
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
    FITSform* FITStable::readColumn(const std::string& name)
    {
        return readColumn(name, 1);
    }
    */
    
    
    /*
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param colnum non-null usinged integer number of the column of interest
     @return container that embed the column's properties and data
    
    FITSform* FITStable::readColumn(const size_t& colnum)
    {
        return readColumn(colnum, 1);
    }
    */
    
    /*
     @brief Get part of a FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param name name of the column of interest, defined by the TTYPEn fits keyword.
     @param start starting row one which to extract the data
     @return container that embed the column's properties and data
     @note The input name may be either the correct name of the searched column or, it may contains wild card caracthers (*,? or #) or the non-null unsigned integer number of the desired column. Wild card caracters allows to provide REGEX pattern to search for the first column with TTYPEn that match the REGEX. The '*' match any sequence of caraters while '?' matches any single caracter. The '#' will matches any consecutive string of decimal digit (0-9). If more than one matches is found, the first match is returned and \c this->tbl_status is set to \c COL_NOT_UNIQUE.
    
    FITSform* FITStable::readColumn(const std::string& name, const size_t& start)
    {
        //1- Get columnProperties for the column of interest
        FITSform form = columnProperties(name);
       
        //2- Retrive data
        return readColumn(form.getPosition(), start);
    }
    */
    
    /**
     @brief Get FITS column
     @details Search FITStable header block for a specific column and retrive the entire column
     @param colnum non-null usinged integer number of the column of interest
     @param start starting row one which to extract the data
     @return container that embed the column's properties and data
    
    FITSform* FITStable::readColumn(const size_t& column, const size_t& start)
    {
        //1- Get columnProperties for the column of interest
        FITSform form = columnProperties(column);
        
        return readArray(form, start);
    }
    */
    
#pragma mark * Inserting/Updating data to column
#pragma mark 1- Inseting new column
    
    
    /**
     @brief Append a new column into the FITS file
     @details Create a new empty column into the Fits file. If the column already exist, the column will not be created.

     @param cname The name of the new collumn
     @param dataType The datatype of the new column
     */
    void FITStable::InsertColumn(const std::string& cname, const dtype& type, const std::string& tunit)
    {
        size_t pos = fcolumns.back()->getPosition()+1;
        size_t nrows = fcolumns.front()->size();

        switch(type)
        {
            case tshort:
            case tsbyte:
                {
                    FITScolumn<int8_t>* col = new FITScolumn<int8_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int8_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tbyte:
            case tushort:
                {
                    FITScolumn<uint8_t>* col = new FITScolumn<uint8_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint8_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tint:
                {
                    FITScolumn<int16_t>* col = new FITScolumn<int16_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int16_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tuint:
                {
                    FITScolumn<uint16_t>* col = new FITScolumn<uint16_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint16_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tlong:
                {
                    FITScolumn<int32_t>* col = new FITScolumn<int32_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int32_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tulong:
                {
                    FITScolumn<uint32_t>* col = new FITScolumn<uint32_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint32_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tlonglong:
                {
                    FITScolumn<int64_t>* col = new FITScolumn<int64_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int64_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tulonglong:
                {
                    FITScolumn<uint64_t>* col = new FITScolumn<uint64_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint64_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tfloat:
                {
                    FITScolumn<float>* col = new FITScolumn<float>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(float(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tdouble:
                {
                    FITScolumn<double>* col = new FITScolumn<double>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(double(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tstring:
                {
                    FITScolumn<std::string>* col = new FITScolumn<std::string>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(std::string("NULL"));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tbit:
            case tlogical:
                {
                    FITScolumn<char*>* col = new FITScolumn<char*>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(const_cast<char*>("F"));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tcplx:
                {
                    FITScolumn<FITSform::complex>* col = new FITScolumn<FITSform::complex>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(FITSform::complex(0.0f,0.0f));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tdbcplx:
                {
                    FITScolumn<FITSform::dblcomplex>* col = new FITScolumn<FITSform::dblcomplex>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(FITSform::dblcomplex(0.0,0.0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }
                
            default:
                throw FITSexception(BAD_TFORM_DTYPE,"FITStable","InsertColumn","Unsupported data type for the new column.");
        }

        
        return;
    }
    
    /**
     @brief Append a new column into the FITS file
     @details Insert new column into the FITS file and fill the column rows with the corrsponding value.
     @note Note that if the column already exist, it will reaise an exception.
     
     @param col The column that will be inserted into the table.
     */
    void FITStable::InsertColumn( std::shared_ptr<FITSform> col )
    {
        size_t nrows = fcolumns.front()->size();
        if(col->size() != nrows)
        {
            throw FITSexception(BAD_DIMEN,"FITStable","InsertColumn","The number of rows in the new column does not match the number of rows in the table.");
        }

        col->setPosition(fcolumns.size()+1);
        fcolumns.push_back(col->clone());

    }
    
#pragma mark 2- Inseting value to an existing column
    
    /**
     @brief Write FITS columns to FITS file
     @details Write the content of the FITScolumn to the FITS ASCII or BINARY table referenced by \c this.
     @param tform The data to be written
     @param start The row index to which one whish to start writting data
     */
    void FITStable::writeArray(const std::shared_ptr<fitsfile>& fptr, const int64_t& start)
    {
        int tbl_status = 0;
        
        if(start < 1 || start > static_cast<int64_t>(nrows()+1))
        {
            tbl_status  = BAD_ROW_NUM;
            throw FITSexception(tbl_status,"FITStable","writeArray");
        }
        
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            (*n)->write(fptr, start);
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
        
        out<<"\033[31m --- "<< (hdu.Exists("EXTNAME")? hdu.GetValueForKey("EXTNAME") : "NO NAME")<<" --- \033[0m"<<std::endl;
        
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
        
        out<<"\033[31m   |- N ROWS   :\033[0m"<<nrows()<<std::endl
           <<"\033[31m   |- N COLS   :\033[0m"<<ncols()<<std::endl;
        
        for(columns_list::const_iterator it = fcolumns.cbegin(); it != fcolumns.cend(); it++)
        {
            
            out<<"\033[31m   |- COL #\033[0m"<<(*it)->getPosition()<<std::endl
               <<"\033[31m   |   |- NAME  : \033[0m"<<(*it)->getName()<<std::endl
               <<"\033[31m   |   |- UNIT  : \033[0m"<<(*it)->getUnit()<<std::endl
               <<"\033[31m   |   |- TYPE  : \033[0m"<<FITSform::getDataType((*it)->getType())<<std::endl
               <<"\033[31m   |   |- SCALE : \033[0m"<<(*it)->getScale()<<std::endl;
            
            if((*it)->getNelem() > 1)
                out<<"\033[31m   |   |- NELEM : \033[0m"<<(*it)->getNelem()<<std::endl;
            if((*it)->getWidth() > 0)
                out<<"\033[31m   |   |- WIDTH : \033[0m"<<(*it)->getWidth()<<" bytes/elmts"<<std::endl;
            
            out<<"\033[31m   |   `- ZERO  : \033[0m"<<(*it)->getZero()<<std::endl;
        }
        
        out<<"\033[31m   `- \033[34mDONE\033[0m"<<std::endl;
        
    }
}


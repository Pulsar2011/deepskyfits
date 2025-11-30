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
#include <DSTfits/FITSdata.h>

namespace DSL
{
    
#pragma region - FITSform class implementation

#pragma region -- ctor/dtor
    
    FITSform::FITSform(const size_t& p,const std::string& name, const dtype& t, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(1),fwidth(1),fpos(p)
    {initWithType();}
    
    FITSform::FITSform(const size_t& p, const std::string& name, const dtype& t, const double& s, const double& z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(1),fwidth(1),fpos(p)
    {initWithType();}
    
    FITSform::FITSform(const size_t& p,const std::string& name, const dtype& t, const int64_t& r, const int64_t& w, const std::string unit):fname(name),ftype(t),funit(unit),fscale(1),fzero(0),frepeat(r),fwidth(w),fpos(p)
    {initWithType();}
    
    FITSform::FITSform(const size_t& p, const std::string& name, const dtype& t, const int64_t& r, const int64_t& w, const double& s, const double& z, const std::string unit):fname(name),ftype(t),funit(unit),fscale(s),fzero(z),frepeat(r),fwidth(w),fpos(p)
    {initWithType();}
    
    
    FITSform::FITSform(const FITSform& col):fname(col.fname),ftype(col.ftype),funit(col.funit),fscale(col.fscale),fzero(col.fzero),frepeat(col.frepeat),fwidth(col.fwidth),fpos(col.fpos)
    {};
    
#pragma endregion

#pragma region -- static member function
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
        ss += std::to_string(getNelem());
        
        switch(ftype)
        {
            case tsbyte:
                ss += "S";
                fzero=-128;
                fscale=1.0;
                break;
                
            case tshort:
                ss += "I";
                break;
                
            case tushort:
                ss += "U";
                fzero=32768;
                fscale=1.0;
                break;

            case tlong:
            case tint:
                ss += "J";
                break;
            
            case tulong:
            case tuint:
                ss += "V";
                fzero=2147483648;
                fscale=1.0;
                break;
                
            case tlonglong:
                ss += "K";
                break;
            
            case tulonglong:
                ss += "W";
                fzero = static_cast<double>(1ULL << 63);
                fscale=1.0;
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
                    ss += std::to_string(getWidth());
                    ss += "A";
                }
                else
                {
                    ss += std::to_string(getNelem()*getWidth());
                    ss += "A";
                    ss += std::to_string(getWidth());
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
 
    
#pragma endregion

#pragma region -- modifier
    void FITSform::Dump( std::ostream& out) const
    {
        out<<"\033[32m   |- COL #\033[0m "<<fpos<<std::endl
           <<"\033[32m   |   |- NAME  : \033[0m"<<fname<<std::endl
           <<"\033[32m   |   |- UNIT  : \033[0m"<<funit<<std::endl
           <<"\033[32m   |   |- TYPE  : \033[0m"<<getDataType(ftype)<<std::endl;
        
        if(frepeat > 1)
            out<<"\033[32m   |   |- NELEM : \033[0m"<<frepeat<<std::endl;
        if(fwidth > 0)
            out<<"\033[32m   |   |- WIDTH : \033[0m"<<fwidth<<" bytes/elmts"<<std::endl;
        
        out<<"\033[32m   |   |- SCALE : \033[0m"<<fscale<<std::endl
           <<"\033[32m   |   `- ZERO  : \033[0m"<<fzero<<std::endl;
        
        return;
    }
    
    
#pragma endregion
#pragma endregion

#pragma region - FITStable class implementation

#pragma region -- template specialization
    
    template< >
    void FITStable::read(FITScolumn<FITSform::complex>* data,  const std::shared_ptr<fitsfile>& fptr,  const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complex>");

        nrows_cache = static_cast<size_t>(nrows);
        

        int64_t nelem = (static_cast<int64_t>(nrows)-(row-1))*2;
        
        float   *array  = new float [nelem];
        char *nullarray = new char  [nelem];
        int   anynull   = 0;

        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, &anynull, &tbl_status))
        {
            delete [] array;
            nullarray = NULL;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(nullarray[k] && anynull)
                data->push_back(FITSform::complex(std::numeric_limits<float>::quiet_NaN() ,std::numeric_limits<float>::quiet_NaN()));
            else
                data->push_back(FITSform::complex(array[k],array[k+1]));
                
        }
        
        delete [] array;
        nullarray = NULL;
        
        return;
        
    }
    
    template< >
    void FITStable::read(FITScolumn<FITSform::dblcomplex>* data,  const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complex>");

        nrows_cache = static_cast<size_t>(nrows);

        int64_t nelem = (static_cast<int64_t>(nrows)-(row-1))*2;
        
        double   *array = new double [nelem];
        char *nullarray = new char   [nelem];
        int   anynull   = 0;
        
        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, &anynull, &tbl_status))
        {
            delete [] array;
            nullarray = NULL;
            
            throw FITSexception(tbl_status,"FITStable","read<T>");
        }
        
        for(size_t k = 0; k < static_cast<size_t>(nelem); k+=2)
        {
            if(nullarray[k] && anynull)
                data->push_back(FITSform::dblcomplex(std::numeric_limits<double>::quiet_NaN() ,std::numeric_limits<double>::quiet_NaN()));
            else
                data->push_back(FITSform::dblcomplex(array[k],array[k+1]));
        }
        
        delete [] array;
        nullarray = NULL;        
    }
    
    template< >
    void FITStable::read( FITScolumn<std::string>* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<std::vector<std::string>>");

        nrows_cache = static_cast<size_t>(nrows);

        const int64_t nrow   = (static_cast<int64_t>(nrows) - (row-1));
       // const int64_t width  = static_cast<int64_t>(data->getWidth()); // chars per string

        // Read each row independently
        // Allocate array of pointers (one per string in this row)
        std::vector<char*> ptrs(static_cast<size_t>(nrow), nullptr);
        std::vector<std::unique_ptr<char[]>> storage;
        storage.reserve(static_cast<size_t>(nrow));

        for(int64_t s = 0; s < nrow; ++s)
        {
            auto buf = std::make_unique<char[]>(static_cast<size_t>(nrow + 1));
            buf[nrow] = '\0';
            ptrs[static_cast<size_t>(s)] = buf.get();
            storage.emplace_back(std::move(buf));
        }

        // Read row r+row from FITS (startcol=1 to read all elements of the vector cell)
        if(ffgcvs(fptr.get(),
                  static_cast<int>(data->getPosition()),
                  static_cast<LONGLONG>(row), /* firstrow */
                  1,                              /* firstelem */
                  nrow,                             /* nelem rows to read: 1 row */
                  nullptr,                        /* nulval */
                  ptrs.data(),                    /* array of char* (nstr entries) */
                  nullptr,                        /* anynull */
                  &tbl_status))
        {
            throw FITSexception(tbl_status,"FITStable","read<std::vector<std::string>>");
        }

        // Build the vector for this row
        for(int64_t s = 0; s < nrow; ++s)
        {
            std::string this_str(ptrs[static_cast<size_t>(s)]);
            auto end = this_str.find_last_not_of(' ');
            if(end != std::string::npos)
                this_str.erase(end + 1);
            else
                this_str.clear();

            while(this_str.find_last_of('\0') != std::string::npos)
                this_str.erase(this_str.find_last_of('\0'));

            data->push_back(std::move(this_str));
        }
    }

    template< >
    void FITStable::readVector(FITScolumn< FITSform::complexVector> * data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complexVector>");

        nrows_cache = static_cast<size_t>(nrows);
        int64_t nelem = ( static_cast<int64_t>(nrows)-static_cast<int64_t>(row-1) )* static_cast<int64_t>(data->getNelem())*2;
        
        float* array = new float[nelem];
        char* nullarray = new char[nelem];
        int   anynull   = 0;

        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, &anynull, &tbl_status))
        {
            delete[] nullarray;
            delete[] array;
            
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complexVector>");
        }
        for(int64_t k = 0; k < nelem; k += data->getNelem()*2)
        {
            FITSform::complexVector tmpv = FITSform::complexVector(data->getNelem());
            
            for(int64_t l = 0; l < data->getNelem(); l++ )
            {
                tmpv[l] = FITSform::complex(array[k+2*l], array[k+2*l+1]);
            }
            
            data->push_back(tmpv);
        }
        delete[] array;
        delete[] nullarray;
        return;
    }

    template< >
    void FITStable::readVector(FITScolumn< FITSform::dblcomplexVector> * data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<FITSform::complexVector>");

        nrows_cache = static_cast<size_t>(nrows);
        int64_t nelem = ( static_cast<int64_t>(nrows)-static_cast<int64_t>(row-1) )* static_cast<int64_t>(data->getNelem())*2;
        
        double* array = new double[nelem];
        char* nullarray = new char[nelem];
        int   anynull   = 0;

        if(ffgcf(fptr.get(), static_cast<int>(data->getType()), static_cast<int>(data->getPosition()), row, 1, nelem/2, array, nullarray, &anynull, &tbl_status))
        {
            delete[] nullarray;
            delete[] array;
            
            throw FITSexception(tbl_status,"FITStable","read<FITSform::dblcomplexVector>");
        }
        for(int64_t k = 0; k < nelem; k += data->getNelem()*2)
        {
            FITSform::dblcomplexVector tmpv = FITSform::dblcomplexVector(data->getNelem());
            
            for(int64_t l = 0; l < data->getNelem(); l++ )
            {
                tmpv[l] = FITSform::dblcomplex(array[k+2*l], array[k+2*l+1]);
            }
            
            data->push_back(tmpv);
        }
        delete[] array;
        delete[] nullarray;
        return;
    }
    
    template< >
    void FITStable::readVector(FITScolumn< std::vector<std::string> >* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        long nrows = 0; 
        int tbl_status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &tbl_status);
        if(tbl_status)
            throw FITSexception(tbl_status,"FITStable","read<std::vector<std::string>>");

        nrows_cache = static_cast<size_t>(nrows);

        const int64_t nrow   = (static_cast<int64_t>(nrows) - (row-1));
        //const int64_t nstr   = static_cast<int64_t>(data->getNelem()); // strings per row
        const int64_t width  = static_cast<int64_t>(data->getWidth()); // chars per string

        // Read each row independently
        for(int64_t r = 0; r < nrow; ++r)
        {
            // Allocate array of pointers (one per string in this row)
            std::vector<char*> ptrs(static_cast<size_t>(width), nullptr);
            std::vector<std::unique_ptr<char[]>> storage;
            storage.reserve(static_cast<size_t>(width));

            for(int64_t s = 0; s < width; ++s)
            {
                auto buf = std::make_unique<char[]>(static_cast<size_t>(width + 1));
                buf[width] = '\0';
                ptrs[static_cast<size_t>(s)] = buf.get();
                storage.emplace_back(std::move(buf));
            }

            // Read row r+row from FITS (startcol=1 to read all elements of the vector cell)
            if(ffgcvs(fptr.get(),
                      static_cast<int>(data->getPosition()),
                      static_cast<LONGLONG>(row + r), /* firstrow */
                      1,                              /* firstelem */
                      width,                             /* nelem rows to read: 1 row */
                      nullptr,                        /* nulval */
                      ptrs.data(),                    /* array of char* (nstr entries) */
                      nullptr,                        /* anynull */
                      &tbl_status))
            {
                throw FITSexception(tbl_status,"FITStable","read<std::vector<std::string>>");
            }

            // Build the vector for this row
            std::vector<std::string> out;
            out.reserve(static_cast<size_t>(width));
            for(int64_t s = 0; s < width; ++s)
            {
                std::string this_str(ptrs[static_cast<size_t>(s)],
                                     ptrs[static_cast<size_t>(s)] + static_cast<size_t>(width));
                auto end = this_str.find_last_not_of(' ');
                if(end != std::string::npos)
                    this_str.erase(end + 1);
                else
                    this_str.clear();

                while(this_str.find_last_of('\0') != std::string::npos)
                    this_str.erase(this_str.find_last_of('\0'));

                out.emplace_back(std::move(this_str));
            }

            data->push_back(out);
        }
    }

    template< >
    void FITScolumn< uint32_t >::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
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
                unsigned long array = static_cast<unsigned int>(*it);

                if(ffpcl(fptr.get(),static_cast<int>(getType()), static_cast<int>(getPosition()), n+1, 1, getNelem(),  &array, &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITStable","write<T>");
                }
            }
            n++;
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
        const int cfitsType = static_cast<int>(getType());// getCFITSIOStorageType(); // maps tuint/tulong -> tlong
        for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
        {
            if(row < static_cast<size_t>(first_row-1)) continue;
            for(int64_t i=0;i<nelem;++i) buffer[i] = (i < static_cast<int64_t>(it->size())) ? (*it)[static_cast<size_t>(i)] : uint16_t(0);
            if(ffpcl(fptr.get(), cfitsType, static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
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

        int tbl_status = 0;
        size_t row=0;

        if (getType() == tint)
        {
            int* buffer = new int[nelem];
            
            for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;
                
                for(int64_t i=0;i<nelem;++i)
                    buffer[i] = (i < static_cast<int64_t>(it->size())) ? static_cast<int>((*it)[static_cast<size_t>(i)]) : (long)0;
                
                if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
                { 
                    delete [] buffer;
                    throw FITSexception(tbl_status,"FITScolumn<int32Vector>","write");
                }
            }
            delete [] buffer;
        }
        else if (getType() == tlong)
        {
            long* buffer = new long[nelem];
            
            for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;
                
                for(int64_t i=0;i<nelem;++i)
                    buffer[i] = (i < static_cast<int64_t>(it->size())) ? static_cast<long>((*it)[static_cast<size_t>(i)]) : (long)0;
                
                if(ffpcl(fptr.get(), static_cast<int>(getType()), static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
                { 
                    delete [] buffer;
                    throw FITSexception(tbl_status,"FITScolumn<int32Vector>","write");
                }
            }
            delete [] buffer;
        }
    }

    template<>
    void FITScolumn<FITSform::uint32Vector>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr) throw FITSexception(FILE_NOT_OPENED,"FITScolumn<uint32Vector>","write");
        if(data.empty()) throw FITSexception(NOT_TABLE,"FITScolumn<uint32Vector>","write");
        const int64_t nelem = getNelem();

        int tbl_status = 0;
        size_t row=0;
        const int cfitsType =  static_cast<int>(getType());// getCFITSIOStorageType(); // maps tuint/tulong -> tlong

        if(getType() == tuint)
        {
            unsigned int* buffer = new unsigned int[nelem];
            
            for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;

                for(int64_t i=0;i<nelem;++i)
                    buffer[i] = (i < static_cast<int64_t>(it->size())) ? static_cast<unsigned int>((*it)[static_cast<size_t>(i)]) :  (unsigned int)0;
                if(ffpcl(fptr.get(), cfitsType, static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
                {
                    delete [] buffer;
                    throw FITSexception(tbl_status,"FITScolumn<uint32Vector>","write");
                }
            }
            
            delete [] buffer;
        }
        else if (getType() == tulong)
        {
            unsigned long* buffer = new unsigned long[nelem];
            
            for(auto it=data.cbegin(); it!=data.cend(); ++it,++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;

                for(int64_t i=0;i<nelem;++i)
                    buffer[i] = (i < static_cast<int64_t>(it->size())) ? static_cast<unsigned long>((*it)[static_cast<size_t>(i)]) :  0ul;
                if(ffpcl(fptr.get(), cfitsType, static_cast<int>(getPosition()), row+1, 1, nelem, buffer, &tbl_status))
                {
                    delete [] buffer;
                    throw FITSexception(tbl_status,"FITScolumn<uint32Vector>","write");
                }
            }
            
            delete [] buffer;
        }
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

    // Write a single logical per row ("T"/"F") into an L column
    template<>
    void FITScolumn<bool>::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<char*>","write");
        if(data.size() < 1)
            throw FITSexception(NOT_TABLE,"FITScolumn<char*>","write");

        int tbl_status = 0;
        size_t row = 0;

        if(getType() == tlogical)
        {
            for(col_map::const_iterator it = data.cbegin(); it != data.cend(); ++it, ++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;

                char val = (*it) ? 1 : 0;

                if(ffpcl(fptr.get(),
                         static_cast<int>(getType()),          // should be tlogical
                         static_cast<int>(getPosition()),
                        static_cast<LONGLONG>(row+1),
                        1,                                     // firstelem
                        1,                                     // nelem
                        &val,
                        &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITScolumn<char*>","write");
                }
            }
        }
        else if (getType() == tbit)
        {
            for(col_map::const_iterator it = data.cbegin(); it != data.cend(); ++it, ++row)
            {
                if(row < static_cast<size_t>(first_row-1))
                    continue;

                char val = (*it) ? 1 : 0;

                if(ffpclx(fptr.get(),
                            static_cast<int>(getPosition()),
                            static_cast<LONGLONG>(row+1),
                            1,                                     // firstelem
                            1,                                     // nelem
                            &val,
                            &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITScolumn<char*>","write");
                }
            }
        }
    }

    // Write repeated logicals per row (vector of "T"/"F") into an L column
    template<>
    void FITScolumn< FITSform::boolVector >::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& first_row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITScolumn<std::vector<FITSform::boolVector>","write");
        
            if(data.empty())
            throw FITSexception(NOT_TABLE,"FITScolumn<std::vector<FITSform::boolVector>","write");

        const int64_t nelem = getNelem(); // number of logicals per cell
        int tbl_status = 0;
        size_t row = 0;

        

        for(auto it = data.cbegin(); it != data.cend(); ++it, ++row)
        {
            if(row < static_cast<size_t>(first_row-1))
                    continue;

            if(getType() == tlogical)
            {

                std::vector<char> buffer(static_cast<size_t>(nelem), false);

                for(int64_t i = 0; i < nelem; ++i)
                    buffer[static_cast<size_t>(i)] = (it->at(static_cast<size_t>(i))) ? 1 : 0;

                if(ffpcll(fptr.get(),
                         static_cast<int>(getPosition()),
                         static_cast<LONGLONG>(row+1),
                         1,                                      // firstelem
                         nelem,                                  // nelem
                         buffer.data(),
                         &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITScolumn<std::vector<FITSform::boolVector>","write");
                }
            }
            else if( getType() == tbit )
            {
                std::vector<char> buffer(static_cast<size_t>(nelem), 0x0);

                for(int64_t i = 0; i < nelem; ++i)
                    buffer[static_cast<size_t>(i)] = (it->at(static_cast<size_t>(i))) ? 1 : 0;

                if(ffpclx(fptr.get(),
                         static_cast<int>(getPosition()),
                         static_cast<LONGLONG>(row+1),
                         1,                                      // firstelem
                         nelem,                                  // nelem
                         buffer.data(),
                         &tbl_status))
                {
                    throw FITSexception(tbl_status,"FITScolumn<std::vector<FITSform::boolVector>","write");
                }
            }
        }

    }

    template<>
    void FITStable::read(FITScolumn<bool>* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITStable","read<bool>");

        long nrows = 0;
        int status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &status);
        if(status)
            throw FITSexception(status,"FITStable","read<bool>");

        nrows_cache = static_cast<size_t>(nrows);

        const LONGLONG nrow = static_cast<LONGLONG>(static_cast<int64_t>(nrows) - (static_cast<int64_t>(row) - 1));
        
        if(nrow <= 0)
            return;

        std::unique_ptr<char[]> values(new char[nrow]);

        if (data->getType() == tlogical)
        {
                
            // CFITSIO logical reader returns 'T'/'F' in values; nullarray marks nulls when anynull != 0.
            const char nulval = 'F'; // default for nulls
            int anynul = 0;

            if(ffgcvl(fptr.get(),
                        static_cast<int>(data->getPosition()),
                        static_cast<LONGLONG>(row),   // firstrow
                        static_cast<LONGLONG>(1),     // firstelem
                        static_cast<LONGLONG>(nrow),  // nelem
                        nulval,
                        values.get(),
                        &anynul,
                        &status))
            {
                throw FITSexception(status,"FITStable","read<bool>");
            }

            for(LONGLONG i = 0; i < nrow; ++i)
            {
                // Allocate C-string "T"/"F" per row
                bool out = false;
                out |= (values[static_cast<size_t>(i)] == 0x1 ||  values[static_cast<size_t>(i)] == 'T' || values[static_cast<size_t>(i)] == 't');
                data->push_back(out);
            }

            return;
        }
        else if (data->getType() == tbit)
        {
            for( int64_t irow =0; irow < static_cast<int64_t>(nrows); ++irow )
            {
                if(irow < static_cast<int64_t>(row-1))
                    continue;
            
                if(ffgcx(   fptr.get(),
                            static_cast<int>(data->getPosition()),
                            static_cast<LONGLONG>(irow+1),   // firstrow
                            static_cast<LONGLONG>(1),     // firstelem
                            static_cast<LONGLONG>(1),  // nelem
                            values.get(),
                            &status))
                {
                    throw FITSexception(status,"FITStable","read<bool>");
                }

                // Allocate C-string "T"/"F" per row
                bool out = false;
                out |= (values[0] == 0x1 ||  values[0] == 'T' || values[0] == 't');
                data->push_back(out);
            }
            return;
        }

        throw FITSexception(BAD_TFORM_DTYPE,"FITStable::read<bool>","Unsupported dtype for char* column");

    }

    template<>
    void FITStable::readVector(FITScolumn<FITSform::boolVector>* data, const std::shared_ptr<fitsfile>& fptr, const size_t& row)
    {
        if(!fptr)
            throw FITSexception(FILE_NOT_OPENED,"FITStable","readVector<bool>");

        long nrows = 0;
        int status = 0;
        fits_get_num_rows(fptr.get(), &nrows, &status);
        
        if(status)
            throw FITSexception(status,"FITStable","readVector<bool>");
        
        nrows_cache = static_cast<size_t>(nrows);

        const LONGLONG nrow   = static_cast<LONGLONG>(static_cast<int64_t>(nrows) - (static_cast<int64_t>(row) - 1));
        const LONGLONG nelem  = static_cast<LONGLONG>(data->getNelem());
        
        if(nrow <= 0 || nelem <= 0)
            return;

        std::unique_ptr<char[]> values(new char[nelem]);

        // Read each row's repeated logical elements
        for(LONGLONG r = 0; r < nrow; ++r)
        {
            if(data->getType() == tlogical)
            {
                const char nulval = 'F'; // default for nulls            
                int anynull = 0;
                
                if(ffgcvl(fptr.get(),
                          static_cast<int>(data->getPosition()),
                          static_cast<LONGLONG>(row + r), // firstrow
                          1,                              // firstelem
                          nelem,                          // number of elements in this row's cell
                          nulval,
                          values.get(),
                          &anynull,
                          &status))
                {
                    throw FITSexception(status,"FITStable","readVector<bool>");
                }
            
                FITSform::boolVector out;
                for(LONGLONG e = 0; e < nelem; ++e)
                {
                    bool cstr = false;
                    cstr |=  (values[static_cast<size_t>(e)] == 0x1 ||  values[static_cast<size_t>(e)] == 'T' || values[static_cast<size_t>(e)] == 't');
                    out.push_back(cstr);
                }
                data->push_back(out);
                
            }
            else if (data->getType() == tbit)
            {
                if(ffgcx(fptr.get(),
                         static_cast<int>(data->getPosition()),
                         static_cast<LONGLONG>(row + r), // firstrow
                         1,                              // firstelem
                         nelem,                          // number of elements in this row's cell
                         values.get(),
                         &status))
                {
                    throw FITSexception(status,"FITStable","readVector<bool>");
                }
                
                FITSform::boolVector out;
                out.reserve(static_cast<size_t>(nelem));
                for(LONGLONG e = 0; e < nelem; ++e)
                {
                    bool val = false;
                    val |= (values[static_cast<size_t>(e)] == 0x1 ||  values[static_cast<size_t>(e)] == 'T' || values[static_cast<size_t>(e)] == 't');
                    out.push_back(val);
                }
                data->push_back(out);

            }
        }

    }

#pragma endregion

#pragma region -- ctor/dtor

    /**
     @brief Default Constructor
     @details Construct an empty DSL::FITStable instance.
     */
    FITStable::FITStable():fcolumns(),ftbl_type(ttype::tbinary),hdu(FITShdu()),nrows_cache(0)
    {
        hdu.ValueForKey("XTENSION","BINTABLE",fChar);
        hdu.ValueForKey("BITPIX",(uint16_t) 8,"Number of bits per data pixel");
        hdu.ValueForKey("NAXIS",(uint16_t) 2,"Number of data axes");
    }
    
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
                        const int& iHDU):ftbl_type(ttype::tbinary),hdu(FITShdu()),nrows_cache(0)
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
        if(!fits_get_hdu_num(fits.get(), &fhdu_num))
            throw FITSexception(SEEK_ERROR, "FITStable","ctor ["+std::to_string(__LINE__)+"]");

        // Move to the desired HDU if it isn't the current HDU
        if(fhdu_num != iHDU)
        {
            if(fits_movabs_hdu(fits.get(), fhdu_num, &hdu_type, &tbl_status))
                throw FITSexception(tbl_status, "FITStable","ctor ["+std::to_string(__LINE__)+"]");
        }
        else
        {
            //Register current HDU type
            fits_get_hdu_type(fits.get(), &hdu_type, &tbl_status);

            if(tbl_status)
                throw FITSexception(tbl_status, "FITStable","ctor ["+std::to_string(__LINE__)+"]");
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
            throw FITSexception(tbl_status,"FITStable","ctor","Current HDU isn't a BINARY nor a ASCII FITS table. ["+std::to_string(__LINE__)+"]");
        }

        FITShdu tmp(fits);
        hdu.swap(tmp);

        load(fits,1);
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
                         const std::string& extname):ftbl_type(ttype::tbinary),hdu(FITShdu()),nrows_cache(0)
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
        
        load(fits,1);
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
    
#pragma endregion
#pragma region -- Load table

    /**
     @brief List all column in the fitsfile.
     @details Retrive column name and column datatype of all columns of the FITS table.
     */
    void FITStable::load(const std::shared_ptr<fitsfile>& fptr, const size_t & start)
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
        
        for(size_t n = 1; n <= ncols; n++)
        {
            fcolumns.push_back(readColumn(fptr,n,start));
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
        long   tbcol    = 0;
        char   tunit   [100];
        char   ttype   [100];
        char   TFORM   [100];
        char   dataType[100];
        char   tdisp   [100];
        char   cnull   [100];
        double tscale   = 0;
        double tzero    = 0;
        long   tnull    = 0;
        long ttrepeat   = 0; 

        switch (ftbl_type)
        {
            case tascii:
                if(ffgacl(fptr.get(), colnum, ttype, &tbcol, tunit, TFORM, &tscale, &tzero, cnull, tdisp, &tbl_status))
                    throw FITSexception(tbl_status,"FITStable","listColumns");
                break;
                
            case tbinary:
                if(ffgbcl(fptr.get(), colnum, ttype, tunit, dataType, &ttrepeat, &tscale, &tzero, &tnull, tdisp, &tbl_status))
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
                case tsbyte:
                    {
                        FITScolumn<int8_t>* tform = new FITScolumn<int8_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<int8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tbyte:
                    {
                        FITScolumn<uint8_t>* tform = new FITScolumn<uint8_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<uint8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tshort:
                    {
                        FITScolumn<int16_t>* tform = new FITScolumn<int16_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<int16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tushort:
                    {
                        FITScolumn<uint16_t>* tform = new FITScolumn<uint16_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<uint16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tint:
                case tlong:
                    {
                        FITScolumn<int32_t>* tform = new FITScolumn<int32_t>(std::string(TFIELD),static_cast<dtype>(data_type),std::string(tunit),n);
                        read<int32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }

                case tuint:
                case tulong:
                    {
                        FITScolumn<uint32_t>* tform = new FITScolumn<uint32_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<uint32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlonglong:
                    {
                        FITScolumn<int64_t>* tform = new FITScolumn<int64_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<int64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tulonglong:
                    {
                        FITScolumn<uint64_t>* tform = new FITScolumn<uint64_t>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<uint64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tfloat:
                    {
                        FITScolumn<float>* tform = new FITScolumn<float>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<float>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdouble:
                    {
                        FITScolumn<double>* tform = new FITScolumn<double>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<double>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tstring:
                    {
                        FITScolumn<std::string>* tform = new FITScolumn<std::string>(std::string(TFIELD),static_cast<dtype>(data_type),0,twidth,tscale,tzero,std::string(tunit),n);
                        read<std::string>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                  
                case tbit:
                case tlogical:
                    {
                        FITScolumn<bool>* tform = new FITScolumn<bool>(std::string(TFIELD),static_cast<dtype>(data_type),std::string(tunit),n);
                        read<bool>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tcplx:
                    {
                        FITScolumn<FITSform::complex>* tform = new FITScolumn<FITSform::complex>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
                        read<FITSform::complex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdbcplx:
                    {
                        FITScolumn<FITSform::dblcomplex>* tform = new FITScolumn<FITSform::dblcomplex>(std::string(TFIELD),static_cast<dtype>(data_type),tscale,tzero,std::string(tunit),n);
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
                    {
                        FITScolumn< FITSform::int8Vector >* tform = new FITScolumn<FITSform::int8Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<int8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tbyte:
                    {
                        FITScolumn< FITSform::uint8Vector >* tform = new FITScolumn<FITSform::uint8Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<uint8_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tshort:
                    {
                        FITScolumn< FITSform::int16Vector >* tform = new FITScolumn<FITSform::int16Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<int16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tushort:
                    {
                        FITScolumn< FITSform::uint16Vector >* tform = new FITScolumn<FITSform::uint16Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<uint16_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tint:
                case tlong:
                    {
                        FITScolumn< FITSform::int32Vector >* tform = new FITScolumn<FITSform::int32Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<int32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                
                case tuint:
                case tulong:
                    {
                        FITScolumn< FITSform::uint32Vector >* tform = new FITScolumn<FITSform::uint32Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<uint32_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tlonglong:
                    {
                        FITScolumn< FITSform::int64Vector >* tform = new FITScolumn<FITSform::int64Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<int64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tulonglong:
                    {
                        FITScolumn< FITSform::uint64Vector >* tform = new FITScolumn<FITSform::uint64Vector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<uint64_t>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tfloat:
                    {
                        FITScolumn< FITSform::floatVector >* tform = new FITScolumn<FITSform::floatVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<float>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdouble:
                    {
                        FITScolumn< FITSform::doubleVector >* tform = new FITScolumn<FITSform::doubleVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<double>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tstring:
                    {
                        if(trepeat / twidth <= 1)
                        {
                            FITScolumn< std::string >* tform = new FITScolumn<std::string>(std::string(TFIELD),static_cast<dtype>(data_type),0,twidth,tscale,tzero,std::string(tunit),n);
                            read<std::string>(tform, fptr, start);
                            return std::unique_ptr<FITSform>(tform);
                        }
                        else
                        {
                            FITScolumn< FITSform::stringVector >* tform = new FITScolumn<FITSform::stringVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                            readVector<std::string>(tform, fptr, start);
                            return std::unique_ptr<FITSform>(tform);
                        }
                    }
                  
                case tbit:
                case tlogical:
                    {
                        FITScolumn< FITSform::boolVector >* tform = new FITScolumn<FITSform::boolVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<bool>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tcplx:
                    {
                        FITScolumn< FITSform::complexVector >* tform = new FITScolumn<FITSform::complexVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
                        readVector<FITSform::complex>(tform, fptr, start);
                        return std::unique_ptr<FITSform>(tform);
                    }
                    
                case tdbcplx:
                    {
                        FITScolumn< FITSform::dblcomplexVector >* tform = new FITScolumn<FITSform::dblcomplexVector>(std::string(TFIELD),static_cast<dtype>(data_type),trepeat,twidth,tscale,tzero,std::string(tunit),n);
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

#pragma region -- Accessing data from column
    /**
     @brief Access a specific column by its name.
     @details Retrive a specific column from the FITS table by providing its name.

     @param cname The name of the column to access
     @return A pointer to the requested column
     @throw out_of_range if the requested column name does not exist in the FITS table.
     */
    const std::unique_ptr<FITSform>& FITStable::getColumn(const std::string& cname) const
    {
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            if((*n)->getName() == cname)
                return (*n);
        }
        
        throw std::out_of_range("FITStable::getColumn: Column name '"+cname+"' does not exist in the FITS table.");
    }

    /**
     @brief Access a specific column by its name.
     @details Retrive a specific column from the FITS table by providing its name.

     @param cname The name of the column to access
     @return A pointer to the requested column
     @throw out_of_range if the requested column name does not exist in the FITS table.
     */
    const std::unique_ptr<FITSform>& FITStable::getColumn(const size_t& cindex) const
    {
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            if((*n)->getPosition() == cindex)
                return (*n);
        }
        
        throw std::out_of_range("FITStable::getColumn: Column @ '"+std::to_string(cindex)+"' does not exist in the FITS table.");
    }

#pragma endregion

#pragma region -- Properties
    
    /**
     @brief Get the number of rows.
     @return The number of rows in each columns
     */
    size_t FITStable::nrows() const
    {
        if(fcolumns.empty()) return 0;

        nrows_cache = 0;
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            nrows_cache = (nrows_cache < n->get()->size()) ? n->get()->size() : nrows_cache;
        }

        return nrows_cache;
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
    
#pragma endregion

#pragma region -- Inserting/Updating data to column
#pragma endregion
#pragma region 1- Inseting new column
    
    
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
            case tsbyte:
                {
                    FITScolumn<int8_t>* col = new FITScolumn<int8_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int8_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tbyte:
                {
                    FITScolumn<uint8_t>* col = new FITScolumn<uint8_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint8_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tshort:
                {
                    FITScolumn<int16_t>* col = new FITScolumn<int16_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int16_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }
            
            case tushort:
                {
                    FITScolumn<uint16_t>* col = new FITScolumn<uint16_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(uint16_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tint:
            case tlong:
                {
                    FITScolumn<int32_t>* col = new FITScolumn<int32_t>(cname, type, tunit,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(int32_t(0));

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tuint:
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
                {
                    FITScolumn<bool>* col = new FITScolumn<bool>(cname, type, tunit, pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(0u);

                    fcolumns.push_back(std::unique_ptr<FITSform>(col));
                    break;
                }

            case tlogical:
                {
                    FITScolumn<bool>* col = new FITScolumn<bool>(cname, type, tunit ,pos);
                    for(size_t n = 0; n < nrows; n++)
                        col->push_back(false);

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
        bool isEmpty = false;
        
        isEmpty |= (fcolumns.size() == 0); 

        if(!isEmpty && fcolumns.front()->size() < 1)
            isEmpty = true;

        if(!isEmpty)
        {
            size_t nrows = fcolumns.front()->size();
            if(col->size() != nrows)
            {
                throw FITSexception(BAD_DIMEN,"FITStable","InsertColumn","The number of rows in the new column does not match the   number of rows in the table.");
            }
        }

        col->setPosition(fcolumns.size()+1);
        fcolumns.push_back(col->clone());

    }
    
#pragma endregion
#pragma region 2- Inseting value to an existing column
    
    /**
     @brief Write FITS columns to FITS file
     @details Write the content of the FITScolumn to the FITS ASCII or BINARY table referenced by \c this.
     @param tform The data to be written
     @param start The row index to which one whish to start writting data
     */
    void FITStable::write(const std::shared_ptr<fitsfile>& fptr, const int64_t& start)
    {
        int tbl_status = 0;

        const int tfields = static_cast<int>(ncols());
        // Persistent storage for strings
        std::vector<std::string> name_store;  name_store.reserve(tfields);
        std::vector<std::string> form_store;  form_store.reserve(tfields);
        std::vector<std::string> unit_store;  unit_store.reserve(tfields);

        for(const auto& c : fcolumns)
        {
            name_store.push_back(c->getName());        // TTYPEn
            form_store.push_back(c->getTTYPE());       // TFORMn
            unit_store.push_back(c->getUnit());        // TUNITn (may be empty)
        }

        // Arrays of C pointers referencing stable storage
        std::vector<char*> ttype_arr; ttype_arr.reserve(tfields);
        std::vector<char*> tform_arr; tform_arr.reserve(tfields);
        std::vector<char*> tunit_arr; tunit_arr.reserve(tfields);
        for(int i=0;i<tfields;++i)
        {
            ttype_arr.push_back(const_cast<char*>(name_store[static_cast<size_t>(i)].c_str()));
            tform_arr.push_back(const_cast<char*>(form_store[static_cast<size_t>(i)].c_str()));
            tunit_arr.push_back(unit_store[static_cast<size_t>(i)].empty()
                                ? nullptr
                                : const_cast<char*>(unit_store[static_cast<size_t>(i)].c_str()));
        }

        //1- Move to the corret HDU if it exist in the corresponding FITS file. Create new table otherwize
        if(hdu.Exists("EXTNAME"))
        {
            char extname[81];
            std::strncpy(extname, hdu.GetValueForKey("EXTNAME").c_str(), 80);
            extname[80] = '\0';
            try
            {
                fits_movnam_hdu(fptr.get(), ANY_HDU, extname, 0, &tbl_status);
            }
            catch(...)
            {
                fits_create_tbl(fptr.get(),
                                (ftbl_type == tbinary)?BINARY_TBL:ASCII_TBL,
                                (LONGLONG) 0,
                                tfields,
                                ttype_arr.data(),
                                tform_arr.data(),
                                tunit_arr.data(),
                                extname,
                                &tbl_status);
            }
        }
        else
        {
            fits_create_tbl(fptr.get(),
                            (ftbl_type == tbinary)?BINARY_TBL:ASCII_TBL,
                            (LONGLONG)  0,
                            tfields,
                            ttype_arr.data(),
                            tform_arr.data(),
                            tunit_arr.data(),
                            NULL,
                            &tbl_status);
        }

        int64_t first_row = (start <= 0) ? 1 : start;
        
        if(first_row < 1 || first_row > static_cast<int64_t>(nrows()+1))
        {
            tbl_status  = BAD_ROW_NUM;
            throw FITSexception(tbl_status,"FITStable","writeArray");
        }
        
        for(columns_list::const_iterator n = fcolumns.cbegin(); n != fcolumns.cend(); n++)
        {
            (*n)->write(fptr, first_row);
        }

        return;
    }

    /**
     @brief Write FITS columns to FITS file
     @details Write the content of the FITScolumn to the FITS ASCII or BINARY table referenced by \c this.
     @param tform The data to be written
     @param start The row index to which one whish to start writting data
     */
    void FITStable::write(const std::string& filename, const int64_t& start, bool replace)
    {
        int img_status = 0;
        std::string _filename = filename;

        if(replace)
            _filename.insert(0,"!");
        else if(_filename[0] == '!')
        {
            _filename.erase(0,1);
        }
        
        fitsfile * raw_fptr = nullptr;
        if( fits_create_file(&raw_fptr, (char*) _filename.c_str(), &img_status ) )
        {
            throw FITSexception(img_status,"FITScube","Write","FILE : "+_filename);
        }
        std::shared_ptr<fitsfile> fptr(raw_fptr, [](fitsfile* p){ int status=0; fits_close_file(p, &status); });

        write(fptr, start);
        
        return;
    }
    
#pragma endregion
#pragma region 3- Updating value from an existing column
    
    
#pragma endregion

#pragma region -- Diagnoze
    
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
        
        out<<"\033[32m   |- N ROWS   :\033[0m"<<nrows()<<std::endl
           <<"\033[32m   |- N COLS   :\033[0m"<<ncols()<<std::endl;
        
        for(columns_list::const_iterator it = fcolumns.cbegin(); it != fcolumns.cend(); it++)
        {
            (*it)->Dump(out);
        }
        
        out<<"\033[34m   `- \033[34mDONE\033[0m"<<std::endl;
        
    }

#pragma endregion
#pragma endregion

#pragma region - FITScolumn template specialization
    
    template class FITScolumn< bool >;
    template class FITScolumn< uint32_t>;
    template class FITScolumn< FITSform::complex>;
    template class FITScolumn< FITSform::dblcomplex>;
    template class FITScolumn< std::string >;
    template class FITScolumn< std::vector<std::string> >;
    template class FITScolumn< FITSform::boolVector >;

#pragma endregion

}


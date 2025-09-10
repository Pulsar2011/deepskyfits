//
//  FITSimg.h++
//  
//
//  Created by GILLARD William on 31/03/15.
//	Copyright (c) 2015, All rights reserved
//  CPPM
//

#include <cstdio>
#include <vector>
#include <valarray>
#include <limits>
#include <cstdint>
#include <stdexcept>

#include <fitsio.h>

#include <DSTfits/FITSimg.h>



namespace DSL
{
class FITSmanager;

#pragma mark - FITScube class implementation
    
    bool FITScube::debug = false;

#pragma mark • protected member function
    
    /**
     *  @brief construct NAXIS size std::vector for n dimension
     *
     *  @param ndim  Number of dimension
     *  @param naxes Size of each dimension
     *  @return A std::vector of ndim size for wich each value represents the length of the axis
     */
    std::vector<size_t> FITScube::Build_axis(unsigned int ndim, size_t _iaxis0, va_list argptr)
    {
        std::vector<size_t> axis = {_iaxis0};
        size_t _iaxis;
        
        while(axis.size() < ndim)
        {
            _iaxis = va_arg(argptr, size_t);
            axis.push_back(_iaxis);
        }
        
        return axis;
    }
    
#pragma mark • Initialization

    void FITScube::init()
    {
        hdu = NULL;
        
        //mask = pxMask();
        
        BITPIX  = 0;
        eqBITPIX= 0;
        
        Naxis = std::vector<size_t>();
        name = std::string();
        img_status = 0;
    }
    
#pragma mark • ctor/dtor
    /**
     *  @details Create a new empty FITS cube
     */
    FITScube::FITScube()
    {
        init();
    }

    /**
     *  @details Read current HDU of the fitsfile to extract a 2D images
     *  @param fptr: Pointer to the fitfile
     */
    FITScube::FITScube(const std::shared_ptr<fitsfile>& fptr)
    {
        init();
        
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            img_status = SHARED_NULPTR;
            throw FITSexception(img_status,"FITScube","ctor","received nullptr");
        }
        
        //- GET CURRENT HDU TYPE. RETURN AN ERROR IF IT ISN'T AN IMAGE
        int hdu_type = 0;
        if( fits_get_hdu_type(fptr.get(), &hdu_type, &img_status) )
        {
            throw FITSexception(img_status,"FITScube","ctor");
        }
        
        if(hdu_type != IMAGE_HDU)
        {
            img_status = NOT_IMAGE;
            throw FITSexception(img_status,"FITScube","ctor","Current HDU isn't an FITS image");
        }
        
        hdu = new FITShdu(fptr);
        
        //- GET BASIC INFORMATION RELATED TO THE IMAGE
        //  • GET NUMBER OF AXIS
        uint64_t number_axis = hdu->GetUInt64ValueForKey("NAXIS");
        
        if( number_axis < 1 )
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","Current HDU doesn't contain data");
        }
        
        //  • GET AXIS DIMENSSION
        for(long long unsigned int iAxe = 1; iAxe <= number_axis; iAxe++)
#if __cplusplus < 201103L
            Naxis.push_back(static_cast<size_t>( hdu->GetIntValueForKey("NAXIS"+std::to_string(static_cast<long long>(iAxe)))));
#else
            Naxis.push_back(static_cast<size_t>( hdu->GetUInt64ValueForKey("NAXIS"+std::to_string(iAxe))));
#endif
        
        if(Naxis.size() < 1)
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","PROBLEM READING AXIS.");
        }
        
        //  • GET DATA TYPE
        if( fits_get_img_type(fptr.get(), &BITPIX, &img_status) )
        {
            throw FITSexception(img_status,"FITScube","ctor");
        }
        
        if( fits_get_img_equivtype(fptr.get(), &eqBITPIX, &img_status) )
        {
            throw FITSexception(img_status,"FITScube","ctor");
        }
        
        //  • GET EXTENSION NAME
        name = hdu->GetValueForKey("EXTNAME");
        if(name.size() < 1)
            name += "PRIMARY";
        
        mask = pxMask(static_cast<size_t>( Nelements() ));
        
#if __cplusplus < 201103L
        mask.resize(static_cast<size_t>( Nelements() ),0);
#endif
        
        if( mask.size() == 0 )
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","Current HDU doesn't contain data");
        }
    }
    
    /**
     * @details Copie constructor
     * @param cube: FITS data cube to be copied.
     */
    FITScube::FITScube(const FITScube& cube)
    {
        if(mask.size() > 1)
            mask.~valarray<bool>();
        
        mask = cube.mask;
        
        hdu = new FITShdu(*(cube.hdu));
        
        BITPIX = cube.BITPIX;
        eqBITPIX = cube.eqBITPIX;
        name   = cube.name;
        img_status = 0;
        
        Naxis = std::vector<size_t>(cube.Naxis.size());
        for(size_t n = 0; n < Naxis.size(); n++)
            Naxis[n] = cube.Naxis[n];
    }
    
    /**
     *  @details Destructor
     */
    FITScube::~FITScube()
    {
        if(hdu != NULL)
            delete hdu;
        
#if __cplusplus >= 201103L
        mask.resize(0);
#endif
        Naxis.clear();
        name.clear();
    }
    
    FITScube* FITScube::UByteFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
       
        return UByteFITSimg(axis);
    }
    
    FITScube* FITScube::UByteFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint8_t>(axis);
        return img;
        
    }
    
    FITScube* FITScube::ByteFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
        return ByteFITSimg(axis);
        
    }
    
    FITScube* FITScube::ByteFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int8_t>(axis);
        
        return img;
    }
    
    FITScube* FITScube::UShortFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        return UShortFITSimg(axis);
    }
    
    FITScube* FITScube::UShortFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint16_t>(axis);
        return img;
    }
    
    FITScube* FITScube::ShortFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
        return ShortFITSimg(axis);
    }
    
    FITScube* FITScube::ShortFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int16_t>(axis);
        return img;
    }
    
    FITScube* FITScube::UIntFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        return UIntFITSimg(axis);
    }
    
    FITScube* FITScube::UIntFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint32_t>(axis);

        return img;
    }
    
    FITScube* FITScube::IntFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        return IntFITSimg(axis);
    }
    
    FITScube* FITScube::IntFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int32_t>(axis);

        return img;
    }
    
    FITScube* FITScube::LongFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);

        va_end(argptr);
        
        return LongFITSimg(axis);
    }
    
    FITScube* FITScube::LongFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int32_t>(axis);
        return img;
    }
    
    FITScube* FITScube::ULongFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
        return ULongFITSimg(axis);
    }
    
    FITScube* FITScube::ULongFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint32_t>(axis);
        return img;
    }
    
    FITScube* FITScube::LongLongFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
        return LongLongFITSimg(axis);
    }
    
    FITScube* FITScube::LongLongFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int64_t>(axis);
        return img;
    }
    
    FITScube* FITScube::FloatFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);
        
        va_end(argptr);
        
        return FloatFITSimg(axis);
    }
    
    FITScube* FITScube::FloatFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<float>(axis);
        return img;
    }
    
    FITScube* FITScube::DoubleFITSimg(unsigned int _naxis, size_t _iaxis, ...)
    {
        va_list argptr;
        va_start(argptr,_iaxis);
        
        std::vector<size_t> axis =Build_axis(_naxis, _iaxis, argptr);

        va_end(argptr);
        
        return DoubleFITSimg(axis);
    }
    
    FITScube* FITScube::DoubleFITSimg(std::vector<size_t> axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<double>(axis);
        return img;
    }
    
#pragma mark • accessor
    
    /**
     *  Obtain the number of pixel along one of the dimension of the FITS datacube
     *  @param i: the dimension one are interested in, starting from 1 for the NAXIS1 of the FITS datacube
     *  @return The number of pixel along the ith axis.
     *  @note If i = 0, on get the total number of pixels in the FITS datacube
     */
    unsigned long long FITScube::Size(unsigned int i) const
    {
        unsigned long long size = 0;
        if(i == 0)
            size = Nelements();
        else if(i > Naxis.size())
            size = 0;
        else
            size = Naxis[i-1];
        
        return size;
    }
    
    /**
     *  Get the total dimenssion of the data array.
     *  @return Total number of pixel in the data-cube.
     */
    size_t FITScube::Nelements() const
    {
        size_t nElements = 1;
        for(size_t iAxe = 0; iAxe < Naxis.size(); iAxe++)
        {
            nElements *= Naxis[iAxe];
        }
        
        return nElements;
    }
    
    /**
     *  Obtain the physical coordinate of a pixel
     *  @param size_t pixel index position in the pixel array
     *  @return pixel world coordinate
     */
    std::vector<double> FITScube::WorldCoordinates(size_t k) const
    {
#if __cplusplus >= 199711L
        auto pixels_coo = std::bind( &FITScube::PixelCoordinates, this,std::placeholders::_1);
        return WorldCoordinates(pixels_coo(k));
#else
        return WorldCoordinates(PixelCoordinates(k));
#endif
    }
    
    std::vector<double> FITScube::WorldCoordinates(std::vector<unsigned long long> pixel) const
    {
        std::vector<double> dbl_px = std::vector<double>(pixel.size());
        for(size_t i = 0; i < pixel.size(); i++)
            dbl_px[i] = static_cast<double>(pixel[i]);
        
        std::vector<double> wcs = WorldCoordinates(dbl_px);
        dbl_px.clear();
        
        return wcs;
    }
    
    std::vector<double> FITScube::WorldCoordinates(std::vector<double> pixel) const
    {
        std::vector<double> wcs = std::vector<double>(pixel.size());
        for(size_t i = 0; i < pixel.size(); i++)
        {
#if __cplusplus < 201103L
            double crval = (pHDU().Exists("CRVAL"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(static_cast<long long>(i+1))) : 0.;
            double cdelt = (pHDU().Exists("CDELT"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CDELT"+std::to_string(static_cast<long long>(i+1))) : 1.;
            double crpix = (pHDU().Exists("CRPIX"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CRPIX"+std::to_string(static_cast<long long>(i+1))) : 0.;

#else
            double crval = (pHDU().Exists("CRVAL"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(i+1)) : 0.;
            double cdelt = (pHDU().Exists("CDELT"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CDELT"+std::to_string(i+1)) : 1.;
            double crpix = (pHDU().Exists("CRPIX"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CRPIX"+std::to_string(i+1)) : 0.;
#endif
            
            wcs[i] = (pixel[i] - crpix) * cdelt + crval;
        }
        
        return wcs;
    }
    
    /**
     *  Get the pixel coordinates given the pixel index from the 1D pixel array of the FITS datacube
     *  @param k: 1D pixel index
     *  @return Pixel coordinates on each dimension of the FITS datacube
     */
    std::vector<unsigned long long> FITScube::PixelCoordinates(size_t k) const
    {
        std::vector<unsigned long long> xPixel = std::vector<unsigned long long>(Naxis.size());
        
        unsigned long long size = 1;
        
        for(size_t i = 0; i < Naxis.size(); i++)
        {
            size = 1;
            for(size_t l = 0; l < i; l++)
            {
                size *= static_cast<unsigned long long>( Naxis[l] );
            }
            
            xPixel[i] = static_cast<unsigned long long>( k/size % Naxis[i] );
        }
        
        return xPixel;
    }
    
    /**
     *  Get the pixel coordinates given the pixel coordinates
     *  @param coo: 1D pixel index
     *  @return Pixel coordinates on each dimension of the FITS datacube
     */
    std::vector<double> FITScube::World2Pixel(std::vector<double> coo) const
    {
        std::vector<double> xPixel = std::vector<double>(coo);
        
        for(size_t i = 0; i < xPixel.size(); i++)
        {
            //std::cout<<xPixel[i]<<"  ->  ";
#if __cplusplus < 201103L
            double crval = (pHDU().Exists("CRVAL"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(static_cast<long long>(i+1))) : 0.;
            double cdelt = (pHDU().Exists("CDELT"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CDELT"+std::to_string(static_cast<long long>(i+1))) : 1.;
            double crpix = (pHDU().Exists("CRPIX"+std::to_string(static_cast<long long>(i+1))))? pHDU().GetDoubleValueForKey("CRPIX"+std::to_string(static_cast<long long>(i+1))) : 0.;

#else
            double crval = (pHDU().Exists("CRVAL"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CRVAL"+std::to_string(i+1)) : 0.;
            double cdelt = (pHDU().Exists("CDELT"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CDELT"+std::to_string(i+1)) : 1.;
            double crpix = (pHDU().Exists("CRPIX"+std::to_string(i+1)))? pHDU().GetDoubleValueForKey("CRPIX"+std::to_string(i+1)) : 0.;
#endif
            
            xPixel[i] -= crval;
            xPixel[i] /= cdelt;
            xPixel[i] += crpix;
            
            //std::cout<<xPixel[i]<<std::endl;
        }
        
        return xPixel;
    }
    
    size_t FITScube::PixelIndex(std::vector<unsigned long long> iPx) const
    {
        size_t index = 0;
        size_t arg   = 0;
        
        size_t nDim = ( Naxis.size() <= iPx.size() )? Naxis.size() : iPx.size();
        
        for(size_t naxe = 0; naxe < nDim; naxe++)
        {
            if(iPx[naxe] >= Size(naxe+1))
            {
                //std::cout<<" ["<<naxe<<"]"<<iPx[naxe]<<" is out of bound!"<<std::endl;
                return Nelements();
            }
            
            arg = static_cast<size_t>( iPx[naxe] );
            
            for(size_t iX = 0; iX < naxe; iX++)
                arg *= Naxis[iX];
            
            index += arg;
        }

        return (index < Nelements())? index : Nelements();
    }
    
    size_t FITScube::PixelIndex(std::vector<double> Px) const
    {
        std::vector<unsigned long long> pix_index;
        for(size_t k = 0; k < Px.size(); k++ )
        {
            /*
             if( ( Px[k] + 0.5 ) <= -1*std::numeric_limits<double>::min() )
               return Nelements();
            
            if( (Px[k] - 0.5) >= static_cast<double>( Size(k+1) ) + std::numeric_limits<double>::min() )
                return Nelements();
            */
            
            unsigned long long px = static_cast<unsigned long long>( Px[k] );
            
            if( Px[k] - static_cast<double>(px) >= 0.5)
                px++;
            
            pix_index.push_back(px);
        }

        size_t index = PixelIndex(pix_index);
        
        pix_index.clear();
        
        return index;
    }
    
#pragma mark • I/O
    
    /**
     *  @details Write DataCube to the current HDU of a fits file
     *  @param fptr : Pointer to the HDU where data will be written
     */
    void FITScube::Write(const std::shared_ptr<fitsfile>& fptr)
    {
        if(fptr == nullptr || fptr.use_count() < 1)
        {
            throw FITSexception(NULL_INPUT_PTR,"FITScube","Write","received nullptr");
        }
        
        hdu->Write(fptr);
        WriteDataCube(fptr);
    }
    
    /**
     *  @details Create a new FITS file and write DataCube as primary image array
     *  @param fileName : Path and location of the file to be created.
     *  @param replace  : Overwrite an existing file.
     *  @note If replace is set to true and a file with the given filename and path already exist, the existing file is replaced. To prevent to lose data, the default value of the replace parameters is set to false.
     */
    void FITScube::Write(std::string fileName, bool replace)
    {
        if(replace)
            fileName.insert(0,"!");
        else if(fileName[0] == '!')
        {
            fileName.erase(0,1);
        }
        
        fitsfile * raw_fptr = nullptr;
        if( fits_create_file(&raw_fptr, (char*) fileName.c_str(), &img_status ) )
        {
            throw FITSexception(img_status,"FITScube","Write","FILE : "+fileName);
        }
        std::shared_ptr<fitsfile> fptr(raw_fptr, [](fitsfile* p){ int status=0; fits_close_file(p, &status); });
        
        std::cout<<"\033[31m[FITScube::Write]\033[0m: Create image of "<<Naxis.size()<<" axis (";
        long long int *axis = new long long int [Naxis.size()];
        for(unsigned int i = 0; i < Naxis.size()-1; i++)
        {
            axis[i] = static_cast<long long int>( Naxis[i] );
             std::cout<<axis[i]<<",";
        }
        axis[Naxis.size()-1] = static_cast<long long int>( Naxis[Naxis.size()-1] );
        std::cout<<axis[Naxis.size()-1]<<")"<<std::endl;
        
        if(BITPIX == 0)
        {
            img_status = BAD_BITPIX;
            throw FITSexception(img_status,"FITScube","Write","FILE : "+fileName);
        }
    
        if( fits_create_imgll(fptr.get(), BITPIX, static_cast<int>( Naxis.size() ), axis,  &img_status ) )
        {
            fptr.reset();          
            throw FITSexception(img_status,"FITScube","ctor","FILE : "+fileName);
        }
        
        delete [] axis;
        if(fptr == nullptr || fptr.use_count() < 1)
            return;
        
        Write(fptr);
        
        try
        {
            if( fits_close_file(fptr.get(), &img_status) )
            {
                throw FITSexception(img_status,"FITScube","Close","FILE : "+fileName);
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
        }
        
        fptr.reset();

    }
    
  
    
#pragma mark • Modifier
    
    /**
     *  Modify the pixel length of an axis of a FITS datacube. To be used with care and only when images are croped.
     *
     *  @param n    Identification of the axis
     *  @param size New size, in pixel, of the axis
     */
    void FITScube::SetAxisLength(unsigned long int n, long long size)
    {
        try
        {
            if(n-1 >= Naxis.size())
            {
                img_status = BAD_NAXIS;
                throw FITSexception(img_status,"FITScube","ctor","AXIS SIZE OUT OF SCOPE");
            }
        
            if(size < 0)
            {
                img_status = BAD_NAXIS;
                throw FITSexception(img_status,"FITScube","ctor","AXIS SIZE CAN'T BE NEGATIVE");
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
        
        Naxis[n-1] = size;
#if __cplusplus >= 201103L
        hdu->valueForKey(std::string("NAXIS")+std::to_string(n),size);
#else
        hdu->valueForKey(std::string("NAXIS")+std::to_string(static_cast<long long unsigned int>( n )),size);
#endif
    }
    
    /**
     *  Delete HEADER information related to the last axis. To be used with care and only when images reduced to (n-1)D.
     */
    void FITScube::DeleteLastAxis()
    {
        size_t nAxis = Naxis.size();
        Naxis.resize(Naxis.size()-1);
        
#if __cplusplus >= 201103L
        hdu->deleteKey(std::string("NAXIS")+std::to_string(nAxis));
#else
        hdu->deleteKey(std::string("NAXIS")+std::to_string(static_cast<long long unsigned int>( nAxis )));
#endif
        
        hdu->valueForKey("NAXIS",Naxis.size());
    }
    
    void FITScube::BitPerPixel(int _bit, int eq)
    {
        BITPIX = _bit;
        if(eq == 0)
            eqBITPIX = BITPIX;
        else
            eqBITPIX = eq;
        
        hdu->valueForKey( "BITPIX" , static_cast<int long long>(BITPIX) );
    }
    
    
    void FITScube::SetName(std::string _name)
    {
        if(name.size() > 1)
            name.clear();
        
        name += _name;
        hdu->valueForKey("EXTNAME",name);
    }
    
    /**
     *  @details Check either the pixel at pixel coordinates iPx is masked or note
     *  @param iPx Pixel coordinates of the pixel
     * @return true if the pixel is masked
     */
    bool FITScube::Masked(unsigned long long iPx, ...) const
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(size_t naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        size_t index = PixelIndex(pix_index);
        
        return Masked(index);
        
    }
    
    bool FITScube::Masked(size_t idx) const
    {
        if(idx >= mask.size())
            return true;
        
        return mask[idx];
    }
    
    /**
     *  @details Mask a single pixel
     *  @param iPx coordinates of the pixel to mask
     */
    void FITScube::MaskPixel(unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(size_t naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        unsigned long long index = PixelIndex(pix_index);
        
        if(index >= Nelements())
            return;
        
        mask[index] |= true;
        
    }
    
    /**
     *  @details Mask pixels according to a mask map
     *  @param _map pixel mask map
     *  @Note : Pixel already masked are not unmasked
     */
    void FITScube::MaskPixel(const std::valarray<bool> _m)
    {
        mask |= _m;
    }
    
    /**
     *  @details Unmask a single pixel
     *  @param iPx coordinates of the pixel to unmask
     */
    void FITScube::UnmaskPixel(unsigned long long iPx, ...)
    {
        unsigned long long arg = iPx;
        std::vector<unsigned long long> pix_index;
        pix_index.push_back(arg);
        
        va_list ap;
        va_start(ap, iPx);
        
        for(size_t naxe = 1; naxe < Naxis.size(); naxe++)
        {
            arg = va_arg(ap, unsigned long long);
            
            if(arg == 0)
                break;
            
            pix_index.push_back(arg);
        }
        va_end(ap);
        
        unsigned long long index = PixelIndex(pix_index);
        if(index >= Nelements())
            return;
        
        mask[index] &= false;
        
    }
    
    /**
     *  @details Unmask pixels according to a unmask map
     *  @param _map pixel map of pixel to unmask
     *  @note : The pixel map should be set at 'true' for each pixel that need to be unmasked. The resulting pixel mask will be set as mask &= (!_m)
     */
    void FITScube::UnmaskPixel(const std::valarray<bool> _m)
    {
        mask &= (!_m) ;
    }

    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<uint8_t>::template_init()
    {
        BitPerPixel(8);
        Bscale(1. );
        Bzero (0. );
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<int8_t>::template_init()
    {
        BitPerPixel(8);
        Bscale(static_cast<double>(1));
        Bzero(static_cast<double>(0));
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<uint16_t>::template_init()
    {
        BitPerPixel(16,20);
        Bscale(static_cast<double>(1.));
        Bzero(static_cast<double>(32768.));
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<int16_t>::template_init()
    {
        BitPerPixel(static_cast<int>(16));
        Bscale(1. );
        Bzero (0. );
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<int32_t>::template_init()
    {
        BitPerPixel(32);
        Bscale(1. );
        Bzero (0. );
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<uint32_t>::template_init()
    {
        BitPerPixel(32,40);
        Bscale(static_cast<double>(1.));
        Bzero(static_cast<double>(2147483648.));
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<int64_t>::template_init()
    {
        BitPerPixel(static_cast<int>(64));
        Bscale(1. );
        Bzero (0. );
    }
    
    /**
     * @details Initialize type uint8_t FITS variable.
     */
    template< >
    void FITSimg<float>::template_init()
    {
        std::cout<<"SET BITPIX TO -32"<<std::endl;
        BitPerPixel(static_cast<int>(-32));
        Bscale(1. );
        Bzero (0. );
    }
    
    /**
     * @details Initialize type double FITS variable.
     */
    template< >
    void FITSimg<double>::template_init()
    {
        BitPerPixel(static_cast<int>(-64));
        Bscale(1. );
        Bzero (0. );
        
    }
    
template class FITSimg<uint8_t>;
template class FITSimg<int8_t>;
template class FITSimg<int16_t>;
template class FITSimg<uint16_t>;
template class FITSimg<int32_t>;
template class FITSimg<uint32_t>;
template class FITSimg<int64_t>;
template class FITSimg<float>;
template class FITSimg<double>;
    
}

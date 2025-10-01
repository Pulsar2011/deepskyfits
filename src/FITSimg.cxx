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

#pragma region - FITScube class implementation
    
    bool FITScube::debug = false;

#pragma region • protected member function
    
    /**
     *  @brief construct NAXIS size std::vector for n dimension
     *
     *  @param ndim  Number of dimension
     *  @param naxes Size of each dimension
     *  @return A std::vector of ndim size for wich each value represents the length of the axis
     */
    std::vector<size_t> FITScube::Build_axis(const size_t& ndim, const std::initializer_list<size_t>& _iaxis)
    {
        if(ndim < 1)
            throw std::invalid_argument("FITSimg::Build_axis: ndim must be >= 1");
        
        if(_iaxis.size() < ndim)
            throw std::invalid_argument("FITSimg::Build_axis: number of axis size must be == ndim");
        
        std::vector<size_t> axis;
        axis.reserve(ndim);

        auto it = _iaxis.begin();
        for (size_t i = 0; i < ndim; ++i, ++it)
        {
            if (*it == 0)
                throw std::invalid_argument("FITScube::Build_axis: axis sizes must be >= 1");
            axis.push_back(*it);
        }

        return axis;
    }

#pragma endregion
#pragma region • Initialization

    void FITScube::init()
    {
        hdu=FITShdu();
        
        //mask = pxMask();
        
        BITPIX  = 0;
        eqBITPIX= 0;
        
        Naxis      = std::vector<size_t>();
        name       = std::string();
        img_status = 0;
    }
    
#pragma endregion
#pragma region • ctor/dtor
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
        
        FITShdu tmp(fptr);
        hdu.swap(tmp);
        
        //- GET BASIC INFORMATION RELATED TO THE IMAGE
        //  • GET NUMBER OF AXIS
        uint64_t number_axis = hdu.GetUInt64ValueForKey("NAXIS");
        
        if( number_axis < 1 )
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","Current HDU doesn't contain data");
        }
        
        //  • GET AXIS DIMENSSION
        for(long long unsigned int iAxe = 1; iAxe <= number_axis; iAxe++)
            Naxis.push_back(static_cast<size_t>( hdu.GetUInt64ValueForKey("NAXIS"+std::to_string(iAxe))));
        
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

        if(eqBITPIX == 64)
        {
            unsigned long long bz = 0;
            if(ffgkyujj(fptr.get(), "BZERO", &bz, NULL, &img_status))
            {
                bz=0ULL;
                img_status = 0;
            }
            
            if (bz == 9223372036854775808ULL)
                eqBITPIX = 80;
        }
        
        //  • GET EXTENSION NAME
        name.clear();
        
        if(hdu.Exists("EXTNAME"))
            name = hdu.GetValueForKey("EXTNAME");
        if(name.size() < 1)
            name += "PRIMARY";
        
        mask = pxMask(static_cast<size_t>( Nelements() ));
        
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
        hdu = FITShdu(cube.hdu);
        
        BITPIX   = cube.BITPIX;
        eqBITPIX = cube.eqBITPIX;
        name     = cube.name;
        img_status = 0;
        
        Naxis = std::vector<size_t>(cube.Naxis);
    }
    
    /**
     *  @details Destructor
     */
    FITScube::~FITScube()
    {        
        mask.resize(0);
        Naxis.clear();
        name.clear();
    }
    
    FITScube* FITScube::UByteFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return UByteFITSimg(axis);
    }
    
    FITScube* FITScube::UByteFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint8_t>(axis);
        return img;
        
    }
    
    FITScube* FITScube::ByteFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return ByteFITSimg(axis);
        
    }
    
    FITScube* FITScube::ByteFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int8_t>(axis);
        
        return img;
    }
    
    FITScube* FITScube::UShortFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return UShortFITSimg(axis);
    }
    
    FITScube* FITScube::UShortFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint16_t>(axis);
        return img;
    }
    
    FITScube* FITScube::ShortFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return ShortFITSimg(axis);
    }
    
    FITScube* FITScube::ShortFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int16_t>(axis);
        return img;
    }
    
    FITScube* FITScube::UIntFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return UIntFITSimg(axis);
    }
    
    FITScube* FITScube::UIntFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint32_t>(axis);

        return img;
    }
    
    FITScube* FITScube::IntFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return IntFITSimg(axis);
    }
    
    FITScube* FITScube::IntFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int32_t>(axis);

        return img;
    }
    
    FITScube* FITScube::LongFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return LongFITSimg(axis);
    }
    
    FITScube* FITScube::LongFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int32_t>(axis);
        return img;
    }
    
    FITScube* FITScube::ULongFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return ULongFITSimg(axis);
    }
    
    FITScube* FITScube::ULongFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<uint32_t>(axis);
        return img;
    }
    
    FITScube* FITScube::LongLongFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return LongLongFITSimg(axis);
    }
    
    FITScube* FITScube::LongLongFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<int64_t>(axis);
        return img;
    }
    
    FITScube* FITScube::FloatFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return FloatFITSimg(axis);
    }
    
    FITScube* FITScube::FloatFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<float>(axis);
        return img;
    }
    
    FITScube* FITScube::DoubleFITSimg(const size_t& _naxis, const std::initializer_list<size_t>& _iaxis)
    {       
        std::vector<size_t> axis = Build_axis(_naxis, _iaxis);
        return DoubleFITSimg(axis);
    }
    
    FITScube* FITScube::DoubleFITSimg(const std::vector<size_t>& axis)
    {
        FITScube *img = NULL;
        img = new FITSimg<double>(axis);
        return img;
    }
    
#pragma endregion
#pragma region • accessor        
    /**
     *  Obtain the number of pixel along one of the dimension of the FITS datacube
     *  @param i: the dimension one are interested in, starting from 1 for the NAXIS1 of the FITS datacube
     *  @return The number of pixel along the ith axis.
     *  @note If i = 0, on get the total number of pixels in the FITS datacube
     */
    size_t FITScube::Size(const size_t& i) const
    {
        size_t size = 0;
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
        for(std::vector<size_t>::const_iterator iAxe = Naxis.cbegin(); iAxe != Naxis.cend(); iAxe++)
        {
            nElements *= *(iAxe);
        }
        
        return nElements;
    }
    
    /**
     *  Obtain the physical coordinate of a pixel
     *  @param size_t pixel index position in the pixel array
     *  @return pixel world coordinate
     */
    std::vector<double> FITScube::WorldCoordinates(const size_t& k) const
    {
        auto pixels_coo = std::bind( &FITScube::PixelCoordinates, this,std::placeholders::_1);
        return WorldCoordinates(pixels_coo(k));
    }
    
    std::vector<double> FITScube::WorldCoordinates(const std::vector<size_t>& pixel) const
    {
        std::vector<double> dbl_px = std::vector<double>(pixel.size());
        for(size_t i = 0; i < pixel.size(); i++)
            dbl_px[i] = static_cast<double>(pixel[i]);
        
        std::vector<double> wcs = WorldCoordinates(dbl_px);
        dbl_px.clear();
        
        return wcs;
    }
    
    std::vector<double> FITScube::WorldCoordinates(const std::vector<double>& pixel) const
    {
        std::vector<double> wcs = std::vector<double>(pixel.size());
        for(size_t i = 0; i < pixel.size(); i++)
        {
            double crval = (hdu.Exists("CRVAL"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CRVAL"+std::to_string(i+1)) : 0.;
            double cdelt = (hdu.Exists("CDELT"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CDELT"+std::to_string(i+1)) : 1.;
            double crpix = (hdu.Exists("CRPIX"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CRPIX"+std::to_string(i+1)) : 0.;

            wcs[i] = (pixel[i] - crpix) * cdelt + crval;
        }
        
        return wcs;
    }
    
    /**
     *  Get the pixel coordinates given the pixel index from the 1D pixel array of the FITS datacube
     *  @param k: 1D pixel index
     *  @return Pixel coordinates on each dimension of the FITS datacube
     */
    std::vector<size_t> FITScube::PixelCoordinates(const size_t& k) const
    {
        std::vector<size_t> xPixel = std::vector<size_t>(Naxis.size());
        
        size_t size = 1;
        
        for(size_t i = 0; i < Naxis.size(); i++)
        {
            size = 1;
            for(size_t l = 0; l < i; l++)
            {
                size *=  Naxis[l] ;
            }
            
            xPixel[i] =  k/size % Naxis[i] ;
        }
        
        return xPixel;
    }
    
    /**
     *  Get the pixel coordinates given the pixel coordinates
     *  @param coo: 1D pixel index
     *  @return Pixel coordinates on each dimension of the FITS datacube
     */
    std::vector<double> FITScube::World2Pixel(const std::vector<double>& coo) const
    {
        std::vector<double> xPixel = std::vector<double>(coo);
        
        for(size_t i = 0; i < xPixel.size(); i++)
        {
            //std::cout<<xPixel[i]<<"  ->  ";
            double crval = (hdu.Exists("CRVAL"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CRVAL"+std::to_string(i+1)) : 0.;
            double cdelt = (hdu.Exists("CDELT"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CDELT"+std::to_string(i+1)) : 1.;
            double crpix = (hdu.Exists("CRPIX"+std::to_string(i+1)))? hdu.GetDoubleValueForKey("CRPIX"+std::to_string(i+1)) : 0.;
            
            xPixel[i] -= crval;
            xPixel[i] /= cdelt;
            xPixel[i] += crpix;
            
            //std::cout<<xPixel[i]<<std::endl;
        }
        
        return xPixel;
    }
    
    size_t FITScube::PixelIndex(const std::vector<size_t>& iPx) const
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
            
            arg = iPx[naxe];
            
            for(size_t iX = 0; iX < naxe; iX++)
                arg *= Naxis[iX];
            
            index += arg;
        }

        return (index < Nelements())? index : Nelements();
    }
    
    size_t FITScube::PixelIndex(const std::vector<double>& Px) const
    {
        std::vector<size_t> pix_index;
        for(size_t k = 0; k < Px.size(); k++ )
        {            
            pix_index.push_back(static_cast<size_t>( Px[k]+0.5 ));
        }

        size_t index = PixelIndex(pix_index);
        pix_index.clear();
        
        return index;
    }
    
#pragma endregion
#pragma region • I/O
    
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
        
        hdu.Write(fptr);
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
        
        if((verbose & verboseLevel::VERBOSE_IMG) == verboseLevel::VERBOSE_IMG)
            std::cout<<"\033[31m[FITScube::Write]\033[0m: Create image of "<<Naxis.size()<<" axis (";

        long long int *axis = new long long int [Naxis.size()];
        for(unsigned int i = 0; i < Naxis.size()-1; i++)
        {
            axis[i] = static_cast<long long int>( Naxis[i] );

            if((verbose & verboseLevel::VERBOSE_IMG) == verboseLevel::VERBOSE_IMG)
                std::cout<<axis[i]<<",";
        }
        axis[Naxis.size()-1] = static_cast<long long int>( Naxis[Naxis.size()-1] );

        if((verbose & verboseLevel::VERBOSE_IMG) == verboseLevel::VERBOSE_IMG)
            std::cout<<axis[Naxis.size()-1]<<")"<<std::endl;
        
        if( BITPIX == 0 )
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
        
        fptr.reset();
    }
  
    
#pragma endregion
#pragma region • Modifier
    
    /**
     *  Modify the pixel length of an axis of a FITS datacube. To be used with care and only when images are croped.
     *
     *  @param n    Identification of the axis
     *  @param size New size, in pixel, of the axis
     */
    void FITScube::SetAxisLength(const size_t& n, const size_t& size)
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
        
        
        Naxis[n-1] = size;
        hdu.ValueForKey(std::string("NAXIS")+std::to_string(n),size);
    }
    
    /**
     *  Delete HEADER information related to the last axis. To be used with care and only when images reduced to (n-1)D.
     */
    void FITScube::DeleteLastAxis()
    {
        size_t nAxis = Naxis.size();
        Naxis.resize(Naxis.size()-1);
        
        hdu.DeleteKey(std::string("NAXIS")+std::to_string(nAxis));
        
        hdu.ValueForKey("NAXIS",Naxis.size());
    }
    
    void FITScube::BitPerPixel(const int& _bit, const int& eq)
    {
        BITPIX = _bit;
        if(eq == 0)
            eqBITPIX = BITPIX;
        else
            eqBITPIX = eq;
        
        hdu.ValueForKey( "BITPIX" , BITPIX );
    }
    
    
    void FITScube::SetName(const std::string& _name)
    {
        if(name.size() > 1)
            name.clear();
        
        name += _name;
        hdu.ValueForKey("EXTNAME",name);
    }
    
    /**
     *  @details Check either the pixel at pixel coordinates iPx is masked or note
     *  @param iPx Pixel coordinates of the pixel
     * @return true if the pixel is masked
     */
    bool FITScube::Masked(const std::initializer_list<size_t>& _l) const
    {
        std::vector<size_t> pix_index;
        for (size_t idx : _l)
            pix_index.push_back( idx );
        
        return Masked(pix_index);
        
    }
    
    bool FITScube::Masked(const std::vector<size_t>& _v) const
    {
        size_t index = PixelIndex(_v);
        
        return Masked(index);
    }

    bool FITScube::Masked(size_t idx) const
    {
        if(idx >= mask.size())
            throw std::out_of_range("FITSimg::Masked - index out of range");
        
        return mask[idx];
    }
    
    /**
     *  @details Mask pixels according to a mask map
     *  @param _map pixel mask map
     *  @Note : Pixel already masked are not unmasked
     */
    void FITScube::MaskPixels(const std::initializer_list<size_t>& _l)
    {
        for (size_t idx : _l)
        {
            if (idx < mask.size())
                mask[idx] |= true;
            else 
                throw std::out_of_range("FITSimg::MaskPixel - index out of range");
        }
    }

    /**
     *  @details Mask pixels according to a mask map
     *  @param _map pixel mask map
     *  @Note : Pixel already masked are not unmasked
     */
    void FITScube::MaskPixels(const std::valarray<bool>& _m)
    {
        if (_m.size() != mask.size())
            throw std::length_error("FITSimg::MaskPixels - mask size mismatch");

        mask |= _m;
    }
    
    /**
     *  @details Unmask a single pixel
     *  @param iPx coordinates of the pixel to unmask
     */
    void FITScube::UnmaskPixels(const std::initializer_list<size_t>& _l)
    {
        for (size_t idx : _l)
        {
            if (idx < mask.size())
                mask[idx] &= false;
            else 
                throw std::out_of_range("FITSimg::UnmaskPixels - index out of range");
        }
        
    }
    
    /**
     *  @details Unmask pixels according to a unmask map
     *  @param _map pixel map of pixel to unmask
     *  @note : The pixel map should be set at 'true' for each pixel that need to be unmasked. The resulting pixel mask will be set as mask &= (!_m)
     */
    void FITScube::UnmaskPixels(const std::valarray<bool>& _m)
    {
        if (_m.size() != mask.size())
            throw std::length_error("FITSimg::MaskPixels - mask size mismatch");

        mask &= (!_m) ;
    }

#pragma endregion

#pragma region • Data operation
    /**
     *  @details Compute the mean of all unmasked pixel values
     *  @return Average of all unmasked pixel values
     */
    double FITScube::GetQuadraticMean() const
    {
        size_t nPix = mask[!mask].size();

        if(!data || nPix < 1)
            return 0.0;

        double sum = 0.0;
        bool handled = false;

        // fast typed paths (add the types you commonly use)
        handled = WithTypedData<int8_t>([&](const std::valarray<int8_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint8_t>([&](const std::valarray<uint8_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int16_t>([&](const std::valarray<int16_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint16_t>([&](const std::valarray<uint16_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int32_t>([&](const std::valarray<int32_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint32_t>([&](const std::valarray<uint32_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int64_t>([&](const std::valarray<int64_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint64_t>([&](const std::valarray<uint64_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<size_t>([&](const std::valarray<size_t>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<float>([&](const std::valarray<float>& arr){ sum += static_cast<double>( pow(arr[!mask],2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<double>([&](const std::valarray<double>& arr){ sum += pow(arr[!mask],2).sum(); });
        if(handled) return sum;

       // fallback generic per-element access via FitsArrayBase::get()
        for(size_t i = 0; i < data->size(); ++i)
        {
            if(i < mask.size() && mask[i]) continue;
            sum += pow(static_cast<double>(data->get(i)),2.);
        }

        return (nPix > 0) ? sum /= static_cast<double>(nPix) : 0;
    }

    /**
     *  @details Compute the mean of all unmasked pixel values
     *  @return Average of all unmasked pixel values
     */
    double FITScube::GetVariance() const
    {
        size_t nPix = mask[!mask].size();

        if(!data || nPix < 1)
            return 0.0;

        double sum = 0.0;
        bool handled = false;

        // fast typed paths (add the types you commonly use)
        handled = WithTypedData<int8_t>([&](const std::valarray<int8_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<int8_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint8_t>([&](const std::valarray<uint8_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<uint8_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int16_t>([&](const std::valarray<int16_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<int16_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint16_t>([&](const std::valarray<uint16_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<uint16_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int32_t>([&](const std::valarray<int32_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<int32_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint32_t>([&](const std::valarray<uint32_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<uint32_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<int64_t>([&](const std::valarray<int64_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<int64_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<uint64_t>([&](const std::valarray<uint64_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<uint64_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<size_t>([&](const std::valarray<size_t>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<size_t>(GetMean()+0.5),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<float>([&](const std::valarray<float>& arr){ sum += static_cast<double>( pow(arr[!mask]-static_cast<float>(GetMean()),2).sum() ); });
        if(handled) return sum;

        handled = WithTypedData<double>([&](const std::valarray<double>& arr){ sum += pow(arr[!mask]-GetMean(),2).sum(); });
        if(handled) return sum;

       // fallback generic per-element access via FitsArrayBase::get()
        for(size_t i = 0; i < data->size(); ++i)
        {
            if(i < mask.size() && mask[i]) continue;
            sum += pow(static_cast<double>(data->get(i)) - GetMean(),2.);
        }

        return (nPix > 0) ? sum /= static_cast<double>(nPix) : 0;
    }

    double FITScube::Getpercentil(double fpp) const
    {
        if(fpp < 0. || fpp > 1.)
            throw FITSexception(BAD_OPTION,"FITScube","Getpercentil","fpp should be in the range [0,1]");

        if(data == nullptr || data->size() < 1)
            throw FITSexception(BAD_OPTION,"FITScube","Getpercentil","Empty data array, invalid data");

        std::vector<double> sorted;
        for(size_t i = 0; i < data->size(); i++)
            if(!mask[i] && std::abs(static_cast<double>(data->get(i))) > std::numeric_limits<double>::min()) sorted.push_back( static_cast<double>( data->get(i) ));
        
        std::sort(sorted.begin(), sorted.end());

        double pos = fpp * (sorted.size() - 1);
        std::size_t idx = static_cast<std::size_t>(pos);
        double frac = pos - static_cast<double>(idx);
        double pp_value = 0.0;

        if (idx + 1 < sorted.size())
            pp_value = sorted[idx] * (1.0 - frac) + sorted[idx + 1] * frac;
        else
            pp_value = sorted[idx];
        
        return pp_value;
    }

    double FITScube::GetMedian() const
    {        
        return Getpercentil(0.5);
    }
    
    double FITScube::Get5thpercentil() const
    {
        return Getpercentil(0.05);
    }

    double FITScube::Get25thpercentil() const
    {
        return Getpercentil(0.25);
    }

    double FITScube::Get75thpercentil() const
    {
        return Getpercentil(0.75);
    }

    double FITScube::Get95thpercentil() const
    {
        return Getpercentil(0.95);
    }

    /**
     *  @details Compute the minimum of all unmasked pixel values
     *  @return Minimum value of all unmasked pixel values
     */
    double FITScube::GetMinimum() const
    {
        if(!data)
            throw FITSexception(NOT_IMAGE,"FITScube","GetMinimum","No data available");
    
        bool handled = false;
        double minVal = 0;

        // fast typed paths (add the types you commonly use)
        handled = WithTypedData<int8_t>([&](const std::valarray<int8_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<uint8_t>([&](const std::valarray<uint8_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<int16_t>([&](const std::valarray<int16_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<uint16_t>([&](const std::valarray<uint16_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<int32_t>([&](const std::valarray<int32_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<uint32_t>([&](const std::valarray<uint32_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<int64_t>([&](const std::valarray<int64_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<uint64_t>([&](const std::valarray<uint64_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<size_t>([&](const std::valarray<size_t>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<float>([&](const std::valarray<float>& arr){ minVal += static_cast<double>( arr[!mask].min() ); });
        if(handled) return minVal;

        handled = WithTypedData<double>([&](const std::valarray<double>& arr){ minVal += arr[!mask].min(); });
        if(handled) return minVal;

       // fallback generic per-element access via FitsArrayBase::get()
        for(size_t i = 0; i < data->size(); ++i)
        {
            if(i < mask.size() && mask[i]) continue;
            minVal = (static_cast<double>(data->get(i)) < minVal) ? static_cast<double>(data->get(i)) : minVal;
        }

        return minVal;
    }

    /**
     *  @details Compute the maximum of all unmasked pixel values
     *  @return Maximum value of all unmasked pixel values
     */
    double FITScube::GetMaximum() const
    {
        if(!data)
            throw FITSexception(NOT_IMAGE,"FITScube","GetMinimum","No data available");
    
        bool handled = false;
        double maxVal = 0;

        // fast typed paths (add the types you commonly use)
        handled = WithTypedData<int8_t>([&](const std::valarray<int8_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<uint8_t>([&](const std::valarray<uint8_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<int16_t>([&](const std::valarray<int16_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<uint16_t>([&](const std::valarray<uint16_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<int32_t>([&](const std::valarray<int32_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<uint32_t>([&](const std::valarray<uint32_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<int64_t>([&](const std::valarray<int64_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<uint64_t>([&](const std::valarray<uint64_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<size_t>([&](const std::valarray<size_t>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<float>([&](const std::valarray<float>& arr){ maxVal += static_cast<double>( arr[!mask].max() ); });
        if(handled) return maxVal;

        handled = WithTypedData<double>([&](const std::valarray<double>& arr){ maxVal += arr[!mask].max(); });
        if(handled) return maxVal;

       // fallback generic per-element access via FitsArrayBase::get()
        for(size_t i = 0; i < data->size(); ++i)
        {
            if(i < mask.size() && mask[i]) continue;
            maxVal = (static_cast<double>(data->get(i)) > maxVal) ? static_cast<double>(data->get(i)) : maxVal;
        }

        return maxVal;
    }

#pragma endregion
#pragma endregion

#pragma region Class template instantiation
template class FITSimg<uint8_t>;
template class FITSimg<int8_t>;
template class FITSimg<int16_t>;
template class FITSimg<uint16_t>;
template class FITSimg<int32_t>;
template class FITSimg<uint32_t>;
template class FITSimg<int64_t>;
template class FITSimg<uint64_t>;
template class FITSimg<size_t>;
template class FITSimg<float>;
template class FITSimg<double>;
#pragma endregion
    
}

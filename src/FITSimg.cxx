//
//  FITSimg.h++
//  
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


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

#pragma region * protected member function
    
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
#pragma region * Initialization

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
#pragma region * ctor/dtor
    /**
     *  @details Create a new empty FITS cube
     */
    FITScube::FITScube():fwcs()
    {
        init();
    }

    /**
     *  @details Read current HDU of the fitsfile to extract a 2D images
     *  @param fptr: Pointer to the fitfile
     */
    FITScube::FITScube(const std::shared_ptr<fitsfile>& fptr):fwcs()
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

        FITSwcs _wcs(fptr);
        std::swap(fwcs, _wcs);
        
        
        //- GET BASIC INFORMATION RELATED TO THE IMAGE
        //  * GET NUMBER OF AXIS
        uint64_t number_axis = hdu.GetUInt64ValueForKey("NAXIS");
        
        if( number_axis < 1 )
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","Current HDU doesn't contain data");
        }
        
        //  * GET AXIS DIMENSSION
        for(long long unsigned int iAxe = 1; iAxe <= number_axis; iAxe++)
            Naxis.push_back(static_cast<size_t>( hdu.GetUInt64ValueForKey("NAXIS"+std::to_string(iAxe))));
        
        if(Naxis.size() < 1)
        {
            img_status = BAD_NAXIS;
            throw FITSexception(img_status,"FITScube","ctor","PROBLEM READING AXIS.");
        }
        
        //  * GET DATA TYPE
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
        
        //  * GET EXTENSION NAME
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
    FITScube::FITScube(const FITScube& cube):fwcs()
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
#pragma region * accessor        
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
     * @details Obtain the physical coordinate of a pixel in a specified coordinate system
     * @param k pixel index position in the pixel array
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return pixel world coordinate
     */
    worldCoords FITScube::WorldCoordinates(const size_t& k, const int& wcsIndex) const
    {
        auto pixels_coo = std::bind( &FITScube::PixelCoordinates, this,std::placeholders::_1);
        return WorldCoordinates(pixels_coo(k));
    }
    
    /**
     * @brief Obtain the physical coordinate of a pixel in a specified coordinate system
     * 
     * @param pixel Pixel coordinates on each dimension of the FITS datacube
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return pixel world coordinate
     */
    worldCoords FITScube::WorldCoordinates(const std::vector<size_t>& pixel, const int& wcsIndex) const
    {
        pixelCoords dbl_px = pixelCoords(pixel.size());
        size_t idx = 0;
        for(std::vector<size_t>::const_iterator i = pixel.cbegin() ; i != pixel.cend(); i++)
        {
            dbl_px[idx] = static_cast<double>(*i);
            idx++;
        }
        
        worldCoords wcs = WorldCoordinates(dbl_px);
        dbl_px.clear();
        
        return wcs;
    }
    
    worldCoords FITScube::WorldCoordinates(const pixelCoords& pixel, const int& wcsIndex) const
    {
        if(fwcs.getNumberOfWCS() == 0)
        {
            return worldCoords(pixel);
        }

        if(wcsIndex < 0 || wcsIndex >= fwcs.getNumberOfWCS())
        {
            throw WCSexception(WCSERR_NULL_POINTER,"FITScube","WorldCoordinates","No WCS at index "+std::to_string(wcsIndex)+" defined in this FITS image");
        }

        pixelVectors pxVec;
        pxVec.push_back(pixel);
        worldCoords wcs = fwcs.pixel2world(wcsIndex,pxVec)[0];

        return wcs;
    }

    /** 
     * @brief Obtain the physical coordinates of multiple pixels in a specified coordinate system
     * @param pixels List of pixel index of the FITS datacube
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return a vector of pixel world coordinates
     */
    worldVectors FITScube::WorldCoordinatesVector(const std::initializer_list<size_t>& pixels, const int& wcsIndex) const
    {
        return WorldCoordinatesVector(std::vector<size_t>(pixels), wcsIndex);
    }

    /** 
     * @brief Obtain the physical coordinates of multiple pixels in a specified coordinate system
     * @param pixels List of pixel index of the FITS datacube
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return a vector of pixel world coordinates
     */
    worldVectors FITScube::WorldCoordinatesVector(const std::valarray<size_t>& pixels, const int& wcsIndex) const
    {
        std::vector<size_t> px_vec;
        for(size_t i = 0; i < pixels.size(); i++)
        {
            px_vec.push_back(pixels[i]);
        }
        return WorldCoordinatesVector(px_vec, wcsIndex);
    }

    /** 
     * @brief Obtain the physical coordinates of multiple pixels in a specified coordinate system
     * @param pixels vector of pixel index of the FITS datacube
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return a vector of pixel world coordinates
     */
    worldVectors FITScube::WorldCoordinatesVector(const std::vector<size_t>& pixels, const int& wcsIndex) const
    {
        pixelVectors pxs;
        for(std::vector<size_t>::const_iterator it = pixels.cbegin(); it != pixels.cend(); it++)
        {
            pixelCoords k;
            std::vector<size_t> px_coord = PixelCoordinates(*it);
            
            for(std::vector<size_t>::const_iterator ix = px_coord.cbegin(); ix != px_coord.cend(); ix++)
            {
                k.push_back(static_cast<double>(*ix));
            }
            pxs.push_back(k);    
        }

        return WorldCoordinatesVector(pxs, wcsIndex);
    }

    /**
     * @brief Obtain the physical coordinates of multiple pixels in a specified coordinate system
     * 
     * @param pixels vector of pixel coordinates with floating point value on each axis
     * @param wcsIndex Index of the WCS to be used (default is 0)
     * @return vector of pixel world coordinates
     */
    worldVectors FITScube::WorldCoordinatesVector(const pixelVectors& pixels, const int& wcsIndex) const
    {
        if(fwcs.getNumberOfWCS() == 0)
        {
            worldVectors wc_vec;
            for(const auto& px : pixels)
            {
                wc_vec.push_back(worldCoords(px));
            }
            return wc_vec;
        }

        if(wcsIndex < 0 || wcsIndex >= fwcs.getNumberOfWCS())
        {
            throw WCSexception(WCSERR_NULL_POINTER,"FITScube","WorldCoordinatesMatrix","No WCS at index "+std::to_string(wcsIndex)+" defined in this FITS image");
        }

        return fwcs.pixel2world(wcsIndex, pixels);
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
    pixelCoords FITScube::World2Pixel(const worldCoords& coo, const int& wcsIndex) const
    {
        if(fwcs.getNumberOfWCS() == 0)
        {
            throw WCSexception(WCSERR_NULL_POINTER,"FITScube","World2Pixel","No WCS defined in this FITS image");
        }

        if(wcsIndex < 0 || wcsIndex >= fwcs.getNumberOfWCS())
        {
            throw  WCSexception(WCSERR_NULL_POINTER,"FITScube","World2Pixel","No WCS at index "+std::to_string(wcsIndex)+" defined in this FITS image");
        }
        
        worldVectors VWCoo;
        VWCoo.push_back(coo);
        return fwcs.world2pixel(wcsIndex, VWCoo)[0];
    }

    /**
     * @brief Get the pixel coordinates of multiple pixels given their world coordinates
     * 
     * @param coo World coordinates of the pixels array
     * @param wcsIndex index of the WCS to be used
     * @return vector of pixel coordinates in floating point representation
     */
    pixelVectors FITScube::World2PixelVector(const worldVectors& coo, const int& wcsIndex) const
    {
        if(fwcs.getNumberOfWCS() == 0)
        {
            throw WCSexception(WCSERR_NULL_POINTER,"FITScube","World2PixelMatrix","No WCS defined in this FITS image");
        }

        if(wcsIndex < 0 || wcsIndex >= fwcs.getNumberOfWCS())
        {
            throw  WCSexception(WCSERR_NULL_POINTER,"FITScube","World2PixelMatrix","No WCS at index "+std::to_string(wcsIndex)+" defined in this FITS image");
        }
        
        return fwcs.world2pixel(wcsIndex, coo);
    }

    /**
     * @brief Get the pixel indices in the 1D pixel array given the world coordinates of multiple pixels
     * 
     * @param coo World coordinates of the pixels array
     * @param wcsIndex Index of the WCS to be used
     * @return array of 1D pixel indices
     */
    std::valarray<size_t> FITScube::World2PixelArray(const worldVectors& coo, const int& wcsIndex) const
    {
        pixelVectors pxs = World2PixelVector(coo,wcsIndex);
        std::valarray<size_t> result(pxs.size());

        size_t ij = 0;
        for(pixelVectors::const_iterator it = pxs.cbegin(); it != pxs.cend(); it++)
        {
            size_t idx = PixelIndex(*it);
            result[ij] = idx;
            ij++;
        }

        return result;
    }

    /**
     *  Get the pixel index in the 1D pixel array given the pixel coordinates on each dimension of the FITS datacube
     *  @param iPx: Pixel coordinates on each dimension of the FITS datacube
     *  @return 1D pixel index
     */
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
#pragma region * I/O
    
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
#pragma region * Modifier
    
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

#pragma region * Data operation


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
#ifdef Darwinx86_64
template class FITSimg<size_t>;
#endif
template class FITSimg<float>;
template class FITSimg<double>;
#pragma endregion
    
}

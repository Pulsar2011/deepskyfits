//
//  FITSmanager.cxx
//  DeepSkyLib
//
//  Created by GILLARD William on 01/04/15.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#include <DSTfits/FITSexception.h>
#include <DSTfits/FITSmanager.h>
#include <DSTfits/FITSimg.h>
#include <DSTfits/DSF_version.h>

#include<stdexcept>
#include<sstream>

namespace DSL
{
    
#pragma mark • Initialization
    
    /**
     * @details Initialize local variables to decent default-values.
     */
    void FITSmanager::init()
    {
#if __cplusplus < 201103L
        fptr = NULL;
#else
        fptr = std::shared_ptr<fitsfile>(nullptr);
#endif
        fits_status = 0;
        num_hdu=0;
    }
    
    /**
     * @details Explore the content of a FITS file to retrive basic informations.
     */
    void FITSmanager::explore()
    {
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            num_hdu = 0;
            fits_status = BAD_FILEPTR;
            throw FITSexception(fits_status,"FITSmanager","ctor","Current fitsfile is not defined");
        }

#if __cplusplus < 201103L
        fits_get_num_hdus(fptr, &num_hdu, &fits_status);
#else
        fits_get_num_hdus(fptr.get(), &num_hdu, &fits_status);
#endif
        
        if(fits_status)
        {
            num_hdu = 0;
            throw FITSexception(fits_status,"FITSmanager","explore");
        }
        
        if(FITSmanager::isDebug())
            std::cout<<" `-- Number of HDU in Fits file : "<<num_hdu<<std::endl;
    }
    
#pragma mark • ctor/dtor
    /**
     *  @details Construct FITSmanager object and initialyse value to
     */
    FITSmanager::FITSmanager()
    {
        init();
    }
    
    /**
     *  @details Open, or create, a fits file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     */
    FITSmanager::FITSmanager(std::string fileName, bool readOnly)
    {
        init();
#if __cplusplus >= 201103L
        fitsfile * fits = static_cast<fitsfile*>(fptr.get());
#endif
        
        if(
#if __cplusplus < 201103L
           fits_open_file(&fptr, fileName.c_str(), readOnly, &fits_status);
#else
           fits_open_file(&fits, fileName.c_str(), readOnly, &fits_status)
#endif
           )
        {
#if __cplusplus < 201103L
            fptr = NULL;
#else
            fptr.reset();
#endif
            throw FITSexception(fits_status,"FITSmanager","ctor","FILE : "+fileName);
        }
        
#if __cplusplus >= 201103L
        fptr.reset(fits, FITSmanager::CloseFile);
#endif
        explore();
    }
    
    /**
     *  @details Attribute an existing fitsfile pointer to current FITSmanager.
     *  @param inPtr Address of an existing fitsfile pointer.
     */
    FITSmanager::FITSmanager(fitsfile& inPtr)
    {
        init();
#if __cplusplus < 201103L
        fptr = &inPtr;
#else
        fptr.reset(&inPtr, FITSmanager::CloseFile);
#endif
        explore();
    }
    
    /**
     *  @details Copy a FITSmanager to this.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    FITSmanager::FITSmanager(const FITSmanager& inFits)
    {
#if __cplusplus < 201103L
        *fptr = *(inFits.fptr);
#else
        fptr = std::make_shared<fitsfile>(*(inFits.fptr));
#endif
        fits_status = 0;
        
        explore();
    }
    
    /**
     *  @details Destructor.
     */
    FITSmanager::~FITSmanager()
    {
        if(
#if __cplusplus < 201103L
           fptr != NULL
#else
           fptr.use_count() == 1
#endif
           )
        {
            Close();
        }

#if __cplusplus < 201103L
	delete fptr;
#endif
        
    }

#pragma mark • Diagnoze
    
    const std::string FITSmanager::getFileName(fitsfile *fptr)
    {
        if(fptr == NULL)
            return std::string();
        
        char ffname[999];
        int  this_status;
        ffflnm(fptr, ffname, &this_status);
        
        return std::string(ffname);
    }
    
#pragma mark • Accessing FITS file
    /**
     *  @details Create a new empty FITS file.
     *  @param fileName Name of the file to be created
     *  @param replace boolean flag to specify either an existing file should be overwritten or not
     *  @return Poniter to the newly created fitsfile
     *  @note Once created the fits file is automatically opened but on have to remind that the file is still empty at this level.
     *  @remarks If the boolean flag replace is set to false and a file with the same name and at the same location already exists, the FITS manager will trigger an error that will return a NULL pointer.
     */
    FITSmanager* FITSmanager::Create(std::string fileName, bool replace)
    {
        if(replace)
            fileName.insert(0,"!");
        else if(fileName[0] == '!')
        {
            fileName.erase(0,1);
        }
        
        // CREATE FITS FILE ON HDD
        int status = 0;
        fitsfile *fptr = NULL;
        if(fits_create_file(&fptr, (char*) fileName.c_str(), &status ))
        {
            fptr = NULL;
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        // CREATE EMPTY PRIMARY HEADER
        if(fits_create_imgll(fptr, USHORT_IMG, 0, NULL, &status))
        {
           throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        // APPEND Appropriate KEYWORDS
        std::string fits_comment = std::string("FITS created with DST library ")+ DSF::gGIT::version();
        if(fits_write_comment(fptr, (char*) fits_comment.c_str(), &status))
        {
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        if(fits_write_date(fptr, &status ))
        {
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        if(fits_flush_file(fptr, &status ))
        {
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        FITSmanager* new_file = new FITSmanager(*fptr);
        new_file->MoveToPrimary();
        
        return new_file;

    }
    
    /**
     *  @details Open an existing FITS file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    FITSmanager* FITSmanager::Open(std::string fileName, bool readOnly)
    {
        return new FITSmanager(fileName, readOnly);
    }
    
    /**
     *  @details Open an existing FITS file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    void FITSmanager::OpenFile(std::string fileName, bool readOnly)
    {
#if __cplusplus >= 201103L
        fitsfile *fits = NULL;
#endif
        
        if(
#if __cplusplus < 201103L
           fits_open_file(&fptr, fileName.c_str(), readOnly, &fits_status)
#else
           fits_open_file(&fits, fileName.c_str(), readOnly, &fits_status)
#endif
           )
        {
#if __cplusplus < 201103L
            fptr = NULL;
#else
            fptr.reset();
#endif
            throw FITSexception(fits_status,"FITSmanager","OpenFile","FILE : "+fileName);
        }
        
#if __cplusplus >= 201103L
        fptr.reset(fits, FITSmanager::CloseFile);
#endif
        
        explore();
    }
    
    /**
     *  @details Close the fitsfile refered by this.
     */
    void FITSmanager::Close()
    {
        try
        {
#if __cplusplus < 201103L
            if(fptr == NULL)
                throw FITSexception(NULL_INPUT_PTR,"FITSmanager","Close","No FITS file opened !");
#endif
        
        
#if __cplusplus < 201103L
            fits_close_file(fptr);
            fptr = NULL;
            
            std::cout<<"File CLOSED"<<std::endl;
#else
            fptr.~shared_ptr();
#endif
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
    }
    
    
    void FITSmanager::CloseFile(fitsfile* in)
    {
        int status = 0;
        if(fits_close_file(in, &status))
            throw FITSexception(status,"FITSmanager","CloseFile");
        
        std::cout<<"File CLOSED"<<std::endl;
        
        return;
    }
    
#pragma mark • Writing FITS file to disk
    void FITSmanager::Write()
    {
        if(
#if __cplusplus < 201103L
           fits_flush_file(fptr, &fits_status )
#else
           fits_flush_file(fptr.get(), &fits_status )
#endif
           )
            throw FITSexception(fits_status,"FITSmanager","Write");
        
    }

    
#pragma mark • Accessing FITS HDU
    /**
     *  @details Retrive primary header of the FITS file
     *  @return Pointer to the primary header
     *  @note It will return a NULL pointer if the header haven't been found or in case of errors while reading the FITS data
     */
    FITShdu *FITSmanager::GetPrimaryHeader()
    {
        return GetHeaderAtIndex(1);
    }
    
    /**
     *  @details Retrive specific header from the fits file
     *  @param hdu_index Absolute index of the header.
     *  @return Pointer to the requested header
     *  @note It will return a NULL pointer if the header haven't been found or in case of errors while reading the FITS data
     */
    FITShdu *FITSmanager::GetHeaderAtIndex(int hdu_index)
    {
        FITShdu *hdu = NULL;
        
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        MoveToHDU(hdu_index);
        
        if(fits_status)
            return NULL;

#if __cplusplus < 201103L
        hdu = new FITShdu(fptr);
#else
        hdu = new FITShdu(fptr.get());
#endif
        
        return hdu;
    }
    
#pragma mark • Accessing FITS image
    FITScube *FITSmanager::GetPrimary()
    {
        return GetImageAtIndex(1);
    }
    
    FITScube *FITSmanager::GetImageAtIndex(int hdu_index)
    {
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        int hdu_type = MoveToHDU(hdu_index);
        
        if(fits_status)
            return NULL;
        
        try
        {
            if(hdu_type != IMAGE_HDU)
            {
                fits_status = NOT_IMAGE;
#if __cplusplus < 201103L
                throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex","FILE "+getFileName(fptr)+"\nCurrent HDU isn't an FITS image!");
#else
                throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex","FILE "+getFileName(fptr.get())+"\nCurrent HDU isn't an FITS image!");
#endif
            }
            
        
            FITScube *img = NULL;
        
            int BITPIX = 0;
        
            if(
#if __cplusplus < 201103L
               fits_get_img_equivtype(fptr, &BITPIX, &fits_status)
#else
               fits_get_img_equivtype(fptr.get(), &BITPIX, &fits_status)
#endif
               )
            {
                throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex");
            }
        
            switch (BITPIX)
            {
                case 8:
#if __cplusplus < 201103L
                    img = new FITSimg<uint8_t>(fptr);
#else
                    img = new FITSimg<uint8_t>(fptr.get());
#endif
                    break;
            
                case 10:
#if __cplusplus < 201103L
                    img = new FITSimg<int8_t>(fptr);
#else
                    img = new FITSimg<int8_t>(fptr.get());
#endif
                    break;
                
                case 16:
#if __cplusplus < 201103L
                    img = new FITSimg<int16_t>(fptr);
#else
                    img = new FITSimg<int16_t>(fptr.get());
#endif
                    break;
                
                case 20:
#if __cplusplus < 201103L
                    img = new FITSimg<uint16_t>(fptr);
#else
                    img = new FITSimg<uint16_t>(fptr.get());
#endif
                    break;
                
                case 32:
#if __cplusplus < 201103L
                    img = new FITSimg<int32_t>(fptr);
#else
                    img = new FITSimg<int32_t>(fptr.get());
#endif
                    break;
                
                case 40:
#if __cplusplus < 201103L
                    img = new FITSimg<uint32_t>(fptr);
#else
                    img = new FITSimg<uint32_t>(fptr.get());
#endif
                    break;
                
                case 64:
#if __cplusplus < 201103L
                    img = new FITSimg<int64_t>(fptr);
#else
                    img = new FITSimg<int64_t>(fptr.get());
#endif
                    break;
                
                case -32:
#if __cplusplus < 201103L
                    img = new FITSimg<float>(fptr);
#else
                    img = new FITSimg<float>(fptr.get());
#endif
                    break;
                
                case -64:
#if __cplusplus < 201103L
                    img = new FITSimg<double>(fptr);
#else
                    img = new FITSimg<double>(fptr.get());
#endif
                    break;
                
                default:
                    fits_status = BAD_BITPIX;
                    throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN");
            }
        
                fits_status = img->Status();
        
                return img;
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            return NULL;
        }
    }
    
#pragma mark • Accessing FITS table
    
    /**
     @brief Access FITS table from HDU index
     @param iHDU FITS HDU index that contains the FITS table of interest
     @return pointer to the FITStable.
     */
    FITStable *FITSmanager::GetTableAtIndex(int iHDU)
    {
        FITStable* tbl = NULL;
        
        tbl = new FITStable(fptr, iHDU);
        
        return tbl;
    }
    
    /**
     @brief Access FITS table from HDU name
     @param tname FITS extension name that contains the table of interest
     @return pointer to the FITStable.
     */
    FITStable *FITSmanager::GetTable(std::string tname)
    {
        FITStable* tbl = NULL;
        
        tbl = new FITStable(fptr, tname);
                
        return tbl;
    }
    
    FITStable *FITSmanager::CreateTable(std::string tname, const ttype& tt)
    {
        
        char** ttype = NULL;
        char** tform = NULL;
        char** tunit = NULL;
        
        ttype = new char*[1]; ttype[0] = const_cast<char*>(std::string("TFLOAT").c_str());
        tform = new char*[1]; tform[0] = const_cast<char*>(std::string("COL0").c_str());
        tunit = new char*[1]; tunit[0] = const_cast<char*>(std::string("ARB").c_str());
         
        
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","CreateTable","CAN'T CREATE HEADER FROM NULL POINTER");
        }
        
        if(
#if __cplusplus < 201103L
           ffcrtb(fptr, static_cast<int>(tt), 0, 1, ttype, tform, tunit, const_cast<char*>(tname.c_str()), &fits_status)
#else
           ffcrtb(fptr.get(), static_cast<int>(tt), 0, 1, ttype, tform, tunit, const_cast<char*>(tname.c_str()), &fits_status)
#endif
           )
        {
            throw FITSexception(fits_status,"FITSmanager","CreateTable");
        }

        if(
#if __cplusplus < 201103L
           ffdcol(fptr, 1, &fits_status)
#else
           ffdcol(fptr.get(), 1, &fits_status)
#endif
           )
        {
            throw FITSexception(fits_status,"FITSmanager","CreateTable");
        }
     
        
        return GetTable(tname) ;
    }
    
#pragma mark • Accessing File block
    int FITSmanager::MoveToPrimary()
    {
        return MoveToHDU(1);
    }

    int FITSmanager::MoveToHDU(int hdu_index)
    {
        int hdu_type = 0;
        
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        if(hdu_index > num_hdu)
        {
            std::stringstream ss;
            
            ss<<"FILE "<<
#if __cplusplus < 201103L
            getFileName(fptr)
#else
            getFileName(fptr.get())
#endif
              <<std::endl
              <<"HEADER #"<<hdu_index<<" doesn't exist"<<"\033[0m"<<std::endl;
            
            fits_status = BAD_HDU_NUM;
            
            throw FITSexception(fits_status,"FITSmanager","MoveToHDU",ss.str());
        }
        
        
        if(
#if __cplusplus < 201103L
           fits_movabs_hdu( fptr, hdu_index, &hdu_type, &fits_status ) == BAD_HDU_NUM
#else
           fits_movabs_hdu( fptr.get(), hdu_index, &hdu_type, &fits_status ) == BAD_HDU_NUM
#endif
           )
        {
            std::stringstream ss;
            ss<<"HEADER #"<<hdu_index<<" @ FILE "<<
#if __cplusplus < 201103L
            getFileName(fptr)
#else
            getFileName(fptr.get())
#endif
            <<std::endl;
            
            throw FITSexception(fits_status,"FITSmanager","MoveToHDU",ss.str());
        }
        
        return hdu_type;
    }
    
#pragma mark • Modifying File
    /**
     *  @details Append image extention to the end of the fits file.
     *  @param img Image data cube to be added to the fits file.
     */
    void FITSmanager::AppendImage(FITScube &img)
    {
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        if(img.GetBitPerPixel() == 0)
        {
            throw std::invalid_argument("\033[31m[FITSmanager::AppendImage]\033[0minvalid BITPIX");
            return;
        }
        
        std::cout<<"\033[31m[FITSmanager::AppendImage]\033[0m: Append image of "<<img.GetDimension()<<" axis (";
        LONGLONG *axis = new LONGLONG [img.GetDimension()];
        for(size_t i = 0; i < img.GetDimension(); i++)
        {
            axis[i] = static_cast<LONGLONG>(img.Size(i+1));
            std::cout<<axis[i];
            if(i != img.GetDimension()-1)
                std::cout<<",";
            else
                std::cout<<")";
        }
        
        if(
#if __cplusplus < 201103L
           fits_create_imgll(fptr, img.GetBitPerPixel(), static_cast<int>( img.GetDimension() ), axis,  &fits_status )
#else
           fits_create_imgll(fptr.get(), img.GetBitPerPixel(), static_cast<int>( img.GetDimension() ), axis,  &fits_status )
#endif
           )
        {
            throw FITSexception(fits_status,"FITSmanager","AppendImage");
        }
                
        num_hdu++;

#if __cplusplus < 201103L
        img.Write(fptr);
#else
        img.Write(fptr.get());
#endif
    }
    
#pragma mark • Modifying specific header keyword   
    
    void FITSmanager::AppendKeyToPrimary(std::string key, const FITSkeyword &val)
    {
        AppendKeyToHeader(1, key, val);
    }
    
    void FITSmanager::AppendKeyToHeader(int HDU, std::string key, const FITSkeyword &val)
    {
        char *comment = NULL;
        if(val.comment().size() > 0 )
            comment = const_cast<char*>(val.comment().c_str());
        
        try
        {
        switch (val.type()) {
            case FITSkeyword::fChar :
                if(val.value().size() < 68)
                {
                    if(
#if __cplusplus < 201103L
                       ffukys(fptr, (char*) key.c_str(), (char*) val.c_str(), comment, &fits_status )
#else
                       ffukys(fptr.get(), const_cast<char*>( key.c_str() ), const_cast<char*>( val.value().c_str() ), comment, &fits_status )
#endif
                       )
                    {
                        throw FITSexception(fits_status,"FITSmanager","AppendKey");
                    }
                }
                else
                {
                    if(
#if __cplusplus < 201103L
                       ffukys(fptr, (char*) key.c_str(), (char*) val.c_str(), comment, &fits_status )
#else
                       ffukys(fptr.get(), const_cast<char*>( key.c_str() ), const_cast<char*>( val.value().c_str() ), comment, &fits_status )
#endif
                       )
                    {
                        throw FITSexception(fits_status,"FITSmanager","AppendKey");
                    }
                }
                break;
                
            case FITSkeyword::fShort :
                AppendKeyToHeader(HDU, key, TSHORT, val.value(), val.comment());
                break;
                
            case FITSkeyword::fInt :
                AppendKeyToHeader(HDU, key, TINT, val.value(), val.comment());
                break;
                
            case FITSkeyword::fLong :
                AppendKeyToHeader(HDU, key, TLONG, val.value(), val.comment());
                break;
                
            case FITSkeyword::fLongLong :
                AppendKeyToHeader(HDU, key, TLONGLONG, val.value(), val.comment());
                break;
                
            case FITSkeyword::fBool :
                AppendKeyToHeader(HDU, key, TBYTE, val.value(), val.comment());
                break;
                
            case FITSkeyword::fFloat :
                AppendKeyToHeader(HDU, key, TFLOAT, val.value(), val.comment());
                break;
                
            case FITSkeyword::fDouble :
                AppendKeyToHeader(HDU, key, TDOUBLE, val.value(), val.comment());
                break;
                
            default:
                AppendKeyToHeader(HDU, key, TSTRING, val.value(), val.comment());
                break;
        }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
    }
    
    void FITSmanager::AppendKeyToHeader(int HDU, std::string key, int TYPE, std::string val, std::string cmt)
    {
        try
        {
            if(num_hdu == 0)
            {
                throw FITSwarning("FITSmanager","AppendKeyToHeader","FILE "+
#if __cplusplus < 201103L
                                  getFileName(fptr)
#else
                                  getFileName(fptr.get())
#endif
                                  +" do not yet contains HDU blocks.\nKeyword string will be added to PRIMARY HDU block in memory.");
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            
            AppendKey(key, TYPE, val, cmt);
            return;
        }

        MoveToHDU(HDU);
        
        if(fits_status)
            return;
        
        if(cmt.size() > 1)
            AppendKey(key, TYPE, val, cmt);
        else
            AppendKey(key, TYPE, val);
            
    }
    
    void FITSmanager::AppendKey(std::string key, int TYPE, std::string val, std::string cmt)
    {
        if(
#if __cplusplus < 201103L
           fptr == NULL
#else
           fptr.use_count() == 0
#endif
           )
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","AppendKey","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        char * comment = NULL;
        if(cmt.size() > 1)
            comment = const_cast<char*>( cmt.c_str() );
        
        //std::cout<<"• "<<key<<" = "<<val<<((comment == NULL)?"":std::string("  |  ")+std::string(comment))<<"  "<<TYPE<<std::endl;
        
        try
        {
            if(TYPE == TFLOAT)
            {
                float dval = std::stof(val);
                
                if(
#if __cplusplus < 201103L
                   fits_update_key_fixflt(fptr, (char*) key.c_str(), dval,8, comment, &fits_status )
#else
                   fits_update_key_fixflt(fptr.get(), const_cast<char*>( key.c_str() ),  dval, 8, comment, &fits_status )
#endif
                   )
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TDOUBLE)
            {
                double dval = std::stod(val);
                
                if(
#if __cplusplus < 201103L
                   fits_update_key_fixdbl(fptr, (char*) key.c_str(), dval, 14, comment, &fits_status )
#else
                   fits_update_key_fixdbl(fptr.get(), const_cast<char*>( key.c_str() ),  dval, 14, comment, &fits_status )
#endif
                   )
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TSHORT  ||
                    TYPE == TINT    ||
                    TYPE == TUINT     )
            {
                int dval = std::stol(val);
                
                if(
#if __cplusplus < 201103L
                   fits_update_key_log(fptr, const_cast<char*>( key.c_str() ),  dval, comment, &fits_status )
#else
                   fits_update_key_log(fptr.get(), const_cast<char*>( key.c_str() ),  dval, comment, &fits_status )
#endif
                   )
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TLONG   ||
                    TYPE == TULONG  ||
                    TYPE == TLONGLONG )
            {
                LONGLONG dval = std::stoll(val);
                
                if(
#if __cplusplus < 201103L
                   fits_update_key_lng(fptr, const_cast<char*>( key.c_str() ),  dval, comment, &fits_status )
#else
                   fits_update_key_lng(fptr.get(), const_cast<char*>( key.c_str() ),  dval, comment, &fits_status )
#endif
                   )
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
             else
            {
                if(
#if __cplusplus < 201103L
                   fits_update_key(fptr, TYPE, (char*) key.c_str(), (char*) val.c_str(), comment, &fits_status )
#else
                   fits_update_key(fptr.get(), TYPE, const_cast<char*>( key.c_str() ), const_cast<char*>( val.c_str() ), comment, &fits_status )
#endif
                   )
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
    }
    
}

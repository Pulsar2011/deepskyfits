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
#include <DSTfits/FITSdata.h>
#include <DSTfits/DSF_version.h>

#include<stdexcept>
#include<sstream>
#include<memory>

namespace DSL
{
    verboseLevel operator|(verboseLevel a, verboseLevel b)
    {
        return static_cast<verboseLevel>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    verboseLevel operator&(verboseLevel a, verboseLevel b)
    {
        return static_cast<verboseLevel>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    verboseLevel& operator|=(verboseLevel& a, verboseLevel b)
    {
        a = a | b;
        return a;
    }

    verboseLevel& operator&=(verboseLevel& a, verboseLevel b)
    {
        a = a & b;
        return a;
    }

    verboseLevel operator~(verboseLevel a)
    {
        return static_cast<verboseLevel>(~static_cast<uint8_t>(a));
    }

    // Add operator<< for verboseLevel
    //inline std::ostream& operator<<(std::ostream& os, verboseLevel v)
    //{
    //    os << "0x" << std::hex << static_cast<int>(v) << std::dec;
    //    return os;
    //}

    // Display verboseLevel as binary (e.g., 0000 0001)
    std::ostream& operator<<(std::ostream& os, verboseLevel v)
    {
        uint8_t val = static_cast<uint8_t>(v);
        for (int i = 7; i >= 0; --i) {
            os << ((val >> i) & 1);
            if (i % 4 == 0 && i != 0) os << ' ';
        }
        return os;
    }

#pragma region • Initialization

    /**
     * @details Explore the content of a FITS file to retrive basic informations.
     */
    void FITSmanager::explore()
    {
        if(fptr.use_count() == 0 || fptr == nullptr)
        {
            num_hdu = 0;
            fits_status = BAD_FILEPTR;
            throw FITSexception(fits_status,"FITSmanager","ctor","Current fitsfile is not defined");
        }

        fits_get_num_hdus(fptr.get(), &num_hdu, &fits_status);
        
        if(fits_status)
        {
            num_hdu = 0;
            throw FITSexception(fits_status,"FITSmanager","explore");
        }
        
        if((verbose & verboseLevel::VERBOSE_BASIC)== verboseLevel::VERBOSE_BASIC)
            std::cout<<std::endl<<"\033[32mOPEN\033[0m file \033[33m"<<GetFileName()<<"\033[0m"<<std::endl
                <<" \033[31m`--\033[0m Number of HDU in Fits file : \033[32m"<<num_hdu<<"\033[0m"<<std::endl;
    }
    
#pragma endregion
#pragma region • ctor/dtor
    /**
     *  @details Construct FITSmanager object and initialyse value to
     */
    FITSmanager::FITSmanager():fptr(nullptr),num_hdu(0),fits_status(0)
    {    }
    
    /**
     *  @details Open, or create, a fits file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     */
    FITSmanager::FITSmanager(const std::string& fileName):FITSmanager(fileName,true)
    { }

    /**
     *  @details Open, or create, a fits file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     */
    FITSmanager::FITSmanager(const std::string& fileName, const bool& readOnly):fptr(nullptr),num_hdu(0),fits_status(0)
    {
        fitsfile * fits = static_cast<fitsfile*>(fptr.get());
        
        if(fits_open_file(&fits, fileName.c_str(), !readOnly, &fits_status))
        {
            fptr.reset();
            throw FITSexception(fits_status,"FITSmanager","ctor","FILE : "+fileName);
        }
        
        fptr.reset(fits, FITSmanager::CloseFile);
        explore();
    }
    
    /**
     *  @details Attribute an existing fitsfile pointer to current FITSmanager.
     *  @param inPtr Address of an existing fitsfile pointer.
     */
    FITSmanager::FITSmanager(fitsfile& inPtr):fptr(&inPtr, FITSmanager::CloseFile),num_hdu(0),fits_status(0)
    {
        //fptr.reset(&inPtr, FITSmanager::CloseFile);
        explore();
    }

    /**
     *  @details Attribute an existing fitsfile pointer to current FITSmanager.
     *  @param inPtr Address of an existing fitsfile pointer.
     */
    FITSmanager::FITSmanager(const std::shared_ptr<fitsfile>& fin):fptr(fin),num_hdu(0),fits_status(0)
    {
        explore();
    }
    
    /**
     *  @details Copy a FITSmanager to this.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    FITSmanager::FITSmanager(const FITSmanager& inFits):fptr(inFits.fptr),num_hdu(0),fits_status(0)
    {
        explore();
    }
    
    /**
     *  @details Destructor.
     */
    FITSmanager::~FITSmanager()
    {
        if(fptr.use_count() == 1 && fptr != nullptr)
            Close();        
    }

#pragma endregion
#pragma region • Diagnoze
    
    const std::string FITSmanager::GetFileName() const
    {
        return GetFileName(fptr.get());
    }

    const std::string FITSmanager::GetFileName(fitsfile *inFits) const
    {
        if(inFits == NULL)
            return std::string();
        
        char ffname[999];
        int  this_status;
        ffflnm(inFits, ffname, &this_status);
        
        return std::string(ffname);
    }

    const std::string FITSmanager::GetFileName(const std::shared_ptr<fitsfile>& inFits) const
    {
        return GetFileName(fptr.get());
    }
    
#pragma endregion
#pragma region • Accessing FITS file
    /**
     *  @details Create a new empty FITS file.
     *  @param fileName Name of the file to be created
     *  @param replace boolean flag to specify either an existing file should be overwritten or not
     *  @return Poniter to the newly created fitsfile
     *  @note Once created the fits file is automatically opened but on have to remind that the file is still empty at this level.
     *  @remarks If the boolean flag replace is set to false and a file with the same name and at the same location already exists, the FITS manager will trigger an error that will return a NULL pointer.
     */
    FITSmanager FITSmanager::Create(const std::string& fName, bool replace)
    {
        std::string fileName = fName;
        
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
            if(fptr != NULL)
            {
                fits_close_file(fptr, &status);
                delete fptr;
            }
            fptr = NULL;
            
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        // CREATE EMPTY PRIMARY HEADER
        if(fits_create_imgll(fptr, USHORT_IMG, 0, NULL, &status))
        {
            if(fptr != NULL)
            {
                fits_close_file(fptr, &status);
                delete fptr;
            }
            fptr = NULL;
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        // APPEND Appropriate KEYWORDS
        std::string fits_comment = std::string("FITS created with DST library ")+ DSF::gGIT::version();
        if(fits_write_comment(fptr, (char*) fits_comment.c_str(), &status))
        {
            if(fptr != NULL)
            {
                fits_close_file(fptr, &status);
                delete fptr;
            }
            fptr = NULL;
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        if(fits_write_date(fptr, &status ))
        {
            if(fptr != NULL)
            {
                fits_close_file(fptr, &status);
                delete fptr;
            }
            fptr = NULL;
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        if(fits_flush_file(fptr, &status ))
        {
            if(fptr != NULL)
            {
                fits_close_file(fptr, &status);
                delete fptr;
            }
            fptr = NULL;
            throw FITSexception(status,"FITSmanager","Create","FILE : "+fileName);
        }
        
        FITSmanager new_file(*fptr);
        new_file.MoveToPrimary();
        return new_file;

    }
    
    /**
     *  @details Open an existing FITS file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    FITSmanager FITSmanager::Open(const std::string& fileName, bool readOnly)
    {
        return FITSmanager(fileName, readOnly);
    }
    
    /**
     *  @details Open an existing FITS file.
     *  @param fileName Name of the file to open
     *  @param readOnly Determine the read/write acces to the file
     *  @return Poniter to the opened fitsfile
     */
    void FITSmanager::OpenFile(const std::string& fileName, bool readOnly)
    {
        fitsfile *fits = nullptr;
        
        if(fits_open_file(&fits, fileName.c_str(), readOnly, &fits_status))
        {
            fptr.reset();
            throw FITSexception(fits_status,"FITSmanager","OpenFile","FILE : "+fileName);
        }
        
        fptr.reset(fits, FITSmanager::CloseFile);
        
        explore();
    }
    
    /**
     *  @details Close the fitsfile refered by this.
     */
    void FITSmanager::Close()
    {
        try
        {
            // Use reset to release ownership and call deleter if needed
            std::string fname=GetFileName();
            fptr.reset();
            num_hdu = 0;
            fits_status = 0;
            
            if((verbose & verboseLevel::VERBOSE_DEBUG)== verboseLevel::VERBOSE_DEBUG)
                std::cout<<"File \033[33m"<<fname<<"\033[32m CLOSED\033[0m"<<std::endl;
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::flush;
            return;
        }
    }
    
    
    void FITSmanager::CloseFile(fitsfile* in)
    {
        char ffname[999];
        int status = 0;

        ffflnm(in, ffname, &status);
        std::string fname=std::string(ffname);

        if(fits_close_file(in, &status))
            throw FITSexception(status,"FITSmanager","CloseFile");
        
        if((verbose & verboseLevel::VERBOSE_BASIC)== verboseLevel::VERBOSE_BASIC)
            std::cout<<"File \033[33m"<<fname<<"\033[32m CLOSED\033[0m"<<std::endl;
        
        return;
    }
    
#pragma endregion
#pragma region • Writing FITS file to disk
    void FITSmanager::Write()
    {
        if(fits_flush_file(fptr.get(), &fits_status ))
            throw FITSexception(fits_status,"FITSmanager","Write");
        
    }

    
#pragma endregion
#pragma region • Accessing FITS HDU
    /**
     *  @details Retrive primary header of the FITS file
     *  @return Pointer to the primary header
     *  @note It will return a NULL pointer if the header haven't been found or in case of errors while reading the FITS data
     */
    const std::shared_ptr<FITShdu> FITSmanager::GetPrimaryHeader()
    {
        return GetHeaderAtIndex(1);
    }
    
    /**
     *  @details Retrive specific header from the fits file
     *  @param hdu_index Absolute index of the header.
     *  @return Pointer to the requested header
     *  @note It will return a NULL pointer if the header haven't been found or in case of errors while reading the FITS data
     */
     const std::shared_ptr<FITShdu> FITSmanager::GetHeaderAtIndex(const int& hdu_index)
    {
        FITShdu *hdu = NULL;
        
        if(fptr.use_count() == 0)
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        MoveToHDU(hdu_index);
        
        if(fits_status)
            return NULL;
        hdu = new FITShdu(fptr);

        return std::shared_ptr<FITShdu>(hdu);
    }
    
#pragma endregion
#pragma region • Accessing FITS image
    const std::shared_ptr<FITScube> FITSmanager::GetPrimary()
    {
        return GetImageAtIndex(1);
    }
    
    const std::shared_ptr<FITScube> FITSmanager::GetImageAtIndex(const int& hdu_index)
    {
        if(fptr.use_count() == 0)
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

                throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex","FILE "+GetFileName()+"\nCurrent HDU isn't an FITS image!");
            }
            
        
            FITScube *img = NULL;
        
            int BITPIX = 0;
        
            if(fits_get_img_equivtype(fptr.get(), &BITPIX, &fits_status))
            {
                throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex");
            }
        
            switch (BITPIX)
            {
                case 8:
                    img = new FITSimg<uint8_t>(fptr);
                    break;
            
                case 10:
                    img = new FITSimg<int8_t>(fptr);
                    break;
                
                case 16:
                    img = new FITSimg<int16_t>(fptr);
                    break;
                
                case 20:
                    img = new FITSimg<uint16_t>(fptr);
                    break;
                
                case 32:
                    img = new FITSimg<int32_t>(fptr);
                    break;
                
                case 40:
                    img = new FITSimg<uint32_t>(fptr);
                    break;
                
                case 64:
                    img = new FITSimg<int64_t>(fptr);
                    break;
                
                case -32:
                    img = new FITSimg<float>(fptr);
                    break;
                
                case -64:
                    img = new FITSimg<double>(fptr);
                    break;
                
                default:
                    fits_status = BAD_BITPIX;
                    throw FITSexception(fits_status,"FITSmanager","GetImageAtIndex","CAN'T GET IMAGES, DATA TYPE "+std::to_string(BITPIX)+" IS UNKNOWN");
            }
        
                fits_status = img->Status();
        
                return std::shared_ptr<FITScube>(img);
        }
        catch(std::exception& e)
        {
            std::cerr<<e.what()<<std::endl;
            return NULL;
        }
    }
    
#pragma endregion
#pragma region • Accessing FITS table
    
    /**
     @brief Access FITS table from HDU index
     @param iHDU FITS HDU index that contains the FITS table of interest
     @return pointer to the FITStable.
     */
    const std::shared_ptr<FITStable> FITSmanager::GetTableAtIndex(const int& iHDU)
    {      
        return std::shared_ptr<FITStable>(new FITStable(fptr, iHDU));
    }
    
    /**
     @brief Access FITS table from HDU name
     @param tname FITS extension name that contains the table of interest
     @return pointer to the FITStable.
     */
    const std::shared_ptr<FITStable> FITSmanager::GetTable(std::string tname)
    {
        return std::shared_ptr<FITStable>(new FITStable(fptr, tname));
    }
    
    const std::shared_ptr<FITStable> FITSmanager::CreateTable(std::string tname, const ttype& tt)
    {
        
        std::vector<char*> ttype_vec(1), tform_vec(1), tunit_vec(1);
        std::string ttype_str = "TFLOAT";
        std::string tform_str = "COL0";
        std::string tunit_str = "ARB";
        ttype_vec[0] = const_cast<char*>(ttype_str.c_str());
        tform_vec[0] = const_cast<char*>(tform_str.c_str());
        tunit_vec[0] = const_cast<char*>(tunit_str.c_str());
         
        
        if(fptr.use_count() == 0)
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","CreateTable","CAN'T CREATE HEADER FROM NULL POINTER");
        }
        
        if(ffcrtb(fptr.get(), static_cast<int>(tt), 0, 1, ttype_vec.data(), tform_vec.data(), tunit_vec.data(), const_cast<char*>(tname.c_str()), &fits_status))
        {
            throw FITSexception(fits_status,"FITSmanager","CreateTable");
        }

        if(ffdcol(fptr.get(), 1, &fits_status))
        {
            throw FITSexception(fits_status,"FITSmanager","CreateTable");
        }
     
        
        return GetTable(tname) ;
    }
    
#pragma endregion
#pragma region • Accessing File block
    int FITSmanager::MoveToPrimary()
    {
        return MoveToHDU(1);
    }

    int FITSmanager::MoveToHDU(const int& hdu_index)
    {
        if(fptr.use_count() == 0)
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }

        if(hdu_index< 1 || hdu_index > num_hdu)
        {
            std::stringstream ss;
            
            ss<<"FILE "<<GetFileName()
              <<std::endl
              <<"HEADER #"<<hdu_index<<" doesn't exist"<<"\033[0m"<<std::endl;
            
            fits_status = BAD_HDU_NUM;
            
            throw FITSexception(fits_status,"FITSmanager","MoveToHDU",ss.str());
        }

        fits_status = 0;
        int hdu_type = 0;
        
        if(fits_movabs_hdu( fptr.get(), hdu_index, &hdu_type, &fits_status ) == BAD_HDU_NUM)
        {
            std::stringstream ss;
            ss<<"HEADER #"<<hdu_index<<" @ FILE "<<GetFileName ()<<std::endl;
            
            throw FITSexception(fits_status,"FITSmanager","MoveToHDU",ss.str());
        }
        
        return hdu_type;
    }
    
#pragma endregion
#pragma region • Modifying File
    /**
     *  @details Append image extention to the end of the fits file.
     *  @param img Image data cube to be added to the fits file.
     */
    void FITSmanager::AppendImage(FITScube &img)
    {
        if(fptr.use_count() == 0)
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","GetHeaderAtIndex","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        if(img.GetBitPerPixel() == 0)
        {
            throw std::invalid_argument("\033[31m[FITSmanager::AppendImage]\033[0minvalid BITPIX");
            return;
        }

        fits_status = 0;
        
        if((verbose & verboseLevel::VERBOSE_DETAIL)== verboseLevel::VERBOSE_DETAIL)
            std::cout<<"\033[31m[FITSmanager::AppendImage]\033[0m: Append image of "<<img.GetDimension()<<" axis (";

        std::vector<LONGLONG> axis(img.GetDimension());
        for(size_t i = 0; i < img.GetDimension(); i++)
        {
            axis[i] = static_cast<LONGLONG>(img.Size(i+1));
            
            if((verbose & verboseLevel::VERBOSE_DETAIL)== verboseLevel::VERBOSE_DETAIL)
            {
                std::cout<<axis[i];
                if(i != img.GetDimension()-1)
                    std::cout<<",";
                else
                    std::cout<<")";
            }
        }
        
        if(fits_create_imgll(fptr.get(), img.GetBitPerPixel(), static_cast<int>( img.GetDimension() ), axis.data(),  &fits_status ))
        {
            throw FITSexception(fits_status,"FITSmanager","AppendImage");
        }
                
        num_hdu++;
        img.Write(fptr);
    }
    
#pragma endregion
#pragma region • Modifying specific header keyword   
    
    void FITSmanager::AppendKeyToPrimary(std::string key, const FITSkeyword &val)
    {
        AppendKeyToHeader(1, key, val);
    }
    
    void FITSmanager::AppendKeyToHeader(int HDU, std::string key, const FITSkeyword &val)
    {
        char *comment = NULL;
        fits_status = 0;

        if(val.comment().size() > 0 )
            comment = const_cast<char*>(val.comment().c_str());
        
        try
        {
        switch (val.type()) {
            case fChar :
                if(val.value().size() < 68)
                {
                    if(ffukys(fptr.get(), const_cast<char*>( key.c_str() ), const_cast<char*>( val.value().c_str() ), comment, &fits_status ))
                    {
                        throw FITSexception(fits_status,"FITSmanager","AppendKey");
                    }
                }
                else
                {
                    if(ffukys(fptr.get(), const_cast<char*>( key.c_str() ), const_cast<char*>( val.value().c_str() ), comment, &fits_status ))
                    {
                        throw FITSexception(fits_status,"FITSmanager","AppendKey");
                    }
                }
                break;
                
            case fShort :
                AppendKeyToHeader(HDU, key, TSHORT, val.value(), val.comment());
                break;
                
            case fInt :
                AppendKeyToHeader(HDU, key, TINT, val.value(), val.comment());
                break;
                
            case fLong :
                AppendKeyToHeader(HDU, key, TLONG, val.value(), val.comment());
                break;
                
            case fLongLong :
                AppendKeyToHeader(HDU, key, TLONGLONG, val.value(), val.comment());
                break;
                
            case fBool :
                AppendKeyToHeader(HDU, key, TBYTE, val.value(), val.comment());
                break;
                
            case fFloat :
                AppendKeyToHeader(HDU, key, TFLOAT, val.value(), val.comment());
                break;
                
            case fDouble :
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
        if(num_hdu == 0)
        {
            fits_status = BAD_HDU_NUM;
            throw FITSexception(fits_status,"FITSmanager","AppendKeyToHeader","FILE "+GetFileName()+" do not yet contains HDU blocks.");
        }

        try
        {
            if(HDU > num_hdu)
            {
                throw FITSwarning("FITSmanager","AppendKeyToHeader","FILE "+
                                  GetFileName()
                                  +" do not yet contains HDU blocks.\n     Keyword string will be added to PRIMARY HDU block in memory.");
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
        fits_status = 0;
        
        if(fptr.use_count() == 0)
        {
            fits_status = SHARED_NULPTR;
            throw FITSexception(fits_status,"FITSmanager","AppendKey","CAN'T GET HEADER FROM NULL POINTER");
        }
        
        char * comment = NULL;
        if(cmt.size() > 1)
            comment = const_cast<char*>( cmt.c_str() );
                
        try
        {
            if(TYPE == TFLOAT)
            {
                float dval = std::stof(val);
                
                if(fits_update_key_fixflt(fptr.get(), const_cast<char*>( key.c_str() ),  dval, 8, comment, &fits_status ))
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TDOUBLE)
            {
                double dval = std::stod(val);
                
                if(fits_update_key_fixdbl(fptr.get(), const_cast<char*>( key.c_str() ),  dval, 14, comment, &fits_status ))
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TSHORT  ||
                    TYPE == TINT    ||
                    TYPE == TUINT     )
            {
                int dval = std::stol(val);
                
                if(fits_update_key_log(fptr.get(), const_cast<char*>( key.c_str() ),  dval, comment, &fits_status ))
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
            else if(TYPE == TLONG   ||
                    TYPE == TULONG  ||
                    TYPE == TLONGLONG )
            {
                LONGLONG dval = std::stoll(val);
                
                if(fits_update_key_lng(fptr.get(), const_cast<char*>( key.c_str() ),  dval, comment, &fits_status ))
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }   
            }
             else
            {
                if(fits_update_key(fptr.get(), TYPE, const_cast<char*>( key.c_str() ), const_cast<char*>( val.c_str() ), comment, &fits_status ))
                {
                    throw FITSexception(fits_status,"FITSmanager","AppendKey");
                }
            }
        }
        catch(std::exception& e)
        {
            if((verbose & verboseLevel::VERBOSE_BASIC)== verboseLevel::VERBOSE_BASIC)
                std::cerr<<e.what()<<std::flush;
            throw;
        }
    }

#pragma endregion
#pragma endregion
    
}

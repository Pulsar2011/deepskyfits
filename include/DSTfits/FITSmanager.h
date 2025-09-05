//
//  FITSmanager.h
//  DeepSkyLib
//
//  Created by GILLARD William on 01/04/15.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#ifndef _DSL_FITSmanager_
#define _DSL_FITSmanager_


#include <fitsio.h>

#include <limits>
#if __cplusplus < 201103L
#include <memory>
#endif

#include "FITShdu.h"
#include "FITSimg.h"
#include "FITStable.h"


namespace DSL
{
    /**
     *  @namespace DSL
     *  @brief DeepSkyLibrary namespace
     *  @details DSL namespace stand for DeepSkyLibrary. This namespace regroup a collection of tools to manage and process data in FITS files.
     */
#pragma mark - FITSmanager class definition
    class FITSmanager
    {
        /**
         * @class DSL::FITSmanager FITSmanager.h "fitsExtractor/FITSmanager.h"
         * @author GILLARD William
         * @version 1.0
         * @date 31/03/2015
         * @brief FITS file manager
         * @details FITSmanager manage FITS file and access to the different HDU stored into the FITS file.
         */
    private:
#if __cplusplus < 201103L
        fitsfile *fptr;                                     //!< Pointer to a fits file.
#else
        std::shared_ptr<fitsfile> fptr;
        static void CloseFile(fitsfile*);
#endif
        
        int fits_status;                                    //!< FITS access status
        int num_hdu;                                        //!< Number of hdu in the fitsfile

#pragma mark • Initialization
        void init();                                        //<! Initialize to default value
        void explore();                                     //<! Explore fits pointer and retrive basic information on its content
        
    public:
        
#pragma mark • ctor/dtor
        FITSmanager();                                      //!< Default constructor
        FITSmanager(std::string, bool readOnly = true);     //!< Construct with FITS file name
        FITSmanager(fitsfile&);                             //!< Construct from an existing FITS file
        FITSmanager(const FITSmanager&);                    //!< Copy constructor
    
        virtual ~FITSmanager();                                     //!< Destructor

#pragma mark • Diagnoze
        inline const int Status() const {return fits_status;}
        
        static const std::string getFileName(fitsfile *);
        
#pragma mark • Accessing FITS file
        static FITSmanager* Open(std::string, bool readOnly = true);
        static FITSmanager* Create(std::string, bool replace = false);
        void OpenFile(std::string, bool readOnly = true);
        
        void Close();
        
#pragma mark • Writing FITS file to disk
        void Write();
        
#if __cplusplus < 201103L
        inline bool isOpen(){return (fptr != NULL);}
#else
        inline bool isOpen(){return (fptr.use_count()>0);}
#endif
        virtual inline void Debug(bool debug = true){FITScube::debug = debug; FITShdu::debug = debug;}
        virtual inline bool isDebug(){return (FITScube::debug || FITShdu::debug);}
        
#pragma mark • Accessing FITS HDU
        inline const int NumberOfHeader(){return num_hdu;}  //!< Get the total number of HDU
        FITShdu *GetPrimaryHeader();                        //!< Retrive primary header
        FITShdu *GetHeaderAtIndex(int);                     //!< Retrive specific HDU
        
#pragma mark • Accessing FITS image
        FITScube *GetPrimary();
        FITScube *GetImageAtIndex(int);
        
#pragma mark • Accessing FITS table
        FITStable *GetTableAtIndex(int);
        FITStable *GetTable(std::string);
        FITStable *CreateTable(std::string, const ttype&);
        
#pragma mark • Accessing file block
#if __cplusplus < 201103L
        inline const fitsfile& pCurrentHDU() const {return *fptr;}
        inline       fitsfile* CurrentHDU()  const {return fptr;}
#else
        inline const std::shared_ptr<fitsfile> pCurrentHDU() const {return fptr;}
        inline       std::shared_ptr<fitsfile> CurrentHDU()  const {return fptr;}
#endif
        
        
        int MoveToHDU(int);
        int MoveToPrimary();
        
        
#pragma mark • Modifying File
        void AppendImage(FITScube &);

#pragma mark • Modifying specific header keyword        
        void AppendKeyToPrimary(std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, int, std::string, std::string cmt = "");
        
        void AppendKey(std::string, int, std::string, std::string cmt = "");
        
    };
}

#endif /* defined(__DeepSkyLib__FITSmanager__) */

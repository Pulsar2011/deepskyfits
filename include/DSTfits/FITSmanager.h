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
#include <iostream> // Add this for std::ostream

#include "FITShdu.h"
#include "FITSimg.h"
#include "FITStable.h"
#include "DSF_version.h"


namespace DSL
{

    /**
     *  @namespace DSL
     *  @brief DeepSkyLibrary namespace
     *  @details DSL namespace stand for DeepSkyLibrary. This namespace regroup a collection of tools to manage and process data in FITS files.
     */
#pragma region - FITSmanager class definition
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
        std::shared_ptr<fitsfile> fptr;
        static void CloseFile(fitsfile*);
        
        int num_hdu;                                        //!< Number of hdu in the fitsfile
        int fits_status;                                    //!< FITS access status

#pragma region • Initialization
        void explore();                                     //<! Explore fits pointer and retrive basic information on its content
        
    public:
        
#pragma endregion
#pragma region • ctor/dtor
        FITSmanager();                                                  //!< Default constructor
        FITSmanager(const std::string&);                                //!< Construct with FITS file name
        FITSmanager(const std::string&, const bool& readOnly);          //!< Construct with FITS file name
        FITSmanager(fitsfile&);                                         //!< Construct from an existing FITS file
        FITSmanager(const std::shared_ptr<fitsfile>&);                  //!< Construct from an existing FITS file
        FITSmanager(const FITSmanager&);                                //!< Copy constructor
    
        virtual ~FITSmanager();                                         //!< Destructor

#pragma endregion
#pragma region • Diagnoze
        inline const int Status() const {return fits_status;}
        
        const std::string GetFileName() const ;
        const std::string GetFileName(fitsfile *) const ;
        const std::string GetFileName(const std::shared_ptr<fitsfile>&) const;

        
#pragma endregion
#pragma region • Accessing FITS file
        FITSmanager Open(const std::string&, bool readOnly = true);
        FITSmanager Create(const std::string&, bool replace=false);
        void OpenFile(const std::string&, bool readOnly = true);
        
        void Close();
        
#pragma endregion
#pragma region • Writing FITS file to disk
        void Write();
        
        inline bool isOpen(){return (fptr.use_count()>0);}
        
#pragma endregion
#pragma region • Accessing FITS HDU
        inline const int NumberOfHeader(){return num_hdu;}              //!< Get the total number of HDU
        const std::shared_ptr<FITShdu> GetPrimaryHeader();              //!< Retrieve primary header
        const std::shared_ptr<FITShdu> GetHeaderAtIndex(const int&);    //!< Retrive specific HDU
        
#pragma endregion
#pragma region • Accessing FITS image
        const std::shared_ptr<FITScube> GetPrimary();
        const std::shared_ptr<FITScube> GetImageAtIndex(const int&);
        
#pragma endregion
#pragma region • Accessing FITS table
        const std::shared_ptr<FITStable> GetTableAtIndex(const int&);
        const std::shared_ptr<FITStable> GetTable(std::string);
        const std::shared_ptr<FITStable> CreateTable(std::string, const ttype&);
        
#pragma endregion
#pragma region • Accessing file block
        inline const std::shared_ptr<fitsfile>& CurrentHDU() const {return fptr;}
        
        int MoveToHDU(const int&);
        int MoveToPrimary();
        
        
#pragma endregion
#pragma region • Modifying File
        void AppendImage(FITScube &);

#pragma endregion
#pragma region • Modifying specific header keyword        
        void AppendKeyToPrimary(std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, int, std::string, std::string cmt = "");
        
        void AppendKey(std::string, int, std::string, std::string cmt = "");
#pragma endregion

    };
#pragma endregion

}

#endif /* defined(__DeepSkyLib__FITSmanager__) */

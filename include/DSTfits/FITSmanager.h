//
//  FITSmanager.h
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


#ifndef _DSL_FITSmanager_
#define _DSL_FITSmanager_


#include <fitsio.h>
#include <limits>
#include <iostream> // Add this for std::ostream
#include <shared_mutex> // added to support thread-safe access to fptr

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
    private:
        std::shared_ptr<fitsfile> fptr;
        mutable std::shared_mutex fptr_mtx;                //!< mutex to protect fptr for thread-safety
        static void CloseFile(fitsfile*);
        
        int num_hdu;                                        //!< Number of hdu in the fitsfile
        int fits_status;                                    //!< FITS access status

#pragma region * Initialization
        void explore();                                     //<! Explore fits pointer and retrive basic information on its content
        
    public:
        FITSmanager();                                                  //!< Default constructor
        FITSmanager(const std::string&);                                //!< Construct with FITS file name
        FITSmanager(const std::string&, const bool& readOnly);          //!< Construct with FITS file name
        FITSmanager(fitsfile&);                                         //!< Construct from an existing FITS file
        FITSmanager(const std::shared_ptr<fitsfile>&);                  //!< Construct from an existing FITS file
        FITSmanager(const FITSmanager&);                                //!< Copy constructor

        // Add assignment operators to allow `fm = FITSmanager::Create(...)`
        FITSmanager& operator=(const FITSmanager& other);               //!< Copy assignment
        FITSmanager& operator=(FITSmanager&& other) noexcept;           //!< Move assignment

        virtual ~FITSmanager();                                         //!< Destructor

        // Query lock state (best-effort, non-blocking)
        // Returns true if an exclusive lock appears to be held by someone (try_lock failed)
        bool IsExclusivelyLocked() const;
        // Returns true if a shared lock can be acquired (no exclusive owner at the moment)
        bool CanAcquireSharedLock() const;
        
#pragma endregion
#pragma region * Diagnoze
        inline const int Status() const {return fits_status;}
        
        const std::string GetFileName() const ;
        const std::string GetFileName(fitsfile *) const ;
        const std::string GetFileName(const std::shared_ptr<fitsfile>&) const;

        
#pragma endregion
#pragma region * Accessing FITS file
        FITSmanager Open(const std::string&, bool readOnly = true);
        static FITSmanager Create(const std::string&, bool replace=false);
        void OpenFile(const std::string&, bool readOnly = true);
        
        void Close();
        
#pragma endregion
#pragma region * Writing FITS file to disk
        void Write();
        
        inline bool isOpen(){ std::shared_lock<std::shared_mutex> lk(fptr_mtx); return (fptr && fptr.use_count()>0); }
        
#pragma endregion
#pragma region * Accessing FITS HDU
        inline const int NumberOfHeader(){return num_hdu;}              //!< Get the total number of HDU
        const std::shared_ptr<FITShdu> GetPrimaryHeader();              //!< Retrieve primary header
        const std::shared_ptr<FITShdu> GetHeaderAtIndex(const int&);    //!< Retrive specific HDU
        
#pragma endregion
#pragma region * Accessing FITS image
        const std::shared_ptr<FITScube> GetPrimary();
        const std::shared_ptr<FITScube> GetImageAtIndex(const int&);
        
#pragma endregion
#pragma region * Accessing FITS table
        const std::shared_ptr<FITStable> GetTableAtIndex(const int&);
        const std::shared_ptr<FITStable> GetTable(std::string);
        const std::shared_ptr<FITStable> CreateTable(std::string, const ttype&);
        
#pragma endregion
#pragma region * Accessing file block
        inline const std::shared_ptr<fitsfile>& CurrentHDU() const {return fptr;}
        
        int MoveToHDU(const int&);
        int MoveToPrimary();
        
        
#pragma endregion
#pragma region * Modifying File
        void AppendImage(FITScube &);

#pragma endregion
#pragma region * Modifying specific header keyword        
        void AppendKeyToPrimary(std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, const FITSkeyword&);
        void AppendKeyToHeader(int HDU, std::string, int, std::string, std::string cmt = "");
        
        void AppendKey(std::string, int, std::string, std::string cmt = "");
#pragma endregion

    };
#pragma endregion

}

#endif /* defined(__DeepSkyLib__FITSmanager__) */


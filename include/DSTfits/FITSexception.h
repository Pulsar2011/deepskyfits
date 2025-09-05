//
//  FITSdata.h
//  DeepSkyLib
//
//  Created by GILLARD William on 07/08/17.
//  Centre de Physic des Particules de Marseille
//	Copyright (c) 2015, All rights reserved
//

#ifndef _DSL_FITSexception_
#define _DSL_FITSexception_

//-- C++ system dependency

#include <stdexcept>
#include <string>

namespace DSL
{
    typedef int fitsError;
    
    class FITSexception:public std::exception
    {
    private:
        const fitsError err;
        const std::string cname;
        const std::string cfun;
        const std::string msg;
        
    public:
        FITSexception(const fitsError&);
        FITSexception(const fitsError&, const std::string &);
        FITSexception(const fitsError&, const std::string &, const std::string &);
        FITSexception(const fitsError&, const std::string &, const std::string &, const std::string&);
        
        const char* what() const noexcept;
        
    };
    
    class FITSwarning:public std::exception
    {
    private:
        const std::string cname;
        const std::string cfun;
        const std::string msg;
        
    public:
        FITSwarning();
        FITSwarning(const std::string &);
        FITSwarning(const std::string &, const std::string &);
        FITSwarning(const std::string &, const std::string &, const std::string&);
        
        const char* what() const noexcept;
        
    };
}

#endif

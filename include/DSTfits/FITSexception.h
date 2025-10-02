//
//  FITSdata.h
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution, 
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


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

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


#include <DSTfits/FITSexception.h>
#include <fitsio.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>


namespace DSL
{
    FITSexception::FITSexception(const fitsError& e):std::exception(),err(e),cname(),cfun(),msg()
    {};
    
    FITSexception::FITSexception(const fitsError& e, const std::string & name):std::exception(),err(e),cname(name),cfun(),msg()
    {};
    
    
    FITSexception::FITSexception(const fitsError& e, const std::string & name, const std::string &fun):std::exception(),err(e),cname(name),cfun(fun),msg()
    {};
    
    FITSexception::FITSexception(const fitsError& e, const std::string & name, const std::string &fun, const std::string &m):std::exception(),err(e),cname(name),cfun(fun), msg(m)
    {};

    const char* FITSexception::what() const noexcept
    {
        //--- redirect stderr to a stream
        fflush(stderr);							//<< All remaining data in buffer get send to stdout
        
        char bigOutBuf[8192];
        FILE * pFile;
        pFile = tmpfile ();

        setvbuf(pFile,bigOutBuf,_IOFBF,8192);
        
        std::string ss = std::string("\n");
        
        ss += "\033[1;35;47m***";
        
        if(cname.size() > 1)
        {
            ss += " ["+cname;
            if(cfun.size() > 1)
                ss += "::"+ cfun;
            ss+="]: ";
        }
        
        ss+="Errors *** ";
        
        if(msg.size() > 1)
            ss+=msg+"\n";
        
        fits_report_error(pFile, err);
        fflush(pFile);
        
        if (bigOutBuf[0] == '\n')
            memmove(bigOutBuf, bigOutBuf+1, strlen(bigOutBuf));
        
        ss += "\033[1;35;47m"+std::string(bigOutBuf)+"\033[0m\n";
        
        setbuf(pFile,NULL);//reset to unnamed buffer
        fclose(pFile);
        fflush(stderr);

        // FIX: Use static buffer to avoid returning pointer to local variable
        static std::string static_ss;
        static_ss = ss;
        return static_ss.c_str();
    }
        
        FITSwarning::FITSwarning():std::exception(),cname(),cfun(),msg()
        {};
        
        FITSwarning::FITSwarning(const std::string &name):std::exception(),cname(name),cfun(),msg()
        {};
        
        FITSwarning::FITSwarning(const std::string &name, const std::string &fun):std::exception(),cname(name),cfun(fun),msg()
        {};
        
        FITSwarning::FITSwarning(const std::string &name, const std::string &fun, const std::string& m):std::exception(),cname(name),cfun(fun),msg(m)
        {};
        
        const char* FITSwarning::what() const noexcept
        {
            std::string stream = std::string();
            
            stream+="\n";
            stream+="\033[37;44mWARNING";
            
            if(cname.size() > 1)
            {
                stream+=" ["+cname;
                if(cfun.size() > 1)
                    stream+="::"+cfun;
                stream+="]: ";
            }
            
            stream+=" !!!\033[0m\n";
            
            if(msg.size() > 1)
                stream+="     "+msg+"\033[0m\n";
            
            stream+="\n";

            static std::string static_stream;
            static_stream = stream;
            return static_stream.c_str();  
        }
        
}

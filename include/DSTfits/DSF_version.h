#ifndef DSF_VERSION_H
#define DSF_VERSION_H

#include <string>
#include <vector>

namespace DSF
{
  class gGIT
  {

#ifdef _gTEST_version_
  friend class version_Tester;
  FRIEND_TEST(version_Tester, test_getVersion);
  FRIEND_TEST(version_Tester, test_readVersion);
  FRIEND_TEST(version_Tester, test_private);
  FRIEND_TEST(version_Tester, test_operator_newer);
  FRIEND_TEST(version_Tester, test_operator_older);
#endif

  private:
   const std::string _VERSION;
   const std::string _VERSION_SHORT;
   const std::string _VERSION_SHA1;
  
   ~gGIT();
   gGIT();
   
   static gGIT *this_ver;

   std::vector<int16_t> getVersion() const;
   std::vector<int16_t> readVersion(const std::string& ver) const;

  public:
    static const std::string version();
    static const std::string version_short();
    static const std::string sha1();

    static bool checkVersion(const std::string& ver) {if(this_ver == NULL)gGIT::this_ver = new gGIT();return (ver.compare(this_ver->_VERSION)==0);}
    static bool checkVersionShort(const std::string& ver) {if(this_ver == NULL)gGIT::this_ver = new gGIT();return (ver.compare(this_ver->_VERSION_SHORT)==0);}
    static bool checkSHA1(const std::string& ver) {if(this_ver == NULL)gGIT::this_ver = new gGIT();return (ver.compare(this_ver->_VERSION_SHA1)==0);}

    bool operator<(const std::string& ver);
    bool operator<=(const std::string& ver);
    bool operator>(const std::string& ver);
    bool operator>=(const std::string& ver);
    bool operator==(const std::string& ver);
    bool operator!=(const std::string& ver);

    static bool isEqual(const std::string& ver)     {if(this_ver == NULL) gGIT::this_ver  = new gGIT(); return (*gGIT::this_ver)==ver;};
    static bool isNotEqual(const std::string& ver)  {if(this_ver == NULL) gGIT::this_ver  = new gGIT(); return (*gGIT::this_ver)!=ver;};
    static bool isOlderThan(const std::string& ver) {if(this_ver == NULL) gGIT::this_ver  = new gGIT(); return (*gGIT::this_ver)< ver;};
    static bool isNewerThan(const std::string& ver) {if(this_ver == NULL) gGIT::this_ver  = new gGIT(); return (*gGIT::this_ver)>=ver;};  
  
  };

}

#endif //DST_VERSION_H

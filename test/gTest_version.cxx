#include "gTest_version.h"
#include <gtest/gtest.h>
#include <DSF_version.h>

using namespace DSF;

namespace DSF
{
    class version_Tester: public DSF::gGIT
    {
        protected:
        public:
            static std::vector<int16_t> version_to_test() {return gGIT::this_ver->getVersion();}
    };

    TEST(version_Tester, test_getVersion)
    {
        gGIT::version();
        std::vector<int16_t> ver = gGIT::this_ver->getVersion();

        ASSERT_EQ(ver.size(), 3);
        ASSERT_EQ(gGIT::isEqual(std::string("v")+std::to_string(ver[0])+std::string(".")+std::to_string(ver[1]) +std::string(".")+std::to_string(ver[2])+std::string("-")+this_VERSION_SHA1), true)<<std::string("v")+std::to_string(ver[0])+std::string(".")+std::to_string(ver[1]) +std::string(".")+std::to_string(ver[2])+std::string("-")+this_VERSION_SHA1<<" vs "<<DSF::gGIT::this_ver->version;
    }

    TEST(version_Tester, test_readVersion)
    {
        gGIT::version();
        std::vector<int16_t> ver = gGIT::this_ver->readVersion(this_VERSION_SHORT);

        ASSERT_EQ(ver.size(), 3);
        ASSERT_EQ(gGIT::isEqual(std::string("v")+std::to_string(ver[0])+std::string(".")+std::to_string(ver[1]) +std::string(".")+std::to_string(ver[2])+std::string("-")+this_VERSION_SHA1), true);
    }

    TEST(version_Tester, test_private)
    {
        gGIT::version();
        std::vector<int16_t> ver0 = gGIT::this_ver->getVersion();
        std::vector<int16_t> ver1 = gGIT::this_ver->readVersion(this_VERSION_SHORT);

        ASSERT_EQ(ver1.size(), ver0.size());
        ASSERT_EQ(ver0[0], ver1[0]);
        ASSERT_EQ(ver0[1], ver1[1]);
        ASSERT_EQ(ver0[2], ver1[2]);
    }

    TEST(version_Tester, test_operator_newer)
    {
        gGIT::version();
        std::vector<int16_t> ver = gGIT::this_ver->readVersion(this_VERSION_SHORT);
        std::vector<int16_t> to_test(ver.size());

        for(size_t k=0; k<ver.size()+2; k++)
        {
            for(size_t i =0; i < ver.size(); i++)
                to_test[i] = ver[i];

            if(k>0 && k<=to_test.size())
                to_test[to_test.size()-k] += 1;
            else if (k > to_test.size())
            {
                to_test[0] +=1;
                to_test[1] +=1;
                to_test[2] +=1;
            }

            std::string newer = std::string("v")+std::to_string(to_test[0])
                              + std::string(".")+std::to_string(to_test[1])
                              + std::string(".")+std::to_string(to_test[2]);            
            
            if(to_test[0] == ver[0] &&
               to_test[1] == ver[1] &&
               to_test[2] == ver[2] )
            {
                ASSERT_EQ((*gGIT::this_ver)< newer,false)<<gGIT::version_short()<<" < "<<newer<<" ?!" <<(ver[0]<to_test[0] )<<"||"<<(ver[1]<to_test[1])<<"||"<<(ver[2]<to_test[2]);
                ASSERT_EQ((*gGIT::this_ver)<=newer,true )<<gGIT::version_short()<<" <= "<<newer<<" ?!"<<(ver[0]<=to_test[0])<<"||"<<(ver[1]<=to_test[1])<<"||"<<(ver[2]<=to_test[2]);

                ASSERT_EQ((*gGIT::this_ver)> newer,false)<<gGIT::version_short()<<" > "<<newer<<" ?!" <<(ver[0]>to_test[0] )<<"||"<<(ver[1]>to_test[1] )<<"||"<<(ver[2]>to_test[2] );
                ASSERT_EQ((*gGIT::this_ver)>=newer,true )<<gGIT::version_short()<<" >= "<<newer<<" ?!"<<(ver[0]>=to_test[0])<<"||"<<(ver[1]>=to_test[1])<<"||"<<(ver[2]>=to_test[2]);
            }
            else
            {
                ASSERT_EQ((*gGIT::this_ver)< newer,true)<<gGIT::version_short()<<" <  "<<newer<<" ?!"<<(ver[0]< to_test[0])<<"||"<<(ver[1]< to_test[1])<<"||"<<(ver[2]< to_test[2]);
                ASSERT_EQ((*gGIT::this_ver)<=newer,true)<<gGIT::version_short()<<" <= "<<newer<<" ?!"<<(ver[0]<=to_test[0])<<"||"<<(ver[1]<=to_test[1])<<"||"<<(ver[2]<=to_test[2]);

                ASSERT_EQ((*gGIT::this_ver)> newer,false)<<gGIT::version_short()<<" > "<<newer<<" ?!"<<(ver[0]> to_test[0])<<"||"<<(ver[1]> to_test[1])<<"||"<<(ver[2]> to_test[2]);
                ASSERT_EQ((*gGIT::this_ver)>=newer,false)<<gGIT::version_short()<<" >="<<newer<<" ?!"<<(ver[0]>=to_test[0])<<"||"<<(ver[1]>=to_test[1])<<"||"<<(ver[2]>=to_test[2]);
            }
        }
    }
    TEST(version_Tester, test_operator_older)
    {
        gGIT::version();
        std::vector<int16_t> ver = gGIT::this_ver->readVersion(this_VERSION_SHORT);
        std::vector<int16_t> to_test(ver.size());

        for(size_t k=0; k<ver.size()+2; k++)
        {
            for(size_t i =0; i < ver.size(); i++)
                to_test[i] = ver[i];
            
            if( k > 0 && k <= to_test.size())
                to_test[to_test.size()-k] -= ((to_test[to_test.size()-k]>0)?1:0);
            else if (k > to_test.size())
            {
                to_test[0] -= ((to_test[0]>0)?1:0);
                to_test[1] -= ((to_test[1]>0)?1:0); if(to_test[0] < 1) to_test[1] ++;
                to_test[2] -= ((to_test[2]>0)?1:0); if(to_test[0] < 1) to_test[2] ++;
            }

            std::string older = std::string("v")+std::to_string(to_test[0])
                              + std::string(".")+std::to_string(to_test[1])
                              + std::string(".")+std::to_string(to_test[2]);
            
            if(to_test[0] == ver[0] &&
               to_test[1] == ver[1] &&
               to_test[2] == ver[2] )
            {
                ASSERT_EQ((*gGIT::this_ver)<older ,false) <<gGIT::version_short()<<" < "<<older<<" ?!" <<(ver[0]<to_test[0] )<<"||"<<(ver[1]<to_test[1])<<"||"<<(ver[2]<to_test[2]);
                ASSERT_EQ((*gGIT::this_ver)<=older,true ) <<gGIT::version_short()<<" <= "<<older<<" ?!"<<(ver[0]<=to_test[0])<<"||"<<(ver[1]<=to_test[1])<<"||"<<(ver[2]<=to_test[2]);

                ASSERT_EQ((*gGIT::this_ver)>older ,false) <<gGIT::version_short()<<" > "<<older<<" ?!" <<(ver[0]>to_test[0] )<<"||"<<(ver[1]>to_test[1] )<<"||"<<(ver[2]>to_test[2] );
                ASSERT_EQ((*gGIT::this_ver)>=older,true ) <<gGIT::version_short()<<" >= "<<older<<" ?!"<<(ver[0]>=to_test[0])<<"||"<<(ver[1]>=to_test[1])<<"||"<<(ver[2]>=to_test[2]);
            }
            else
            {
                int32_t lver = static_cast<int32_t>(std::stoi(std::string(std::to_string(ver[0])+std::to_string(ver[1])+std::to_string(ver[2]))));
                int32_t lto_test = static_cast<int32_t>(std::stoi(std::string(std::to_string(to_test[0])+std::to_string(to_test[1])+std::to_string(to_test[2]))));
                
                if(lto_test<lver)
                {
                    ASSERT_EQ((*gGIT::this_ver)<older, false)<<gGIT::version_short()<<" < "<<older<<" ?!" <<(ver[0]<to_test[0] )<<"||"<<(ver[1]<to_test[1] )<<"||"<<(ver[2]<to_test[2])<<" -> "<<((*gGIT::this_ver)<older);

                    ASSERT_EQ((*gGIT::this_ver)<=older,false)<<gGIT::version_short()<<" <= "<<older<<" ?!"<<(ver[0]<=to_test[0])<<"||"<<(ver[1]<=to_test[1])<<"||"<<(ver[2]<=to_test[2])<<" -> "<<((*gGIT::this_ver)<=older);

                    ASSERT_EQ((*gGIT::this_ver)>older, true)<<gGIT::version_short()<<" > "<<older<<" ?!" <<(ver[0]>to_test[0] )<<"||"<<(ver[1]>to_test[1] )<<"||"<<(ver[2]>to_test[2])<<" -> "<<((*gGIT::this_ver)>older);
                    ASSERT_EQ((*gGIT::this_ver)>=older,true)<<gGIT::version_short()<<" >= "<<older<<" ?!"<<(ver[0]>=to_test[0])<<"||"<<(ver[1]>=to_test[1])<<"||"<<(ver[2]>=to_test[2])<<" -> "<<((*gGIT::this_ver)>=older);
                }
                else
                {
                    ASSERT_EQ((*gGIT::this_ver)<older, true)<<gGIT::version_short()<<" < "<<older<<" ?!" <<(ver[0]<to_test[0] )<<"||"<<(ver[1]<to_test[1] )<<"||"<<(ver[2]<to_test[2])<<" -> "<<((*gGIT::this_ver)<older);

                    ASSERT_EQ((*gGIT::this_ver)<=older,true)<<gGIT::version_short()<<" <= "<<older<<" ?!"<<(ver[0]<=to_test[0])<<"||"<<(ver[1]<=to_test[1])<<"||"<<(ver[2]<=to_test[2])<<" -> "<<((*gGIT::this_ver)<=older);

                    ASSERT_EQ((*gGIT::this_ver)>older, false)<<gGIT::version_short()<<" > "<<older<<" ?!" <<(ver[0]>to_test[0] )<<"||"<<(ver[1]>to_test[1] )<<"||"<<(ver[2]>to_test[2])<<" -> "<<((*gGIT::this_ver)>older);
                    ASSERT_EQ((*gGIT::this_ver)>=older,false)<<gGIT::version_short()<<" >= "<<older<<" ?!"<<(ver[0]>=to_test[0])<<"||"<<(ver[1]>=to_test[1])<<"||"<<(ver[2]>=to_test[2])<<" -> "<<((*gGIT::this_ver)>=older);
                }
            }
        }
    }
}

TEST(test_version, version_equal)
{

    std::string ver=gGIT::version();
    ASSERT_EQ(!ver.compare(this_VERSION), true);
    ASSERT_EQ(gGIT::isEqual(ver), true);
    ASSERT_EQ(gGIT::isEqual(this_VERSION), true);

    ASSERT_EQ(gGIT::isNotEqual(ver), false);
    ASSERT_EQ(gGIT::isNotEqual(this_VERSION), false);

    ASSERT_EQ(gGIT::isEqual(this_VERSION_SHORT), false);
    ASSERT_EQ(gGIT::isEqual(this_VERSION_SHA1), false);

    ASSERT_EQ(gGIT::isNotEqual(this_VERSION_SHORT), true);
    ASSERT_EQ(gGIT::isNotEqual(this_VERSION_SHA1), true);
}

TEST(test_version, acessor)
{
    std::string ver = gGIT::version();
    ASSERT_EQ(!ver.compare(this_VERSION), true);

    std::string ver_short = gGIT::version_short();
    ASSERT_EQ(!ver_short.compare(this_VERSION_SHORT), true);

    std::string sha1 = gGIT::sha1();
    ASSERT_EQ(!sha1.compare(this_VERSION_SHA1), true);

    ASSERT_EQ(gGIT::checkVersion(this_VERSION), true);
    ASSERT_EQ(gGIT::checkVersionShort(this_VERSION_SHORT), true);
    ASSERT_EQ(gGIT::checkSHA1(this_VERSION_SHA1), true);
}

TEST(test_version, version_newer)
{
    ASSERT_EQ(gGIT::isNewerThan(this_VERSION_SHORT), true) <<gGIT::version_short()  <<" newer than "<<this_VERSION_SHORT<<" ? "<<gGIT::isNewerThan(this_VERSION_SHORT);
    ASSERT_EQ(gGIT::isOlderThan(this_VERSION_SHORT), false)<<gGIT::version_short()  <<" older than "<<this_VERSION_SHORT<<" ? "<<gGIT::isOlderThan(this_VERSION_SHORT);

    std::vector<int16_t> ver = DSF::version_Tester::version_to_test();
    ASSERT_EQ(ver.size(),3);

    for(int k=0; k<3; k++)
    {
        std::vector<int16_t> to_test = ver;
        to_test[k] = to_test[k]+1;

        std::string newer = std::string("v")+std::to_string(to_test[0])
                          + std::string(".")+std::to_string(to_test[1])
                          + std::string(".")+std::to_string(to_test[2]);
        ASSERT_EQ(gGIT::isOlderThan(newer),true)<<gGIT::version_short()  <<" older than "<<newer<<" ? "<<gGIT::isOlderThan(newer);
        ASSERT_EQ(gGIT::isNewerThan(newer),false) <<gGIT::version_short()<<" newer than "<<newer<<" ? "<<gGIT::isNewerThan(newer);
    }

    for(int k=0; k<3; k++)
    {
        std::vector<int16_t> to_test = ver;
        to_test[k] = to_test[k]-((to_test[k]>0)?1:0);

        std::string older = std::string("v")+std::to_string(to_test[0])
                          + std::string(".")+std::to_string(to_test[1])
                          + std::string(".")+std::to_string(to_test[2]);
        ASSERT_EQ(gGIT::isOlderThan(older),false) <<gGIT::version_short()<<" older than "<<older<<" ? "<<gGIT::isOlderThan(older);;
        ASSERT_EQ(gGIT::isNewerThan(older),true)  <<gGIT::version_short()<<" newer than "<<older<<" ? "<<gGIT::isNewerThan(older);;
    }
}
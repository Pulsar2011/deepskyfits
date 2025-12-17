#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <DSTfits/FITSmanager.h>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <string>

using namespace DSL;

#ifdef Darwinx86_64
std::string testvurl = "./build/testdata/test_vstringtable.fits";
std::string testurl = "./build/testdata/test_stringtable.fits";
#else
std::string testvurl = "./testdata/test_vstringtable.fits";
std::string testurl = "./testdata/test_stringtable.fits";
#endif

std::string random_string(std::size_t length)
{
    static const char charset[] =
        "0123456789+-="
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
        "abcdefghijklmnopqrstuvwxyz ";
    static thread_local std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<std::size_t> dist(0, sizeof(charset) - 2);

    std::string s;
    s.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        s.push_back(charset[dist(rng)]);
    }
    return s;
}

int rand_3_to_7() {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(3, 7); // inclusive
    return dist(rng);
}

TEST(FITStable,col_tstring_ctor)
{
    FITStable table;
    FITScolumn<std::string> col_str("COL_STR", tstring, "", 1);
    col_str.push_back(std::string("AVION"));
    col_str.push_back(std::string("CARS"));
    col_str.push_back(std::string("COSMOLOGY")); 
    col_str.push_back(std::string("STRING THEORY"));
    col_str.push_back(std::string("GAVITATION"));
    col_str.push_back(std::string("DARK"));
    col_str.push_back(std::string("DARK MATTER"));
    col_str.push_back(std::string("BIG bang"));
    col_str.push_back(std::string("THEORY"));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::string> >(col_str)));
    EXPECT_EQ(table.nrows(), 9u);
    EXPECT_EQ(table.ncols(), 1u);

    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr;
    int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0;
    status=0;
    
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    
    FITStable readTable(fptr,2);
    
    auto* col = dynamic_cast<FITScolumn<std::string>*>(readTable.getColumn(1).get());
    EXPECT_EQ(col->getNelem(),1u);
    EXPECT_EQ(col->getWidth(),13u);
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<std::string>().size(),9u);
    
    EXPECT_EQ(col->values<std::string>()[0],"AVION");
    EXPECT_EQ(col->values<std::string>()[1],"CARS");
    EXPECT_EQ(col->values<std::string>()[2],"COSMOLOGY");
    EXPECT_EQ(col->values<std::string>()[3],"STRING THEORY");
    EXPECT_EQ(col->values<std::string>()[4],"GAVITATION");
    EXPECT_EQ(col->values<std::string>()[5],"DARK");
    EXPECT_EQ(col->values<std::string>()[6],"DARK MATTER");
    EXPECT_EQ(col->values<std::string>()[7],"BIG bang");
    EXPECT_EQ(col->values<std::string>()[8],"THEORY");

    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,col_vstring_ctor)
{
    FITStable table;
    FITScolumn<FITSform::stringVector> col_str("COL_STR", tstring, "", 1);

    uint16_t maxSize = 0;
    uint16_t maxCell = 0;
    for(size_t i=0; i<10; i++)
    {
        std::vector<std::string> colV;
        size_t nElem = static_cast<size_t>(rand_3_to_7());
        maxCell = (static_cast<uint16_t>(nElem)>maxCell)? static_cast<uint16_t>(nElem) : maxCell;
        for(size_t k=0; k<nElem; k++)
        {
            int n = rand_3_to_7();
            colV.push_back(random_string(n));
            
            maxSize = (static_cast<uint16_t>(n)>maxSize)? static_cast<uint16_t>(n) : maxSize;
        }

        col_str.push_back(colV);
    }

    EXPECT_EQ(col_str.getNelem(),maxCell);
    EXPECT_EQ(col_str.getWidth(),maxSize);
    EXPECT_EQ(col_str.getTTYPE(),std::to_string(maxSize*maxCell)+"A"+std::to_string(maxSize));

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::stringVector> >(col_str)));
    EXPECT_EQ(table.nrows(), 10u);
    EXPECT_EQ(table.ncols(), 1u);
    table.setName("VSTRING_TEST");

    EXPECT_NO_THROW(table.write(testvurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr;
    int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testvurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0;
    status=0;
    
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    
    FITStable readTable(fptr,2);
    
    auto* col = dynamic_cast<FITScolumn<FITSform::stringVector>*>(readTable.getColumn(1).get());
    EXPECT_EQ(col->getNelem(),maxCell);
    EXPECT_EQ(col->getWidth(),maxSize);
    EXPECT_EQ(col->getTTYPE(),std::to_string(maxSize*maxCell)+"A"+std::to_string(maxCell));
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<FITSform::stringVector>().size(),10u);

    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,col_vstring_update)
{
    FITSmanager testfits(testvurl,false);
    const std::shared_ptr<FITStable> updateTable = testfits.GetTable("VSTRING_TEST");
    
    auto* col = dynamic_cast<FITScolumn<FITSform::stringVector>*>(updateTable->getColumn(1).get());
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<FITSform::stringVector>().size(),10u);

    uint16_t maxSize = col->getWidth();
    uint16_t maxCell = col->getNelem();
    for(size_t i=0; i<10; i++)
    {
        std::vector<std::string> colV;
        size_t nElem = static_cast<size_t>(rand_3_to_7());
        nElem += static_cast<size_t>(static_cast<double>(rand_3_to_7())/2.+0.5);
        
        maxCell = (static_cast<uint16_t>(nElem)>maxCell)? static_cast<uint16_t>(nElem) : maxCell;
        for(size_t k=0; k<nElem; k++)
        {
            int n = rand_3_to_7()+1;
            colV.push_back(random_string(n));
            maxSize = (static_cast<uint16_t>(n)>maxSize)? static_cast<uint16_t>(n) : maxSize;
        }

        updateTable->push_back(col->getName(), colV);
        EXPECT_EQ(col->getNelem(),maxCell);
        EXPECT_EQ(col->getWidth(),maxSize);
    }

    testfits.UpdateTable(updateTable);
    testfits.Write();
    testfits.Close();

    FITSmanager testread(testvurl,false);
    const std::shared_ptr<FITStable> readTable = testread.GetTableAtIndex(2);

    auto* colr = dynamic_cast<FITScolumn<FITSform::stringVector>*>(updateTable->getColumn(1).get());
    EXPECT_EQ(colr->getNelem(),maxCell);
    EXPECT_EQ(colr->getWidth(),maxSize);
    ASSERT_EQ(colr->values<FITSform::stringVector>().size(),20u);
}

TEST(FITStable,col_vstring_update2)
{
    FITSmanager testfits(testvurl,false);
    const std::shared_ptr<FITStable> updateTable = testfits.GetTable("VSTRING_TEST");
    
    auto* col = dynamic_cast<FITScolumn<FITSform::stringVector>*>(updateTable->getColumn(1).get());
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<FITSform::stringVector>().size(),20u);

    uint16_t maxSize = col->getWidth();
    uint16_t maxCell = col->getNelem();
    for(size_t i=0; i<10; i++)
    {
        std::vector<std::string> colV;
        size_t nElem = static_cast<size_t>(rand_3_to_7());
        nElem -=2;
        
        maxCell = (static_cast<uint16_t>(nElem)>maxCell)? static_cast<uint16_t>(nElem) : maxCell;
        for(size_t k=0; k<nElem; k++)
        {
            int n = rand_3_to_7()+rand_3_to_7();
            colV.push_back(random_string(n));
            maxSize = (static_cast<uint16_t>(n)>maxSize)? static_cast<uint16_t>(n) : maxSize;
        }

        updateTable->push_back(col->getName(), colV);
        EXPECT_EQ(col->getNelem(),maxCell);
        EXPECT_EQ(col->getWidth(),maxSize);
    }

    testfits.UpdateTable(updateTable);
    testfits.Write();
    testfits.Close();

    FITSmanager testread(testvurl,false);
    const std::shared_ptr<FITStable> readTable = testread.GetTableAtIndex(2);

    auto* colr = dynamic_cast<FITScolumn<FITSform::stringVector>*>(updateTable->getColumn(1).get());
    EXPECT_EQ(colr->getNelem(),maxCell);
    EXPECT_EQ(colr->getWidth(),maxSize);
    ASSERT_EQ(colr->values<FITSform::stringVector>().size(),30u);
}



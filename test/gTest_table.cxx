#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <DSTfits/FITSmanager.h>
#include <string>
#include <vector>
#include <sstream>

using namespace DSL;

#ifdef Darwinx86_64
std::string testurl = "./build/testdata/test_fitstable.fits";
#else
std::string testurl = "./testdata/test_fitstable.fits";
#endif

// Small helper to validate a freshly constructed empty table header.
static void expect_empty_table_header(const FITStable& table) {
    EXPECT_EQ(table.nrows(), 0);
    EXPECT_EQ(table.ncols(), 0);
    EXPECT_TRUE(table.HDU().Exists("XTENSION"));
    EXPECT_TRUE(table.HDU().Exists("BITPIX"));
    EXPECT_TRUE(table.HDU().Exists("NAXIS"));
    EXPECT_EQ(table.HDU().GetValueForKey("XTENSION"), "BINTABLE");
    EXPECT_EQ(table.HDU().GetInt16ValueForKey("BITPIX"), 8);
    EXPECT_EQ(table.HDU().GetInt16ValueForKey("NAXIS"), 2);
}

// Original reference test kept as-is.
TEST(FITStable,default_tsbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int8_t>    col_sbyte ("COL_SBYTE" , tsbyte   , "", 1);
    col_sbyte.push_back(int8_t(1)); col_sbyte.push_back(int8_t(2)); col_sbyte.push_back(int8_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int8_t> >(col_sbyte)));
    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_EQ(table.ncols(), 1u);
    table.Dump(std::cout);
    EXPECT_NO_THROW(table.write(testurl, 0, true));

    // --- Read-back & value verification (no header keyword checks) ---
    fitsfile* rawFptr = nullptr;
    int status = 0;
    ASSERT_EQ(fits_open_file(&rawFptr, testurl.c_str(), READONLY, &status), 0);
    EXPECT_EQ(status, 0);

    // Wrap in shared_ptr with no-op deleter (manual close below).
    std::shared_ptr<fitsfile> fptr(rawFptr, [](fitsfile*){}); // manual close

    int hdutype = 0;
    status = 0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(), 2, &hdutype, &status), 0);
    FITStable readTable(fptr, 2);

    auto* col = dynamic_cast<FITScolumn<int8_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col, nullptr);
    ASSERT_EQ(col->values<int8_t>().size(), 3u);
    EXPECT_EQ(col->values<int8_t>()[0], int8_t(1));
    EXPECT_EQ(col->values<int8_t>()[1], int8_t(2));
    EXPECT_EQ(col->values<int8_t>()[2], int8_t(3));

    EXPECT_EQ(fits_close_file(fptr.get(), &status), 0);
    EXPECT_EQ(status, 0);
}

// --- Scalar columns as single tests ---
TEST(FITStable,col_tshort_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int16_t> col_short("COL_SHORT", tshort, "", 1);
    col_short.push_back(int16_t(4)); col_short.push_back(int16_t(5)); col_short.push_back(int16_t(6));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int16_t> >(col_short)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    ASSERT_EQ(readTable.nrows(),3u); ASSERT_EQ(readTable.ncols(),1u);
    auto* col = dynamic_cast<FITScolumn<int16_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<int16_t>().size(),3u);
    EXPECT_EQ(col->values<int16_t>()[0],int16_t(4)); EXPECT_EQ(col->values<int16_t>()[1],int16_t(5)); EXPECT_EQ(col->values<int16_t>()[2],int16_t(6));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tushort_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint16_t> col_ushort("COL_USHORT", tushort, "", 1);
    col_ushort.push_back(uint16_t(7)); col_ushort.push_back(uint16_t(8)); col_ushort.push_back(uint16_t(9));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint16_t> >(col_ushort)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    ASSERT_EQ(readTable.nrows(),3u); ASSERT_EQ(readTable.ncols(),1u);
    auto* col = dynamic_cast<FITScolumn<uint16_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<uint16_t>().size(),3u);
    EXPECT_EQ(col->values<uint16_t>()[0],uint16_t(7)); EXPECT_EQ(col->values<uint16_t>()[1],uint16_t(8)); EXPECT_EQ(col->values<uint16_t>()[2],uint16_t(9));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int32_t> col_int16("COL_INT16", tint, "", 1);
    col_int16.push_back(int32_t(-3)); col_int16.push_back(int32_t(0)); col_int16.push_back(int32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
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
    auto* col = dynamic_cast<FITScolumn<int32_t>*>(readTable.getColumn(1).get());

    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<int32_t>().size(),3u);

    EXPECT_EQ(col->values<int32_t>()[0],int32_t(-3));
    EXPECT_EQ(col->values<int32_t>()[1],int32_t(0));
    EXPECT_EQ(col->values<int32_t>()[2],int32_t(3));
    
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tuint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint32_t> col_uint16("COL_UINT16", tuint, "", 1);
    col_uint16.push_back(uint32_t(1)); col_uint16.push_back(uint32_t(2)); col_uint16.push_back(uint32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
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
    
    auto* col = dynamic_cast<FITScolumn<uint32_t>*>(readTable.getColumn(1).get());
    
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<uint32_t>().size(),3u);
    EXPECT_EQ(col->values<uint32_t>()[0],uint32_t(1));
    EXPECT_EQ(col->values<uint32_t>()[1],uint32_t(2));
    EXPECT_EQ(col->values<uint32_t>()[2],uint32_t(3));

    status=0;
    EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,col_tlong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int32_t> col_int32("COL_INT32", tlong, "", 1);
    col_int32.push_back(int32_t(-11)); col_int32.push_back(int32_t(22)); col_int32.push_back(int32_t(33));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<int32_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<int32_t>().size(),3u);
    EXPECT_EQ(col->values<int32_t>()[0],int32_t(-11)); EXPECT_EQ(col->values<int32_t>()[1],int32_t(22)); EXPECT_EQ(col->values<int32_t>()[2],int32_t(33));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tulong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint32_t> col_uint32("COL_UINT32", tulong, "", 1);
    col_uint32.push_back(uint32_t(1)); col_uint32.push_back(uint32_t(2)); col_uint32.push_back(uint32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<uint32_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<uint32_t>().size(),3u);
    EXPECT_EQ(col->values<uint32_t>()[0],uint32_t(1)); EXPECT_EQ(col->values<uint32_t>()[1],uint32_t(2)); EXPECT_EQ(col->values<uint32_t>()[2],uint32_t(3));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tlonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int64_t> col_int64("COL_INT64", tlonglong, "", 1);
    col_int64.push_back(int64_t(-9)); col_int64.push_back(int64_t(0)); col_int64.push_back(int64_t(9));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int64_t> >(col_int64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
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
    auto* col = dynamic_cast<FITScolumn<int64_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<int64_t>().size(),3u);
    EXPECT_EQ(col->values<int64_t>()[0],int64_t(-9));
    EXPECT_EQ(col->values<int64_t>()[1],int64_t(0));
    EXPECT_EQ(col->values<int64_t>()[2],int64_t(9));
    
    status=0;
    EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,col_tulonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint64_t> col_uint64("COL_UINT64", tulonglong, "", 1);
    col_uint64.push_back(uint64_t(9)); col_uint64.push_back(uint64_t(10)); col_uint64.push_back(uint64_t(11));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint64_t> >(col_uint64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<uint64_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<uint64_t>().size(),3u);
    EXPECT_EQ(col->values<uint64_t>()[0],uint64_t(9)); EXPECT_EQ(col->values<uint64_t>()[1],uint64_t(10)); EXPECT_EQ(col->values<uint64_t>()[2],uint64_t(11));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tfloat_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<float> col_float("COL_FLOAT", tfloat, "", 1);
    col_float.push_back(0.f); col_float.push_back(1.f); col_float.push_back(2.f);
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<float> >(col_float)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<float>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<float>().size(),3u);
    EXPECT_FLOAT_EQ(col->values<float>()[0],0.f); EXPECT_FLOAT_EQ(col->values<float>()[1],1.f); EXPECT_FLOAT_EQ(col->values<float>()[2],2.f);
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tdouble_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<double> col_double("COL_DOUBLE", tdouble, "", 1);
    col_double.push_back(-1.0); col_double.push_back(0.5); col_double.push_back(2.5);
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<double> >(col_double)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<double>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<double>().size(),3u);
    EXPECT_DOUBLE_EQ(col->values<double>()[0],-1.0); EXPECT_DOUBLE_EQ(col->values<double>()[1],0.5); EXPECT_DOUBLE_EQ(col->values<double>()[2],2.5);
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<FITSform::complex> col_cplx("COL_CPLX", tcplx, "", 1);
    col_cplx.push_back(FITSform::complex(1.0f,2.0f)); col_cplx.push_back(FITSform::complex(3.0f,4.0f)); col_cplx.push_back(FITSform::complex(5.0f,6.0f));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::complex> >(col_cplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
   
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<FITSform::complex>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<FITSform::complex>().size(),3u);
    EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[0].first,1.0f); EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[0].second,2.0f);
    EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[1].first,3.0f); EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[1].second,4.0f);
    EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[2].first,5.0f); EXPECT_FLOAT_EQ(col->values<FITSform::complex>()[2].second,6.0f);
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tdbcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<FITSform::dblcomplex> col_dcplx("COL_DCPLX", tdbcplx, "", 1);
    col_dcplx.push_back(FITSform::dblcomplex(1.0,2.0)); col_dcplx.push_back(FITSform::dblcomplex(3.0,4.0)); col_dcplx.push_back(FITSform::dblcomplex(5.0,6.0));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::dblcomplex> >(col_dcplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
   
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<FITSform::dblcomplex>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<FITSform::dblcomplex>().size(),3u);
    EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[0].first,1.0); EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[0].second,2.0);
    EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[1].first,3.0); EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[1].second,4.0);
    EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[2].first,5.0); EXPECT_DOUBLE_EQ(col->values<FITSform::dblcomplex>()[2].second,6.0);
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tlogical_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<bool> col_log("COL_LOG", tlogical, "", 1);
    col_log.push_back(true);
    col_log.push_back(false);
    col_log.push_back(true);

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<bool> >(col_log)));
    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_EQ(table.ncols(), 1u);

    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);

    FITStable readTable(fptr,2);
    readTable.Dump(std::cout);
    
    auto* col = dynamic_cast< FITScolumn< bool > *>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);
    EXPECT_TRUE (col->values<bool>()[0]);
    EXPECT_FALSE(col->values<bool>()[1]);
    EXPECT_TRUE (col->values<bool>()[2]);
    
    status=0;
    EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<bool> col_bit("COL_1BIT", tbit, "", 1);
    col_bit.push_back(false);
    col_bit.push_back(true);
    col_bit.push_back(false);

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn< bool > >(col_bit)));
    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_EQ(table.ncols(), 1u);

    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr;
    int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast< FITScolumn< bool > *>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);
    EXPECT_FALSE (col->values<bool>()[0]);
    EXPECT_TRUE  (col->values<bool>()[1]);
    EXPECT_FALSE (col->values<bool>()[2]);
    
    status=0;
    EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint8_t> col_byte("COL_BYTE", tbyte, "", 1);
    col_byte.push_back(uint8_t(0x10)); col_byte.push_back(uint8_t(0x11)); col_byte.push_back(uint8_t(0x12));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint8_t> >(col_byte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<uint8_t>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->size(),3u);
    EXPECT_EQ(col->values<uint8_t>()[0],uint8_t(0x10)); EXPECT_EQ(col->values<uint8_t>()[1],uint8_t(0x11)); EXPECT_EQ(col->values<uint8_t>()[2],uint8_t(0x12));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,col_tstring_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::string> col_str("COL_STR", tstring, "", 1);
    col_str.push_back(std::string("A")); col_str.push_back(std::string("BC")); col_str.push_back(std::string("DEF"));  col_str.push_back(std::string("GHI"));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::string> >(col_str)));
    EXPECT_EQ(table.nrows(), 4u);
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
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<std::string>().size(),4u);
    
    EXPECT_EQ(col->values<std::string>()[0],"A");
    EXPECT_EQ(col->values<std::string>()[1],"BC");
    EXPECT_EQ(col->values<std::string>()[2],"DEF");
    EXPECT_EQ(col->values<std::string>()[3],"GHI");

    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

// --- Vector columns as single tests ---
TEST(FITStable,v_tsbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int8_t>> v_sbyte("V_SBYTE", tsbyte, "", 1);
    v_sbyte.push_back(std::vector<int8_t>{1,2,3}); v_sbyte.push_back(std::vector<int8_t>{4,5,6}); v_sbyte.push_back(std::vector<int8_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int8_t>> >(v_sbyte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<int8_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->size(),3u);
    EXPECT_EQ(col->values<std::vector<int8_t>>()[0],(std::vector<int8_t>{1,2,3}));
    EXPECT_EQ(col->values<std::vector<int8_t>>()[1],(std::vector<int8_t>{4,5,6}));
    EXPECT_EQ(col->values<std::vector<int8_t>>()[2],(std::vector<int8_t>{7,8,9}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint8_t>> v_byte("V_BYTE", tbyte, "", 1);
    v_byte.push_back(std::vector<uint8_t>{0x41,0x42,0x43}); v_byte.push_back(std::vector<uint8_t>{0x44,0x45,0x46}); v_byte.push_back(std::vector<uint8_t>{0x47,0x48,0x49});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint8_t>> >(v_byte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<uint8_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->size(),3u);
    EXPECT_EQ(col->values<std::vector<uint8_t>>()[0],(std::vector<uint8_t>{0x41,0x42,0x43}));
    EXPECT_EQ(col->values<std::vector<uint8_t>>()[1],(std::vector<uint8_t>{0x44,0x45,0x46}));
    EXPECT_EQ(col->values<std::vector<uint8_t>>()[2],(std::vector<uint8_t>{0x47,0x48,0x49}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int32_t>> v_int16("V_INT16", tint, "", 1);
    v_int16.push_back(std::vector<int32_t>{-3,0,3}); v_int16.push_back(std::vector<int32_t>{4,5,6}); v_int16.push_back(std::vector<int32_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
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
    
    auto* col = dynamic_cast<FITScolumn<std::vector<int32_t>>*>(readTable.getColumn(1).get());
    
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->values<std::vector<int32_t>>().size(),3u);
    
    EXPECT_EQ(col->values<std::vector<int32_t>>()[0],(std::vector<int32_t>{-3,0,3}));
    EXPECT_EQ(col->values<std::vector<int32_t>>()[1],(std::vector<int32_t>{4,5,6}));
    EXPECT_EQ(col->values<std::vector<int32_t>>()[2],(std::vector<int32_t>{7,8,9}));
    
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tuint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint32_t>> v_uint16("V_UINT32", tuint, "", 1);
    v_uint16.push_back(std::vector<uint32_t>{1,2,3});
    v_uint16.push_back(std::vector<uint32_t>{4,5,6});
    v_uint16.push_back(std::vector<uint32_t>{7,8,9});
    
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    
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
    
    auto* col = dynamic_cast<FITScolumn<std::vector<uint32_t>>*>(readTable.getColumn(1).get());
    
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<uint32_t>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[0],(std::vector<uint32_t>{1,2,3}));
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[1],(std::vector<uint32_t>{4,5,6}));
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[2],(std::vector<uint32_t>{7,8,9}));
    
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tlong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int32_t>> v_int32("V_INT32", tlong, "", 1);
    v_int32.push_back(std::vector<int32_t>{-10,0,10}); v_int32.push_back(std::vector<int32_t>{11,12,13}); v_int32.push_back(std::vector<int32_t>{14,15,16});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<int32_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<int32_t>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<int32_t>>()[0],(std::vector<int32_t>{-10,0,10}));
    EXPECT_EQ(col->values<std::vector<int32_t>>()[1],(std::vector<int32_t>{11,12,13}));
    EXPECT_EQ(col->values<std::vector<int32_t>>()[2],(std::vector<int32_t>{14,15,16}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tulong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint32_t>> v_uint32("V_UINT32", tulong, "", 1);
    v_uint32.push_back(std::vector<uint32_t>{1,2,3}); v_uint32.push_back(std::vector<uint32_t>{4,5,6}); v_uint32.push_back(std::vector<uint32_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<uint32_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<uint32_t>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[0],(std::vector<uint32_t>{1,2,3}));
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[1],(std::vector<uint32_t>{4,5,6}));
    EXPECT_EQ(col->values<std::vector<uint32_t>>()[2],(std::vector<uint32_t>{7,8,9}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tlonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int64_t>> v_int64("V_INT64", tlonglong, "", 1);
    v_int64.push_back(std::vector<int64_t>{-9,0,9}); v_int64.push_back(std::vector<int64_t>{10,11,12}); v_int64.push_back(std::vector<int64_t>{13,14,15});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int64_t>> >(v_int64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<int64_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<int64_t>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<int64_t>>()[0],(std::vector<int64_t>{-9,0,9}));
    EXPECT_EQ(col->values<std::vector<int64_t>>()[1],(std::vector<int64_t>{10,11,12}));
    EXPECT_EQ(col->values<std::vector<int64_t>>()[2],(std::vector<int64_t>{13,14,15}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tulonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint64_t>> v_uint64("V_UINT64", tulonglong, "", 1);
    v_uint64.push_back(std::vector<uint64_t>{9,10,11}); v_uint64.push_back(std::vector<uint64_t>{12,13,14}); v_uint64.push_back(std::vector<uint64_t>{15,16,17});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint64_t>> >(v_uint64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<uint64_t>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<uint64_t>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<uint64_t>>()[0],(std::vector<uint64_t>{9,10,11}));
    EXPECT_EQ(col->values<std::vector<uint64_t>>()[1],(std::vector<uint64_t>{12,13,14}));
    EXPECT_EQ(col->values<std::vector<uint64_t>>()[2],(std::vector<uint64_t>{15,16,17}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tfloat_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<float>> v_float("V_FLOAT", tfloat, "", 1);
    v_float.push_back(std::vector<float>{0.f,1.f,2.f}); v_float.push_back(std::vector<float>{3.f,4.f,5.f}); v_float.push_back(std::vector<float>{6.f,7.f,8.f});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<float>> >(v_float)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<float>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<float>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<float>>()[0],(std::vector<float>{0.f,1.f,2.f}));
    EXPECT_EQ(col->values<std::vector<float>>()[1],(std::vector<float>{3.f,4.f,5.f}));
    EXPECT_EQ(col->values<std::vector<float>>()[2],(std::vector<float>{6.f,7.f,8.f}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tdouble_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<double>> v_double("V_DOUBLE", tdouble, "", 1);
    v_double.push_back(std::vector<double>{-1.0,0.5,2.5}); v_double.push_back(std::vector<double>{3.5,4.5,5.5}); v_double.push_back(std::vector<double>{6.5,7.5,8.5});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<double>> >(v_double)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<double>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<double>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<double>>()[0],(std::vector<double>{-1.0,0.5,2.5}));
    EXPECT_EQ(col->values<std::vector<double>>()[1],(std::vector<double>{3.5,4.5,5.5}));
    EXPECT_EQ(col->values<std::vector<double>>()[2],(std::vector<double>{6.5,7.5,8.5}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<FITSform::complex>> v_cplx("V_CPLX", tcplx, "", 1);
    v_cplx.push_back(std::vector<FITSform::complex>{ {1.f,2.f},{3.f,4.f},{5.f,6.f} });
    v_cplx.push_back(std::vector<FITSform::complex>{ {7.f,8.f},{9.f,10.f},{11.f,12.f} });
    v_cplx.push_back(std::vector<FITSform::complex>{ {13.f,14.f},{15.f,16.f},{17.f,18.f} });
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::complex>> >(v_cplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<FITSform::complex>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<FITSform::complex>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<FITSform::complex>>()[0],(std::vector<FITSform::complex>{ {1.f,2.f},{3.f,4.f},{5.f,6.f} }));
    EXPECT_EQ(col->values<std::vector<FITSform::complex>>()[1],(std::vector<FITSform::complex>{ {7.f,8.f},{9.f,10.f},{11.f,12.f} }));
    EXPECT_EQ(col->values<std::vector<FITSform::complex>>()[2],(std::vector<FITSform::complex>{ {13.f,14.f},{15.f,16.f},{17.f,18.f} }));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tdbcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<FITSform::dblcomplex>> v_dcplx("V_DCPLX", tdbcplx, "", 1);
    v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {1.0,2.0},{3.0,4.0},{5.0,6.0} });
    v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {7.0,8.0},{9.0,10.0},{11.0,12.0} });
    v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {13.0,14.0},{15.0,16.0},{17.0,18.0} });
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::dblcomplex>> >(v_dcplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
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
    
    auto* col = dynamic_cast<FITScolumn<std::vector<FITSform::dblcomplex>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->values<std::vector<FITSform::dblcomplex>>().size(),3u);
    EXPECT_EQ(col->values<std::vector<FITSform::dblcomplex>>()[0],(std::vector<FITSform::dblcomplex>{ {1.0,2.0},{3.0,4.0},{5.0,6.0} }));
    EXPECT_EQ(col->values<std::vector<FITSform::dblcomplex>>()[1],(std::vector<FITSform::dblcomplex>{ {7.0,8.0},{9.0,10.0},{11.0,12.0} }));
    EXPECT_EQ(col->values<std::vector<FITSform::dblcomplex>>()[2],(std::vector<FITSform::dblcomplex>{ {13.0,14.0},{15.0,16.0},{17.0,18.0} }));
    
    status=0;
    EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,v_tstring_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<std::string>> v_str("V_STR", tstring, "", 1);
    v_str.push_back(std::vector<std::string>{"A","BC","DEF"});
    v_str.push_back(std::vector<std::string>{"G","HI","JKL"});
    v_str.push_back(std::vector<std::string>{"M","NO","PQR"});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<std::string>> >(v_str)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0; status=0; ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    FITStable readTable(fptr,2);
    auto* col = dynamic_cast<FITScolumn<std::vector<std::string>>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr); ASSERT_EQ(col->size(),3u);
    EXPECT_EQ(col->values<std::vector<std::string>>()[0],(std::vector<std::string>{"A","BC","DEF"}));
    EXPECT_EQ(col->values<std::vector<std::string>>()[1],(std::vector<std::string>{"G","HI","JKL"}));
    EXPECT_EQ(col->values<std::vector<std::string>>()[2],(std::vector<std::string>{"M","NO","PQR"}));
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0); EXPECT_EQ(status,0);
}

TEST(FITStable,v_tlogical_ctor)
{
    FITStable table; expect_empty_table_header(table);

    FITScolumn<FITSform::boolVector> v_log("V_LOG", tlogical, "", 1);
    v_log.push_back(FITSform::boolVector{true,false,true});
    v_log.push_back(FITSform::boolVector{false,true,false});
    v_log.push_back(FITSform::boolVector{true,true,false});
    
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_log)));
    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_EQ(table.ncols(), 1u);
    
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    
    fitsfile* rawFptr=nullptr; int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    int hdutype=0;
    status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);
    
    FITStable readTable(fptr,2);
    
    auto* col = dynamic_cast<FITScolumn<FITSform::boolVector>*>(readTable.getColumn(1).get());
    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);
    EXPECT_TRUE (col->values<FITSform::boolVector>()[0][0]);
    EXPECT_FALSE(col->values<FITSform::boolVector>()[0][1]);
    EXPECT_TRUE (col->values<FITSform::boolVector>()[0][2]);
    EXPECT_FALSE(col->values<FITSform::boolVector>()[1][0]);
    EXPECT_TRUE (col->values<FITSform::boolVector>()[1][1]);
    EXPECT_FALSE(col->values<FITSform::boolVector>()[1][2]);
    EXPECT_TRUE (col->values<FITSform::boolVector>()[2][0]);
    EXPECT_TRUE (col->values<FITSform::boolVector>()[2][1]);
    EXPECT_FALSE(col->values<FITSform::boolVector>()[2][2]);
    
    status=0; EXPECT_EQ(fits_close_file(fptr.get(),&status),0);
    EXPECT_EQ(status,0);
}

TEST(FITStable,v_8tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<FITSform::boolVector> v_bit("V_8BIT", tbit, (int64_t) 8, (int64_t) 1, "", 1);
    
    uint8_t row0 = 0; row0 |= 0x88; uint8_t test0 = 0;
    uint8_t row1 = 0; row1 |= 0xF1; uint8_t test1 = 0;
    uint8_t row2 = 0; row2 |= 0x00; uint8_t test2 = 0;

    std::vector<bool> bv0;
    FITSform::toBoolVector(&bv0, row0);
    FITSform::fromBoolVector(bv0, &test0);
    EXPECT_EQ(row0,test0);
    
    FITSform::boolVector bv1; FITSform::toBoolVector(&bv1, row1); FITSform::fromBoolVector(bv1, &test1); EXPECT_EQ(row1,test1);
    FITSform::boolVector bv2; FITSform::toBoolVector(&bv2, row2); FITSform::fromBoolVector(bv2, &test2); EXPECT_EQ(row2,test2);

    v_bit.push_back(bv0);
    v_bit.push_back(bv1);
    v_bit.push_back(bv2);


    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_bit)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    
    int hdutype=0;
    status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);

    FITStable readTable(fptr,2);

    auto* col = dynamic_cast< FITScolumn< FITSform::boolVector >* >(readTable.getColumn(1).get());

    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);

    // Each row: 8 bits packed in 1 byte
    uint8_t vtest0 = 0;
    uint8_t vtest1 = 0;
    uint8_t vtest2 = 0;

    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[0], &vtest0);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[1], &vtest1);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[2], &vtest2);

    EXPECT_EQ(vtest0, row0);
    EXPECT_EQ(vtest1, row1);
    EXPECT_EQ(vtest2, row2);
}

TEST(FITStable,v_16tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    // 16 bits => 2 bytes
    FITScolumn<FITSform::boolVector> v_bit("V_16BIT", tbit, (int64_t)16, (int64_t)2, "", 1);

    // Little-endian layout: low byte first
    uint16_t row0 = 0x8840; uint16_t test0=0; FITSform::boolVector bv0; FITSform::toBoolVector(&bv0, row0); FITSform::fromBoolVector(bv0, &test0); EXPECT_EQ(row0,test0); 
    uint16_t row1 = 0xF188; uint16_t test1=0; FITSform::boolVector bv1; FITSform::toBoolVector(&bv1, row1); FITSform::fromBoolVector(bv1, &test1); EXPECT_EQ(row1,test1); 
    uint16_t row2 = 0x001A; uint16_t test2=0; FITSform::boolVector bv2; FITSform::toBoolVector(&bv2, row2); FITSform::fromBoolVector(bv2, &test2); EXPECT_EQ(row2,test2); 

    v_bit.push_back(bv0);
    v_bit.push_back(bv1);
    v_bit.push_back(bv2);

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_bit)));

    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    
    int hdutype=0;
    status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);

    FITStable readTable(fptr,2);

    auto* col = dynamic_cast< FITScolumn< FITSform::boolVector >* >(readTable.getColumn(1).get());

    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);

    // Each row: 8 bits packed in 1 byte
    uint16_t vtest0 = 0;
    uint16_t vtest1 = 0;
    uint16_t vtest2 = 0;

    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[0], &vtest0);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[1], &vtest1);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[2], &vtest2);

    EXPECT_EQ(vtest0, row0);
    EXPECT_EQ(vtest1, row1);
    EXPECT_EQ(vtest2, row2);

    
}

TEST(FITStable,v_32tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    // 16 bits => 2 bytes
    FITScolumn<FITSform::boolVector> v_bit("V_32BIT", tbit, (int64_t)32, (int64_t)2, "", 1);

    // Little-endian layout: low byte first
    uint32_t row0 = 0x8840FF; uint32_t test0=0; FITSform::boolVector bv0; FITSform::toBoolVector(&bv0, row0); FITSform::fromBoolVector(bv0, &test0); EXPECT_EQ(row0,test0); 
    uint32_t row1 = 0xF188A2; uint32_t test1=0; FITSform::boolVector bv1; FITSform::toBoolVector(&bv1, row1); FITSform::fromBoolVector(bv1, &test1); EXPECT_EQ(row1,test1); 
    uint32_t row2 = 0x001A00; uint32_t test2=0; FITSform::boolVector bv2; FITSform::toBoolVector(&bv2, row2); FITSform::fromBoolVector(bv2, &test2); EXPECT_EQ(row2,test2); 

    v_bit.push_back(bv0);
    v_bit.push_back(bv1);
    v_bit.push_back(bv2);

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_bit)));

    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    
    int hdutype=0;
    status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);

    FITStable readTable(fptr,2);

    auto* col = dynamic_cast< FITScolumn< FITSform::boolVector >* >(readTable.getColumn(1).get());

    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);

    // Each row: 8 bits packed in 1 byte
    uint32_t vtest0 = 0;
    uint32_t vtest1 = 0;
    uint32_t vtest2 = 0;

    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[0], &vtest0);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[1], &vtest1);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[2], &vtest2);

    EXPECT_EQ(vtest0, row0);
    EXPECT_EQ(vtest1, row1);
    EXPECT_EQ(vtest2, row2);
}

TEST(FITStable,v_64tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    // 16 bits => 2 bytes
    FITScolumn<FITSform::boolVector> v_bit("V_64BIT", tbit, (int64_t)64, (int64_t)2, "", 1);

    // Little-endian layout: low byte first
    uint64_t row0 = 0xC8840FF; uint64_t test0=0; FITSform::boolVector bv0; FITSform::toBoolVector(&bv0, row0); FITSform::fromBoolVector(bv0, &test0); EXPECT_EQ(row0,test0); 
    uint64_t row1 = 0xEF188A2; uint64_t test1=0; FITSform::boolVector bv1; FITSform::toBoolVector(&bv1, row1); FITSform::fromBoolVector(bv1, &test1); EXPECT_EQ(row1,test1); 
    uint64_t row2 = 0x001A00F; uint64_t test2=0; FITSform::boolVector bv2; FITSform::toBoolVector(&bv2, row2); FITSform::fromBoolVector(bv2, &test2); EXPECT_EQ(row2,test2); 

    v_bit.push_back(bv0);
    v_bit.push_back(bv1);
    v_bit.push_back(bv2);

    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_bit)));

    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write(testurl, 0, true));
    
    // Read-back
    fitsfile* rawFptr=nullptr; int status=0;
    
    ASSERT_EQ(fits_open_file(&rawFptr,testurl.c_str(),READONLY,&status),0);
    
    std::shared_ptr<fitsfile> fptr(rawFptr,[](fitsfile*){});
    
    int hdutype=0;
    status=0;
    ASSERT_EQ(fits_movabs_hdu(fptr.get(),2,&hdutype,&status),0);

    FITStable readTable(fptr,2);

    auto* col = dynamic_cast< FITScolumn< FITSform::boolVector >* >(readTable.getColumn(1).get());

    ASSERT_NE(col,nullptr);
    ASSERT_EQ(col->size(),3u);

    // Each row: 8 bits packed in 1 byte
    uint64_t vtest0 = 0;
    uint64_t vtest1 = 0;
    uint64_t vtest2 = 0;

    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[0], &vtest0);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[1], &vtest1);
    FITSform::fromBoolVector(col->values<FITSform::boolVector>()[2], &vtest2);

    EXPECT_EQ(vtest0, row0);
    EXPECT_EQ(vtest1, row1);
    EXPECT_EQ(vtest2, row2);
}

TEST(FITStable,all_columns_single_file)
{
    FITStable table; expect_empty_table_header(table);

    // --- Scalar columns ---
    {
        std::cout<<"Inserting COL_SBYTE columns..."<<std::endl;
        FITScolumn<int8_t>    col_sbyte ("COL_SBYTE" , tsbyte   , "", 1);
        col_sbyte.push_back(int8_t(1)); col_sbyte.push_back(int8_t(2)); col_sbyte.push_back(int8_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int8_t> >(col_sbyte)));
    }
    {
        std::cout<<"Inserting COL_SHORT columns..."<<std::endl;
        FITScolumn<int16_t> col_short("COL_SHORT", tshort, "", 1);
        col_short.push_back(int16_t(4)); col_short.push_back(int16_t(5)); col_short.push_back(int16_t(6));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int16_t> >(col_short)));
    }
    {
        std::cout<<"Inserting COL_USHORT columns..."<<std::endl;
        FITScolumn<uint16_t> col_ushort("COL_USHORT", tushort, "", 1);
        col_ushort.push_back(uint16_t(7)); col_ushort.push_back(uint16_t(8)); col_ushort.push_back(uint16_t(9));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint16_t> >(col_ushort)));
    }
    {
        std::cout<<"Inserting COL_INT16 columns..."<<std::endl;
        FITScolumn<int32_t> col_int16("COL_INT16", tint, "", 1);
        col_int16.push_back(int32_t(-3)); col_int16.push_back(int32_t(0)); col_int16.push_back(int32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int16)));
    }
    {
        std::cout<<"Inserting COL_UINT16 columns..."<<std::endl;
        FITScolumn<uint32_t> col_uint16("COL_UINT16", tuint, "", 1);
        col_uint16.push_back(uint32_t(1)); col_uint16.push_back(uint32_t(2)); col_uint16.push_back(uint32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint16)));
    }
    {
        std::cout<<"Inserting COL_INT32 columns..."<<std::endl;
        FITScolumn<int32_t> col_int32("COL_INT32", tlong, "", 1);
        col_int32.push_back(int32_t(-11)); col_int32.push_back(int32_t(22)); col_int32.push_back(int32_t(33));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int32)));
    }
    {
        std::cout<<"Inserting COL_UINT32 columns..."<<std::endl;
        FITScolumn<uint32_t> col_uint32("COL_UINT32", tulong, "", 1);
        col_uint32.push_back(uint32_t(1)); col_uint32.push_back(uint32_t(2)); col_uint32.push_back(uint32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint32)));
    }
    {
        std::cout<<"Inserting COL_INT64 columns..."<<std::endl;
        FITScolumn<int64_t> col_int64("COL_INT64", tlonglong, "", 1);
        col_int64.push_back(int64_t(-9)); col_int64.push_back(int64_t(0)); col_int64.push_back(int64_t(9));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int64_t> >(col_int64)));
    }
    {
        std::cout<<"Inserting COL_UINT64 columns..."<<std::endl;
        FITScolumn<uint64_t> col_uint64("COL_UINT64", tulonglong, "", 1);
        col_uint64.push_back(uint64_t(9)); col_uint64.push_back(uint64_t(10)); col_uint64.push_back(uint64_t(11));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint64_t> >(col_uint64)));
    }
    {
        std::cout<<"Inserting COL_FLOAT columns..."<<std::endl;
        FITScolumn<float> col_float("COL_FLOAT", tfloat, "", 1);
        col_float.push_back(0.f); col_float.push_back(1.f); col_float.push_back(2.f);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<float> >(col_float)));
    }
    {
        std::cout<<"Inserting COL_DOUBLE columns..."<<std::endl;
        FITScolumn<double> col_double("COL_DOUBLE", tdouble, "", 1);
        col_double.push_back(-1.0); col_double.push_back(0.5); col_double.push_back(2.5);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<double> >(col_double)));
    }
    {
        std::cout<<"Inserting COL_CPLX columns..."<<std::endl;
        FITScolumn<FITSform::complex> col_cplx("COL_CPLX", tcplx, "", 1);
        col_cplx.push_back(FITSform::complex(1.0f,2.0f)); col_cplx.push_back(FITSform::complex(3.0f,4.0f)); col_cplx.push_back(FITSform::complex(5.0f,6.0f));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::complex> >(col_cplx)));
    }
    {
        std::cout<<"Inserting COL_DCPLX columns..."<<std::endl;
        FITScolumn<FITSform::dblcomplex> col_dcplx("COL_DCPLX", tdbcplx, "", 1);
        col_dcplx.push_back(FITSform::dblcomplex(1.0,2.0)); col_dcplx.push_back(FITSform::dblcomplex(3.0,4.0)); col_dcplx.push_back(FITSform::dblcomplex(5.0,6.0));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::dblcomplex> >(col_dcplx)));
    }
    {
        std::cout<<"Inserting COL_LOG columns..."<<std::endl;
        FITScolumn<bool> col_log("COL_LOG", tlogical, "", 1);
        col_log.push_back(true); col_log.push_back(false); col_log.push_back(true);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<bool> >(col_log)));
    }
    {
        std::cout<<"Inserting COL_BIT columns..."<<std::endl;
        FITScolumn<bool> col_bit("COL_BIT", tbit, "", 1);
        col_bit.push_back(false); col_bit.push_back(true); col_bit.push_back(false);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<bool> >(col_bit)));
    }
    {
        std::cout<<"Inserting COL_BYTE columns..."<<std::endl;
        FITScolumn<uint8_t> col_byte("COL_BYTE", tbyte, "", 1);
        col_byte.push_back(uint8_t(0x10)); col_byte.push_back(uint8_t(0x11)); col_byte.push_back(uint8_t(0x12));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint8_t> >(col_byte)));
    }
    {
        std::cout<<"Inserting COL_STR columns..."<<std::endl;
        FITScolumn<std::string> col_str("COL_STR", tstring, "", 1);
        col_str.push_back(std::string("A")); col_str.push_back(std::string("BC")); col_str.push_back(std::string("DEF"));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::string> >(col_str)));
    }

    // --- Vector columns ---
    {
        std::cout<<"Inserting V_SBYTE columns..."<<std::endl;
        FITScolumn<std::vector<int8_t>> v_sbyte("V_SBYTE", tsbyte, "", 1);
        v_sbyte.push_back(std::vector<int8_t>{1,2,3}); v_sbyte.push_back(std::vector<int8_t>{4,5,6}); v_sbyte.push_back(std::vector<int8_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int8_t>> >(v_sbyte)));
    }
    {
        std::cout<<"Inserting V_BYTE columns..."<<std::endl;
        FITScolumn<std::vector<uint8_t>> v_byte("V_BYTE", tbyte, "", 1);
        v_byte.push_back(std::vector<uint8_t>{0x41,0x42,0x43}); v_byte.push_back(std::vector<uint8_t>{0x44,0x45,0x46}); v_byte.push_back(std::vector<uint8_t>{0x47,0x48,0x49});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint8_t>> >(v_byte)));
    }
    {
        std::cout<<"Inserting V_SHORT columns..."<<std::endl;
        FITScolumn<std::vector<int32_t>> v_int16("V_INT16", tint, "", 1);
        v_int16.push_back(std::vector<int32_t>{-3,0,3}); v_int16.push_back(std::vector<int32_t>{4,5,6}); v_int16.push_back(std::vector<int32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int16)));
    }
    {
        std::cout<<"Inserting V_USHORT columns..."<<std::endl;
        FITScolumn<std::vector<uint32_t>> v_uint16("V_UINT16", tuint, "", 1);
        v_uint16.push_back(std::vector<uint32_t>{1,2,3}); v_uint16.push_back(std::vector<uint32_t>{4,5,6}); v_uint16.push_back(std::vector<uint32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint16)));
    }
    {
        std::cout<<"Inserting V_INT32 columns..."<<std::endl;
        FITScolumn<std::vector<int32_t>> v_int32("V_INT32", tlong, "", 1);
        v_int32.push_back(std::vector<int32_t>{-10,0,10}); v_int32.push_back(std::vector<int32_t>{11,12,13}); v_int32.push_back(std::vector<int32_t>{14,15,16});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int32)));
    }
    {
        std::cout<<"Inserting V_UINT32 columns..."<<std::endl;
        FITScolumn<std::vector<uint32_t>> v_uint32("V_UINT32", tulong, "", 1);
        v_uint32.push_back(std::vector<uint32_t>{1,2,3}); v_uint32.push_back(std::vector<uint32_t>{4,5,6}); v_uint32.push_back(std::vector<uint32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint32)));
    }
    {
        std::cout<<"Inserting V_INT64 columns..."<<std::endl;
        FITScolumn<std::vector<int64_t>> v_int64("V_INT64", tlonglong, "", 1);
        v_int64.push_back(std::vector<int64_t>{-9,0,9}); v_int64.push_back(std::vector<int64_t>{10,11,12}); v_int64.push_back(std::vector<int64_t>{13,14,15});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int64_t>> >(v_int64)));
    }
    {
        std::cout<<"Inserting V_UINT64 columns..."<<std::endl;
        FITScolumn<std::vector<uint64_t>> v_uint64("V_UINT64", tulonglong, "", 1);
        v_uint64.push_back(std::vector<uint64_t>{9,10,11}); v_uint64.push_back(std::vector<uint64_t>{12,13,14}); v_uint64.push_back(std::vector<uint64_t>{15,16,17});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint64_t>> >(v_uint64)));
    }
    {
        std::cout<<"Inserting V_FLOAT columns..."<<std::endl;
        FITScolumn<std::vector<float>> v_float("V_FLOAT", tfloat, "", 1);
        v_float.push_back(std::vector<float>{0.f,1.f,2.f}); v_float.push_back(std::vector<float>{3.f,4.f,5.f}); v_float.push_back(std::vector<float>{6.f,7.f,8.f});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<float>> >(v_float)));
    }
    {
        std::cout<<"Inserting V_DOUBLE columns..."<<std::endl;
        FITScolumn<std::vector<double>> v_double("V_DOUBLE", tdouble, "", 1);
        v_double.push_back(std::vector<double>{-1.0,0.5,2.5}); v_double.push_back(std::vector<double>{3.5,4.5,5.5}); v_double.push_back(std::vector<double>{6.5,7.5,8.5});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<double>> >(v_double)));
    }
    {
        std::cout<<"Inserting V_CPLX columns..."<<std::endl;
        FITScolumn<std::vector<FITSform::complex>> v_cplx("V_CPLX", tcplx, "", 1);
        v_cplx.push_back(std::vector<FITSform::complex>{ {1.f,2.f},{3.f,4.f},{5.f,6.f} });
        v_cplx.push_back(std::vector<FITSform::complex>{ {7.f,8.f},{9.f,10.f},{11.f,12.f} });
        v_cplx.push_back(std::vector<FITSform::complex>{ {13.f,14.f},{15.f,16.f},{17.f,18.f} });
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::complex>> >(v_cplx)));
    }
    {
        std::cout<<"Inserting V_DCPLX columns..."<<std::endl;
        FITScolumn<std::vector<FITSform::dblcomplex>> v_dcplx("V_DCPLX", tdbcplx, "", 1);
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {1.0,2.0},{3.0,4.0},{5.0,6.0} });
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {7.0,8.0},{9.0,10.0},{11.0,12.0} });
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {13.0,14.0},{15.0,16.0},{17.0,18.0} });
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::dblcomplex>> >(v_dcplx)));
    }
    {
        std::cout<<"Inserting V_STR columns..."<<std::endl;
        FITScolumn<std::vector<std::string>> v_str("V_STR", tstring, "", 1);
        v_str.push_back(std::vector<std::string>{"A","BC","DEF"});
        v_str.push_back(std::vector<std::string>{"G","HI","JKL"});
        v_str.push_back(std::vector<std::string>{"M","NO","PQR"});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<std::string>> >(v_str)));
    }
    {
        std::cout<<"Inserting V_LOG columns..."<<std::endl;
        FITScolumn<FITSform::boolVector> v_log("V_LOG", tlogical, "", 1);
        v_log.push_back(std::vector<bool>{true,false,true});
        v_log.push_back(std::vector<bool>{false,true,false});
        v_log.push_back(std::vector<bool>{true,true,false});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_log)));
    }
    {
        std::cout<<"Inserting V_3BIT columns..."<<std::endl;
        FITScolumn<FITSform::boolVector> v_bit("V_3BIT", tbit, (int64_t) 3, (int64_t) 1, "", 1);
        v_bit.push_back(std::vector<bool>{true,false,true});
        v_bit.push_back(std::vector<bool>{false,true,false});
        v_bit.push_back(std::vector<bool>{true,true,false});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::boolVector> >(v_bit)));
    }

    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_GT(table.ncols(), 1u);

    EXPECT_NO_THROW(table.write(testurl, 0, true));
}

TEST(FITStable, read_all_columns)
{
    // Ensure the file exists from previous write tests (all_columns_single_file).
    fitsfile* rawFptr = nullptr;
    int status = 0;
    EXPECT_EQ(fits_open_file(&rawFptr, testurl.c_str(), READONLY, &status), 0);
    EXPECT_EQ(status, 0);

    // Wrap in shared_ptr with no-op deleter (manual close below).
    std::shared_ptr<fitsfile> fptr(rawFptr, [](fitsfile*){});

    int hdutype = 0;
    status = 0;
    // Move to first table HDU (primary image is HDU 1, table is HDU 2).
    EXPECT_EQ(fits_movabs_hdu(fptr.get(), 2, &hdutype, &status), 0);

    EXPECT_EQ(hdutype, BINARY_TBL);
    EXPECT_EQ(status, 0);

    FITStable table(fptr, 2);

    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_GT(table.ncols(), 1u);

    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE1"),  "COL_SBYTE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE2"),  "COL_SHORT");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE3"),  "COL_USHORT");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE4"),  "COL_INT16");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE5"),  "COL_UINT16");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE6"),  "COL_INT32");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE7"),  "COL_UINT32");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE8"),  "COL_INT64");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE9"),  "COL_UINT64");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE10"), "COL_FLOAT");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE11"), "COL_DOUBLE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE12"), "COL_CPLX");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE13"), "COL_DCPLX");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE14"), "COL_LOG");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE15"), "COL_BIT");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE16"), "COL_BYTE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE17"), "COL_STR");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE18"), "V_SBYTE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE19"), "V_BYTE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE20"), "V_INT16");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE21"), "V_UINT16");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE22"), "V_INT32");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE23"), "V_UINT32");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE24"), "V_INT64");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE25"), "V_UINT64");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE26"), "V_FLOAT");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE27"), "V_DOUBLE");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE28"), "V_CPLX");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE29"), "V_DCPLX");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE30"), "V_STR");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE31"), "V_LOG");
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE32"), "V_3BIT");

    // --- Verify TSCALE/TZERO propagation for required unsigned/sbyte columns (if keywords exist) ---
    auto expectScaleZero = [&](int colIndex, double expectScale, double expectZero) {
        std::ostringstream ks, kz;
        ks << "TSCAL" << colIndex;
        kz << "TZERO" << colIndex;
        if (table.HDU().Exists(ks.str()))
            EXPECT_DOUBLE_EQ(table.HDU().GetDoubleValueForKey(ks.str()), expectScale);
        if (table.HDU().Exists(kz.str()))
            EXPECT_DOUBLE_EQ(table.HDU().GetDoubleValueForKey(kz.str()), expectZero);
    };

    // Column index mapping from earlier insert order:
    // 1: COL_SBYTE (SBYTE) -> scale=1.0, zero=-128
    expectScaleZero(1, 1.0, -128.0);
    // 3: COL_USHORT (USHORT) -> scale=1.0, zero=32768
    expectScaleZero(3, 1.0, 32768.0);
    // 5: COL_UINT16 (UINT) stored as 32-bit unsigned -> scale=1.0, zero=2147483648
    expectScaleZero(5, 1.0, 2147483648.0);
    // 7: COL_UINT32 (ULONG) -> scale=1.0, zero=2147483648
    expectScaleZero(7, 1.0, 2147483648.0);
    // 9: COL_UINT64 (ULONGLONG) -> scale=1.0, zero=2^63
    expectScaleZero(9, 1.0, static_cast<double>(1ULL << 63));

    // Validate scalar column values
    {
        auto* c1 = dynamic_cast<FITScolumn<int8_t>*>(table.getColumn(1).get());
        ASSERT_NE(c1, nullptr);
        EXPECT_EQ(c1->values<int8_t>().size(), 3u);
        EXPECT_EQ(c1->values<int8_t>()[0], int8_t(1));
        EXPECT_EQ(c1->values<int8_t>()[1], int8_t(2));
        EXPECT_EQ(c1->values<int8_t>()[2], int8_t(3));
    }
    {
        auto* c2 = dynamic_cast<FITScolumn<int16_t>*>(table.getColumn(2).get());
        ASSERT_NE(c2, nullptr);
        EXPECT_EQ(c2->values<int16_t>().size(), 3u);
        EXPECT_EQ(c2->values<int16_t>()[0], int16_t(4));
        EXPECT_EQ(c2->values<int16_t>()[1], int16_t(5));
        EXPECT_EQ(c2->values<int16_t>()[2], int16_t(6));
    }
    {
        auto* c3 = dynamic_cast<FITScolumn<uint16_t>*>(table.getColumn(3).get());
        ASSERT_NE(c3, nullptr);
        EXPECT_EQ(c3->values<uint16_t>().size(), 3u);
        EXPECT_EQ(c3->values<uint16_t>()[0], uint16_t(7));
        EXPECT_EQ(c3->values<uint16_t>()[1], uint16_t(8));
        EXPECT_EQ(c3->values<uint16_t>()[2], uint16_t(9));
    }
    {
        auto* c4 = dynamic_cast<FITScolumn<int32_t>*>(table.getColumn(4).get());
        ASSERT_NE(c4, nullptr);
        EXPECT_EQ(c4->values<int32_t>().size(), 3u);
        EXPECT_EQ(c4->values<int32_t>()[0], int32_t(-3));
        EXPECT_EQ(c4->values<int32_t>()[1], int32_t(0));
        EXPECT_EQ(c4->values<int32_t>()[2], int32_t(3));
    }
    {
        auto* c5 = dynamic_cast<FITScolumn<uint32_t>*>(table.getColumn(5).get());
        ASSERT_NE(c5, nullptr);
        EXPECT_EQ(c5->values<uint32_t>().size(), 3u);
        EXPECT_EQ(c5->values<uint32_t>()[0], uint32_t(1));
        EXPECT_EQ(c5->values<uint32_t>()[1], uint32_t(2));
        EXPECT_EQ(c5->values<uint32_t>()[2], uint32_t(3));
    }
    {
        auto* c6 = dynamic_cast<FITScolumn<int32_t>*>(table.getColumn(6).get());
        ASSERT_NE(c6, nullptr);
        EXPECT_EQ(c6->values<int32_t>().size(), 3u);
        EXPECT_EQ(c6->values<int32_t>()[0], int32_t(-11));
        EXPECT_EQ(c6->values<int32_t>()[1], int32_t(22));
        EXPECT_EQ(c6->values<int32_t>()[2], int32_t(33));
    }
    {
        auto* c7 = dynamic_cast<FITScolumn<uint32_t>*>(table.getColumn(7).get());
        ASSERT_NE(c7, nullptr);
        EXPECT_EQ(c7->values<uint32_t>().size(), 3u);
        EXPECT_EQ(c7->values<uint32_t>()[0], uint32_t(1));
        EXPECT_EQ(c7->values<uint32_t>()[1], uint32_t(2));
        EXPECT_EQ(c7->values<uint32_t>()[2], uint32_t(3));
    }
    {
        auto* c8 = dynamic_cast<FITScolumn<int64_t>*>(table.getColumn(8).get());
        ASSERT_NE(c8, nullptr);
        EXPECT_EQ(c8->values<int64_t>().size(), 3u);
        EXPECT_EQ(c8->values<int64_t>()[0], int64_t(-9));
        EXPECT_EQ(c8->values<int64_t>()[1], int64_t(0));
        EXPECT_EQ(c8->values<int64_t>()[2], int64_t(9));
    }
    {
        auto* c9 = dynamic_cast<FITScolumn<uint64_t>*>(table.getColumn(9).get());
        ASSERT_NE(c9, nullptr);
        EXPECT_EQ(c9->values<uint64_t>().size(), 3u);
        EXPECT_EQ(c9->values<uint64_t>()[0], uint64_t(9));
        EXPECT_EQ(c9->values<uint64_t>()[1], uint64_t(10));
        EXPECT_EQ(c9->values<uint64_t>()[2], uint64_t(11));
    }
    {
        auto* c10 = dynamic_cast<FITScolumn<float>*>(table.getColumn(10).get());
        ASSERT_NE(c10, nullptr);
        EXPECT_EQ(c10->values<float>().size(), 3u);
        EXPECT_FLOAT_EQ(c10->values<float>()[0], 0.f);
        EXPECT_FLOAT_EQ(c10->values<float>()[1], 1.f);
        EXPECT_FLOAT_EQ(c10->values<float>()[2], 2.f);
    }
    {
        auto* c11 = dynamic_cast<FITScolumn<double>*>(table.getColumn(11).get());
        ASSERT_NE(c11, nullptr);
        EXPECT_EQ(c11->values<double>().size(), 3u);
        EXPECT_DOUBLE_EQ(c11->values<double>()[0], -1.0);
        EXPECT_DOUBLE_EQ(c11->values<double>()[1], 0.5);
        EXPECT_DOUBLE_EQ(c11->values<double>()[2], 2.5);
    }
    {
        auto* c12 = dynamic_cast<FITScolumn<FITSform::complex>*>(table.getColumn(12).get());
        ASSERT_NE(c12, nullptr);
        EXPECT_EQ(c12->values<FITSform::complex>().size(), 3u);
        EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[0].first, 1.0f); EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[0].second, 2.0f);
        EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[1].first, 3.0f); EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[1].second, 4.0f);
        EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[2].first, 5.0f); EXPECT_FLOAT_EQ(c12->values<FITSform::complex>()[2].second, 6.0f);
    }
    {
        auto* c13 = dynamic_cast<FITScolumn<FITSform::dblcomplex>*>(table.getColumn(13).get());
        ASSERT_NE(c13, nullptr);
        EXPECT_EQ(c13->size(), 3u);
        EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[0].first, 1.0); EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[0].second, 2.0);
        EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[1].first, 3.0); EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[1].second, 4.0);
        EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[2].first, 5.0); EXPECT_DOUBLE_EQ(c13->values<FITSform::dblcomplex>()[2].second, 6.0);
    }
    {
        auto* c14 = dynamic_cast<FITScolumn<bool>*>(table.getColumn(14).get());
        ASSERT_NE(c14, nullptr);
        ASSERT_EQ(c14->size(), 3u);
        EXPECT_TRUE (c14->values<bool>()[0]);
        EXPECT_FALSE(c14->values<bool>()[1]);
        EXPECT_TRUE (c14->values<bool>()[2]);
    }
    {
        auto* c15 = dynamic_cast<FITScolumn<bool>*>(table.getColumn(15).get());
        ASSERT_NE(c15, nullptr);
        ASSERT_EQ(c15->size(), 3u);

        EXPECT_EQ(c15->values<bool>()[0], false);
        EXPECT_EQ(c15->values<bool>()[1], true);
        EXPECT_EQ(c15->values<bool>()[2], false);
    }
    {
        auto* c16 = dynamic_cast<FITScolumn<uint8_t>*>(table.getColumn(16).get());
        ASSERT_NE(c16, nullptr);
        EXPECT_EQ(c16->size(), 3u);
        EXPECT_EQ(c16->values<uint8_t>()[0], uint8_t(0x10));
        EXPECT_EQ(c16->values<uint8_t>()[1], uint8_t(0x11));
        EXPECT_EQ(c16->values<uint8_t>()[2], uint8_t(0x12));
    }
    {
        auto* c17 = dynamic_cast<FITScolumn<std::string>*>(table.getColumn(17).get());
        ASSERT_NE(c17, nullptr);
        EXPECT_EQ(c17->size(), 3u);
        EXPECT_EQ(c17->values<std::string>()[0], std::string("A"));
        EXPECT_EQ(c17->values<std::string>()[1], std::string("BC"));
        EXPECT_EQ(c17->values<std::string>()[2], std::string("DEF"));
    }

    // Validate vector column values (per row)
    {
        auto* v18 = dynamic_cast<FITScolumn<std::vector<int8_t>>*>(table.getColumn(18).get());
        ASSERT_NE(v18, nullptr);
        ASSERT_EQ(v18->size(), 3u);
        EXPECT_EQ(v18->values<std::vector<int8_t>>()[0], (std::vector<int8_t>{1,2,3}));
        EXPECT_EQ(v18->values<std::vector<int8_t>>()[1], (std::vector<int8_t>{4,5,6}));
        EXPECT_EQ(v18->values<std::vector<int8_t>>()[2], (std::vector<int8_t>{7,8,9}));
    }
    {
        auto* v19 = dynamic_cast<FITScolumn<std::vector<uint8_t>>*>(table.getColumn(19).get());
        ASSERT_NE(v19, nullptr);
        ASSERT_EQ(v19->size(), 3u);
        EXPECT_EQ(v19->values<std::vector<uint8_t>>()[0], (std::vector<uint8_t>{0x41,0x42,0x43}));
        EXPECT_EQ(v19->values<std::vector<uint8_t>>()[1], (std::vector<uint8_t>{0x44,0x45,0x46}));
        EXPECT_EQ(v19->values<std::vector<uint8_t>>()[2], (std::vector<uint8_t>{0x47,0x48,0x49}));
    }
    {
        auto* v20 = dynamic_cast<FITScolumn<std::vector<int32_t>>*>(table.getColumn(20).get());
        ASSERT_NE(v20, nullptr);
        ASSERT_EQ(v20->size(), 3u);
        EXPECT_EQ(v20->values<std::vector<int32_t>>()[0], (std::vector<int32_t>{-3,0,3}));
        EXPECT_EQ(v20->values<std::vector<int32_t>>()[1], (std::vector<int32_t>{4,5,6}));
        EXPECT_EQ(v20->values<std::vector<int32_t>>()[2], (std::vector<int32_t>{7,8,9}));
    }
    {
        auto* v21 = dynamic_cast<FITScolumn<std::vector<uint32_t>>*>(table.getColumn(21).get());
        ASSERT_NE(v21, nullptr);
        ASSERT_EQ(v21->size(), 3u);
        EXPECT_EQ(v21->values<std::vector<uint32_t>>()[0], (std::vector<uint32_t>{1,2,3}));
        EXPECT_EQ(v21->values<std::vector<uint32_t>>()[1], (std::vector<uint32_t>{4,5,6}));
        EXPECT_EQ(v21->values<std::vector<uint32_t>>()[2], (std::vector<uint32_t>{7,8,9}));
    }
    {
        auto* v22 = dynamic_cast<FITScolumn<std::vector<int32_t>>*>(table.getColumn(22).get());
        ASSERT_NE(v22, nullptr);
        ASSERT_EQ(v22->size(), 3u);
        EXPECT_EQ(v22->values<std::vector<int32_t>>()[0], (std::vector<int32_t>{-10,0,10}));
        EXPECT_EQ(v22->values<std::vector<int32_t>>()[1], (std::vector<int32_t>{11,12,13}));
        EXPECT_EQ(v22->values<std::vector<int32_t>>()[2], (std::vector<int32_t>{14,15,16}));
    }
    {
        auto* v23 = dynamic_cast<FITScolumn<std::vector<uint32_t>>*>(table.getColumn(23).get());
        ASSERT_NE(v23, nullptr);
        ASSERT_EQ(v23->size(), 3u);
        EXPECT_EQ(v23->values<std::vector<uint32_t>>()[0], (std::vector<uint32_t>{1,2,3}));
        EXPECT_EQ(v23->values<std::vector<uint32_t>>()[1], (std::vector<uint32_t>{4,5,6}));
        EXPECT_EQ(v23->values<std::vector<uint32_t>>()[2], (std::vector<uint32_t>{7,8,9}));
    }
    {
        auto* v24 = dynamic_cast<FITScolumn<std::vector<int64_t>>*>(table.getColumn(24).get());
        ASSERT_NE(v24, nullptr);
        ASSERT_EQ(v24->size(), 3u);
        EXPECT_EQ(v24->values<std::vector<int64_t>>()[0], (std::vector<int64_t>{-9,0,9}));
        EXPECT_EQ(v24->values<std::vector<int64_t>>()[1], (std::vector<int64_t>{10,11,12}));
        EXPECT_EQ(v24->values<std::vector<int64_t>>()[2], (std::vector<int64_t>{13,14,15}));
    }
    {
        auto* v25 = dynamic_cast<FITScolumn<std::vector<uint64_t>>*>(table.getColumn(25).get());
        ASSERT_EQ(v25->size(), 3u);
        EXPECT_EQ(v25->values<std::vector<uint64_t>>()[0], (std::vector<uint64_t>{9,10,11}));
        EXPECT_EQ(v25->values<std::vector<uint64_t>>()[1], (std::vector<uint64_t>{12,13,14}));
        EXPECT_EQ(v25->values<std::vector<uint64_t>>()[2], (std::vector<uint64_t>{15,16,17}));
    }
    {
        auto* v26 = dynamic_cast<FITScolumn<std::vector<float>>*>(table.getColumn(26).get());
        ASSERT_NE(v26, nullptr);
        ASSERT_EQ(v26->size(), 3u);
        EXPECT_EQ(v26->values<std::vector<float>>()[0], (std::vector<float>{0.f,1.f,2.f}));
        EXPECT_EQ(v26->values<std::vector<float>>()[1], (std::vector<float>{3.f,4.f,5.f}));
        EXPECT_EQ(v26->values<std::vector<float>>()[2], (std::vector<float>{6.f,7.f,8.f}));
    }
    {
        auto* v27 = dynamic_cast<FITScolumn<std::vector<double>>*>(table.getColumn(27).get());
        ASSERT_NE(v27, nullptr);
        ASSERT_EQ(v27->size(), 3u);
        EXPECT_EQ(v27->values<std::vector<double> >()[0], (std::vector<double>{-1.0,0.5,2.5}));
        EXPECT_EQ(v27->values<std::vector<double> >()[1], (std::vector<double>{3.5,4.5,5.5}));
        EXPECT_EQ(v27->values<std::vector<double> >()[2], (std::vector<double>{6.5,7.5,8.5}));
    }
    {
        auto* v28 = dynamic_cast<FITScolumn<std::vector<FITSform::complex>>*>(table.getColumn(28).get());
        ASSERT_NE(v28, nullptr);
        ASSERT_EQ(v28->size(), 3u);
        EXPECT_EQ(v28->values<std::vector<FITSform::complex>>()[0], (std::vector<FITSform::complex>{ {1.f,2.f},{3.f,4.f},{5.f,6.f} }));
        EXPECT_EQ(v28->values<std::vector<FITSform::complex>>()[1], (std::vector<FITSform::complex>{ {7.f,8.f},{9.f,10.f},{11.f,12.f} }));
        EXPECT_EQ(v28->values<std::vector<FITSform::complex>>()[2], (std::vector<FITSform::complex>{ {13.f,14.f},{15.f,16.f},{17.f,18.f} }));
    }
    {
        auto* v29 = dynamic_cast<FITScolumn<std::vector<FITSform::dblcomplex>>*>(table.getColumn(29).get());
        ASSERT_NE(v29, nullptr);
        ASSERT_EQ(v29->size(), 3u);
        EXPECT_EQ(v29->values<std::vector<FITSform::dblcomplex>>()[0], (std::vector<FITSform::dblcomplex>{ {1.0,2.0},{3.0,4.0},{5.0,6.0} }));
        EXPECT_EQ(v29->values<std::vector<FITSform::dblcomplex>>()[1], (std::vector<FITSform::dblcomplex>{ {7.0,8.0},{9.0,10.0},{11.0,12.0} }));
        EXPECT_EQ(v29->values<std::vector<FITSform::dblcomplex>>()[2], (std::vector<FITSform::dblcomplex>{ {13.0,14.0},{15.0,16.0},{17.0,18.0} }));
    }
    {
        auto* v30 = dynamic_cast<FITScolumn<std::vector<std::string>>*>(table.getColumn(30).get());
        ASSERT_NE(v30, nullptr);
        ASSERT_EQ(v30->size(), 3u);
        EXPECT_EQ(v30->values<std::vector<std::string>>()[0], (std::vector<std::string>{"A","BC","DEF"}));
        EXPECT_EQ(v30->values<std::vector<std::string>>()[1], (std::vector<std::string>{"G","HI","JKL"}));
        EXPECT_EQ(v30->values<std::vector<std::string>>()[2], (std::vector<std::string>{"M","NO","PQR"}));
    }
    {
        auto* v31 = dynamic_cast<FITScolumn< std::vector<bool> >*>(table.getColumn(31).get());
        ASSERT_NE(v31, nullptr);
        ASSERT_EQ(v31->size(), 3u);
        ASSERT_EQ(v31->values<std::vector<bool>>()[0].size(), 3u);
        ASSERT_EQ(v31->values<std::vector<bool>>()[1].size(), 3u);
        ASSERT_EQ(v31->values<std::vector<bool>>()[2].size(), 3u);

        EXPECT_TRUE (v31->values<std::vector<bool>>()[0][0]); EXPECT_FALSE(v31->values<std::vector<bool>>()[0][1]); EXPECT_TRUE (v31->values<std::vector<bool>>()[0][2]);
        EXPECT_FALSE(v31->values<std::vector<bool>>()[1][0]); EXPECT_TRUE (v31->values<std::vector<bool>>()[1][1]); EXPECT_FALSE(v31->values<std::vector<bool>>()[1][2]);
        EXPECT_TRUE (v31->values<std::vector<bool>>()[2][0]); EXPECT_TRUE (v31->values<std::vector<bool>>()[2][1]); EXPECT_FALSE(v31->values<std::vector<bool>>()[2][2]);
    }
    {
        auto* v32 = dynamic_cast< FITScolumn< std::vector<bool> >*>(table.getColumn(32).get());
        ASSERT_NE(v32, nullptr);
        ASSERT_EQ(v32->size(), 3u);

        EXPECT_TRUE (v32->values<std::vector<bool> >()[0][0]); EXPECT_FALSE(v32->values<std::vector<bool> >()[0][1]); EXPECT_TRUE (v32->values<std::vector<bool> >()[0][2]);
        EXPECT_FALSE(v32->values<std::vector<bool> >()[1][0]); EXPECT_TRUE (v32->values<std::vector<bool> >()[1][1]); EXPECT_FALSE(v32->values<std::vector<bool> >()[1][2]);
        EXPECT_TRUE (v32->values<std::vector<bool> >()[2][0]); EXPECT_TRUE (v32->values<std::vector<bool> >()[2][1]); EXPECT_FALSE(v32->values<std::vector<bool> >()[2][2]);
    }

    status = 0;
    EXPECT_EQ(fits_close_file(fptr.get(), &status), 0);
    EXPECT_EQ(status, 0);
}



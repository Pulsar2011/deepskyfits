#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <DSTfits/FITSmanager.h>
#include <string>
#include <vector>
#include <sstream>

using namespace DSL;

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
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

// --- Scalar columns as single tests ---
TEST(FITStable,col_tshort_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int16_t> col_short("COL_SHORT", tshort, "", 1);
    col_short.push_back(int16_t(4)); col_short.push_back(int16_t(5)); col_short.push_back(int16_t(6));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int16_t> >(col_short)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tushort_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint16_t> col_ushort("COL_USHORT", tushort, "", 1);
    col_ushort.push_back(uint16_t(7)); col_ushort.push_back(uint16_t(8)); col_ushort.push_back(uint16_t(9));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint16_t> >(col_ushort)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int32_t> col_int16("COL_INT16", tint, "", 1);
    col_int16.push_back(int32_t(-3)); col_int16.push_back(int32_t(0)); col_int16.push_back(int32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tuint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint32_t> col_uint16("COL_UINT16", tuint, "", 1);
    col_uint16.push_back(uint32_t(1)); col_uint16.push_back(uint32_t(2)); col_uint16.push_back(uint32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tlong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int32_t> col_int32("COL_INT32", tlong, "", 1);
    col_int32.push_back(int32_t(-11)); col_int32.push_back(int32_t(22)); col_int32.push_back(int32_t(33));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tulong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint32_t> col_uint32("COL_UINT32", tulong, "", 1);
    col_uint32.push_back(uint32_t(1)); col_uint32.push_back(uint32_t(2)); col_uint32.push_back(uint32_t(3));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tlonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<int64_t> col_int64("COL_INT64", tlonglong, "", 1);
    col_int64.push_back(int64_t(-9)); col_int64.push_back(int64_t(0)); col_int64.push_back(int64_t(9));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int64_t> >(col_int64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tulonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint64_t> col_uint64("COL_UINT64", tulonglong, "", 1);
    col_uint64.push_back(uint64_t(9)); col_uint64.push_back(uint64_t(10)); col_uint64.push_back(uint64_t(11));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint64_t> >(col_uint64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tfloat_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<float> col_float("COL_FLOAT", tfloat, "", 1);
    col_float.push_back(0.f); col_float.push_back(1.f); col_float.push_back(2.f);
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<float> >(col_float)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tdouble_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<double> col_double("COL_DOUBLE", tdouble, "", 1);
    col_double.push_back(-1.0); col_double.push_back(0.5); col_double.push_back(2.5);
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<double> >(col_double)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<FITSform::complex> col_cplx("COL_CPLX", tcplx, "", 1);
    col_cplx.push_back(FITSform::complex(1.0f,2.0f)); col_cplx.push_back(FITSform::complex(3.0f,4.0f)); col_cplx.push_back(FITSform::complex(5.0f,6.0f));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::complex> >(col_cplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tdbcplx_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<FITSform::dblcomplex> col_dcplx("COL_DCPLX", tdbcplx, "", 1);
    col_dcplx.push_back(FITSform::dblcomplex(1.0,2.0)); col_dcplx.push_back(FITSform::dblcomplex(3.0,4.0)); col_dcplx.push_back(FITSform::dblcomplex(5.0,6.0));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::dblcomplex> >(col_dcplx)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tlogical_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<char*> col_log("COL_LOG", tlogical, "", 1);
    col_log.push_back((char*)"T"); col_log.push_back((char*)"F"); col_log.push_back((char*)"T");
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<char*> >(col_log)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<char*> col_bit("COL_BIT", tbit, "", 1);
    col_bit.push_back((char*)"F"); col_bit.push_back((char*)"T"); col_bit.push_back((char*)"F");
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<char*> >(col_bit)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<uint8_t> col_byte("COL_BYTE", tbyte, "", 1);
    col_byte.push_back(uint8_t(0x10)); col_byte.push_back(uint8_t(0x11)); col_byte.push_back(uint8_t(0x12));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint8_t> >(col_byte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,col_tstring_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::string> col_str("COL_STR", tstring, "", 1);
    col_str.push_back(std::string("A")); col_str.push_back(std::string("BC")); col_str.push_back(std::string("DEF"));
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::string> >(col_str)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

// --- Vector columns as single tests ---
TEST(FITStable,v_tsbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int8_t>> v_sbyte("V_SBYTE", tsbyte, "", 1);
    v_sbyte.push_back(std::vector<int8_t>{1,2,3}); v_sbyte.push_back(std::vector<int8_t>{4,5,6}); v_sbyte.push_back(std::vector<int8_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int8_t>> >(v_sbyte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tbyte_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint8_t>> v_byte("V_BYTE", tbyte, "", 1);
    v_byte.push_back(std::vector<uint8_t>{0x41,0x42,0x43}); v_byte.push_back(std::vector<uint8_t>{0x44,0x45,0x46}); v_byte.push_back(std::vector<uint8_t>{0x47,0x48,0x49});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint8_t>> >(v_byte)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int32_t>> v_int16("V_INT16", tint, "", 1);
    v_int16.push_back(std::vector<int32_t>{-3,0,3}); v_int16.push_back(std::vector<int32_t>{4,5,6}); v_int16.push_back(std::vector<int32_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tuint_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint32_t>> v_uint16("V_UINT16", tuint, "", 1);
    v_uint16.push_back(std::vector<uint32_t>{1,2,3}); v_uint16.push_back(std::vector<uint32_t>{4,5,6}); v_uint16.push_back(std::vector<uint32_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint16)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tlong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int32_t>> v_int32("V_INT32", tlong, "", 1);
    v_int32.push_back(std::vector<int32_t>{-10,0,10}); v_int32.push_back(std::vector<int32_t>{11,12,13}); v_int32.push_back(std::vector<int32_t>{14,15,16});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tulong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint32_t>> v_uint32("V_UINT32", tulong, "", 1);
    v_uint32.push_back(std::vector<uint32_t>{1,2,3}); v_uint32.push_back(std::vector<uint32_t>{4,5,6}); v_uint32.push_back(std::vector<uint32_t>{7,8,9});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint32)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tlonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<int64_t>> v_int64("V_INT64", tlonglong, "", 1);
    v_int64.push_back(std::vector<int64_t>{-9,0,9}); v_int64.push_back(std::vector<int64_t>{10,11,12}); v_int64.push_back(std::vector<int64_t>{13,14,15});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int64_t>> >(v_int64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tulonglong_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<uint64_t>> v_uint64("V_UINT64", tulonglong, "", 1);
    v_uint64.push_back(std::vector<uint64_t>{9,10,11}); v_uint64.push_back(std::vector<uint64_t>{12,13,14}); v_uint64.push_back(std::vector<uint64_t>{15,16,17});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint64_t>> >(v_uint64)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tfloat_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<float>> v_float("V_FLOAT", tfloat, "", 1);
    v_float.push_back(std::vector<float>{0.f,1.f,2.f}); v_float.push_back(std::vector<float>{3.f,4.f,5.f}); v_float.push_back(std::vector<float>{6.f,7.f,8.f});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<float>> >(v_float)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tdouble_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<double>> v_double("V_DOUBLE", tdouble, "", 1);
    v_double.push_back(std::vector<double>{-1.0,0.5,2.5}); v_double.push_back(std::vector<double>{3.5,4.5,5.5}); v_double.push_back(std::vector<double>{6.5,7.5,8.5});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<double>> >(v_double)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
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
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
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
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
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
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tlogical_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<char*>> v_log("V_LOG", tlogical, "", 1);
    v_log.push_back(std::vector<char*>{(char*)"T",(char*)"F",(char*)"T"});
    v_log.push_back(std::vector<char*>{(char*)"F",(char*)"T",(char*)"F"});
    v_log.push_back(std::vector<char*>{(char*)"T",(char*)"T",(char*)"F"});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<char*>> >(v_log)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,v_tbit_ctor)
{
    FITStable table; expect_empty_table_header(table);
    FITScolumn<std::vector<char*>> v_bit("V_BIT", tbit, "", 1);
    v_bit.push_back(std::vector<char*>{(char*)"F",(char*)"F",(char*)"T"});
    v_bit.push_back(std::vector<char*>{(char*)"T",(char*)"F",(char*)"T"});
    v_bit.push_back(std::vector<char*>{(char*)"F",(char*)"T",(char*)"F"});
    EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<char*>> >(v_bit)));
    EXPECT_EQ(table.nrows(), 3u); EXPECT_EQ(table.ncols(), 1u);
    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable,all_columns_single_file)
{
    FITStable table; expect_empty_table_header(table);

    // --- Scalar columns ---
    {
        FITScolumn<int8_t>    col_sbyte ("COL_SBYTE" , tsbyte   , "", 1);
        col_sbyte.push_back(int8_t(1)); col_sbyte.push_back(int8_t(2)); col_sbyte.push_back(int8_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int8_t> >(col_sbyte)));
    }
    {
        FITScolumn<int16_t> col_short("COL_SHORT", tshort, "", 1);
        col_short.push_back(int16_t(4)); col_short.push_back(int16_t(5)); col_short.push_back(int16_t(6));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int16_t> >(col_short)));
    }
    {
        FITScolumn<uint16_t> col_ushort("COL_USHORT", tushort, "", 1);
        col_ushort.push_back(uint16_t(7)); col_ushort.push_back(uint16_t(8)); col_ushort.push_back(uint16_t(9));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint16_t> >(col_ushort)));
    }
    {
        FITScolumn<int32_t> col_int16("COL_INT16", tint, "", 1);
        col_int16.push_back(int32_t(-3)); col_int16.push_back(int32_t(0)); col_int16.push_back(int32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int16)));
    }
    {
        FITScolumn<uint32_t> col_uint16("COL_UINT16", tuint, "", 1);
        col_uint16.push_back(uint32_t(1)); col_uint16.push_back(uint32_t(2)); col_uint16.push_back(uint32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint16)));
    }
    {
        FITScolumn<int32_t> col_int32("COL_INT32", tlong, "", 1);
        col_int32.push_back(int32_t(-11)); col_int32.push_back(int32_t(22)); col_int32.push_back(int32_t(33));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int32_t> >(col_int32)));
    }
    {
        FITScolumn<uint32_t> col_uint32("COL_UINT32", tulonglong, "", 1);
        col_uint32.push_back(uint32_t(1)); col_uint32.push_back(uint32_t(2)); col_uint32.push_back(uint32_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint32_t> >(col_uint32)));
    }
    {
        FITScolumn<int64_t> col_int64("COL_INT64", tlonglong, "", 1);
        col_int64.push_back(int64_t(-9)); col_int64.push_back(int64_t(0)); col_int64.push_back(int64_t(9));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int64_t> >(col_int64)));
    }
    {
        FITScolumn<uint64_t> col_uint64("COL_UINT64", tulonglong, "", 1);
        col_uint64.push_back(uint64_t(9)); col_uint64.push_back(uint64_t(10)); col_uint64.push_back(uint64_t(11));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint64_t> >(col_uint64)));
    }
    {
        FITScolumn<float> col_float("COL_FLOAT", tfloat, "", 1);
        col_float.push_back(0.f); col_float.push_back(1.f); col_float.push_back(2.f);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<float> >(col_float)));
    }
    {
        FITScolumn<double> col_double("COL_DOUBLE", tdouble, "", 1);
        col_double.push_back(-1.0); col_double.push_back(0.5); col_double.push_back(2.5);
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<double> >(col_double)));
    }
    {
        FITScolumn<FITSform::complex> col_cplx("COL_CPLX", tcplx, "", 1);
        col_cplx.push_back(FITSform::complex(1.0f,2.0f)); col_cplx.push_back(FITSform::complex(3.0f,4.0f)); col_cplx.push_back(FITSform::complex(5.0f,6.0f));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::complex> >(col_cplx)));
    }
    {
        FITScolumn<FITSform::dblcomplex> col_dcplx("COL_DCPLX", tdbcplx, "", 1);
        col_dcplx.push_back(FITSform::dblcomplex(1.0,2.0)); col_dcplx.push_back(FITSform::dblcomplex(3.0,4.0)); col_dcplx.push_back(FITSform::dblcomplex(5.0,6.0));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<FITSform::dblcomplex> >(col_dcplx)));
    }
    {
        FITScolumn<char*> col_log("COL_LOG", tlogical, "", 1);
        col_log.push_back((char*)"T"); col_log.push_back((char*)"F"); col_log.push_back((char*)"T");
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<char*> >(col_log)));
    }
    {
        FITScolumn<char*> col_bit("COL_BIT", tbit, "", 1);
        col_bit.push_back((char*)"F"); col_bit.push_back((char*)"T"); col_bit.push_back((char*)"F");
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<char*> >(col_bit)));
    }
    {
        FITScolumn<uint8_t> col_byte("COL_BYTE", tbyte, "", 1);
        col_byte.push_back(uint8_t(0x10)); col_byte.push_back(uint8_t(0x11)); col_byte.push_back(uint8_t(0x12));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<uint8_t> >(col_byte)));
    }
    {
        FITScolumn<std::string> col_str("COL_STR", tstring, "", 1);
        col_str.push_back(std::string("A")); col_str.push_back(std::string("BC")); col_str.push_back(std::string("DEF"));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::string> >(col_str)));
    }

    // --- Vector columns ---
    {
        FITScolumn<std::vector<int8_t>> v_sbyte("V_SBYTE", tsbyte, "", 1);
        v_sbyte.push_back(std::vector<int8_t>{1,2,3}); v_sbyte.push_back(std::vector<int8_t>{4,5,6}); v_sbyte.push_back(std::vector<int8_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int8_t>> >(v_sbyte)));
    }
    {
        FITScolumn<std::vector<uint8_t>> v_byte("V_BYTE", tbyte, "", 1);
        v_byte.push_back(std::vector<uint8_t>{0x41,0x42,0x43}); v_byte.push_back(std::vector<uint8_t>{0x44,0x45,0x46}); v_byte.push_back(std::vector<uint8_t>{0x47,0x48,0x49});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint8_t>> >(v_byte)));
    }
    {
        FITScolumn<std::vector<int32_t>> v_int16("V_INT16", tint, "", 1);
        v_int16.push_back(std::vector<int32_t>{-3,0,3}); v_int16.push_back(std::vector<int32_t>{4,5,6}); v_int16.push_back(std::vector<int32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int16)));
    }
    {
        FITScolumn<std::vector<uint32_t>> v_uint16("V_UINT16", tuint, "", 1);
        v_uint16.push_back(std::vector<uint32_t>{1,2,3}); v_uint16.push_back(std::vector<uint32_t>{4,5,6}); v_uint16.push_back(std::vector<uint32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint16)));
    }
    {
        FITScolumn<std::vector<int32_t>> v_int32("V_INT32", tlong, "", 1);
        v_int32.push_back(std::vector<int32_t>{-10,0,10}); v_int32.push_back(std::vector<int32_t>{11,12,13}); v_int32.push_back(std::vector<int32_t>{14,15,16});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int32_t>> >(v_int32)));
    }
    {
        FITScolumn<std::vector<uint32_t>> v_uint32("V_UINT32", tulonglong, "", 1);
        v_uint32.push_back(std::vector<uint32_t>{1,2,3}); v_uint32.push_back(std::vector<uint32_t>{4,5,6}); v_uint32.push_back(std::vector<uint32_t>{7,8,9});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint32_t>> >(v_uint32)));
    }
    {
        FITScolumn<std::vector<int64_t>> v_int64("V_INT64", tlonglong, "", 1);
        v_int64.push_back(std::vector<int64_t>{-9,0,9}); v_int64.push_back(std::vector<int64_t>{10,11,12}); v_int64.push_back(std::vector<int64_t>{13,14,15});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<int64_t>> >(v_int64)));
    }
    {
        FITScolumn<std::vector<uint64_t>> v_uint64("V_UINT64", tulonglong, "", 1);
        v_uint64.push_back(std::vector<uint64_t>{9,10,11}); v_uint64.push_back(std::vector<uint64_t>{12,13,14}); v_uint64.push_back(std::vector<uint64_t>{15,16,17});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<uint64_t>> >(v_uint64)));
    }
    {
        FITScolumn<std::vector<float>> v_float("V_FLOAT", tfloat, "", 1);
        v_float.push_back(std::vector<float>{0.f,1.f,2.f}); v_float.push_back(std::vector<float>{3.f,4.f,5.f}); v_float.push_back(std::vector<float>{6.f,7.f,8.f});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<float>> >(v_float)));
    }
    {
        FITScolumn<std::vector<double>> v_double("V_DOUBLE", tdouble, "", 1);
        v_double.push_back(std::vector<double>{-1.0,0.5,2.5}); v_double.push_back(std::vector<double>{3.5,4.5,5.5}); v_double.push_back(std::vector<double>{6.5,7.5,8.5});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<double>> >(v_double)));
    }
    {
        FITScolumn<std::vector<FITSform::complex>> v_cplx("V_CPLX", tcplx, "", 1);
        v_cplx.push_back(std::vector<FITSform::complex>{ {1.f,2.f},{3.f,4.f},{5.f,6.f} });
        v_cplx.push_back(std::vector<FITSform::complex>{ {7.f,8.f},{9.f,10.f},{11.f,12.f} });
        v_cplx.push_back(std::vector<FITSform::complex>{ {13.f,14.f},{15.f,16.f},{17.f,18.f} });
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::complex>> >(v_cplx)));
    }
    {
        FITScolumn<std::vector<FITSform::dblcomplex>> v_dcplx("V_DCPLX", tdbcplx, "", 1);
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {1.0,2.0},{3.0,4.0},{5.0,6.0} });
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {7.0,8.0},{9.0,10.0},{11.0,12.0} });
        v_dcplx.push_back(std::vector<FITSform::dblcomplex>{ {13.0,14.0},{15.0,16.0},{17.0,18.0} });
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<FITSform::dblcomplex>> >(v_dcplx)));
    }
    {
        FITScolumn<std::vector<std::string>> v_str("V_STR", tstring, "", 1);
        v_str.push_back(std::vector<std::string>{"A","BC","DEF"});
        v_str.push_back(std::vector<std::string>{"G","HI","JKL"});
        v_str.push_back(std::vector<std::string>{"M","NO","PQR"});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<std::string>> >(v_str)));
    }
    {
        FITScolumn<std::vector<char*>> v_log("V_LOG", tlogical, "", 1);
        v_log.push_back(std::vector<char*>{(char*)"T",(char*)"F",(char*)"T"});
        v_log.push_back(std::vector<char*>{(char*)"F",(char*)"T",(char*)"F"});
        v_log.push_back(std::vector<char*>{(char*)"T",(char*)"T",(char*)"F"});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<char*>> >(v_log)));
    }
    {
        FITScolumn<std::vector<char*>> v_bit("V_BIT", tbit, "", 1);
        v_bit.push_back(std::vector<char*>{(char*)"F",(char*)"F",(char*)"T"});
        v_bit.push_back(std::vector<char*>{(char*)"T",(char*)"F",(char*)"T"});
        v_bit.push_back(std::vector<char*>{(char*)"F",(char*)"T",(char*)"F"});
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<std::vector<char*>> >(v_bit)));
    }

    EXPECT_EQ(table.nrows(), 3u);
    EXPECT_GT(table.ncols(), 1u);

    EXPECT_NO_THROW(table.write("./build/testdata/test_fitstable.fits", 0, true));
}

TEST(FITStable, read_all_columns)
{
    // Ensure the file exists from previous write tests (all_columns_single_file).
    fitsfile* rawFptr = nullptr;
    int status = 0;
    EXPECT_EQ(fits_open_file(&rawFptr, "./build/testdata/test_fitstable.fits", READONLY, &status), 0);
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
    EXPECT_EQ(table.HDU().GetValueForKey("TTYPE32"), "V_BIT");

    status = 0;
    EXPECT_EQ(fits_close_file(fptr.get(), &status), 0);
    EXPECT_EQ(status, 0);
}


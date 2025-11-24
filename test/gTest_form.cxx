#include <gtest/gtest.h>
#include <DSTfits/FITStable.h>
#include <string>
#include <vector>
#include <sstream>

using namespace DSL;

// Helper to expect TTYPE pattern "1<code>"
static void ExpectTTYPE(const FITSform& f, const std::string& code)
{
    EXPECT_EQ(f.getTTYPE(), "1" + code);
}

// Helper to expect TTYPE pattern "<nelem><code>" for vector columns
static void ExpectTTYPEVec(const FITSform& f, const std::string& code)
{
    EXPECT_EQ(f.getTTYPE(), std::to_string(f.getNelem()) + code);
}

TEST(FITSform_TypeMapping, getDataType_and_getDataTypeID_All)
{
    struct Case { dtype id; const char* name; const char* ttypeSuffix; };
    std::vector<Case> cases = {
        { tsbyte,     "SBYTE",  "S" },
        { tshort,     "SHORT",  "I" },
        { tushort,    "USHORT", "U" },
        { tint,       "INT",    "I4" },
        { tuint,      "UINT",   "V" },
        { tlong,      "LONG",   "I8" },
        { tlonglong,  "LONGLONG","K" },
        { tulong,     "ULONG",  "U8" },
        { tfloat,     "FLOAT",  "E" },
        { tdouble,    "DOUBLE", "D" },
        { tlogical,   "BOOL",   "L" },
        { tbit,       "BIT",    "X" },
        { tbyte,      "BYTE",   "B" },
        { tcplx,      "COMPLEX","C" },
        { tdbcplx,    "DOUBLE COMPLEX","M" }
    };

    for(const auto& c : cases)
    {
        EXPECT_EQ(FITSform::getDataType(c.id), std::string(c.name));
        EXPECT_EQ(FITSform::getDataTypeID(c.name), c.id);
    }
    // STRING special case
    EXPECT_EQ(FITSform::getDataType(tstring), "STRING");
    EXPECT_EQ(FITSform::getDataTypeID("string"), tstring);
}

TEST(FITScolumn_Scalar, SByte)
{
    FITScolumn<int8_t> col("COL_SB", tsbyte, "", 1);
    col.push_back(std::any(int8_t(42)));
    EXPECT_EQ(col.getName(), "COL_SB");
    EXPECT_EQ(col.getType(), tsbyte);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int8_t));
    ExpectTTYPE(col, "S");
    auto dup = col.clone();
    EXPECT_EQ(dup->size(), col.size());
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_SB"), std::string::npos);
    EXPECT_NE(dump.find("SBYTE"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("0x2A"), std::string::npos);
}

TEST(FITScolumn_Scalar, Byte)
{
    FITScolumn<uint8_t> col("COL_UB", tbyte, "", 1);
    col.push_back((uint8_t) 17 );
    EXPECT_EQ(col.getType(), tbyte);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint8_t));
    ExpectTTYPE(col, "B");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_UB"), std::string::npos);
    EXPECT_NE(dump.find("BYTE"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("0x11"), std::string::npos);
}

TEST(FITScolumn_Scalar, Int8)
{
    FITScolumn<int8_t> col("COL_S8", tshort, "", 1);
    col.push_back(std::any(int8_t(42)));
    EXPECT_EQ(col.getName(), "COL_S8");
    EXPECT_EQ(col.getType(), tshort);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int8_t));
    ExpectTTYPE(col, "I");
    auto dup = col.clone();
    EXPECT_EQ(dup->size(), col.size());
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_S8"), std::string::npos);
    EXPECT_NE(dump.find("SHORT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("42"), std::string::npos);
}

TEST(FITScolumn_Scalar, UInt8)
{
    FITScolumn<uint8_t> col("COL_UB", tushort, "", 1);
    col.push_back((uint8_t) 17 );
    EXPECT_EQ(col.getType(), tushort);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint8_t));
    ExpectTTYPE(col, "U");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_UB"), std::string::npos);
    EXPECT_NE(dump.find("USHORT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("17"), std::string::npos);
}

TEST(FITScolumn_Scalar, Int16)
{
    FITScolumn<int16_t> col("COL_I16", tint, "", 2);
    col.push_back(std::any(int16_t(-3)));
    EXPECT_EQ(col.getType(), tint);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int16_t));
    ExpectTTYPE(col, "I4");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_I16"), std::string::npos);
    EXPECT_NE(dump.find("INT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("-3"), std::string::npos);
}

TEST(FITScolumn_Scalar, Uint16)
{
    FITScolumn<uint16_t> col("COL_U16", tuint, "", 3);
    col.push_back(std::any(uint16_t(65535)));
    EXPECT_EQ(col.getType(), tuint);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint16_t));
    ExpectTTYPE(col, "V");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_U16"), std::string::npos);
    EXPECT_NE(dump.find("UINT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("65535"), std::string::npos);
}

TEST(FITScolumn_Scalar, Int32)
{
    FITScolumn<int32_t> col("COL_I32", tlong, "", 4);
    col.push_back(std::any(int32_t(-123456)));
    EXPECT_EQ(col.getType(), tlong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int32_t));
    ExpectTTYPE(col, "I8");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_I32"), std::string::npos);
    EXPECT_NE(dump.find("LONG"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("-123456"), std::string::npos);
}

TEST(FITScolumn_Scalar, UInt32)
{
    FITScolumn<uint32_t> col("COL_U32", tulong, "", 5);
    col.push_back(std::any(uint32_t(123456)));
    EXPECT_EQ(col.getType(), tulong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint32_t));
    ExpectTTYPE(col, "U8");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_U32"), std::string::npos);
    EXPECT_NE(dump.find("ULONG"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("123456"), std::string::npos);
}

TEST(FITScolumn_Scalar, Int64)
{
    FITScolumn<int64_t> col("COL_I64", tlonglong, "", 6);
    col.push_back(std::any(int64_t(-9)));
    EXPECT_EQ(col.getType(), tlonglong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int64_t));
    ExpectTTYPE(col, "K");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_I64"), std::string::npos);
    EXPECT_NE(dump.find("LONGLONG"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("-9"), std::string::npos);
}

TEST(FITScolumn_Scalar, Float)
{
    FITScolumn<float> col("COL_F", tfloat, "", 7);
    col.push_back(std::any(float(1.25f)));
    EXPECT_EQ(col.getType(), tfloat);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(float));
    ExpectTTYPE(col, "E");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_F"), std::string::npos);
    EXPECT_NE(dump.find("FLOAT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("1.25"), std::string::npos);
}

TEST(FITScolumn_Scalar, Double)
{
    FITScolumn<double> col("COL_D", tdouble, "", 8);
    col.push_back(std::any(double(-2.5)));
    EXPECT_EQ(col.getType(), tdouble);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(double));
    ExpectTTYPE(col, "D");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_D"), std::string::npos);
    EXPECT_NE(dump.find("DOUBLE"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("-2.5"), std::string::npos);
}

TEST(FITScolumn_Scalar, ComplexFloatPair)
{
    FITScolumn<FITSform::complex> col("COL_CF", tcplx, "", 9);
    col.push_back(std::any(FITSform::complex(1.0f,2.0f)));
    EXPECT_EQ(col.getType(), tcplx);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)(sizeof(float)*2));
    ExpectTTYPE(col, "C");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_CF"), std::string::npos);
    EXPECT_NE(dump.find("COMPLEX"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("1 , 2"), std::string::npos);
}

TEST(FITScolumn_Scalar, ComplexDoublePair)
{
    FITScolumn<FITSform::dblcomplex> col("COL_CD", tdbcplx, "", 10);
    col.push_back(std::any(FITSform::dblcomplex(3.0,4.0)));
    EXPECT_EQ(col.getType(), tdbcplx);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)(sizeof(double)*2));
    ExpectTTYPE(col, "M");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_CD"), std::string::npos);
    EXPECT_NE(dump.find("DOUBLE COMPLEX"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("3 , 4"), std::string::npos);
}

TEST(FITScolumn_Scalar, LogicalCharPtr)
{
    FITScolumn<char*> col("COL_LOG", tlogical, "", 11);
    col.push_back(std::any((char*)"T"));
    EXPECT_EQ(col.getType(), tlogical);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(char*));
    ExpectTTYPE(col, "L");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_LOG"), std::string::npos);
    EXPECT_NE(dump.find("BOOL"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("T"), std::string::npos);
}

TEST(FITScolumn_Scalar, BitCharPtr)
{
    FITScolumn<char*> col("COL_BIT", tbit, "", 12);
    col.push_back(std::any((char*)"F"));
    EXPECT_EQ(col.getType(), tbit);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(char*));
    ExpectTTYPE(col, "X");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_BIT"), std::string::npos);
    EXPECT_NE(dump.find("BIT"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("F"), std::string::npos);
}

TEST(FITScolumn_Scalar, String)
{
    FITScolumn<std::string> col("COL_STR", tstring, "", 13);
    col.push_back(std::any(std::string("ABC")));
    EXPECT_EQ(col.getType(), tstring);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 1);
    EXPECT_EQ(col.getWidth(), 3); // length of "ABC"
    EXPECT_EQ(col.getTTYPE(), "3A");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("COL_STR"), std::string::npos);
    EXPECT_NE(dump.find("STRING"), std::string::npos);
    EXPECT_NE(dump.find("[0]"), std::string::npos);
    EXPECT_NE(dump.find("ABC"), std::string::npos);
}

TEST(FITScolumn_Clone, Independence)
{
    FITScolumn<int8_t> col("BASE", tsbyte, "", 1);
    col.push_back(std::any(int8_t(5)));
    auto clone1 = col.clone();
    EXPECT_EQ(clone1->size(), col.size());
    // Modify original
    col.push_back(std::any(int8_t(6)));
    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(clone1->size(), 1u); // Clone unaffected
}

// -------------------- Vector columns --------------------

TEST(FITScolumn_Vector, SByteVector)
{
    FITScolumn<std::vector<int8_t>> col("V_SB", tsbyte, "", 1);
    col.push_back(std::vector<int8_t>{ int8_t('A'), int8_t('B'), int8_t('C') });
    EXPECT_EQ(col.getType(), tsbyte);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int8_t));
    ExpectTTYPEVec(col, "S");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_SB"), std::string::npos);
    EXPECT_NE(dump.find("SBYTE"), std::string::npos);
    EXPECT_NE(dump.find("(0)  0x41"), std::string::npos);
    EXPECT_NE(dump.find("(1)  0x42"), std::string::npos);
    EXPECT_NE(dump.find("(2)  0x43"), std::string::npos);
}

TEST(FITScolumn_Vector, ByteVector)
{
    FITScolumn<std::vector<uint8_t>> col("V_UB", tbyte, "", 2);
    col.push_back(std::vector<uint8_t>{ uint8_t('Q'), uint8_t('W'), uint8_t('E') });
    EXPECT_EQ(col.getType(), tbyte);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint8_t));
    ExpectTTYPEVec(col, "B");
    std::ostringstream oss;
    col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_UB"), std::string::npos);
    EXPECT_NE(dump.find("BYTE"), std::string::npos);
    EXPECT_NE(dump.find("(0)  0x51"), std::string::npos);
    EXPECT_NE(dump.find("(1)  0x57"), std::string::npos);
    EXPECT_NE(dump.find("(2)  0x45"), std::string::npos);
}

TEST(FITScolumn_Vector, Int16Vector)
{
    FITScolumn<std::vector<int16_t>> col("V_I16", tint, "", 3);
    col.push_back(std::vector<int16_t>{ -3, 4, 8 });
    EXPECT_EQ(col.getType(), tint);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int16_t));
    ExpectTTYPEVec(col, "I4");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_I16"), std::string::npos);
    EXPECT_NE(dump.find("INT"), std::string::npos);
    EXPECT_NE(dump.find("(0)   -3"), std::string::npos);
    EXPECT_NE(dump.find("(1)   4"), std::string::npos);
    EXPECT_NE(dump.find("(2)   8"), std::string::npos);
}

TEST(FITScolumn_Vector, UInt16Vector)
{
    FITScolumn<std::vector<uint16_t>> col("V_U16", tuint, "", 4);
    col.push_back(std::vector<uint16_t>{ 65535, 1, 2 });
    EXPECT_EQ(col.getType(), tuint);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint16_t));
    ExpectTTYPEVec(col, "V");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_U16"), std::string::npos);
    EXPECT_NE(dump.find("UINT"), std::string::npos);
    EXPECT_NE(dump.find("(0)   65535"), std::string::npos);
    EXPECT_NE(dump.find("(1)   1"), std::string::npos);
    EXPECT_NE(dump.find("(2)   2"), std::string::npos);
}

TEST(FITScolumn_Vector, Int32Vector)
{
    FITScolumn<std::vector<int32_t>> col("V_I32", tlong, "", 5);
    col.push_back(std::vector<int32_t>{ -123456, 7, 99 });
    EXPECT_EQ(col.getType(), tlong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int32_t));
    ExpectTTYPEVec(col, "I8");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_I32"), std::string::npos);
    EXPECT_NE(dump.find("LONG"), std::string::npos);
    EXPECT_NE(dump.find("(0)   -123456"), std::string::npos);
    EXPECT_NE(dump.find("(1)   7"), std::string::npos);
    EXPECT_NE(dump.find("(2)   99"), std::string::npos);
}

TEST(FITScolumn_Vector, UInt32Vector)
{
    FITScolumn<std::vector<uint32_t>> col("V_U32", tulong, "", 6);
    col.push_back(std::vector<uint32_t>{ 123456, 42, 314 });
    EXPECT_EQ(col.getType(), tulong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(uint32_t));
    ExpectTTYPEVec(col, "U8");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_U32"), std::string::npos);
    EXPECT_NE(dump.find("ULONG"), std::string::npos);
    EXPECT_NE(dump.find("(0)   123456"), std::string::npos);
    EXPECT_NE(dump.find("(1)   42"), std::string::npos);
    EXPECT_NE(dump.find("(2)   314"), std::string::npos);
}

TEST(FITScolumn_Vector, Int64Vector)
{
    FITScolumn<std::vector<int64_t>> col("V_I64", tlonglong, "", 7);
    col.push_back(std::vector<int64_t>{ -9, 0, 9, 18 });
    EXPECT_EQ(col.getType(), tlonglong);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 4);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(int64_t));
    ExpectTTYPEVec(col, "K");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_I64"), std::string::npos);
    EXPECT_NE(dump.find("LONGLONG"), std::string::npos);
    EXPECT_NE(dump.find("(0)   -9"), std::string::npos);
    EXPECT_NE(dump.find("(1)   0"), std::string::npos);
    EXPECT_NE(dump.find("(2)   9"), std::string::npos);
}

TEST(FITScolumn_Vector, FloatVector)
{
    FITScolumn<std::vector<float>> col("V_F", tfloat, "", 8);
    col.push_back(std::vector<float>{ 1.25f, -2.5f, 0.5f });
    EXPECT_EQ(col.getType(), tfloat);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(float));
    ExpectTTYPEVec(col, "E");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_F"), std::string::npos);
    EXPECT_NE(dump.find("FLOAT"), std::string::npos);
    EXPECT_NE(dump.find("(0)   1.25"), std::string::npos);
    EXPECT_NE(dump.find("(1)   -2.5"), std::string::npos);
    EXPECT_NE(dump.find("(2)   0.5"), std::string::npos);
}

TEST(FITScolumn_Vector, DoubleVector)
{
    FITScolumn<std::vector<double>> col("V_D", tdouble, "", 9);
    col.push_back(std::vector<double>{ -2.5, 3.75, 0.0 });
    EXPECT_EQ(col.getType(), tdouble);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)sizeof(double));
    ExpectTTYPEVec(col, "D");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_D"), std::string::npos);
    EXPECT_NE(dump.find("DOUBLE"), std::string::npos);
    EXPECT_NE(dump.find("(0)   -2.5"), std::string::npos);
    EXPECT_NE(dump.find("(1)   3.75"), std::string::npos);
    EXPECT_NE(dump.find("(2)   0"), std::string::npos);
}

TEST(FITScolumn_Vector, ComplexFloatVector)
{
    FITScolumn<std::vector<FITSform::complex>> col("V_CF", tcplx, "", 10);
    col.push_back(std::vector<FITSform::complex>{ {1.0f,2.0f}, {3.0f,4.0f}, {5.0f,6.0f} });
    EXPECT_EQ(col.getType(), tcplx);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)(sizeof(float)*2));
    ExpectTTYPEVec(col, "C");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_CF"), std::string::npos);
    EXPECT_NE(dump.find("COMPLEX"), std::string::npos);
    EXPECT_NE(dump.find("(0)   1 , 2"), std::string::npos);
    EXPECT_NE(dump.find("(1)   3 , 4"), std::string::npos);
    EXPECT_NE(dump.find("(2)   5 , 6"), std::string::npos);
    col.Dump(std::cout);
}

TEST(FITScolumn_Vector, ComplexDoubleVector)
{
    FITScolumn<std::vector<FITSform::dblcomplex>> col("V_CD", tdbcplx, "", 11);
    col.push_back(std::vector<FITSform::dblcomplex>{ {3.0,4.0}, {5.5,6.5}, {7.0,8.0} });
    EXPECT_EQ(col.getType(), tdbcplx);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), (int64_t)(sizeof(double)*2));
    ExpectTTYPEVec(col, "M");
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_CD"), std::string::npos);
    EXPECT_NE(dump.find("DOUBLE COMPLEX"), std::string::npos);
    EXPECT_NE(dump.find("(0)   3 , 4"), std::string::npos);
    EXPECT_NE(dump.find("(1)   5.5 , 6.5"), std::string::npos);
    EXPECT_NE(dump.find("(2)   7 , 8"), std::string::npos);
}

TEST(FITScolumn_Vector, StringVector)
{
    FITScolumn<std::vector<std::string>> col("V_STR", tstring, "", 12);
    col.push_back(std::vector<std::string>{ "AB", "XYZ", "HIJ" }); // width=3, nelem=3
    EXPECT_EQ(col.getType(), tstring);
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col.getNelem(), 3);
    EXPECT_EQ(col.getWidth(), 3); // max length across the vector
    EXPECT_EQ(col.getTTYPE(), "9A3"); // (nelem*width)Awidth
    std::ostringstream oss; col.Dump(oss);
    std::string dump = oss.str();
    EXPECT_NE(dump.find("V_STR"), std::string::npos);
    EXPECT_NE(dump.find("STRING"), std::string::npos);
    EXPECT_NE(dump.find("(0)   AB"), std::string::npos);
    EXPECT_NE(dump.find("(1)   XYZ"), std::string::npos);
    EXPECT_NE(dump.find("(2)   HIJ"), std::string::npos);
}

TEST(FITScolumn_Vector_Clone, Independence)
{
    FITScolumn<std::vector<int32_t>> col("V_BASE", tlong, "", 13);
    col.push_back(std::vector<int32_t>{1,2,3});
    auto clone1 = col.clone();
    EXPECT_EQ(clone1->size(), col.size());
    // Modify original
    col.push_back(std::vector<int32_t>{4});
    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(clone1->size(), 1u); // Clone unaffected
}

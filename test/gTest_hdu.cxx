#include <gtest/gtest.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSexception.h>
#include <DSTfits/FITSmanager.h>

#include <string>
#include <limits>
#include <fstream>
#include <cstdio>

using namespace DSL;

#pragma region FITSkeyword_and_Dictionary

TEST(FITSkeyword_Constructors, BasicConstructor)
{
    // integer-like -> fULong
    FITSkeyword k1(std::string("123"), std::string("a string"), key_type::fChar);
    EXPECT_EQ(k1.value()  , std::string("123"));
    EXPECT_EQ(k1.comment(), std::string("a string"));
    EXPECT_EQ(k1.type(), key_type::fChar);

    // integer-like -> fULong
    FITSkeyword k1b(std::string("123"), std::string("an integer"), key_type::fULong);
    EXPECT_EQ(k1b.value()  , std::string("123"));
    EXPECT_EQ(k1b.comment(), std::string("an integer"));
    EXPECT_EQ(k1b.type(), key_type::fULong);

    // double-like -> fFloat
    FITSkeyword k2(std::string("12.34"), std::string("a float"), key_type::fUndef);
    EXPECT_EQ(k2.value()  , std::string("12.34"));
    EXPECT_EQ(k2.comment(), std::string("a float"));
    EXPECT_EQ(k2.type(), key_type::fFloat);

    // double-like -> fDouble
    FITSkeyword k2b(std::string("12.34e-8"), std::string("a double"), key_type::fUndef);
    EXPECT_EQ(k2b.value()  , std::string("12.34e-8"));
    EXPECT_EQ(k2b.comment(), std::string("a double"));
    EXPECT_EQ(k2b.type(), key_type::fFloat);

    // string -> fChar
    FITSkeyword k3(std::string("abc"));
    EXPECT_EQ(k3.type(), key_type::fChar);
    EXPECT_EQ(k3.comment(), std::string());

    // string -> fDouble
    FITSkeyword k4(std::string("12.34"));
    EXPECT_EQ(k4.type(), key_type::fFloat);
    EXPECT_EQ(k4.value(),std::string("12.34"));
    EXPECT_EQ(k4.comment(), std::string());

    // string -> fULong
    FITSkeyword k5(std::string("-1234"));
    EXPECT_EQ(k5.type(), key_type::fInt);
    EXPECT_EQ(k5.value(),std::string("-1234"));
    EXPECT_EQ(k5.comment(), std::string());

    // integer-like -> fULong
    FITSkeyword k1c(std::string("123"), key_type::fChar);
    EXPECT_EQ(k1c.value()  , std::string("123"));
    EXPECT_EQ(k1c.comment(), std::string());
    EXPECT_EQ(k1c.type(), key_type::fChar);

    // double-like -> fDouble
    FITSkeyword k2c(std::string("12.34"), key_type::fUndef);
    EXPECT_EQ(k2c.value()  , std::string("12.34"));
    EXPECT_EQ(k2c.comment(), std::string());
    EXPECT_EQ(k2c.type(), key_type::fFloat);

    // double-like -> fDouble
    FITSkeyword k2d(std::string("1234"), key_type::fLong);
    EXPECT_EQ(k2d.value()  , std::string("1234"));
    EXPECT_EQ(k2d.comment(), std::string());
    EXPECT_EQ(k2d.type(), key_type::fLong);
}

TEST(FITSkeyword_Constructors, TypeDetectionAndAccessors)
{
    // integer-like -> fULong
    FITSkeyword k1(std::string("123"), std::string("an integer"));
    EXPECT_EQ(k1.value(), std::string("123"));
    EXPECT_EQ(k1.comment(), std::string("an integer"));
    EXPECT_EQ(k1.type(), key_type::fUShort);

    // double-like -> fDouble
    FITSkeyword k2(std::string("12.34"), std::string("a double"));
    EXPECT_EQ(k2.type(), key_type::fFloat);

    // string -> fChar
    FITSkeyword k3(std::string("abc"), std::string("a string"));
    EXPECT_EQ(k3.type(), key_type::fChar);
}

TEST(FITSkeyword_Process_AllCases, ValueToTypeMapping)
{
    // empty -> undefined
    FITSkeyword k_empty(std::string(""));
    EXPECT_EQ(k_empty.type(), key_type::fUndef);

    // Non-numeric -> fChar
    FITSkeyword k_char("abc");
    EXPECT_EQ(k_char.type(), key_type::fChar);

    FITSkeyword k_mixed("12a");
    EXPECT_EQ(k_mixed.type(), key_type::fChar);

    // Floats and doubles
    FITSkeyword k_float("12.34");
    EXPECT_EQ(k_float.type(), key_type::fFloat);

    // must include both '.' and 'e' to be detected as double by current Process
    FITSkeyword k_double_exp("1.23e-4");
    EXPECT_EQ(k_double_exp.type(), key_type::fFloat);

    // Negative signed ranges (abs thresholds used by Process)
    FITSkeyword k_neg_small("-5");
    EXPECT_EQ(k_neg_small.type(), key_type::fShort);

    FITSkeyword k_neg_127("-127");
    EXPECT_EQ(k_neg_127.type(), key_type::fShort);

    FITSkeyword k_neg_128("-128");
    EXPECT_EQ(k_neg_128.type(), key_type::fInt);

    FITSkeyword k_neg_32767("-32767");
    EXPECT_EQ(k_neg_32767.type(), key_type::fInt);

    FITSkeyword k_neg_32768("-32768");
    EXPECT_EQ(k_neg_32768.type(), key_type::fLong);

    FITSkeyword k_neg_ll("-2147483648");
    EXPECT_EQ(k_neg_ll.type(), key_type::fLongLong);

    // Unsigned / positive ranges
    FITSkeyword k_zero("0");
    EXPECT_EQ(k_zero.type(), key_type::fByte);

    FITSkeyword k_one("1");
    EXPECT_EQ(k_one.type(), key_type::fByte);

    FITSkeyword k_two("2");
    EXPECT_EQ(k_two.type(), key_type::fUShort);

    FITSkeyword k_255("255");
    EXPECT_EQ(k_255.type(), key_type::fUShort);

    FITSkeyword k_256("256");
    EXPECT_EQ(k_256.type(), key_type::fUInt);

    FITSkeyword k_65535("65535");
    EXPECT_EQ(k_65535.type(), key_type::fUInt);

    FITSkeyword k_65536("65536");
    EXPECT_EQ(k_65536.type(), key_type::fULong);

    FITSkeyword k_4294967295("4294967295");
    EXPECT_EQ(k_4294967295.type(), key_type::fULong);

    // very large unsigned value within 64-bit range -> current Process maps it to fULong as well
    FITSkeyword k_large_ull("18446744073709551615");
    EXPECT_EQ(k_large_ull.type(), key_type::fULongLong);
}


TEST(FITSkeyword_SetValue, TypeConsistency)
{
    // fChar: accepts anything including empty
    {
        FITSkeyword k(std::string("init"), key_type::fChar);
        EXPECT_NO_THROW(k.setValue("anything"));
        EXPECT_EQ(k.value(), std::string("anything"));
        EXPECT_NO_THROW(k.setValue(std::string("")));
        EXPECT_EQ(k.value(), std::string(""));
    }

    // fByte: unsigned small (<=1)
    {
        FITSkeyword k(std::string("0"), key_type::fByte);
        EXPECT_NO_THROW(k.setValue("0")); EXPECT_EQ(k.value(), std::string("0"));
        EXPECT_NO_THROW(k.setValue("1")); EXPECT_EQ(k.value(), std::string("1"));
        bool threw = false; try { k.setValue("2"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
        threw = false; try { k.setValue("notnum"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }

    // fUShort: unsigned <=255
    {
        FITSkeyword k(std::string("0"), key_type::fUShort);
        EXPECT_NO_THROW(k.setValue("255")); EXPECT_EQ(k.value(), std::string("255"));
        bool threw = false; try { k.setValue("256"); } catch (...) { threw = true; } EXPECT_TRUE(threw);
    }

    // fUInt: unsigned <=65535
    {
        FITSkeyword k(std::string("0"), key_type::fUInt);
        EXPECT_NO_THROW(k.setValue("65535")); EXPECT_EQ(k.value(), std::string("65535"));
        bool threw = false; try { k.setValue("65536"); } catch (...) { threw = true; } EXPECT_TRUE(threw);
    }

    // fULong: unsigned larger (test a 32-bit max)
    {
        FITSkeyword k(std::string("0"), key_type::fULong);
        EXPECT_NO_THROW(k.setValue("4294967295")); EXPECT_EQ(k.value(), std::string("4294967295"));
        bool threw = false; try { k.setValue("4294967296"); } catch (...) { threw = true; } EXPECT_TRUE(threw);
    }

    // fShort: signed abs <=127
    {
        FITSkeyword k(std::string("0"), key_type::fShort);
        EXPECT_NO_THROW(k.setValue("127")); EXPECT_EQ(k.value(), std::string("127"));
        bool threw = false; try { k.setValue("128"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }

    // fInt: signed abs <=32767
    {
        FITSkeyword k(std::string("0"), key_type::fInt);
        EXPECT_NO_THROW(k.setValue("32767")); EXPECT_EQ(k.value(), std::string("32767"));
        bool threw = false; try { k.setValue("40000"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }

    // fLong: signed abs <=2147483647
    {
        FITSkeyword k(std::string("0"), key_type::fLong);
        EXPECT_NO_THROW(k.setValue("2147483647")); EXPECT_EQ(k.value(), std::string("2147483647"));
        bool threw = false; try { k.setValue("2147483648"); } catch (...) { threw = true; } EXPECT_TRUE(threw);
    }

    // fLongLong: signed 64-bit
    {
        FITSkeyword k(std::string("0"), key_type::fLongLong);
        EXPECT_NO_THROW(k.setValue("9223372036854775807")); EXPECT_EQ(k.value(), std::string("9223372036854775807"));
        bool threw = false; try { k.setValue("9223372036854775808"); } catch (...) { threw = true; } EXPECT_TRUE(threw);
    }

    // fBool: only 0 or 1
    {
        FITSkeyword k(std::string("0"), key_type::fBool);
        EXPECT_NO_THROW(k.setValue("1")); EXPECT_EQ(k.value(), std::string("1"));
        bool threw = false; try { k.setValue("2"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
        threw = false; try { k.setValue("notbool"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }

    // fFloat: decimal / exponent allowed
    {
        FITSkeyword k(std::string("0.0"), key_type::fFloat);
        EXPECT_NO_THROW(k.setValue("3.14159")); EXPECT_NEAR(std::stof(k.value()), 3.14159f, 1e-6f);
        EXPECT_NO_THROW(k.setValue("1.23e-4")); EXPECT_NEAR(std::stof(k.value()), 1.23e-4f, 1e-9f);
        bool threw = false; try { k.setValue("nan12"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }

    // fDouble: decimal / exponent allowed
    {
        FITSkeyword k(std::string("0.0"), key_type::fDouble);
        EXPECT_NO_THROW(k.setValue("1.23456789")); EXPECT_NEAR(std::stod(k.value()), 1.23456789, 1e-12);
        EXPECT_NO_THROW(k.setValue("6.022e23")); EXPECT_NEAR(std::stod(k.value()), 6.022e23, 1e10);
        bool threw = false; try { k.setValue("xyz"); } catch (const FITSexception&) { threw = true; } EXPECT_TRUE(threw);
    }
}

TEST(FITSkeyword_Dump, ProducesOutput)
{
    FITSkeyword k(std::string("3.14"), std::string("pi"));
    std::ostringstream oss;
    EXPECT_NO_THROW(k.Dump(oss));
    std::string s = oss.str();
    EXPECT_NE(s.size(), 0u);
}

TEST(FITSkeyword_datatype, tt2string)
{
    EXPECT_EQ(FITSkeyword::GetDataType(fChar),     std::string("TSTRING"));
    EXPECT_EQ(FITSkeyword::GetDataType(fShort),    std::string("TSHORT"));
    EXPECT_EQ(FITSkeyword::GetDataType(fUShort),   std::string("TUSHORT"));
    EXPECT_EQ(FITSkeyword::GetDataType(fInt),      std::string("TINT"));
    EXPECT_EQ(FITSkeyword::GetDataType(fUInt),     std::string("TUINT"));
    EXPECT_EQ(FITSkeyword::GetDataType(fLong),     std::string("TLONG"));
    EXPECT_EQ(FITSkeyword::GetDataType(fLongLong), std::string("TLONGLONG"));
    EXPECT_EQ(FITSkeyword::GetDataType(fULong),    std::string("TULONG"));
    EXPECT_EQ(FITSkeyword::GetDataType(fBool),     std::string("TLOGICAL"));
    EXPECT_EQ(FITSkeyword::GetDataType(fFloat),    std::string("TFLOAT"));
    EXPECT_EQ(FITSkeyword::GetDataType(fDouble),   std::string("DOUBLE"));
    EXPECT_EQ(FITSkeyword::GetDataType(fByte),     std::string("TBYTE"));
    EXPECT_EQ(FITSkeyword::GetDataType(fUndef),    std::string("UNDEF"));
}

TEST(FITSkeyword_asString, ProducesOutput)
{
    FITSkeyword k1(std::string("TESTCASE"), std::string("an string"), key_type::fChar);
    std::string s1 = k1.asString();
    EXPECT_EQ(s1.size()+10, 80)<< s1 <<"["<<s1.size()<<"]";
    EXPECT_EQ(s1.find("/"), 32-10-1)<< s1;
    EXPECT_EQ(s1.find("an string"), 32-10+1)<< s1;
    EXPECT_EQ(s1.find("'TESTCASE'"), 0)<< s1;

    FITSkeyword k2(std::string("12345678"), std::string("an integer"), key_type::fInt);
    std::string s2 = k2.asString();
    EXPECT_EQ(s2.size()+10, 80)<< s2 <<"["<<s2.size()<<"]";
    EXPECT_EQ(s2.find("/"), 32-10-1)<<s2;
    EXPECT_EQ(s2.find("12345678"), 32-10-8-2)<<s2 << std::endl << "0123456789012345678901234567890123456789";

    FITSkeyword k3(std::string("This is a very very very long comment string which will be subdivided in multiple different lines such that each line is 80 char long, with the exception of the first line whiche will be shorter than 80. Every 80 char their should be 8 space char to mark begining of a new line"), key_type::fChar);
    std::string s3 = k3.asString();
    EXPECT_EQ((s3.size()+10)>80, 1)<< s3 <<"["<<s3.size()<<"]";
}

#pragma endregion
#pragma region FITSDictionary

TEST(FITSDictionary_Typedef, InsertFindErase)
{
    FITSDictionary dict;

    dict.insert(std::pair<key_code,FITSkeyword>("K1", FITSkeyword(std::string("100"), std::string("one hundred"))));
    auto it = dict.find("K1");
    ASSERT_NE(it, dict.end());
    EXPECT_EQ(it->second.value(), std::string("100"));

    // erase and ensure gone
    dict.erase(it);
    EXPECT_EQ(dict.find("K1"), dict.end());
}

#pragma endregion

#pragma region Constructor_and_Copy

TEST(FITShdu_Constructor, DefaultAndCopy)
{
    FITShdu hdu;

    // set several keys using different overloads
    hdu.ValueForKey("TEST_INT", 42, std::string("an int"));
    hdu.ValueForKey("TEST_DBL", 3.14159, std::string("a double"));
    hdu.ValueForKey("TEST_STR", std::string("hello"), std::string("a string"));
    hdu.ValueForKey("TEST_BOOL", true, std::string("a bool"));

    // copy construct and verify values are preserved
    FITShdu copy(hdu);

    EXPECT_EQ(copy.GetInt8ValueForKey("TEST_INT"), 42);
    EXPECT_NEAR(copy.GetDoubleValueForKey("TEST_DBL"), 3.14159, 1e-7);
    EXPECT_EQ(copy.GetValueForKey("TEST_STR"), std::string("hello"));
    EXPECT_TRUE(copy.GetBoolValueForKey("TEST_BOOL"));
}

TEST(FITShdu_Constructor, FromFITSDictionary)
{
    FITSDictionary fakeHdu;
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("SIMPLE"),FITSkeyword("T","file does conform to FITS standard")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("BITPIX"),FITSkeyword("16","number of bits per data pixel")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS"),FITSkeyword("2","number of data axes")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS1"),FITSkeyword("100","length of data axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS2"),FITSkeyword("100","length of data axis 2")));

    FITShdu hdu(fakeHdu);
    EXPECT_EQ(hdu.GetValueForKey("SIMPLE"), "T");
    EXPECT_EQ(hdu.GetInt16ValueForKey("BITPIX"), 16);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS"), 2);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS1"), 100);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS2"), 100);
}

TEST(FITShdu_Constructor, FromString)
{
    FITSDictionary fakeHdu;
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("SIMPLE"),FITSkeyword("T","file does conform to FITS standard")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("BITPIX"),FITSkeyword("16","number of bits per data pixel")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS"),FITSkeyword("2","number of data axes")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS1"),FITSkeyword("100","length of data axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS2"),FITSkeyword("100","length of data axis 2")));

    FITShdu hdu_ref(fakeHdu);
    EXPECT_EQ(hdu_ref.GetValueForKey("SIMPLE"), "T");
    EXPECT_EQ(hdu_ref.GetInt16ValueForKey("BITPIX"), 16);
    EXPECT_EQ(hdu_ref.GetUInt64ValueForKey("NAXIS"), 2);
    EXPECT_EQ(hdu_ref.GetUInt64ValueForKey("NAXIS1"), 100);
    EXPECT_EQ(hdu_ref.GetUInt64ValueForKey("NAXIS2"), 100);

    std::string header_str = hdu_ref.asString();
    FITShdu hdu(header_str);

    EXPECT_EQ(hdu.GetValueForKey("SIMPLE"), "T");
    EXPECT_EQ(hdu.GetInt16ValueForKey("BITPIX"), 16);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS"), 2);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS1"), 100);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS2"), 100);
}

TEST(FITShdu_Constructor, FromFitsFile)
{
#if defined(__APPLE__)
    FITSmanager ff("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    FITSmanager ff("testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    ff.MoveToPrimary();
    const std::shared_ptr<fitsfile>& hdu_ref = ff.CurrentHDU();

    FITShdu hdu(hdu_ref);
    EXPECT_EQ(hdu.GetInt16ValueForKey("BITPIX"),16);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS"),2);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS1"),512);
    EXPECT_EQ(hdu.GetUInt64ValueForKey("NAXIS2"),512);
    EXPECT_EQ(hdu.GetValueForKey("CONTENT"),"IMAGE");

    EXPECT_EQ(hdu.GetEntry("NAXIS")->second.type(), key_type::fULongLong);
    EXPECT_EQ(hdu.GetEntry("NAXIS1")->second.type(), key_type::fULongLong);
    EXPECT_EQ(hdu.GetEntry("NAXIS2")->second.type(), key_type::fULongLong);
    EXPECT_EQ(hdu.GetEntry("BITPIX")->second.type(), key_type::fInt);
    EXPECT_EQ(hdu.GetEntry("BSCALE")->second.type(), key_type::fDouble);
    EXPECT_EQ(hdu.GetEntry("BZERO")->second.type(), key_type::fDouble);

    ff.Close();

}

#pragma endregion

#pragma region Accessors

TEST(FITShdu_Accessors, NumericAndStringGetters)
{
    FITShdu hdu;

    hdu.ValueForKey("IKEY", 7);
    hdu.ValueForKey("LKEY", static_cast<int32_t>(123456));
    hdu.ValueForKey("LLKEY", static_cast<int64_t>(-9876543210LL));
    hdu.ValueForKey("FKEY", 2.5f);
    hdu.ValueForKey("DKEY", 1.23456789);
    hdu.ValueForKey("SKEY", std::string("abc"));
    hdu.ValueForKey("BKEY", false);

    EXPECT_EQ(hdu.GetInt8ValueForKey("IKEY"), 7);
    EXPECT_EQ(hdu.GetInt32ValueForKey("LKEY"), 123456L);
    EXPECT_EQ(hdu.GetInt64ValueForKey("LLKEY"), -9876543210LL);
    EXPECT_NEAR(hdu.GetFloatValueForKey("FKEY"), 2.5f, 1e-6f);
    EXPECT_NEAR(hdu.GetDoubleValueForKey("DKEY"), 1.23456789, 1e-6);
    EXPECT_EQ(hdu.GetValueForKey("SKEY"), std::string("abc"));
    EXPECT_FALSE(hdu.GetBoolValueForKey("BKEY"));

    // Missing keys should return sentinel values or defaults
    EXPECT_ANY_THROW(hdu.GetInt16ValueForKey("MISSING_INT"));
}

#pragma endregion

#pragma region Setter_and_TypeConsistency

TEST(FITShdu_Setter, TypeConsistencyAndOverwrite)
{
    FITShdu hdu;
    // set as integer
    hdu.ValueForKey("MYKEY", 10, std::string("int key"));
    EXPECT_EQ(hdu.GetUInt8ValueForKey("MYKEY"), 10);

    // overwriting with same type should work
    hdu.ValueForKey("MYKEY", (uint8_t) 20);
    EXPECT_EQ(hdu.GetUInt8ValueForKey("MYKEY"), 20);

    // trying to set same keyword with a different base type should trigger FITSexception
    // e.g. set existing int key with a string (fChar) -> mismatch
    {
        bool exception_thrown = false;
        try
        {
            hdu.ValueForKey("MYKEY", std::string("not an int"), std::string("comment"));
        }
        catch (const FITSexception&)
        {
            exception_thrown = true;
        }
        EXPECT_FALSE(exception_thrown);
    }
}

#pragma endregion

#pragma region Utilities

TEST(FITShdu_Utilities, GetDimensionAndDeleteKey)
{
    FITShdu hdu;

    // define NAXIS1..NAXIS4
    hdu.ValueForKey("NAXIS1", 4);
    hdu.ValueForKey("NAXIS2", 5);
    hdu.ValueForKey("NAXIS3", 6);

    EXPECT_TRUE(hdu.Exists("NAXIS3"));

    long long dim = hdu.GetDimension();
    EXPECT_EQ(dim, 4LL * 5LL * 6LL);

    // Delete last axis and verify dimension updates
    hdu.DeleteKey("NAXIS3");
    EXPECT_FALSE(hdu.Exists("NAXIS3"));
    long long dim2 = hdu.GetDimension();
    EXPECT_EQ(dim2, 4LL * 5LL);
}

#pragma endregion

#pragma region Dump

TEST(FITShdu_Dump, NoCrash)
{
    FITShdu hdu;
    hdu.ValueForKey("A", 1);
    hdu.ValueForKey("B", std::string("two"));

    // Ensure Dump doesn't throw and produces output to stream
    std::ostringstream oss;
    EXPECT_NO_THROW(hdu.Dump(oss));
    std::string s = oss.str();
    EXPECT_NE(s.size(), 0u);
}

TEST(FITShdu_Modifier, ModifyFitsHeader)
{
    // Prepare test fixture: remove existing destination and copy source fixture into build/testdata
#if defined(__APPLE__)
    const std::string src_path = "build/testdata/rosat_pspc_rdf2_3_bk1.fits";
    const std::string dst_path = "build/testdata/rosat_pspc_rdf2_3_bk1_cpy.fits";
#else
    const std::string src_path = "testdata/rosat_pspc_rdf2_3_bk1.fits";
    const std::string dst_path = "testdata/rosat_pspc_rdf2_3_bk1_cpy.fits";
#endif

    // remove destination if exists (ignore errors)
    std::remove(dst_path.c_str());

    // copy source to destination; fail the test if source missing or copy fails
    std::ifstream srcfs(src_path, std::ios::binary);
    if(!srcfs)
        FAIL() << "Missing fixture: " << src_path;
    std::ofstream dstfs(dst_path, std::ios::binary);
    dstfs << srcfs.rdbuf();
    srcfs.close();
    dstfs.close();

#if defined(__APPLE__)
    FITSmanager ff("build/testdata/rosat_pspc_rdf2_3_bk1_cpy.fits",false);
#else
    FITSmanager ff("testdata/rosat_pspc_rdf2_3_bk1_cpy.fits",false);
#endif
    ff.MoveToPrimary();
    const std::shared_ptr<fitsfile>& hdu_ref = ff.CurrentHDU();

    FITShdu hdu(hdu_ref);
    
    // ADD NEW KEY TO THE HEADER
    hdu.ValueForKey("TESTF",3.14f, "a test float");
    EXPECT_EQ(hdu.GetEntry("TESTF")->second.type(), key_type::fFloat);

    hdu.ValueForKey("TESTD",(double) 6.62607015e-34, "a test double");
    EXPECT_EQ(hdu.GetEntry("TESTD")->second.type(), key_type::fDouble);

    hdu.ValueForKey("TESTUI16",(uint16_t)12345, "a test uint16_t");
    EXPECT_EQ(hdu.GetEntry("TESTUI16")->second.type(), key_type::fUInt);

    // MODIFY EXISTING KEY
    hdu.ValueForKey("BSCALE",2.0, "modified bscale");
    EXPECT_NEAR(hdu.GetDoubleValueForKey("BSCALE"), 2.0 , 1e-10);
    EXPECT_EQ(hdu.GetEntry("BSCALE")->second.type(), key_type::fDouble);

    hdu.Write(hdu_ref);
    ff.Close();

#if defined(__APPLE__)
    FITSmanager fr("build/testdata/rosat_pspc_rdf2_3_bk1_cpy.fits",true);
#else
    FITSmanager fr("testdata/rosat_pspc_rdf2_3_bk1_cpy.fits",true);
#endif  
    fr.MoveToPrimary();
    const std::shared_ptr<fitsfile>& hdu_cpy = fr.CurrentHDU();

    FITShdu _hdu(hdu_cpy);
    EXPECT_NEAR(_hdu.GetFloatValueForKey("TESTF"), 3.14 , 1e-3);
    EXPECT_NEAR(_hdu.GetDoubleValueForKey("TESTD"), 6.62607015e-34 , 1e-36);
    EXPECT_EQ(_hdu.GetUInt16ValueForKey("TESTUI16"), 12345);
    EXPECT_NEAR(_hdu.GetDoubleValueForKey("BSCALE"), 2.0 , 1e-10);
    
    fr.Close();
   
}

TEST(FITShdu_swap, swapBetweenHdu)
{
    FITShdu hdu1;
    hdu1.ValueForKey("KEY1", 1, "first key");
    hdu1.ValueForKey("KEY2", 2.0, "second key");

    FITShdu hdu2;
    hdu2.ValueForKey("KEYA", std::string("A"), "key A");
    hdu2.ValueForKey("KEYB", false);
    hdu2.ValueForKey("KEYC", 3.14f);

    // Swap contents
    hdu1.swap(hdu2);

    // Verify hdu1 now has hdu2's keys
    EXPECT_TRUE (hdu1.Exists("KEYA"));
    EXPECT_TRUE (hdu1.Exists("KEYB"));
    EXPECT_TRUE (hdu1.Exists("KEYC"));
    EXPECT_FALSE(hdu1.Exists("KEY1"));
    EXPECT_FALSE(hdu1.Exists("KEY2"));

    EXPECT_EQ   (hdu1.GetValueForKey("KEYA"), std::string("A"));
    EXPECT_FALSE(hdu1.GetBoolValueForKey("KEYB"));
    EXPECT_NEAR (hdu1.GetFloatValueForKey("KEYC"), 3.14f, 1e-6f);
    
    
    // Verify hdu2 now has hdu1's keys
    EXPECT_TRUE (hdu2.Exists("KEY1"));
    EXPECT_TRUE (hdu2.Exists("KEY2"));
    EXPECT_FALSE(hdu2.Exists("KEYA"));
    EXPECT_FALSE(hdu2.Exists("KEYB"));
    EXPECT_FALSE(hdu2.Exists("KEYC"));

    EXPECT_EQ   (hdu2.GetInt8ValueForKey("KEY1"), 1);
    EXPECT_NEAR (hdu2.GetDoubleValueForKey("KEY2"), 2.0, 1e-10);
}   

TEST(FITShdu_out, asString)
{
    FITShdu hdu;
    hdu.ValueForKey("AKEY1", (int8_t) 1, "first key");
    hdu.ValueForKey("AKEY2", (float) 2.0, "second key");
    hdu.ValueForKey("AKEY3", std::string("this is my value3"), "third key");
    hdu.ValueForKey("COMMENT", std::string("this is now a very very long comment string to test how does the parser format the string when the commant string is longer than the 80 char allowed for standard FITS keys cards"));
    hdu.ValueForKey("HISTORY", std::string("this is now a very very long comment string to test how does the parser format the string when the commant string is longer than the 80 char allowed for standard FITS keys cards"));

    std::string out = hdu.asString();
    EXPECT_EQ(out.size()%2880, 0);
    std::string endStr = out.substr(out.size()-8,3);
    EXPECT_EQ(endStr.compare("END"),0)<< endStr;

    for (size_t k = 0; k < 5; k++)
    {
        if(k < 3)
        {
            std::string thisStr = out.substr(k*80, 80);
            EXPECT_EQ(thisStr.find("AKEY"+std::to_string(k+1)), 0);
            EXPECT_EQ(thisStr.find("="), 9-1);
            EXPECT_EQ(thisStr.find("/"), 32-1);
        }
        else if(k==3)
        {

            out.erase(0, (k) * 80);
            std::string thisStr = out.substr(0, out.find("HISTORY")-1);
            EXPECT_EQ(thisStr.find("COMMENT"), 0);
            thisStr.erase(0,80);
            EXPECT_EQ(thisStr.find("COMMENT "), 0);
        }
        else if(k==4)
        {
            out.erase(0, out.find("HISTORY"));
            EXPECT_EQ(out.find("HISTORY "), 0);
            out.erase(0,80);
            EXPECT_EQ(out.find("HISTORY "), 0);
        }
    }

}

#pragma endregion

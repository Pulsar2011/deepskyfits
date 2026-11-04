#include <gtest/gtest.h>
#include <DSTfits/FITSnumeric.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSmanager.h>

#include "gTest_locale.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace DSL;

namespace
{
#ifndef CI_CIWORKER
    const std::string kTestFile = "build/testdata/test_fitsnumeric.fits";
#else
    const std::string kTestFile = "testdata/test_fitsnumeric.fits";
#endif

    // Creates an empty primary HDU and returns it still open.
    std::shared_ptr<fitsfile> createEmptyFile()
    {
        const std::string overwrite = "!" + kTestFile;

        fitsfile* rawFptr = nullptr;
        int status = 0;
        fits_create_file(&rawFptr, overwrite.c_str(), &status);
        fits_create_img(rawFptr, BYTE_IMG, 0, nullptr, &status);
        if (status != 0)
            throw FITSexception(status, "gTest_numeric", "createEmptyFile");

        return std::shared_ptr<fitsfile>(rawFptr, [](fitsfile*){}); // manual close
    }

    std::shared_ptr<fitsfile> openFile(int mode = READONLY)
    {
        fitsfile* rawFptr = nullptr;
        int status = 0;
        if (fits_open_file(&rawFptr, kTestFile.c_str(), mode, &status) != 0)
            throw FITSexception(status, "gTest_numeric", "openFile");

        return std::shared_ptr<fitsfile>(rawFptr, [](fitsfile*){}); // manual close
    }

    void closeFile(const std::shared_ptr<fitsfile>& fptr)
    {
        int status = 0;
        EXPECT_EQ(fits_close_file(fptr.get(), &status), 0);
    }
}

#define SKIP_WITHOUT_COMMA_C_LOCALE(guard)                                               \
    if (!(guard).cLocaleUsesComma())                                                     \
        GTEST_SKIP() << "No comma-decimal C locale (fr_FR, de_DE) on this machine."

#pragma region helpers

TEST(FITSnumeric_Parse, ReadsLikeStod)
{
    EXPECT_EQ(parseDouble("0.5"), 0.5);
    EXPECT_EQ(parseDouble("-1.5e3"), -1500.0);
    EXPECT_EQ(parseDouble("1.5E+3"), 1500.0);
    EXPECT_EQ(parseDouble("+2.5"), 2.5);
    EXPECT_EQ(parseDouble(".5"), 0.5);
    EXPECT_EQ(parseDouble("5."), 5.0);
    EXPECT_EQ(parseDouble("42"), 42.0);
    EXPECT_TRUE(std::isnan(parseDouble("nan")));
    EXPECT_TRUE(std::isinf(parseDouble("-inf")));
}

TEST(FITSnumeric_Parse, IsCorrectlyRounded)
{
    EXPECT_EQ(parseDouble("0.1"), 0.1);
    EXPECT_EQ(parseDouble("1.2345678901234567"), 1.2345678901234567);
    EXPECT_EQ(parseFloat("0.1"), 0.1f);
    EXPECT_EQ(parseFloat("3.1415927"), 3.1415927f);
}

TEST(FITSnumeric_Parse, ReportsWhatItConsumed)
{
    size_t consumed = 0;

    EXPECT_EQ(parseDouble("  3.25", &consumed), 3.25);
    EXPECT_EQ(consumed, 6u);

    EXPECT_EQ(parseDouble("1.5abc", &consumed), 1.5);
    EXPECT_EQ(consumed, 3u);

    EXPECT_EQ(parseDouble("1e", &consumed), 1.0);
    EXPECT_EQ(consumed, 1u);

    EXPECT_EQ(parseFloat("+2.5 ", &consumed), 2.5f);
    EXPECT_EQ(consumed, 4u);
}

TEST(FITSnumeric_Parse, CommaIsNotADecimalPoint)
{
    size_t consumed = 0;

    EXPECT_EQ(parseDouble("3,14", &consumed), 3.0);
    EXPECT_EQ(consumed, 1u);
}

TEST(FITSnumeric_Parse, HexadecimalIsRead)
{
    size_t consumed = 0;

    EXPECT_EQ(parseDouble("0x10", &consumed), 16.0);
    EXPECT_EQ(consumed, 4u);

    EXPECT_EQ(parseDouble("-0x10", &consumed), -16.0);
    EXPECT_EQ(consumed, 5u);

    EXPECT_EQ(parseDouble("+0XfF"), 255.0);
    EXPECT_EQ(parseDouble("  0x10", &consumed), 16.0);
    EXPECT_EQ(consumed, 6u);

    EXPECT_EQ(parseDouble("0x1.8p1"), 3.0);
    EXPECT_EQ(parseDouble("0x.8"), 0.5);
    EXPECT_EQ(parseFloat("0x10"), 16.f);
    EXPECT_EQ(parseFloat("-0x1.8p0"), -1.5f);

    // No digit after the prefix: that's the number 0, then an 'x'.
    EXPECT_EQ(parseDouble("0x", &consumed), 0.0);
    EXPECT_EQ(consumed, 1u);
    EXPECT_EQ(parseDouble("0xZZ", &consumed), 0.0);
    EXPECT_EQ(consumed, 1u);
    EXPECT_EQ(parseDouble("0x-5", &consumed), 0.0);
    EXPECT_EQ(consumed, 1u);

    EXPECT_THROW(parseDouble("0x1p99999"), std::out_of_range);
    EXPECT_THROW(parseFloat("0x1p200"), std::out_of_range);
}

TEST(FITSnumeric_Parse, IntegerBase)
{
    EXPECT_EQ(integerBase("0x10"), 16);
    EXPECT_EQ(integerBase("0XFF"), 16);
    EXPECT_EQ(integerBase("-0x10"), 16);
    EXPECT_EQ(integerBase("+0x10"), 16);
    EXPECT_EQ(integerBase("  0x10"), 16);

    EXPECT_EQ(integerBase("16"), 10);
    EXPECT_EQ(integerBase("010"), 10); // not octal
    EXPECT_EQ(integerBase(""), 10);
    EXPECT_EQ(integerBase("0x"), 10);
    EXPECT_EQ(integerBase("0xZZ"), 10);
    EXPECT_EQ(integerBase("0x.8"), 10);
    EXPECT_EQ(integerBase("+-0x10"), 10);
}

TEST(FITSnumeric_Parse, IsHexNumber)
{
    EXPECT_TRUE(isHexNumber("0x10"));
    EXPECT_TRUE(isHexNumber("-0xFF"));
    EXPECT_TRUE(isHexNumber("0x1.8p1"));
    EXPECT_TRUE(isHexNumber("0xFFFFFFFFFFFFFFFF"));

    EXPECT_FALSE(isHexNumber(""));
    EXPECT_FALSE(isHexNumber("16"));
    EXPECT_FALSE(isHexNumber("hello"));
    EXPECT_FALSE(isHexNumber("0x"));
    EXPECT_FALSE(isHexNumber("0x10 foo"));
    EXPECT_FALSE(isHexNumber("0xZZ"));
}

TEST(FITSnumeric_Parse, NoNumberThrowsInvalidArgument)
{
    EXPECT_THROW(parseDouble(""), std::invalid_argument);
    EXPECT_THROW(parseDouble("   "), std::invalid_argument);
    EXPECT_THROW(parseDouble("abc"), std::invalid_argument);
    EXPECT_THROW(parseDouble("."), std::invalid_argument);
    EXPECT_THROW(parseDouble("+-1"), std::invalid_argument);
    EXPECT_THROW(parseFloat("abc"), std::invalid_argument);
}

TEST(FITSnumeric_Parse, TooLargeOrSmallThrowsOutOfRange)
{
    EXPECT_THROW(parseDouble("1e999"), std::out_of_range);
    EXPECT_THROW(parseDouble("-1e999"), std::out_of_range);
    EXPECT_THROW(parseDouble("1e-999"), std::out_of_range);
    EXPECT_THROW(parseFloat("1e50"), std::out_of_range);
    EXPECT_THROW(parseFloat("1e-50"), std::out_of_range);
}

TEST(FITSnumeric_Parse, SubnormalsAreValues)
{
    EXPECT_GT(parseDouble("1e-320"), 0.0);
    EXPECT_GT(parseFloat("1e-40"), 0.f);
}

TEST(FITSnumeric_Format, MatchesUppercaseG)
{
    EXPECT_EQ(formatReal(0.5, 15), "0.5");
    EXPECT_EQ(formatReal(4.0, 15), "4");
    EXPECT_EQ(formatReal(-150.5, 15), "-150.5");
    EXPECT_EQ(formatReal(0.0018888888, 15), "0.0018888888");
    EXPECT_EQ(formatReal(1.5e300, 15), "1.5E+300");
    EXPECT_EQ(formatReal(1e-5, 15), "1E-05");
    EXPECT_EQ(formatReal(1.0 / 3.0, 6), "0.333333");
}

TEST(FITSnumeric_Format, FixedMatchesToString)
{
    EXPECT_EQ(formatFixed(1.5, 6), "1.500000");
    EXPECT_EQ(formatFixed(-0.25, 2), "-0.25");
    EXPECT_EQ(formatFixed(1.5, 6), std::to_string(1.5));
}

TEST(FITSnumeric_Format, ClassicLocaleScopeGivesTheLocaleBack)
{
    CommaDecimalLocale guard;

    std::ostringstream stream;
    {
        const ClassicLocaleScope classicNumbers(stream);
        stream << 1.5 << ' ';
    }
    stream << 1.5;

    EXPECT_EQ(stream.str(), "1.5 1,5");
}

TEST(FITSnumeric_CommaLocale, HelpersStillUseADot)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    // Proves the locale really breaks what the helpers replace.
    EXPECT_EQ(std::stod("0.5"), 0.0);

    size_t consumed = 0;
    EXPECT_EQ(parseDouble("0.5", &consumed), 0.5);
    EXPECT_EQ(consumed, 3u);
    EXPECT_EQ(parseFloat("0.1"), 0.1f);
    EXPECT_EQ(parseDouble("1.5e3"), 1500.0);

    EXPECT_EQ(parseDouble("3,14", &consumed), 3.0);
    EXPECT_EQ(consumed, 1u);

    EXPECT_EQ(formatReal(0.5, 15), "0.5");
    EXPECT_EQ(formatReal(1.5e300, 15), "1.5E+300");
    EXPECT_EQ(formatFixed(0.5, 6), "0.500000");
}

TEST(FITSnumeric_CommaLocale, FormatIgnoresTheCppGlobalLocale)
{
    // Needs no installed locale: this is the state a stream would pick up by default.
    CommaDecimalLocale guard;

    std::ostringstream printed;
    printed << 1.5;
    ASSERT_EQ(printed.str(), "1,5");

    EXPECT_EQ(formatReal(1.5, 15), "1.5");
}

#pragma endregion

#pragma region header keywords

TEST(FITShdu_Hexadecimal, ReturnedInTheTypeRequested)
{
    FITShdu hdu;

    hdu.ValueForKey("KEYH", std::string("0x7F"), std::string("hexadecimal"));

    EXPECT_EQ(hdu.GetValueForKey("KEYH"), "0x7F");
    EXPECT_EQ(hdu.GetInt8ValueForKey("KEYH"), int8_t(127));
    EXPECT_EQ(hdu.GetUInt8ValueForKey("KEYH"), uint8_t(127));
    EXPECT_EQ(hdu.GetInt16ValueForKey("KEYH"), int16_t(127));
    EXPECT_EQ(hdu.GetUInt16ValueForKey("KEYH"), uint16_t(127));
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYH"), int32_t(127));
    EXPECT_EQ(hdu.GetUInt32ValueForKey("KEYH"), uint32_t(127));
    EXPECT_EQ(hdu.GetInt64ValueForKey("KEYH"), int64_t(127));
    EXPECT_EQ(hdu.GetUInt64ValueForKey("KEYH"), uint64_t(127));
    EXPECT_EQ(hdu.GetFloatValueForKey("KEYH"), 127.f);
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYH"), 127.0);
}

TEST(FITShdu_Hexadecimal, FullWidthOfEachIntegerType)
{
    FITShdu hdu;

    hdu.ValueForKey("U8MAX", std::string("0xFF"), std::string());
    EXPECT_EQ(hdu.GetUInt8ValueForKey("U8MAX"), std::numeric_limits<uint8_t>::max());

    hdu.ValueForKey("I16MAX", std::string("0x7FFF"), std::string());
    EXPECT_EQ(hdu.GetInt16ValueForKey("I16MAX"), std::numeric_limits<int16_t>::max());

    hdu.ValueForKey("U16MAX", std::string("0xFFFF"), std::string());
    EXPECT_EQ(hdu.GetUInt16ValueForKey("U16MAX"), std::numeric_limits<uint16_t>::max());

    hdu.ValueForKey("I32MIN", std::string("-0x80000000"), std::string());
    EXPECT_EQ(hdu.GetInt32ValueForKey("I32MIN"), std::numeric_limits<int32_t>::min());

    hdu.ValueForKey("U32MAX", std::string("0xFFFFFFFF"), std::string());
    EXPECT_EQ(hdu.GetUInt32ValueForKey("U32MAX"), std::numeric_limits<uint32_t>::max());

    hdu.ValueForKey("I64MIN", std::string("-0x8000000000000000"), std::string());
    EXPECT_EQ(hdu.GetInt64ValueForKey("I64MIN"), std::numeric_limits<int64_t>::min());

    hdu.ValueForKey("I64MAX", std::string("0x7FFFFFFFFFFFFFFF"), std::string());
    EXPECT_EQ(hdu.GetInt64ValueForKey("I64MAX"), std::numeric_limits<int64_t>::max());

    hdu.ValueForKey("U64MAX", std::string("0xFFFFFFFFFFFFFFFF"), std::string());
    EXPECT_EQ(hdu.GetUInt64ValueForKey("U64MAX"), std::numeric_limits<uint64_t>::max());

    // 2^53+1 doesn't survive a double.
    hdu.ValueForKey("U64ODD", std::string("0x20000000000001"), std::string());
    EXPECT_EQ(hdu.GetUInt64ValueForKey("U64ODD"), uint64_t(9007199254740993ULL));
}

TEST(FITShdu_Hexadecimal, OtherStringsAreStillNotNumbers)
{
    FITShdu hdu;

    hdu.ValueForKey("KEYS", std::string("hello"), std::string());
    EXPECT_THROW(hdu.GetInt32ValueForKey("KEYS"), FITSexception);
    EXPECT_THROW(hdu.GetDoubleValueForKey("KEYS"), FITSexception);

    hdu.ValueForKey("KEYT", std::string("0x10 foo"), std::string());
    EXPECT_THROW(hdu.GetInt32ValueForKey("KEYT"), FITSexception);
    EXPECT_THROW(hdu.GetDoubleValueForKey("KEYT"), FITSexception);
}

namespace
{
    // What the getter returned, and whether it warned.
    template<typename Getter>
    auto callAndCaptureWarning(Getter getter, bool& warned)
    {
        testing::internal::CaptureStderr();
        const auto value = getter();
        warned = testing::internal::GetCapturedStderr().find("WARNING") != std::string::npos;
        return value;
    }
}

TEST(FITShdu_RealAsInteger, HexadecimalRealReturnsIntegerPartWithWarning)
{
    FITShdu hdu;
    bool warned = false;

    hdu.ValueForKey("KEYH", std::string("0x1.8p1"), std::string()); // exactly 3

    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt8ValueForKey("KEYH"); }, warned), int8_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetUInt8ValueForKey("KEYH"); }, warned), uint8_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt16ValueForKey("KEYH"); }, warned), int16_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetUInt16ValueForKey("KEYH"); }, warned), uint16_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYH"); }, warned), int32_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetUInt32ValueForKey("KEYH"); }, warned), uint32_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt64ValueForKey("KEYH"); }, warned), int64_t(3));
    EXPECT_TRUE(warned);
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetUInt64ValueForKey("KEYH"); }, warned), uint64_t(3));
    EXPECT_TRUE(warned);

    hdu.ValueForKey("KEYF", std::string("0x1.8p0"), std::string()); // 1.5
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYF"); }, warned), 1);
    EXPECT_TRUE(warned);

    hdu.ValueForKey("KEYN", std::string("-0x1.8p0"), std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYN"); }, warned), -1);
    EXPECT_TRUE(warned);

    hdu.ValueForKey("KEYP", std::string("0x.8"), std::string()); // 0.5
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYP"); }, warned), 0);
    EXPECT_TRUE(warned);

    // The real getters have nothing to warn about.
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetDoubleValueForKey("KEYF"); }, warned), 1.5);
    EXPECT_FALSE(warned);
}

TEST(FITShdu_RealAsInteger, DecimalWarnsOnlyWhenAFractionIsLost)
{
    FITShdu hdu;
    bool warned = false;

    hdu.ValueForKey("KEYD", 1.5, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYD"); }, warned), 1);
    EXPECT_TRUE(warned);

    hdu.ValueForKey("KEYN", -2.75, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYN"); }, warned), -2);
    EXPECT_TRUE(warned);

    // The exponent counts: this is 1500, not 1.
    hdu.ValueForKey("KEYE", std::string("1.5E+3"), key_type::fFloat, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYE"); }, warned), 1500);
    EXPECT_FALSE(warned);

    // How FITS files commonly store BZERO: nothing lost, so nothing to say.
    hdu.ValueForKey("KEYW", std::string("32768."), key_type::fFloat, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetUInt16ValueForKey("KEYW"); }, warned), uint16_t(32768));
    EXPECT_FALSE(warned);

    hdu.ValueForKey("KEYO", 1.0, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYO"); }, warned), 1);
    EXPECT_FALSE(warned);
}

TEST(FITShdu_RealAsInteger, IntegersDoNotWarn)
{
    FITShdu hdu;
    bool warned = false;

    hdu.ValueForKey("KEYI", 42, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYI"); }, warned), 42);
    EXPECT_FALSE(warned);

    hdu.ValueForKey("KEYH", std::string("0x2A"), std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYH"); }, warned), 42);
    EXPECT_FALSE(warned);
}

TEST(FITShdu_RealAsInteger, IntegerPartMustFit)
{
    FITShdu hdu;

    hdu.ValueForKey("KEYB", std::string("1.5E+300"), key_type::fDouble, std::string());
    EXPECT_THROW(hdu.GetInt32ValueForKey("KEYB"), std::out_of_range);
    EXPECT_THROW(hdu.GetUInt64ValueForKey("KEYB"), std::out_of_range);

    hdu.ValueForKey("KEYN", -1.5, std::string());
    EXPECT_THROW(hdu.GetUInt64ValueForKey("KEYN"), std::out_of_range);

    hdu.ValueForKey("KEYH", std::string("0x1p64"), std::string());
    EXPECT_THROW(hdu.GetUInt64ValueForKey("KEYH"), std::out_of_range);
}

TEST(FITShdu_CommaLocale, RealAsIntegerReadsAFullValue)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    FITShdu hdu;
    bool warned = false;

    hdu.ValueForKey("KEYD", 2.5, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYD"); }, warned), 2);
    EXPECT_TRUE(warned);

    hdu.ValueForKey("KEYE", std::string("1.5E+3"), key_type::fFloat, std::string());
    EXPECT_EQ(callAndCaptureWarning([&]{ return hdu.GetInt32ValueForKey("KEYE"); }, warned), 1500);
    EXPECT_FALSE(warned);
}

TEST(FITShdu_Hexadecimal, StaysAStringInTheFile)
{
    {
        const std::shared_ptr<fitsfile> fptr = createEmptyFile();

        FITShdu hdu(fptr);
        hdu.ValueForKey("KEYH", std::string("0x7F"), std::string("hexadecimal"));
        hdu.Write(fptr);

        closeFile(fptr);
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetValueForKey("KEYH"), "0x7F");
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYH"), 127);
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYH"), 127.0);
}

TEST(FITSmanager_Hexadecimal, AppendKeyReadsItAsTheRequestedType)
{
    closeFile(createEmptyFile());

    {
        FITSmanager manager(kTestFile, false);
        manager.MoveToPrimary();
        manager.AppendKey("KEYI", TINT, "0x10");
        manager.AppendKey("KEYL", TLONGLONG, "0x7FFFFFFFFFFFFFFF");
        manager.AppendKey("KEYD", TDOUBLE, "0x10");
        manager.AppendKey("KEYF", TFLOAT, "0x1.8p0");
        manager.Close();
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYI"), 16);
    EXPECT_EQ(hdu.GetInt64ValueForKey("KEYL"), std::numeric_limits<int64_t>::max());
    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("KEYD"), 16.0);
    EXPECT_FLOAT_EQ(hdu.GetFloatValueForKey("KEYF"), 1.5f);
}

TEST(FITSkeyword_CommaLocale, TypeDetectionReadsTheValue)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    // The type depends on the magnitude: read as 1, these would all be floats.
    EXPECT_EQ(FITSkeyword(std::string("0.5"), std::string(), key_type::fUndef).type(), key_type::fFloat);
    EXPECT_EQ(FITSkeyword(std::string("1.5E-50"), std::string(), key_type::fUndef).type(), key_type::fDouble);
    EXPECT_EQ(FITSkeyword(std::string("1.5E+300"), std::string(), key_type::fUndef).type(), key_type::fDouble);
    EXPECT_EQ(FITSkeyword(std::string("0.00000001"), std::string(), key_type::fUndef).type(), key_type::fDouble);
}

TEST(FITShdu_CommaLocale, SetAndGetUseADot)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    FITShdu hdu;

    hdu.ValueForKey("KEYD", 0.0018888888, std::string("a double"));
    EXPECT_EQ(hdu.GetValueForKey("KEYD"), "0.0018888888");
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYD"), 0.0018888888);

    hdu.ValueForKey("KEYF", 0.25f, std::string("a float"));
    EXPECT_EQ(hdu.GetValueForKey("KEYF"), "0.25");
    EXPECT_EQ(hdu.GetFloatValueForKey("KEYF"), 0.25f);

    hdu.ValueForKey("KEYI", 4.0, std::string("integral, but real"));
    EXPECT_EQ(hdu.GetValueForKey("KEYI"), "4.0");
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYI"), 4.0);

    hdu.ValueForKey("KEYE", 6.62607015e-34, std::string("with an exponent"));
    EXPECT_EQ(hdu.GetValueForKey("KEYE"), "6.62607015E-34");
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYE"), 6.62607015e-34);
}

TEST(FITShdu_RealKeywords, ExtremeValuesRoundTrip)
{
    FITShdu hdu;

    // Rounded to too few digits these go past the largest value, and that
    // must only cost more digits, not an exception.
    hdu.ValueForKey("DMAX", std::numeric_limits<double>::max(), std::string());
    EXPECT_EQ(hdu.GetDoubleValueForKey("DMAX"), std::numeric_limits<double>::max());

    hdu.ValueForKey("DLOW", std::numeric_limits<double>::lowest(), std::string());
    EXPECT_EQ(hdu.GetDoubleValueForKey("DLOW"), std::numeric_limits<double>::lowest());

    hdu.ValueForKey("FMAX", std::numeric_limits<float>::max(), std::string());
    EXPECT_EQ(hdu.GetFloatValueForKey("FMAX"), std::numeric_limits<float>::max());

    hdu.ValueForKey("DSUB", std::numeric_limits<double>::denorm_min(), std::string());
    EXPECT_EQ(hdu.GetDoubleValueForKey("DSUB"), std::numeric_limits<double>::denorm_min());

    hdu.ValueForKey("FSUB", std::numeric_limits<float>::denorm_min(), std::string());
    EXPECT_EQ(hdu.GetFloatValueForKey("FSUB"), std::numeric_limits<float>::denorm_min());
}

TEST(FITShdu_CommaLocale, ExtremeValuesRoundTrip)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    FITShdu hdu;

    hdu.ValueForKey("DMAX", std::numeric_limits<double>::max(), std::string());
    EXPECT_EQ(hdu.GetDoubleValueForKey("DMAX"), std::numeric_limits<double>::max());

    hdu.ValueForKey("FMAX", std::numeric_limits<float>::max(), std::string());
    EXPECT_EQ(hdu.GetFloatValueForKey("FMAX"), std::numeric_limits<float>::max());
}

TEST(FITShdu_CommaLocale, FileRoundTrip)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    {
        const std::shared_ptr<fitsfile> fptr = createEmptyFile();

        FITShdu hdu(fptr);
        hdu.ValueForKey("KEYD", 0.0018888888, std::string("a double"));
        hdu.ValueForKey("KEYF", 0.25f, std::string("a float"));
        hdu.ValueForKey("KEYE", 6.62607015e-34, std::string("with an exponent"));
        hdu.Write(fptr);

        closeFile(fptr);
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetValueForKey("KEYD").find(','), std::string::npos);
    EXPECT_EQ(hdu.GetValueForKey("KEYF").find(','), std::string::npos);
    EXPECT_EQ(hdu.GetValueForKey("KEYE").find(','), std::string::npos);

    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("KEYD"), 0.0018888888);
    EXPECT_FLOAT_EQ(hdu.GetFloatValueForKey("KEYF"), 0.25f);
    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("KEYE"), 6.62607015e-34);
}

TEST(FITSmanager_CommaLocale, AppendKeyReadsTheValue)
{
    CommaDecimalLocale guard;
    SKIP_WITHOUT_COMMA_C_LOCALE(guard);

    closeFile(createEmptyFile());

    {
        FITSmanager manager(kTestFile, false);
        manager.MoveToPrimary();
        manager.AppendKey("KEYD", TDOUBLE, "0.5");
        manager.AppendKey("KEYF", TFLOAT, "0.25");
        manager.Close();
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("KEYD"), 0.5);
    EXPECT_FLOAT_EQ(hdu.GetFloatValueForKey("KEYF"), 0.25f);
}

#pragma endregion

#pragma region setters refuse a value of another type

namespace
{
    // The card as it stands in the file, where a quote means a string.
    std::string rawCard(const std::shared_ptr<fitsfile>& fptr, const char* key)
    {
        char card[FLEN_CARD];
        int status = 0;
        if (fits_read_card(fptr.get(), const_cast<char*>(key), card, &status) != 0)
            throw FITSexception(status, "gTest_numeric", "rawCard");

        return std::string(card);
    }
}

TEST(FITSnumeric_Strict, ParseIntegerTakesTheWholeText)
{
    EXPECT_EQ(parseInteger<int>("42"), 42);
    EXPECT_EQ(parseInteger<int>("-42"), -42);
    EXPECT_EQ(parseInteger<int>("+42"), 42);
    EXPECT_EQ(parseInteger<int>("0x2A"), 42);
    EXPECT_EQ(parseInteger<int>("-0x2A"), -42);
    EXPECT_EQ(parseInteger<int>("010"), 10);
    EXPECT_EQ(parseInteger<int8_t>("-0x80"), std::numeric_limits<int8_t>::min());
    EXPECT_EQ(parseInteger<uint64_t>("0xFFFFFFFFFFFFFFFF"), std::numeric_limits<uint64_t>::max());

    for (const char* text : {"", "abc", "1.5", "1.5E+3", "1e3", "32768.", "0x1.8p1", "12abc", " 12", "12 ", "0x", "+-1"})
    {
        EXPECT_THROW(parseInteger<int>(text), std::invalid_argument) << text;
    }

    EXPECT_THROW(parseInteger<int8_t>("128"), std::out_of_range);
    EXPECT_THROW(parseInteger<int8_t>("0x80"), std::out_of_range);
    EXPECT_THROW(parseInteger<uint8_t>("-1"), std::out_of_range);
    EXPECT_THROW(parseInteger<uint64_t>("-0x1"), std::out_of_range);
}

TEST(FITSnumeric_Strict, ParseWholeRealTakesTheWholeText)
{
    EXPECT_EQ(parseWholeReal<double>("1.5"), 1.5);
    EXPECT_EQ(parseWholeReal<double>("3"), 3.0);
    EXPECT_EQ(parseWholeReal<double>("0x1.8p1"), 3.0);
    EXPECT_EQ(parseWholeReal<float>("0.1"), 0.1f);

    EXPECT_THROW(parseWholeReal<double>("1.5abc"), std::invalid_argument);
    EXPECT_THROW(parseWholeReal<double>("3,14"), std::invalid_argument);
    EXPECT_THROW(parseWholeReal<double>("0x"), std::invalid_argument);
    EXPECT_THROW(parseWholeReal<double>(""), std::invalid_argument);
    EXPECT_THROW(parseWholeReal<float>("1e50"), std::out_of_range);
    EXPECT_THROW(parseWholeReal<double>("1e999"), std::out_of_range);
}

TEST(FITSkeyword_Strict, IntegerTypesRefuseAnythingElse)
{
    for (const key_type type : {key_type::fShort, key_type::fInt, key_type::fLong, key_type::fLongLong,
                                key_type::fUShort, key_type::fUInt, key_type::fULong, key_type::fULongLong,
                                key_type::fByte, key_type::fBool})
    {
        FITSkeyword keyword(std::string("1"), type);

        for (const char* text : {"1.5", "1.5E+3", "1e3", "32768.", "0x1.8p1", "12abc", "abc", "", "0x"})
        {
            EXPECT_THROW(keyword.setValue(text), FITSexception) << FITSkeyword::GetDataType(type) << " " << text;
            EXPECT_EQ(keyword.value(), "1") << "a refused value must leave the keyword as it was";
        }

        EXPECT_NO_THROW(keyword.setValue("0x1")) << FITSkeyword::GetDataType(type);
        EXPECT_EQ(keyword.value(), "0x1");
    }
}

TEST(FITSkeyword_Strict, UnsignedTypesRefuseNegatives)
{
    for (const key_type type : {key_type::fUShort, key_type::fUInt, key_type::fULong, key_type::fULongLong,
                                key_type::fByte, key_type::fBool})
    {
        FITSkeyword keyword(std::string("1"), type);
        EXPECT_THROW(keyword.setValue("-1"), FITSexception) << FITSkeyword::GetDataType(type);
    }
}

TEST(FITSkeyword_Strict, HexadecimalFitsEachIntegerType)
{
    const auto accepts = [](key_type type, const char* fits, const char* tooLarge)
    {
        FITSkeyword keyword(std::string("0"), type);
        EXPECT_NO_THROW(keyword.setValue(fits)) << fits;
        EXPECT_THROW(keyword.setValue(tooLarge), FITSexception) << tooLarge;
    };

    accepts(key_type::fShort,     "0x7FFF",             "0x8000");
    accepts(key_type::fInt,       "0x7FFFFFFF",         "0x80000000");
    accepts(key_type::fLong,      "0x7FFFFFFF",         "0x80000000");
    accepts(key_type::fLongLong,  "0x7FFFFFFFFFFFFFFF", "0x8000000000000000");
    accepts(key_type::fUShort,    "0xFFFF",             "0x10000");
    accepts(key_type::fUInt,      "0xFFFFFFFF",         "0x100000000");
    accepts(key_type::fULong,     "0xFFFFFFFF",         "0x100000000");
    accepts(key_type::fULongLong, "0xFFFFFFFFFFFFFFFF", "0x10000000000000000");
    accepts(key_type::fBool,      "0x1",                "0x2");
}

TEST(FITSkeyword_Strict, RealTypesTakeAnyOneNumber)
{
    for (const key_type type : {key_type::fFloat, key_type::fDouble})
    {
        FITSkeyword keyword(std::string("0.0"), type);

        for (const char* text : {"1.5", "1.5E+3", "3", "32768.", "0x10", "0x1.8p1"})
        {
            EXPECT_NO_THROW(keyword.setValue(text)) << text;
        }
        for (const char* text : {"1.5abc", "3,14", "abc", "", "0x", "1e999"})
        {
            EXPECT_THROW(keyword.setValue(text), FITSexception) << text;
        }
    }

    FITSkeyword single(std::string("0.0"), key_type::fFloat);
    EXPECT_THROW(single.setValue("1e50"), FITSexception);

    FITSkeyword twin(std::string("0.0"), key_type::fDouble);
    EXPECT_NO_THROW(twin.setValue("1e50"));
}

TEST(FITSkeyword_Strict, StringsTakeAnything)
{
    FITSkeyword keyword(std::string("x"), key_type::fChar);

    for (const char* text : {"1.5", "abc", "", "0x10"})
    {
        EXPECT_NO_THROW(keyword.setValue(text));
        EXPECT_EQ(keyword.value(), text);
    }
}

TEST(FITSkeyword_Strict, TypedConstructorsRefuseAnotherType)
{
    EXPECT_THROW(FITSkeyword(std::string("1.5"), std::string(), key_type::fLong), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string("1.5"), key_type::fLong), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string("1.5E+3"), key_type::fInt), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string("-1"), key_type::fULong), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string("3000000000"), key_type::fInt), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string("abc"), key_type::fDouble), FITSexception);
    EXPECT_THROW(FITSkeyword(std::string(""), key_type::fDouble), FITSexception);

    EXPECT_NO_THROW(FITSkeyword(std::string("0x10"), key_type::fLong));
    EXPECT_NO_THROW(FITSkeyword(std::string("3"), key_type::fDouble));
    EXPECT_NO_THROW(FITSkeyword(std::string("abc"), key_type::fChar));
    EXPECT_NO_THROW(FITSkeyword(std::string("abc"), key_type::fUndef));
}

TEST(FITSkeyword_Strict, UncheckedIsForWhatAFileHolds)
{
    const FITSkeyword keyword = FITSkeyword::unchecked("1.5", "a comment", key_type::fLong);

    EXPECT_EQ(keyword.value(), "1.5");
    EXPECT_EQ(keyword.comment(), "a comment");
    EXPECT_EQ(keyword.type(), key_type::fLong);
    EXPECT_THROW(keyword.validate(), FITSexception);
}

TEST(FITSkeyword_Strict, SetTypeRefusesAValueOfAnotherType)
{
    FITSkeyword real(std::string("1.5"));
    const key_type before = real.type();

    EXPECT_THROW(real.setType(key_type::fLong), FITSexception);
    EXPECT_EQ(real.type(), before);

    EXPECT_NO_THROW(real.setType(key_type::fDouble));
    EXPECT_EQ(real.type(), key_type::fDouble);
    EXPECT_NO_THROW(real.setType(key_type::fChar));
    EXPECT_EQ(real.type(), key_type::fChar);

    FITSkeyword integer(std::string("5"));
    EXPECT_NO_THROW(integer.setType(key_type::fLong));
    EXPECT_NO_THROW(integer.setType(key_type::fDouble));
}

TEST(FITShdu_Strict, ValueForKeyCanStillChangeTheType)
{
    FITShdu hdu;
    hdu.ValueForKey("KEYE", std::string("7"), key_type::fLong, std::string("a comment"));

    // Value and type change as one: checking the new value against the old
    // type, or the old value against the new one, would refuse both of these.
    EXPECT_NO_THROW(hdu.ValueForKey("KEYE", std::string("1.5"), key_type::fDouble, std::string()));
    EXPECT_EQ(hdu.GetValueForKey("KEYE"), "1.5");
    EXPECT_EQ(hdu.GetEntry("KEYE")->second.type(), key_type::fDouble);
    EXPECT_EQ(hdu.GetEntry("KEYE")->second.comment(), "a comment");

    EXPECT_NO_THROW(hdu.ValueForKey("KEYE", std::string("9"), key_type::fLong, std::string()));
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYE"), 9);
}

TEST(FITSkeyword_Typing, AnExponentMakesAReal)
{
    EXPECT_EQ(FITSkeyword(std::string("1E-5")).type(), key_type::fFloat);
    EXPECT_EQ(FITSkeyword(std::string("1e3")).type(), key_type::fFloat);
    EXPECT_EQ(FITSkeyword(std::string("1E-50")).type(), key_type::fDouble);
    EXPECT_EQ(FITSkeyword(std::string("1E+300")).type(), key_type::fDouble);
    EXPECT_EQ(FITSkeyword(std::string("42")).type(), key_type::fUShort);
    EXPECT_EQ(FITSkeyword(std::string("-42")).type(), key_type::fShort);
}

TEST(FITShdu_Strict, ValueForKeyRefusesAndChangesNothing)
{
    FITShdu hdu;

    // New keyword: not created.
    EXPECT_THROW(hdu.ValueForKey("KEYI", std::string("1.5E+3"), key_type::fLong, std::string()), FITSexception);
    EXPECT_FALSE(hdu.Exists("KEYI"));

    EXPECT_THROW(hdu.ValueForKey("KEYU", std::string("-1"), key_type::fULong, std::string()), FITSexception);
    EXPECT_FALSE(hdu.Exists("KEYU"));

    // Existing keyword: neither its value nor its type change.
    hdu.ValueForKey("KEYE", std::string("7"), key_type::fLong, std::string());
    EXPECT_THROW(hdu.ValueForKey("KEYE", std::string("1.5"), key_type::fInt, std::string()), FITSexception);
    EXPECT_EQ(hdu.GetValueForKey("KEYE"), "7");
    EXPECT_EQ(hdu.GetEntry("KEYE")->second.type(), key_type::fLong);

    // The matching ones go through, hexadecimal included.
    EXPECT_NO_THROW(hdu.ValueForKey("KEYH", std::string("0x10"), key_type::fLong, std::string()));
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYH"), 16);
    EXPECT_NO_THROW(hdu.ValueForKey("KEYR", std::string("3"), key_type::fDouble, std::string()));
    EXPECT_EQ(hdu.GetDoubleValueForKey("KEYR"), 3.0);
}

TEST(FITShdu_Strict, BitpixMayBeNegative)
{
    FITShdu hdu;

    EXPECT_NO_THROW(hdu.ValueForKey("BITPIX", std::string("-32"), std::string()));
    EXPECT_EQ(hdu.GetInt16ValueForKey("BITPIX"), -32);
}

TEST(FITShdu_Strict, WriteRefusesAKeywordBuiltWithTheWrongType)
{
    FITSDictionary keys;
    keys.insert(std::pair<key_code, FITSkeyword>("KEYI", FITSkeyword::unchecked("1.5E+3", std::string(), key_type::fLong)));
    keys.insert(std::pair<key_code, FITSkeyword>("BITPIX", FITSkeyword(std::string("8"), std::string(), key_type::fInt)));
    const FITShdu hdu(keys);

    const std::shared_ptr<fitsfile> fptr = createEmptyFile();
    EXPECT_THROW(hdu.Write(fptr), FITSexception);
    closeFile(fptr);
}

namespace
{
    // Writes a file holding this BZERO, reads it, rewrites it through FITShdu::Write(), and reads it back.
    void writeAndRewriteBzero(double bzero, int decimals)
    {
        {
            const std::shared_ptr<fitsfile> fptr = createEmptyFile();

            int status = 0;
            char bscaleKey[] = "BSCALE", bzeroKey[] = "BZERO";
            fits_update_key_dbl(fptr.get(), bscaleKey, 1.0, 1, nullptr, &status);
            fits_update_key_dbl(fptr.get(), bzeroKey, bzero, decimals, nullptr, &status);
            ASSERT_EQ(status, 0);

            closeFile(fptr);
        }

        const std::shared_ptr<fitsfile> fptr = openFile(READWRITE);
        const FITShdu hdu(fptr);
        EXPECT_NO_THROW(hdu.Write(fptr));
        closeFile(fptr);
    }
}

TEST(FITShdu_FromFile, LegalValuesSurviveRewriting)
{
    {
        const std::shared_ptr<fitsfile> fptr = createEmptyFile();

        int status = 0;
        char blank[] = "BLANK", small[] = "SMALL", name[] = "NAXISNAM", text[] = "foo";
        fits_update_key_lng(fptr.get(), blank, -32768, nullptr, &status);
        fits_update_key_dbl(fptr.get(), small, 1e-5, 1, nullptr, &status);
        fits_update_key_str(fptr.get(), name, text, nullptr, &status);
        ASSERT_EQ(status, 0);

        closeFile(fptr);
    }

    {
        const std::shared_ptr<fitsfile> fptr = openFile(READWRITE);
        const FITShdu hdu(fptr);
        EXPECT_NO_THROW(hdu.Write(fptr));
        closeFile(fptr);
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetInt32ValueForKey("BLANK"), -32768);
    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("SMALL"), 1e-5);
    EXPECT_EQ(hdu.GetValueForKey("NAXISNAM"), "foo");
}

TEST(FITShdu_FromFile, WholeBzeroWrittenAsARealIsAnInteger)
{
    writeAndRewriteBzero(2147483648.0, 9); // 2.147483648E+09: read as 2 by a plain integer conversion

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetEntry("BZERO")->second.type(), key_type::fULongLong);
    EXPECT_EQ(hdu.GetValueForKey("BZERO"), "2147483648");
    EXPECT_EQ(hdu.GetUInt64ValueForKey("BZERO"), uint64_t(2147483648ULL));
    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("BZERO"), 2147483648.0);
}

TEST(FITShdu_FromFile, FractionalBzeroStaysReal)
{
    writeAndRewriteBzero(0.5, 3);

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("BZERO"), 0.5);
}

TEST(FITShdu_FromFile, NegativeBzeroStaysNegative)
{
    writeAndRewriteBzero(-32768.0, 4);

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_DOUBLE_EQ(hdu.GetDoubleValueForKey("BZERO"), -32768.0);
}

TEST(FITSmanager_Strict, AppendKeyRefusesAValueOfAnotherType)
{
    closeFile(createEmptyFile());

    FITSmanager manager(kTestFile, false);
    manager.MoveToPrimary();

    EXPECT_THROW(manager.AppendKey("KEYA", TINT, "1.5E+3"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TINT, "1.5"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TINT, "0x1.8p1"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TINT, "abc"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TLONGLONG, "32768."), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TSHORT, "40000"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TUSHORT, "-1"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TBYTE, "256"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TDOUBLE, "1.5abc"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TFLOAT, "1e50"), FITSexception);
    EXPECT_THROW(manager.AppendKey("KEYA", TLOGICAL, "maybe"), FITSexception);

    // AppendKeyToHeader() reports its own errors and carries on: this one must still get through.
    EXPECT_THROW(manager.AppendKeyToPrimary("KEYA", FITSkeyword::unchecked("1.5", std::string(), key_type::fLong)), FITSexception);

    manager.Close();

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_FALSE(hdu.Exists("KEYA"));
}

TEST(FITSmanager_Strict, AppendKeyWritesEachTypeAsANumber)
{
    closeFile(createEmptyFile());

    {
        FITSmanager manager(kTestFile, false);
        manager.MoveToPrimary();
        manager.AppendKey("KSBYTE", TSBYTE, "-5");
        manager.AppendKey("KBYTE", TBYTE, "5");
        manager.AppendKey("KSHORT", TSHORT, "-300");
        manager.AppendKey("KUSHORT", TUSHORT, "5");
        manager.AppendKey("KUINT", TUINT, "0xFFFFFFFF");
        manager.AppendKey("KULONG", TULONGLONG, "0xFFFFFFFFFFFFFFFF");
        manager.AppendKey("KLOGT", TLOGICAL, "T");
        manager.AppendKey("KLOGF", TLOGICAL, "0");
        manager.AppendKeyToPrimary("KBOOL", FITSkeyword(std::string("1"), std::string(), key_type::fBool));
        manager.Close();
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetInt32ValueForKey("KSBYTE"), -5);
    EXPECT_EQ(hdu.GetInt32ValueForKey("KBYTE"), 5);
    EXPECT_EQ(hdu.GetInt32ValueForKey("KSHORT"), -300);
    EXPECT_EQ(hdu.GetInt32ValueForKey("KUSHORT"), 5);
    EXPECT_EQ(hdu.GetUInt32ValueForKey("KUINT"), std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(hdu.GetUInt64ValueForKey("KULONG"), std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(hdu.GetValueForKey("KLOGT"), "T");
    EXPECT_EQ(hdu.GetValueForKey("KLOGF"), "F");
    EXPECT_EQ(hdu.GetInt32ValueForKey("KBOOL"), 1);
}

TEST(FITSmanager_Numbers, AppendKeyToHeaderDoesNotQuoteThem)
{
    closeFile(createEmptyFile());

    {
        FITSmanager manager(kTestFile, false);
        manager.MoveToPrimary();

        // The types read from a file, as when a header is copied to another one.
        manager.AppendKeyToPrimary("KBYTE", FITSkeyword(std::string("1")));
        manager.AppendKeyToPrimary("KUSHORT", FITSkeyword(std::string("5")));
        manager.AppendKeyToPrimary("KUINT", FITSkeyword(std::string("300")));
        manager.AppendKeyToPrimary("KULONG", FITSkeyword(std::string("70000")));
        manager.AppendKeyToPrimary("KULL", FITSkeyword(std::string("18446744073709551615")));
        manager.AppendKeyToPrimary("KSHORT", FITSkeyword(std::string("-5")));
        manager.AppendKeyToPrimary("KFLOAT", FITSkeyword(std::string("1.5")));
        manager.AppendKeyToPrimary("KHEX", FITSkeyword(std::string("0x10"), key_type::fUInt));
        manager.AppendKeyToPrimary("KSTR", FITSkeyword(std::string("abc")));
        manager.Close();
    }

    const std::shared_ptr<fitsfile> fptr = openFile();

    for (const char* key : {"KBYTE", "KUSHORT", "KUINT", "KULONG", "KULL", "KSHORT", "KFLOAT", "KHEX"})
    {
        EXPECT_EQ(rawCard(fptr, key).find('\''), std::string::npos) << rawCard(fptr, key);
    }
    EXPECT_NE(rawCard(fptr, "KSTR").find('\''), std::string::npos);

    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetUInt8ValueForKey("KBYTE"), uint8_t(1));
    EXPECT_EQ(hdu.GetUInt8ValueForKey("KUSHORT"), uint8_t(5));
    EXPECT_EQ(hdu.GetUInt16ValueForKey("KUINT"), uint16_t(300));
    EXPECT_EQ(hdu.GetUInt32ValueForKey("KULONG"), uint32_t(70000));
    EXPECT_EQ(hdu.GetUInt64ValueForKey("KULL"), std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(hdu.GetInt8ValueForKey("KSHORT"), int8_t(-5));
    EXPECT_EQ(hdu.GetUInt16ValueForKey("KHEX"), uint16_t(16));
    EXPECT_EQ(hdu.GetValueForKey("KSTR"), "abc");
}

TEST(FITShdu_Numbers, WriteDoesNotQuoteZeroAndOne)
{
    {
        const std::shared_ptr<fitsfile> fptr = createEmptyFile();

        FITShdu hdu(fptr);
        hdu.ValueForKey("KONE", std::string("1"), key_type::fByte, std::string());
        hdu.ValueForKey("KBOOL", true, std::string());
        hdu.Write(fptr);

        closeFile(fptr);
    }

    const std::shared_ptr<fitsfile> fptr = openFile();

    EXPECT_EQ(rawCard(fptr, "KONE").find('\''), std::string::npos) << rawCard(fptr, "KONE");
    EXPECT_EQ(rawCard(fptr, "KBOOL").find('\''), std::string::npos) << rawCard(fptr, "KBOOL");

    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetUInt8ValueForKey("KONE"), uint8_t(1));
    EXPECT_TRUE(hdu.GetBoolValueForKey("KBOOL"));
}

TEST(FITSkeyword_Capacity, TypesTakeWhatTheyCanBeWrittenWith)
{
    const auto holds = [](key_type type, const char* lowest, const char* highest, const char* tooLarge)
    {
        EXPECT_NO_THROW(FITSkeyword(std::string(lowest), type)) << lowest;
        EXPECT_NO_THROW(FITSkeyword(std::string(highest), type)) << highest;
        EXPECT_THROW(FITSkeyword(std::string(tooLarge), type), FITSexception) << tooLarge;
    };

    holds(key_type::fShort,     "-32768",               "32767",                "32768");
    holds(key_type::fInt,       "-2147483648",          "2147483647",           "2147483648");
    holds(key_type::fLong,      "-2147483648",          "2147483647",           "2147483648");
    holds(key_type::fLongLong,  "-9223372036854775808", "9223372036854775807",  "9223372036854775808");
    holds(key_type::fByte,      "0",                    "255",                  "256");
    holds(key_type::fUShort,    "0",                    "65535",                "65536");
    holds(key_type::fUInt,      "0",                    "4294967295",           "4294967296");
    holds(key_type::fULong,     "0",                    "4294967295",           "4294967296");
    holds(key_type::fULongLong, "0",                    "18446744073709551615", "18446744073709551616");

    EXPECT_THROW(FITSkeyword(std::string("2"), key_type::fBool), FITSexception);
}

TEST(FITSkeyword_Capacity, SetValueTakesTheSameRangeAsTheConstructor)
{
    FITSkeyword keyword(std::string("12345678"), key_type::fInt);
    EXPECT_EQ(keyword.value(), "12345678");

    EXPECT_NO_THROW(keyword.setValue("40000"));
    EXPECT_EQ(keyword.value(), "40000");
    EXPECT_NO_THROW(keyword.setValue("87654321"));
    EXPECT_EQ(keyword.value(), "87654321");

    EXPECT_THROW(keyword.setValue("3000000000"), FITSexception);
    EXPECT_EQ(keyword.value(), "87654321");

    const auto sameLimits = [](key_type type, const char* highest, const char* tooLarge)
    {
        FITSkeyword k(std::string("0"), type);
        EXPECT_NO_THROW(k.setValue(highest)) << highest;
        EXPECT_NO_THROW(FITSkeyword(std::string(highest), type)) << highest;
        EXPECT_THROW(k.setValue(tooLarge), FITSexception) << tooLarge;
        EXPECT_THROW(FITSkeyword(std::string(tooLarge), type), FITSexception) << tooLarge;
    };

    sameLimits(key_type::fShort,     "32767",                "32768");
    sameLimits(key_type::fInt,       "2147483647",           "2147483648");
    sameLimits(key_type::fLong,      "2147483647",           "2147483648");
    sameLimits(key_type::fLongLong,  "9223372036854775807",  "9223372036854775808");
    sameLimits(key_type::fByte,      "255",                  "256");
    sameLimits(key_type::fUShort,    "65535",                "65536");
    sameLimits(key_type::fUInt,      "4294967295",           "4294967296");
    sameLimits(key_type::fULong,     "4294967295",           "4294967296");
    sameLimits(key_type::fULongLong, "18446744073709551615", "18446744073709551616");
    sameLimits(key_type::fBool,      "1",                    "2");
}

TEST(FITShdu_Capacity, ValueForKeyTakesWhatTheTypeCanHold)
{
    FITShdu hdu;

    EXPECT_NO_THROW(hdu.ValueForKey("KEYN", std::string("12345678"), key_type::fInt, std::string()));
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYN"), 12345678);

    // Same when the keyword already exists.
    EXPECT_NO_THROW(hdu.ValueForKey("KEYN", std::string("87654321"), key_type::fInt, std::string()));
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYN"), 87654321);

    EXPECT_THROW(hdu.ValueForKey("KEYN", std::string("3000000000"), key_type::fInt, std::string()), FITSexception);
    EXPECT_EQ(hdu.GetInt32ValueForKey("KEYN"), 87654321);
}

TEST(FITShdu_Capacity, WriteKeepsTheWholeValueOfEachType)
{
    {
        const std::shared_ptr<fitsfile> fptr = createEmptyFile();

        FITShdu hdu(fptr);
        hdu.ValueForKey("KSHORT", std::string("-32768"), key_type::fShort, std::string());
        hdu.ValueForKey("KINT", std::string("12345678"), key_type::fInt, std::string());
        hdu.ValueForKey("KLONG", std::string("-2147483648"), key_type::fLong, std::string());
        hdu.ValueForKey("KBYTE", std::string("255"), key_type::fByte, std::string());
        hdu.ValueForKey("KUSHORT", std::string("65535"), key_type::fUShort, std::string()); // negative as a signed short
        hdu.ValueForKey("KUINT", std::string("4294967295"), key_type::fUInt, std::string()); // negative as a signed int
        hdu.ValueForKey("KULONG", std::string("4294967295"), key_type::fULong, std::string());
        hdu.Write(fptr);

        closeFile(fptr);
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetValueForKey("KSHORT"), "-32768");
    EXPECT_EQ(hdu.GetValueForKey("KINT"), "12345678");
    EXPECT_EQ(hdu.GetValueForKey("KLONG"), "-2147483648");
    EXPECT_EQ(hdu.GetValueForKey("KBYTE"), "255");
    EXPECT_EQ(hdu.GetValueForKey("KUSHORT"), "65535");
    EXPECT_EQ(hdu.GetValueForKey("KUINT"), "4294967295");
    EXPECT_EQ(hdu.GetValueForKey("KULONG"), "4294967295");
}

TEST(FITSmanager_Capacity, AppendKeyToHeaderKeepsTheWholeValueOfEachType)
{
    closeFile(createEmptyFile());

    {
        FITSmanager manager(kTestFile, false);
        manager.MoveToPrimary();
        manager.AppendKeyToPrimary("KSHORT", FITSkeyword(std::string("-32768"), key_type::fShort));
        manager.AppendKeyToPrimary("KINT", FITSkeyword(std::string("12345678"), key_type::fInt));
        manager.AppendKeyToPrimary("KLONG", FITSkeyword(std::string("-2147483648"), key_type::fLong));
        manager.AppendKeyToPrimary("KBYTE", FITSkeyword(std::string("255"), key_type::fByte));
        manager.AppendKeyToPrimary("KUSHORT", FITSkeyword(std::string("65535"), key_type::fUShort));
        manager.AppendKeyToPrimary("KUINT", FITSkeyword(std::string("4294967295"), key_type::fUInt));
        manager.AppendKeyToPrimary("KULONG", FITSkeyword(std::string("4294967295"), key_type::fULong));
        manager.Close();
    }

    const std::shared_ptr<fitsfile> fptr = openFile();
    const FITShdu hdu(fptr);
    closeFile(fptr);

    EXPECT_EQ(hdu.GetValueForKey("KSHORT"), "-32768");
    EXPECT_EQ(hdu.GetValueForKey("KINT"), "12345678");
    EXPECT_EQ(hdu.GetValueForKey("KLONG"), "-2147483648");
    EXPECT_EQ(hdu.GetValueForKey("KBYTE"), "255");
    EXPECT_EQ(hdu.GetValueForKey("KUSHORT"), "65535");
    EXPECT_EQ(hdu.GetValueForKey("KUINT"), "4294967295");
    EXPECT_EQ(hdu.GetValueForKey("KULONG"), "4294967295");
}

#pragma endregion

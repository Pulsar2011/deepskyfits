#include <gtest/gtest.h>
#include <DSTfits/FITSexception.h>

using namespace DSL;

#pragma region - Exception

TEST(FITSexception, minimal_constructor_returns_non_empty_what)
{
    // Construct with only an error code (fits_report_error output may vary)
    FITSexception ex(0);
    std::string s = ex.what();
    ASSERT_FALSE(s.empty())<<s;
}


TEST(FITSexception, ctor_code_and_name)
{
    FITSexception ex(2, "ClassOnly");
    std::string s = ex.what();
    ASSERT_FALSE(s.empty())<<s;
    ASSERT_NE(s.find("ClassOnly"), std::string::npos)<<s;
}

TEST(FITSexception, ctor_code_name_and_function)
{
    FITSexception ex(3, "ClassX", "FuncY");
    std::string s = ex.what();
    ASSERT_FALSE(s.empty())<<s;
    ASSERT_NE(s.find("ClassX"), std::string::npos)<<s;
    ASSERT_NE(s.find("FuncY"), std::string::npos)<<s;
}

TEST(FITSexception, what_contains_provided_info)
{
    // Construct with error code and full context
    FITSexception ex(42, "MyClass", "MyFunc", "Something went wrong");
    std::string s = ex.what(); // copy to std::string for searches

    // The what() output should include the class name, function name and message
    ASSERT_NE(s.find("MyClass"), std::string::npos)<<s;
    ASSERT_NE(s.find("MyFunc"), std::string::npos)<<s;
    ASSERT_NE(s.find("Something went wrong"), std::string::npos)<<s;

    // should not be empty
    ASSERT_FALSE(s.empty())<<s;
}

#pragma endregion
#pragma region - Warning

TEST(FITSwarning, minimal_constructor_returns_non_empty_what)
{
    FITSwarning w;
    std::string s = w.what();
    ASSERT_FALSE(s.empty())<<s;
    ASSERT_NE(s.find("WARNING"), std::string::npos)<<s;
}

TEST(FITSwarning, ctor_name_only)
{
    FITSwarning w("OnlyClass");
    std::string s = w.what();
    ASSERT_FALSE(s.empty())<<s;
    ASSERT_NE(s.find("OnlyClass"), std::string::npos)<<s;
}

TEST(FITSwarning, ctor_name_and_function)
{
    FITSwarning w("WC", "WF");
    std::string s = w.what();
    ASSERT_FALSE(s.empty())<<s;
    ASSERT_NE(s.find("WC"), std::string::npos)<<s;
    ASSERT_NE(s.find("WF"), std::string::npos)<<s;
}

TEST(FITSwarning, what_contains_warning_and_context)
{
    FITSwarning w("WarnClass", "WarnFunc", "Non-fatal issue occurred");
    std::string s = w.what();

    // The warning text should include "WARNING" and provided context/message
    ASSERT_NE(s.find("WARNING"), std::string::npos)<<s;
    ASSERT_NE(s.find("WarnClass"), std::string::npos)<<s;
    ASSERT_NE(s.find("WarnFunc"), std::string::npos)<<s;
    ASSERT_NE(s.find("Non-fatal issue occurred"), std::string::npos)<<s;
    ASSERT_FALSE(s.empty())<<s;
}

#pragma endregion

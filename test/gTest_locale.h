#ifndef gTest_locale_h
#define gTest_locale_h

#include <clocale>
#include <locale>
#include <string>

// Puts both the C++ and C locales in a comma-decimal state for one scope. The
// facet needs nothing installed; the C locale is only switched where one exists.
class CommaDecimalLocale
{
    struct CommaDecimalPoint : std::numpunct<char>
    {
        char do_decimal_point() const override { return ','; }
    };

    std::locale previousGlobal_;
    std::string previousNumeric_;

public:
    CommaDecimalLocale()
        : previousGlobal_(std::locale::global(std::locale(std::locale::classic(), new CommaDecimalPoint))),
          previousNumeric_(std::setlocale(LC_NUMERIC, nullptr))
    {
        for (const char* name : {"fr_FR.UTF-8", "fr_FR.utf8", "fr_FR", "de_DE.UTF-8", "de_DE.utf8", "de_DE"})
        {
            if (std::setlocale(LC_NUMERIC, name) != nullptr)
                break;
        }
    }

    ~CommaDecimalLocale()
    {
        std::setlocale(LC_NUMERIC, previousNumeric_.c_str());
        std::locale::global(previousGlobal_);
    }

    // printf, strtod and std::stod follow the C locale, so tests aimed
    // at them prove nothing unless this is true.
    bool cLocaleUsesComma() const
    {
        return std::localeconv()->decimal_point[0] == ',';
    }
};

#endif

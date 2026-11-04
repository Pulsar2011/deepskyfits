//
//  FITSnumeric.cxx
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution,
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#include <DSTfits/FITSnumeric.h>

#include <cctype>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <type_traits>

// Floating-point std::from_chars isn't in every standard library yet; this macro
// is only defined by those that have it. The others go through strtod_l, which
// takes its locale as an argument and so ignores the process's.
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
#  define DSL_HAS_FLOAT_FROM_CHARS 1
#else
#  define DSL_HAS_FLOAT_FROM_CHARS 0
#  include <clocale>
#  include <cstdlib>
#  if defined(__APPLE__)
#    include <xlocale.h>
#  endif
#endif

namespace DSL
{
    namespace
    {
        enum class ConversionStatus { Ok, NoNumber, OutOfRange };

        bool isHexDigit(char c)
        {
            return std::isxdigit(static_cast<unsigned char>(c)) != 0;
        }

        // "0x" with at least one hexadecimal digit behind it, the point of a
        // real number allowed before it. "0x" alone is the number 0, then an 'x'.
        bool hasHexPrefix(const char* first, const char* last)
        {
            if(last - first < 3 || first[0] != '0' || (first[1] != 'x' && first[1] != 'X'))
                return false;

            return isHexDigit(first[2]) || (first[2] == '.' && last - first > 3 && isHexDigit(first[3]));
        }

        // Past leading whitespace and one sign.
        const char* skipSpaceAndSign(const std::string& text)
        {
            const char* first = text.c_str();
            const char* const last = first + text.size();

            while(first != last && std::isspace(static_cast<unsigned char>(*first)))
                ++first;
            if(first != last && (*first == '+' || *first == '-'))
                ++first;

            return first;
        }

        /**
         * @brief Convert the number at the start of [first, last).
         * @param[out] value Value read.
         * @param[out] end One past the last character read.
         */
        template<typename T>
        ConversionStatus convert(const char* first, const char* last, T& value, const char*& end)
        {
#if DSL_HAS_FLOAT_FROM_CHARS
            // std::from_chars takes hexadecimal without its "0x", and
            // would read a '-' after it, which isn't a number.
            const char* digits = (first != last && *first == '-') ? first + 1 : first;
            if(hasHexPrefix(digits, last))
            {
                T magnitude{};
                const std::from_chars_result hex = std::from_chars(digits + 2, last, magnitude, std::chars_format::hex);
                end = hex.ptr;

                if(hex.ec == std::errc::result_out_of_range)
                    return ConversionStatus::OutOfRange;

                value = (digits != first) ? -magnitude : magnitude;
                return ConversionStatus::Ok;
            }

            const std::from_chars_result result = std::from_chars(first, last, value);
            end = result.ptr;

            if(result.ec == std::errc::invalid_argument)
                return ConversionStatus::NoNumber;
            if(result.ec == std::errc::result_out_of_range)
                return ConversionStatus::OutOfRange;

            return ConversionStatus::Ok;
#else
            // Never freed: one "C" locale object serves the whole process.
            static const locale_t cLocale = newlocale(LC_ALL_MASK, "C", static_cast<locale_t>(0));

            const std::string bounded(first, last); // strtod_l has no end argument.
            char* parsedEnd = nullptr;

            const int savedErrno = errno;
            errno = 0;
            if constexpr(std::is_same_v<T, float>)
                value = strtof_l(bounded.c_str(), &parsedEnd, cLocale);
            else
                value = strtod_l(bounded.c_str(), &parsedEnd, cLocale);
            const bool rangeError = (errno == ERANGE);
            errno = savedErrno;

            end = first + (parsedEnd - bounded.c_str());

            if(parsedEnd == bounded.c_str())
                return ConversionStatus::NoNumber;

            // ERANGE also flags subnormals, which are perfectly good values.
            if(rangeError && (std::isinf(value) || value == T(0)))
                return ConversionStatus::OutOfRange;

            return ConversionStatus::Ok;
#endif
        }

        template<typename T>
        T parseReal(const std::string& text, size_t* consumed, const char* caller)
        {
            const char* const begin = text.c_str();
            const char* const end   = begin + text.size();

            const char* number = begin;
            while(number != end && std::isspace(static_cast<unsigned char>(*number)))
                ++number;

            // std::from_chars takes a leading '-' but no '+', and "+-1" is no number.
            const bool plus = (number != end && *number == '+');
            if(plus && number + 1 != end && number[1] == '-')
                throw std::invalid_argument(std::string(caller) + ": no number in '" + text + "'");
            if(plus)
                ++number;

            T value{};
            const char* parsedEnd = number;

            switch(convert(number, end, value, parsedEnd))
            {
                case ConversionStatus::NoNumber:
                    throw std::invalid_argument(std::string(caller) + ": no number in '" + text + "'");
                case ConversionStatus::OutOfRange:
                    throw std::out_of_range(std::string(caller) + ": value out of range in '" + text + "'");
                case ConversionStatus::Ok:
                    break;
            }

            if(consumed != nullptr)
                *consumed = static_cast<size_t>(parsedEnd - begin);

            return value;
        }
    }

    double parseDouble(const std::string& text, size_t* consumed)
    {
        return parseReal<double>(text, consumed, "parseDouble");
    }

    float parseFloat(const std::string& text, size_t* consumed)
    {
        return parseReal<float>(text, consumed, "parseFloat");
    }

    int integerBase(const std::string& text)
    {
        const char* const last = text.c_str() + text.size();
        const char* const digits = skipSpaceAndSign(text);

        return (hasHexPrefix(digits, last) && digits[2] != '.') ? 16 : 10;
    }

    bool isHexNumber(const std::string& text)
    {
        if(!hasHexPrefix(skipSpaceAndSign(text), text.c_str() + text.size()))
            return false;

        try
        {
            size_t consumed = 0;
            parseDouble(text, &consumed);
            return consumed == text.size();
        }
        catch(const std::out_of_range&)
        {
            return false;
        }
        catch(const std::invalid_argument&)
        {
            return false;
        }
    }

    std::string formatReal(double value, int significantDigits)
    {
        // A stream with no floatfield set and uppercase on is defined as "%.*G".
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::uppercase << std::setprecision(significantDigits) << value;
        return stream.str();
    }

    std::string formatFixed(double value, int decimals)
    {
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::fixed << std::setprecision(decimals) << value;
        return stream.str();
    }
}

//
//  FITSnumeric.h
//  DeepSkyLib
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution,
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#ifndef _DSL_FITSnumeric_
#define _DSL_FITSnumeric_

#include <charconv>
#include <cstddef>
#include <locale>
#include <ostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

namespace DSL
{
    /**
     * @brief Read a real number with '.' as the decimal point, whatever the locale.
     * @details
     *  Replaces std::stod, which follows the process's C locale: an application that sets
     *  a comma-decimal locale (QApplication does, from the system) would read "0.5" as 0.
     *  Neither reads nor changes any locale setting, so it is safe from any thread.
     *
     *  Otherwise behaves like std::stod: leading whitespace and a sign are accepted, only
     *  the leading number is read, hexadecimal is read ("0x10" is 16, "0x1.8p1" is 3), and
     *  "nan" and "inf" are numbers. One difference: a subnormal value is returned rather
     *  than reported as out of range.
     *
     * @param text Text to read.
     * @param consumed If not null, receives how many characters were read, leading whitespace included.
     * @return The value read.
     * @throws std::invalid_argument if @p text doesn't start with a number.
     * @throws std::out_of_range if the value doesn't fit a double.
     */
    double parseDouble(const std::string& text, size_t* consumed = nullptr);

    /**
     * @brief Read a real number as a float, with '.' as the decimal point, whatever the locale.
     * @details Same as parseDouble(), for a float.
     * @param text Text to read.
     * @param consumed If not null, receives how many characters were read, leading whitespace included.
     * @return The value read.
     * @throws std::invalid_argument if @p text doesn't start with a number.
     * @throws std::out_of_range if the value doesn't fit a float.
     */
    float parseFloat(const std::string& text, size_t* consumed = nullptr);

    /**
     * @brief Base to read an integer in: 16 if the text is written "0x...", otherwise 10.
     * @details
     *  For std::stoi and its family, which take the "0x" and the sign themselves in base 16:
     *  @code
     *  std::stoull(text, nullptr, integerBase(text));
     *  @endcode
     *  Unlike their base 0, this doesn't read a leading zero as octal: "010" is 10.
     * @param text Text to read.
     * @return 16 or 10.
     */
    int integerBase(const std::string& text);

    /**
     * @brief Whether the whole text is one hexadecimal number, integer ("0x10") or real ("0x1.8p1").
     * @details A keyword holding this is stored as a string, as FITS has no hexadecimal
     *          values, but can still be asked for as a number.
     * @param text Text to examine.
     * @return True if the text is a hexadecimal number and nothing else.
     */
    bool isHexNumber(const std::string& text);

    /**
     * @brief Read text that must be exactly one integer of type @p T, decimal or hexadecimal.
     * @details For values whose type is a promise: unlike std::stoi, which stops at the first
     *          character it can't read, nothing in the text is ignored. "1.5", "1.5E+3" and
     *          "12abc" are refused rather than read as 1 or 12, and nothing wraps.
     * @tparam T Integer type to read.
     * @param text Text to read, such as "42", "-42", "0x2A".
     * @return The value read.
     * @throws std::invalid_argument if the text isn't an integer.
     * @throws std::out_of_range if the integer doesn't fit @p T, a negative one in an unsigned type included.
     */
    template<typename T>
    T parseInteger(const std::string& text)
    {
        const int base = integerBase(text);

        // std::from_chars takes hexadecimal without its "0x".
        std::string digits = text;
        if(base == 16)
            digits.erase(digits.find_first_of("xX") - 1, 2);

        const char* first = digits.data();
        const char* const last = first + digits.size();

        // std::from_chars takes a leading '-' but no '+'.
        if(digits.size() > 1 && digits[0] == '+' && digits[1] != '-')
            ++first;

        T value{};
        const std::from_chars_result result = std::from_chars(first, last, value, base);

        // An unsigned parse rejects '-' as a syntax error, but -1 is a
        // perfectly good integer that this type just can't hold.
        const bool negativeOnUnsigned = std::is_unsigned_v<T> && !text.empty() && text[0] == '-';

        if(result.ec == std::errc::result_out_of_range || negativeOnUnsigned)
            throw std::out_of_range("parseInteger: value out of range: '" + text + "'");
        if(result.ec != std::errc() || result.ptr != last)
            throw std::invalid_argument("parseInteger: not an integer: '" + text + "'");

        return value;
    }

    /**
     * @brief Read text that must be exactly one real number of type @p T.
     * @details Same as parseDouble() and parseFloat(), but nothing in the text is ignored.
     * @tparam T float or double.
     * @param text Text to read. An integer is a valid real number: "3" is 3.0.
     * @return The value read.
     * @throws std::invalid_argument if the text isn't a number.
     * @throws std::out_of_range if the value doesn't fit @p T.
     */
    template<typename T>
    T parseWholeReal(const std::string& text)
    {
        size_t consumed = 0;
        T value{};

        if constexpr(std::is_same_v<T, float>)
            value = parseFloat(text, &consumed);
        else
            value = parseDouble(text, &consumed);

        if(consumed != text.size())
            throw std::invalid_argument("parseWholeReal: not a number: '" + text + "'");

        return value;
    }

    /**
     * @brief Write a real number with '.' as the decimal point, whatever the locale.
     * @details Same text as printf's "%.*G", which itself follows the C locale.
     * @param value Value to write.
     * @param significantDigits Number of significant digits.
     * @return The value as text, e.g. "0.5" or "1.5E+300".
     */
    std::string formatReal(double value, int significantDigits);

    /**
     * @brief Write a real number in fixed notation with '.' as the decimal point, whatever the locale.
     * @details Same text as printf's "%.*f" and, with 6 decimals, as std::to_string, which both follow the C locale.
     * @param value Value to write.
     * @param decimals Number of digits after the point.
     * @return The value as text, e.g. "1.500000".
     */
    std::string formatFixed(double value, int decimals);

    /**
     * @brief Makes a stream write numbers with '.' for as long as this object lives.
     * @details For output to a stream the caller owns, whose locale is given back at the end of the scope.
     */
    class ClassicLocaleScope
    {
        std::ostream& stream_;
        const std::locale previous_;

    public:
        explicit ClassicLocaleScope(std::ostream& stream)
            : stream_(stream), previous_(stream.imbue(std::locale::classic()))
        {}

        ~ClassicLocaleScope() { stream_.imbue(previous_); }

        ClassicLocaleScope(const ClassicLocaleScope&) = delete;
        ClassicLocaleScope& operator=(const ClassicLocaleScope&) = delete;
    };
}

#endif

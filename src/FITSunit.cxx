//
//  FITSunit.cxx
//  FitsExtractor
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution,
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/


#include <cctype>
#include <limits>
#include <stdexcept>
#include <string>

#include <fitsio.h>

#include <DSTfits/FITSunit.h>
#include <DSTfits/FITSexception.h>

namespace DSL
{
#pragma region  - unit class implementation

#pragma region -- helpers
    namespace
    {
        //! Class name carried by every exception this file raises. FITSexception::what()
        //! renders it as "[unit::<function>]", so the message itself carries no prefix.
        const std::string kUnitClass = "unit";

        //! The one symbol that is written as a number: the decimal scale factor of
        //! '10**-16 erg/s/cm**2'. No other figure is a unit, so no other figure parses.
        const std::string kScaleSymbol = "10";

        inline bool isSpaceChar(char c) {return std::isspace(static_cast<unsigned char>(c)) != 0;}
        inline bool isAlphaChar(char c) {return std::isalpha(static_cast<unsigned char>(c)) != 0;}
        inline bool isDigitChar(char c) {return std::isdigit(static_cast<unsigned char>(c)) != 0;}
    }
#pragma endregion

#pragma region -- ctor/dtor
    unit::unit():unitMap()
    {
    }

    /**
     @details Scans the whole string rather than searching it for what it recognises:
     every character has to be accounted for, so a separator that is not understood,
     an exponent without digits or a stray symbol is reported instead of being
     quietly dropped. That matters more here than anywhere else in the class — a unit
     that silently loses its `/` still looks like a unit, and would defeat the whole
     reason for holding units as algebra.

     `10` is a symbol here, so the scale factor of `10**-16 erg/s/cm**2` parses and
     travels with the rest: squaring that unit gives `10**-32`, and multiplying it by
     `10**16 erg**-1` leaves `10**-16.s**-1.cm**-2` — bookkeeping the caller would
     otherwise have to do by hand, and the reason the factor is not simply dropped.
     No other number is a unit, and `10**0` is the dimensionless 1 that the algebra
     says it is: prune() drops it like any other term that reached zero.
     @param u Unit string in the FITS syntax. Empty, or all whitespace, is
     dimensionless.
     @throw DSL::FITSexception on anything that is not a well formed unit, naming the
     offset at fault: BAD_KEYCHAR for a character that has no place there — including
     a number other than 10 — BAD_C2I for an exponent marker with no digits behind it,
     NUM_OVERFLOW for an exponent too large to hold.
     */
    unit::unit(const std::string& u):unitMap()
    {
        size_t i    = 0;
        int    sign = 1;   // +1 after a multiplication, -1 after a division

        while(i < u.size() && isSpaceChar(u[i])) i++;

        while(i < u.size())
        {
            //---- symbol ----------------------------------------------------------
            if(!isAlphaChar(u[i]) && !isDigitChar(u[i]))
                throw FITSexception(BAD_KEYCHAR, kUnitClass, "unit",
                                    "expected a unit symbol at offset "+std::to_string(i)
                                   +" of '"+u+"' ["+std::to_string(__LINE__)+"]");

            const size_t s0 = i;

            if(isDigitChar(u[i]))
            {
                // The decimal scale factor. It is a term like any other -- it takes an
                // exponent, it multiplies and divides, and pow() scales it with the
                // rest -- but 10 is the only figure the syntax gives a meaning to, so a
                // run of digits that is not 10 is a malformed symbol rather than a
                // number nobody can interpret.
                while(i < u.size() && isDigitChar(u[i])) i++;

                if(u.compare(s0, i-s0, kScaleSymbol) != 0)
                    throw FITSexception(BAD_KEYCHAR, kUnitClass, "unit",
                                        "'"+u.substr(s0, i-s0)+"' at offset "+std::to_string(s0)
                                       +" of '"+u+"' is not a unit symbol: 10 is the only "
                                        "number a unit may carry ["+std::to_string(__LINE__)+"]");
            }
            else
            {
                while(i < u.size() && isAlphaChar(u[i])) i++;

                // 'e-' and 'e+' are symbols in their own right, so a trailing sign belongs
                // to the symbol -- unless a digit follows it, which would be an exponent
                // written without its ** or ^, and is left to fail as a stray character.
                if(i < u.size() && (u[i] == '-' || u[i] == '+') &&
                   !(i+1 < u.size() && isDigitChar(u[i+1])))
                    i++;
            }

            const std::string sym = u.substr(s0, i-s0);

            //---- optional exponent, '**n' or '^n' ---------------------------------
            long expo = 1;

            if(i < u.size() && (u[i] == '^' || (u[i] == '*' && i+1 < u.size() && u[i+1] == '*')))
            {
                i += (u[i] == '^')? 1 : 2;

                const size_t e0 = i;
                if(i < u.size() && (u[i] == '-' || u[i] == '+')) i++;
                while(i < u.size() && isDigitChar(u[i])) i++;

                if(i == e0 || (i == e0+1 && !isDigitChar(u[e0])))
                    throw FITSexception(BAD_C2I, kUnitClass, "unit",
                                        "exponent of '"+sym+"' has no digits in '"+u
                                       +"' ["+std::to_string(__LINE__)+"]");

                try
                {
                    expo = std::stol(u.substr(e0, i-e0));
                }
                catch(const std::exception&)
                {
                    throw FITSexception(NUM_OVERFLOW, kUnitClass, "unit",
                                        "exponent of '"+sym+"' is out of range in '"+u
                                       +"' ["+std::to_string(__LINE__)+"]");
                }
            }

            const long total = static_cast<long>(unitMap[sym]) + sign*expo;

            if(total >  std::numeric_limits<int16_t>::max() ||
               total <  std::numeric_limits<int16_t>::min())
                throw FITSexception(NUM_OVERFLOW, kUnitClass, "unit",
                                    "exponent of '"+sym+"' overflows in '"+u
                                   +"' ["+std::to_string(__LINE__)+"]");

            unitMap[sym] = static_cast<int16_t>(total);

            //---- separator --------------------------------------------------------
            const size_t beforeGap = i;
            while(i < u.size() && isSpaceChar(u[i])) i++;

            if(i >= u.size())
                break;

            if(u[i] == '/')
            {
                sign = -1;
                i++;
            }
            else if(u[i] == '.' || u[i] == '*')
            {
                sign = 1;
                i++;
            }
            else if(i > beforeGap)
            {
                sign = 1;   // whitespace alone separates two terms: a multiplication
            }
            else
                throw FITSexception(BAD_KEYCHAR, kUnitClass, "unit",
                                    "unexpected '"+std::string(1,u[i])+"' at offset "
                                   +std::to_string(i)+" of '"+u+"' ["+std::to_string(__LINE__)+"]");

            while(i < u.size() && isSpaceChar(u[i])) i++;

            if(i >= u.size())
                throw FITSexception(BAD_KEYCHAR, kUnitClass, "unit",
                                    "'"+u+"' ends on a separator ["+std::to_string(__LINE__)+"]");
        }

        prune();
    }
#pragma endregion

#pragma region -- private member function
    void unit::prune()
    {
        for(std::map<std::string,int16_t>::iterator it = unitMap.begin(); it != unitMap.end(); )
        {
            if(it->second == 0)
                it = unitMap.erase(it);
            else
                ++it;
        }
    }
#pragma endregion

#pragma region -- operator
    unit& unit::operator*=(const unit& rhs)
    {
        for(std::map<std::string,int16_t>::const_iterator it = rhs.unitMap.cbegin(); it != rhs.unitMap.cend(); ++it)
        {
            const long total = static_cast<long>(unitMap[it->first]) + static_cast<long>(it->second);

            if(total > std::numeric_limits<int16_t>::max() || total < std::numeric_limits<int16_t>::min())
                throw FITSexception(NUM_OVERFLOW, kUnitClass, "operator*=",
                                    "exponent of '"+it->first+"' overflows ["+std::to_string(__LINE__)+"]");

            unitMap[it->first] = static_cast<int16_t>(total);
        }

        prune();

        return *this;
    }

    unit& unit::operator/=(const unit& rhs)
    {
        for(std::map<std::string,int16_t>::const_iterator it = rhs.unitMap.cbegin(); it != rhs.unitMap.cend(); ++it)
        {
            const long total = static_cast<long>(unitMap[it->first]) - static_cast<long>(it->second);

            if(total > std::numeric_limits<int16_t>::max() || total < std::numeric_limits<int16_t>::min())
                throw FITSexception(NUM_OVERFLOW, kUnitClass, "operator/=",
                                    "exponent of '"+it->first+"' overflows ["+std::to_string(__LINE__)+"]");

            unitMap[it->first] = static_cast<int16_t>(total);
        }

        prune();

        return *this;
    }

    bool unit::operator==(const unit& rhs) const
    {
        // Both sides are pruned on every mutation, so no term carries a zero exponent
        // and the maps compare directly.
        return unitMap == rhs.unitMap;
    }

    bool unit::operator!=(const unit& rhs) const
    {
        return !(*this == rhs);
    }

    unit operator*(unit lhs, const unit& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    unit operator/(unit lhs, const unit& rhs)
    {
        lhs /= rhs;
        return lhs;
    }
#pragma endregion

#pragma region -- methods
    /**
     @details Raising to zero gives a dimensionless unit, which is what the algebra
     asks for and what prune() leaves behind once every exponent has been zeroed.
     @param n Power to raise every exponent to.
     @throw DSL::FITSexception, NUM_OVERFLOW, when an exponent leaves the range of its
     storage.
     */
    unit unit::pow(int16_t n) const
    {
        unit out;

        if(n == 0)
            return out;

        for(std::map<std::string,int16_t>::const_iterator it = unitMap.cbegin(); it != unitMap.cend(); ++it)
        {
            const long total = static_cast<long>(it->second) * static_cast<long>(n);

            if(total > std::numeric_limits<int16_t>::max() || total < std::numeric_limits<int16_t>::min())
                throw FITSexception(NUM_OVERFLOW, kUnitClass, "pow",
                                    "exponent of '"+it->first+"' overflows ["+std::to_string(__LINE__)+"]");

            out.unitMap[it->first] = static_cast<int16_t>(total);
        }

        out.prune();

        return out;
    }

    bool unit::dimensionless() const
    {
        return unitMap.empty();
    }

    /**
     @details Canonical: the terms come out ordered by symbol, those with a positive
     exponent first and those with a negative one behind a `/`, so that two units
     that are equal render identically. The result parses back to the same unit.

     With nothing in the numerator every term is written with its own signed
     exponent — `s**-1` rather than `1/s` — which keeps that round trip working,
     `1` not being a symbol the constructor accepts. A dimensionless unit is the
     empty string.

     A scale factor leads the string and keeps its own signed exponent, so a flux
     comes out as `10**-16.erg/cm**2/s` and not as `erg/10**16/cm**2/s`: it is
     the form the standard writes, and dividing by a power of ten is a thing to
     read twice.
     @return The unit in FITS syntax.
     */
    std::string unit::asString() const
    {
        if(unitMap.empty())
            return std::string();

        std::string out;

        std::map<std::string,int16_t>::const_iterator scale = unitMap.find(kScaleSymbol);

        if(scale != unitMap.cend())
        {
            out += scale->first;

            if(scale->second != 1)
                out += "**" + std::to_string(scale->second);
        }

        // A scale factor already opens the string, so what follows may be written as
        // a plain denominator: '10**16/A' needs no help to read back.
        bool hasNumerator = !out.empty();

        for(std::map<std::string,int16_t>::const_iterator it = unitMap.cbegin(); it != unitMap.cend() && !hasNumerator; ++it)
            hasNumerator = (it->second > 0);

        if(!hasNumerator)
        {
            // Everything is in the denominator: write signed exponents so that the
            // string still starts on a symbol and reads back.
            for(std::map<std::string,int16_t>::const_iterator it = unitMap.cbegin(); it != unitMap.cend(); ++it)
            {
                if(!out.empty())
                    out += ".";

                out += it->first + "**" + std::to_string(it->second);
            }

            return out;
        }

        for(std::map<std::string,int16_t>::const_iterator it = unitMap.cbegin(); it != unitMap.cend(); ++it)
        {
            if(it->second <= 0 || it == scale)
                continue;

            if(!out.empty())
                out += ".";

            out += it->first;

            if(it->second != 1)
                out += "**" + std::to_string(it->second);
        }

        for(std::map<std::string,int16_t>::const_iterator it = unitMap.cbegin(); it != unitMap.cend(); ++it)
        {
            if(it->second >= 0 || it == scale)
                continue;

            out += "/" + it->first;

            if(it->second != -1)
                out += "**" + std::to_string(-it->second);
        }

        return out;
    }
#pragma endregion

#pragma endregion
} // namespace DSL

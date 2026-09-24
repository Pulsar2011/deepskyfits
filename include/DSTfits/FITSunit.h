//
//  FITSunit.h
//  FitsExtractor
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution,
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#ifndef _DSL_FITSunit_
#define _DSL_FITSunit_

#include <cstdint>
#include <map>
#include <string>

namespace DSL
{
#pragma region  - unit class definition
    /**
     @class unit
     @brief A physical unit, held as base symbol against exponent.

     @details Parses and renders the unit syntax of the FITS standard, and does the
     algebra on it: multiplying two units adds their exponents, dividing subtracts
     them, and a term reaching zero is dropped. Cancellation therefore falls out of
     ordinary arithmetic, building unit strings by
     concatenation produces things like `erg/ph.e-/s.ph/e-` where `ph` and `e-`
     ought to have disappeared, leaving `erg/s`.

     @par Syntax accepted
     A unit is a sequence of terms separated by `.`, `*` or whitespace for
     multiplication and `/` for division. A term is a symbol, optionally followed by
     an exponent written `**n` or `^n`, where @e n may be signed. A symbol is a run
     of letters, optionally closed by a single `+` or `-` so that `e-` and `e+` read
     as the symbols they are rather than as an arithmetic sign — or the decimal scale
     factor `10`, the one number a unit may carry. Anything else is rejected: the
     parse consumes the whole string and never skips what it cannot explain, so a
     malformed unit is an error rather than a plausible wrong answer.

     @par Two conventions the FITS standard leaves open
     - `/` applies to the term that follows it and to that term only, so
       `erg/ph.e-/s` reads as @f$erg \cdot ph^{-1} \cdot e^- \cdot s^{-1}@f$.
     - Prefixes carry no arithmetic. `km` and `m` are unrelated symbols; this class
       does unit bookkeeping, never unit conversion, and will not tell you that one
       is a thousand of the other. The scale factor `10` is the exception that
       proves it: it is written as a number, so it multiplies as one — `10**-16`
       times `10**16` cancels — but it still will not turn `km` into `10**3.m`.

     @par Rendering
     asString() emits a canonical form — the scale factor first if there is one, then
     terms ordered by symbol, positive exponents first, negative ones behind a `/` —
     which round trips through the constructor. A dimensionless unit renders as the
     empty string.

     @code
     DSL::unit f("e-/s");        // electrons per second
     f /= DSL::unit("e-/ph");    // divide out the quantum efficiency
     f *= DSL::unit("erg/ph");   // hc/lambda turns photons into energy
     f.asString();               // "erg/s": ph and e- cancelled on their own

     DSL::unit v = f.pow(2);     // the matching variance unit, derived not retyped

     DSL::unit s("10**-16 erg/s/cm**2");
     s.pow(2).asString();        // "10**-32.erg**2/cm**4/s**2": the factor scales too
     @endcode
     */
    class unit
    {
        private:
            std::map<std::string,int16_t> unitMap;  //!< map unit string to unit exponent

            //! Drop every term whose exponent reached zero, so that two units differing
            //! only by cancelled terms compare equal and render alike.
            void prune();

        public:
            unit();  //!< Default constructor, dimensionless
            explicit unit(const std::string&);  //!< Constructor from unit string

            unit& operator*=(const unit&);  //!< Multiply unit by another unit
            unit& operator/=(const unit&);  //!< Divide unit by another unit
            unit  pow(int16_t) const;  //!< Raise unit to a power

            bool operator==(const unit&) const;  //!< Compare two units for equality
            bool operator!=(const unit&) const;  //!< Compare two units for inequality
            bool dimensionless() const;  //!< Check if the unit is dimensionless

            std::string asString() const;  //!< Return unit as a string
    };

    unit operator*(unit, const unit&);  //!< Product of two units
    unit operator/(unit, const unit&);  //!< Ratio of two units
#pragma endregion
}

#endif

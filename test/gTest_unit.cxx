//
//  gTest_unit.cxx
//  FitsExtractor
//
//  Created by GILLARD William
//  Centre de Physic des Particules de Marseille
//  Licensed under CC BY-NC 4.0
//  You may share and adapt this code with attribution,
//  but not for commercial purposes.
//  Licence text: https://creativecommons.org/licenses/by-nc/4.0/

#include <gtest/gtest.h>
#include <fitsio.h>

#include <DSTfits/FITSunit.h>
#include <DSTfits/FITSexception.h>

#include <string>

namespace DSL
{
#pragma region -- parsing
    // The three forms the extraction code actually produces. Each one used to be
    // mis-parsed: the division was dropped, 'e-' was cut at the sign, and '**' was
    // not recognised as an exponent marker.
    TEST(unit_Tester, parse_division)
    {
        ASSERT_EQ(unit("erg/ph").asString(), "erg/ph");
        ASSERT_EQ(unit("erg/s").asString(),  "erg/s");

        // A division must not read as a multiplication.
        ASSERT_NE(unit("erg/s"), unit("erg.s"));
    }

    TEST(unit_Tester, parse_electron_symbol)
    {
        // 'e-' is one symbol, not 'e' followed by a sign.
        ASSERT_EQ(unit("e-/s").asString(), "e-/s");
        ASSERT_EQ(unit("e-"), unit("e-"));
        ASSERT_NE(unit("e-"), unit("e"));
    }

    TEST(unit_Tester, parse_exponent_markers)
    {
        // FITS writes '**'; '^' is accepted as the common alternative.
        ASSERT_EQ(unit("ph**2/e-**2").asString(), "ph**2/e-**2");
        ASSERT_EQ(unit("ph^2"), unit("ph**2"));
        ASSERT_EQ(unit("ph**2"), unit("ph.ph"));
        ASSERT_EQ(unit("m**-1"), unit("m^-1"));
    }

    TEST(unit_Tester, parse_separators)
    {
        // '.', '*' and whitespace all multiply.
        ASSERT_EQ(unit("erg.s"), unit("erg s"));
        ASSERT_EQ(unit("erg.s"), unit("erg*s"));
        ASSERT_EQ(unit("  erg / s  "), unit("erg/s"));
    }

    TEST(unit_Tester, parse_empty_is_dimensionless)
    {
        ASSERT_TRUE(unit().dimensionless());
        ASSERT_TRUE(unit("").dimensionless());
        ASSERT_TRUE(unit("   ").dimensionless());
        ASSERT_EQ(unit("").asString(), std::string());
    }

    TEST(unit_Tester, parse_rejects_malformed)
    {
        // The parse accounts for every character rather than skipping what it does
        // not recognise: silence here would put a plausible wrong unit into a header.
        ASSERT_THROW(unit("erg2"),   FITSexception);   // exponent without ** or ^
        ASSERT_THROW(unit("erg**"),  FITSexception);   // exponent with no digits
        ASSERT_THROW(unit("erg**-"), FITSexception);   // sign with no digits
        ASSERT_THROW(unit("erg/"),   FITSexception);   // ends on a separator
        ASSERT_THROW(unit("/s"),     FITSexception);   // starts on a separator
        ASSERT_THROW(unit("erg#s"),  FITSexception);   // unknown separator
        ASSERT_THROW(unit("2erg"),   FITSexception);   // does not start on a symbol
    }

    TEST(unit_Tester, parse_scale_factor)
    {
        // The scale factor a flux column carries: '10' is a symbol, so the whole
        // string parses instead of being refused on its first character.
        ASSERT_EQ(unit("10**-16 erg/s/cm**2").asString(), "10**-16.erg/cm**2/s");
        ASSERT_EQ(unit("10.erg").asString(),   "10.erg");
        ASSERT_EQ(unit("10**16/A").asString(), "10**16/A");
        ASSERT_EQ(unit("10**-16").asString(),  "10**-16");

        ASSERT_EQ(unit("10^-16.erg"), unit("10**-16.erg"));
        ASSERT_EQ(unit("10**-16 erg"), unit("10**-16.erg"));

        // A scale factor is part of the unit: it is not quietly equal to the unit
        // without it, which is the whole point of carrying it.
        ASSERT_NE(unit("10**-16.erg"), unit("erg"));
        ASSERT_NE(unit("10**-16.erg"), unit("10**-15.erg"));
    }

    TEST(unit_Tester, parse_rejects_numbers_that_are_not_ten)
    {
        // 10 is the one number a unit may carry. Anything else is a malformed symbol,
        // not a factor to be guessed at.
        ASSERT_THROW(unit("2"),        FITSexception);
        ASSERT_THROW(unit("1"),        FITSexception);
        ASSERT_THROW(unit("100"),      FITSexception);
        ASSERT_THROW(unit("1/s"),      FITSexception);
        ASSERT_THROW(unit("2**-16"),   FITSexception);
        ASSERT_THROW(unit("erg/1000"), FITSexception);

        // ... and it is reported as the stray character it is.
        try
        {
            unit("100**-16.erg");
            FAIL() << "only 10 may be written as a number";
        }
        catch(const FITSexception& e)
        {
            ASSERT_EQ(e.errorCode(), BAD_KEYCHAR);
        }
    }

    TEST(unit_Tester, error_codes)
    {
        // The code says which kind of malformation it was, so a caller can tell a
        // stray character from an exponent it could not read.
        try
        {
            unit("erg#s");
            FAIL() << "a stray separator should not parse";
        }
        catch(const FITSexception& e)
        {
            ASSERT_EQ(e.errorCode(), BAD_KEYCHAR);
        }

        try
        {
            unit("erg**");
            FAIL() << "an exponent with no digits should not parse";
        }
        catch(const FITSexception& e)
        {
            ASSERT_EQ(e.errorCode(), BAD_C2I);
        }
    }
#pragma endregion

#pragma region -- algebra
    TEST(unit_Tester, cancellation)
    {
        // The case that motivated the class: concatenating the conversions by hand
        // gave "erg/ph.e-/s.ph/e-", where ph and e- should have gone.
        ASSERT_EQ(unit("erg/ph.e-/s.ph/e-").asString(), "erg/s");

        unit f("e-/s");
        f /= unit("e-/ph");    // divide out the quantum efficiency
        f *= unit("erg/ph");   // hc/lambda turns photons into energy

        ASSERT_EQ(f.asString(), "erg/s");
        ASSERT_EQ(f, unit("erg/s"));
    }

    TEST(unit_Tester, multiply_and_divide)
    {
        unit m("m");

        ASSERT_EQ((m*m).asString(), "m**2");
        ASSERT_EQ((m/m).asString(), std::string());
        ASSERT_TRUE((m/m).dimensionless());

        unit a("erg/s");
        a *= unit("s");
        ASSERT_EQ(a, unit("erg"));
    }

    TEST(unit_Tester, power)
    {
        const unit f("erg/s");

        ASSERT_EQ(f.pow(2).asString(), "erg**2/s**2");
        ASSERT_EQ(f.pow(1), f);
        ASSERT_TRUE(f.pow(0).dimensionless());
        ASSERT_EQ(f.pow(-1).asString(), "s/erg");

        // The variance unit is derived from the flux unit, never retyped beside it.
        ASSERT_EQ(unit("e-/s").pow(2), unit("e-**2/s**2"));
    }

    TEST(unit_Tester, scale_factor_algebra)
    {
        // The factor is a term like any other, so it cancels, scales and prunes with
        // the rest -- which is the reason for parsing it instead of dropping it.
        ASSERT_EQ(unit("10**-16.erg")*unit("10**16"), unit("erg"));
        ASSERT_EQ((unit("10**-16.erg")/unit("10**-16")).asString(), "erg");

        ASSERT_EQ(unit("10**-16 erg/s/cm**2").pow(2).asString(),
                  "10**-32.erg**2/cm**4/s**2");

        // 10**0 is 1: the term reaches zero and prune() drops it, as it does for any
        // other symbol.
        ASSERT_TRUE(unit("10**0").dimensionless());
        ASSERT_EQ(unit("10**0.erg"), unit("erg"));
        ASSERT_TRUE(unit("10**-16.erg").pow(0).dimensionless());
    }

    TEST(unit_Tester, equality_ignores_order_and_cancelled_terms)
    {
        ASSERT_EQ(unit("erg.s"), unit("s.erg"));
        ASSERT_EQ(unit("erg.m/m"), unit("erg"));
        ASSERT_NE(unit("erg/s"), unit("erg/s**2"));
    }
#pragma endregion

#pragma region -- rendering
    TEST(unit_Tester, asString_round_trips)
    {
        const char* cases[] = {"erg/s", "erg/ph", "e-/s", "ph**2/e-**2",
                               "erg", "erg.s", "erg**2/s**2", "",
                               "10**-16.erg/cm**2/s", "10**16/A", "10.erg",
                               "10**-16", "10**-16.s**-1"};

        for(size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++)
        {
            const unit a((std::string(cases[i])));
            const unit b(a.asString());

            ASSERT_EQ(a, b) << "round trip failed on '" << cases[i]
                            << "' -> '" << a.asString() << "'";
        }
    }

    TEST(unit_Tester, asString_is_canonical)
    {
        // Same unit, four spellings, one rendering.
        ASSERT_EQ(unit("s.erg").asString(),      unit("erg.s").asString());
        ASSERT_EQ(unit("erg s").asString(),      unit("erg.s").asString());
        // '/' binds to the term that follows it and to that one only, so the '.'
        // after m**-1 puts s back in the numerator: this is erg.s, not erg/s.
        ASSERT_EQ(unit("erg.m/m.s").asString(),  unit("erg.s").asString());
        ASSERT_EQ(unit("erg^1/s^1").asString(),  unit("erg/s").asString());
    }

    TEST(unit_Tester, asString_pure_denominator_round_trips)
    {
        // Nothing in the numerator: written with signed exponents rather than as
        // "1/s", so that the string still starts on a symbol and parses back.
        const unit s = unit("s").pow(-1);

        ASSERT_EQ(s.asString(), "s**-1");
        ASSERT_EQ(unit(s.asString()), s);
    }
#pragma endregion
}

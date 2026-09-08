#include <gtest/gtest.h>
#include <DSTfits/FITSwcs.h>

#include <cmath>
#include <stdexcept>

using namespace DSL;
using DSL::CoordFrame;

// NOTE: reference values below are taken from independent, published sources
// (Astropy documentation, IAU-defined constants, elementary spherical geometry)
// rather than recomputed with SuperNOVAS/NOVAS itself, so that a bug in our
// wrapper cannot "agree with itself".

namespace
{
    constexpr double kDeg = 1.0;
    constexpr double kArcsecToDeg = 1.0 / 3600.0;

    // IAU 1976 mean obliquity of the ecliptic at J2000.0 (84381.448 arcsec).
    constexpr double kObliquityJ2000 = 23.43929111;

    // The three constants that *define* the IAU/Hipparcos galactic frame
    // (ESA SP-1200, Vol. 1, Sect. 1.5.3), in J2000 equatorial coordinates.
    constexpr double kNGP_RA   = 192.85948;  //!< RA of the north galactic pole [deg]
    constexpr double kNGP_DEC  =  27.12825;  //!< Dec of the north galactic pole [deg]
    constexpr double kNCP_GLON = 122.93192;  //!< galactic longitude of the north celestial pole [deg]

    // Galactic centre, i.e. galactic (l,b) = (0,0), in J2000 equatorial coordinates.
    constexpr double kGC_RA  = 266.40510;  //!< [deg]
    constexpr double kGC_DEC = -28.93617;  //!< [deg]

    // General precession in ecliptic longitude is ~50.29 arcsec/yr, so a fixed
    // direction shifts by ~0.6985 deg in ecliptic longitude over the 50 Julian
    // years separating B1950.0 from J2000.0.
    constexpr double kPrecessionB1950ToJ2000 = 0.6985;  //!< [deg]

    //! Smallest signed difference between two longitudes, in degrees, so that
    //! comparisons stay valid across the 0/360 wrap.
    double lonDiff(const double& a, const double& b)
    {
        return std::fmod(a - b + 540.0, 360.0) - 180.0;
    }
}

#pragma region angularSeparation - pure spherical geometry, no external library involved

TEST(SkyCoo_angularSeparation, samePointIsZero)
{
    EquatorialCoordinates a(123.4560, -12.3450);
    EquatorialCoordinates b(123.4560, -12.3450);
    EXPECT_NEAR(a.angularSeparation(b), 0.0, 1e-15);
}

TEST(SkyCoo_angularSeparation, quarterCircleOnEquator)
{
    // Two points on the celestial equator, 90 deg apart in RA.
    EquatorialCoordinates a(0.0, 0.0);
    EquatorialCoordinates b(90.0, 0.0);
    EXPECT_NEAR(a.angularSeparation(b), 90.0, 1e-12);
}

TEST(SkyCoo_angularSeparation, antipodalPoints)
{
    EquatorialCoordinates a(10.0, 20.0);
    EquatorialCoordinates b(190.0, -20.0);
    EXPECT_NEAR(a.angularSeparation(b), 180.0, 1e-10);
}

TEST(SkyCoo_angularSeparation, meridianArc)
{
    // Same RA (same meridian): separation is just the declination difference.
    EquatorialCoordinates a(45.0, 0.0);
    EquatorialCoordinates b(45.0, 45.0);
    EXPECT_NEAR(a.angularSeparation(b), 45.0, 1e-12);
}

TEST(SkyCoo_angularSeparation, smallSeparation)
{
    // Two points ~0.1 arcsecond apart
    EquatorialCoordinates a(100.0, 20.0);
    EquatorialCoordinates b(100.0 + 1e-4, 20.0);  // ~0.36 arcsec in RA
    // Expected: 1e-4 * cos(20°) ≈ 9.3969e-5 degrees

    double sepa = a.angularSeparation(b);
    double sepb = b.angularSeparation(a);

    const double expectedSep = 1e-4 * std::cos(20.0 * M_PI / 180.0);
    EXPECT_NEAR(sepa, expectedSep, 1e-12);  // ← Compute expectation, don't hardcode
    EXPECT_NEAR(sepb, expectedSep, 1e-12);  // ← Compute expectation, don't hardcode
}

TEST(SkyCoo_angularSeparation, isSymmetricForLargeSeparations)
{
    // The implementation switches from the haversine to the law of cosines
    // beyond 90 deg; the result must stay symmetric across that switch.
    EquatorialCoordinates a(15.0, 70.0);
    EquatorialCoordinates b(220.0, -55.0);
    EXPECT_NEAR(a.angularSeparation(b), b.angularSeparation(a), 1e-12);
    EXPECT_GT(a.angularSeparation(b), 90.0);
}

TEST(SkyCoo_angularSeparation, wrapsAroundZeroRA)
{
    // 359 deg and 1 deg are 2 deg apart, not 358.
    EquatorialCoordinates a(359.0, 0.0);
    EquatorialCoordinates b(1.0, 0.0);
    EXPECT_NEAR(a.angularSeparation(b), 2.0, 1e-12);
}

TEST(SkyCoo_angularSeparation, poleToPoleIsOneEighty)
{
    EquatorialCoordinates north(0.0, 90.0);
    EquatorialCoordinates south(180.0, -90.0);
    EXPECT_NEAR(north.angularSeparation(south), 180.0, 1e-10);
}

#pragma endregion

#pragma region Equatorial <-> Ecliptic - exact geometric identities (obliquity of the ecliptic)

TEST(SkyCoo_Ecliptic, vernalEquinoxIsCommonOrigin)
{
    // The vernal equinox (RA=0,Dec=0) is, by definition, also (elon=0,elat=0)
    // in the ecliptic system: it is independent of any astrometry library.
    EquatorialCoordinates equ(0.0, 0.0);
    EclipticCoordinates ecl;
    equ.toEcliptic(&ecl);

    EXPECT_NEAR(ecl.getELON(), 0.0, 1e-5);
    EXPECT_NEAR(ecl.getELAT(), 0.0, 1e-5);
}

TEST(SkyCoo_Ecliptic, northCelestialPole)
{
    // The north celestial pole projects onto the ecliptic system at
    // longitude 90 deg and latitude (90 - obliquity). J2000 mean obliquity
    // is the well known IAU value 23.43929111 deg.
    const double obliquityJ2000 = 23.43929111;

    EquatorialCoordinates equ(0.0, 90.0);
    EclipticCoordinates ecl;
    equ.toEcliptic(&ecl);

    EXPECT_NEAR(ecl.getELON(), 90.0, 1e-2);
    EXPECT_NEAR(ecl.getELAT(), 90.0 - obliquityJ2000, 1e-2);
}

TEST(SkyCoo_Ecliptic, autumnalEquinoxIsCommonOrigin)
{
    // RA=180,Dec=0 is the descending node of the ecliptic on the equator, so it
    // maps to ecliptic longitude 180 with zero latitude.
    EquatorialCoordinates equ(180.0, 0.0);
    EclipticCoordinates ecl;
    equ.toEcliptic(&ecl);

    EXPECT_NEAR(lonDiff(ecl.getELON(), 180.0), 0.0, 1e-5);
    EXPECT_NEAR(ecl.getELAT(), 0.0, 1e-5);
}

TEST(SkyCoo_Ecliptic, summerSolsticePoint)
{
    // Rotating (elon=90, elat=0) about the equinox axis by the obliquity puts
    // it at RA = 90 deg, Dec = +obliquity: the June solstice point.
    EclipticCoordinates ecl(90.0, 0.0);
    EquatorialCoordinates equ;
    ecl.toEquatorial(&equ);

    EXPECT_NEAR(lonDiff(equ.getRA(), 90.0), 0.0, 1e-2);
    EXPECT_NEAR(equ.getDEC(), kObliquityJ2000, 1e-2);
}

TEST(SkyCoo_Ecliptic, winterSolsticePoint)
{
    // ... and the antipodal point lands at RA = 270 deg, Dec = -obliquity.
    EclipticCoordinates ecl(270.0, 0.0);
    EquatorialCoordinates equ;
    ecl.toEquatorial(&equ);

    EXPECT_NEAR(lonDiff(equ.getRA(), 270.0), 0.0, 1e-2);
    EXPECT_NEAR(equ.getDEC(), -kObliquityJ2000, 1e-2);
}

TEST(SkyCoo_Ecliptic, northEclipticPole)
{
    // The north ecliptic pole lies on the RA = 18h meridian at
    // Dec = 90 - obliquity; its longitude in the ecliptic system is undefined.
    EclipticCoordinates nep(0.0, 90.0);
    EquatorialCoordinates equ;
    nep.toEquatorial(&equ);

    EXPECT_NEAR(lonDiff(equ.getRA(), 270.0), 0.0, 1e-2);
    EXPECT_NEAR(equ.getDEC(), 90.0 - kObliquityJ2000, 1e-2);
}

TEST(SkyCoo_Ecliptic, poleSeparationIsTheObliquity)
{
    // The angle between the celestial and the ecliptic pole *is* the obliquity.
    EquatorialCoordinates ncp(0.0, 90.0);

    EclipticCoordinates nep(0.0, 90.0);
    EquatorialCoordinates nepAsEqu;
    nep.toEquatorial(&nepAsEqu);

    EXPECT_NEAR(ncp.angularSeparation(nepAsEqu), kObliquityJ2000, 1e-2);
}

TEST(SkyCoo_Ecliptic, roundTripEquatorialEclipticEquatorial)
{
    EquatorialCoordinates start(83.822083, -5.391111);  // arbitrary mid-sky position
    EclipticCoordinates ecl;
    start.toEcliptic(&ecl);

    EquatorialCoordinates back;
    ecl.toEquatorial(&back);

    EXPECT_NEAR(lonDiff(back.getRA(), start.getRA()), 0.0, 1e-8);
    EXPECT_NEAR(back.getDEC(), start.getDEC(), 1e-8);
}

TEST(SkyCoo_Ecliptic, eclipticLatitudeIsBoundedByObliquityOnTheEquator)
{
    // Every point of the celestial equator lies within +/- obliquity of the
    // ecliptic plane, with the extremes reached at the solstice meridians.
    for (double ra = 0.0; ra < 360.0; ra += 15.0)
    {
        EquatorialCoordinates equ(ra, 0.0);
        EclipticCoordinates ecl;
        equ.toEcliptic(&ecl);

        EXPECT_LE(std::fabs(ecl.getELAT()), kObliquityJ2000 + 1e-3)
            << "failed at RA = " << ra;
    }
}

#pragma endregion

#pragma region Equatorial <-> Galactic - Astropy "Getting started" M31 tutorial example
// https://docs.astropy.org/en/stable/coordinates/ : SkyCoord(ra=10.68470, dec=41.26875,
// frame='icrs').galactic -> (l, b) = (121.17431951, -21.57291513) deg

TEST(SkyCoo_Galactic, M31_equatorialToGalactic)
{
    EquatorialCoordinates equ(10.68470, 41.26875);
    GalacticCoordinates gal;
    equ.toGalactic(&gal);

    EXPECT_NEAR(gal.getGLON(), 121.17431951, 1e-3);
    EXPECT_NEAR(gal.getGLAT(), -21.57291513, 1e-3);
}

TEST(SkyCoo_Galactic, M31_galacticToEquatorial)
{
    GalacticCoordinates gal(121.17431951, -21.57291513);
    EquatorialCoordinates equ;
    gal.toEquatorial(&equ);

    EXPECT_NEAR(equ.getRA(),  10.68470, 1e-3);
    EXPECT_NEAR(equ.getDEC(), 41.26875, 1e-3);
}

TEST(SkyCoo_Galactic, northGalacticPoleLatitudeIsNinety)
{
    // IAU-defined North Galactic Pole, in J2000 equatorial coordinates
    // (RA=192.8595 deg, Dec=+27.1284 deg): by construction its galactic
    // latitude is 90 deg (longitude is undefined at the pole).
    EquatorialCoordinates ngp(192.8595, 27.1284);
    GalacticCoordinates gal;
    ngp.toGalactic(&gal);

    EXPECT_NEAR(gal.getGLAT(), 90.0, 5e-2);
}

TEST(SkyCoo_Galactic, northGalacticPoleEquatorialDirection)
{
    // The reverse of the test above: galactic latitude +90 must come back out
    // at the IAU-defining equatorial position, whatever longitude is fed in.
    GalacticCoordinates ngp(0.0, 90.0);
    EquatorialCoordinates equ;
    ngp.toEquatorial(&equ);

    EXPECT_NEAR(lonDiff(equ.getRA(), kNGP_RA), 0.0, 5e-2);
    EXPECT_NEAR(equ.getDEC(), kNGP_DEC, 5e-2);
}

TEST(SkyCoo_Galactic, galacticCentreIsTheOrigin)
{
    // The IAU J2000 galactic frame puts (l,b) = (0,0) at
    // RA = 17h45m37.2s, Dec = -28d56'10".
    EquatorialCoordinates equ(kGC_RA, kGC_DEC);
    GalacticCoordinates gal;
    equ.toGalactic(&gal);

    EXPECT_NEAR(lonDiff(gal.getGLON(), 0.0), 0.0, 5e-2);
    EXPECT_NEAR(gal.getGLAT(), 0.0, 5e-2);
}

TEST(SkyCoo_Galactic, galacticAnticentreIsTheAntipode)
{
    // (l,b) = (180,0) must land exactly opposite the galactic centre.
    GalacticCoordinates gal(180.0, 0.0);
    EquatorialCoordinates equ;
    gal.toEquatorial(&equ);

    EXPECT_NEAR(lonDiff(equ.getRA(), kGC_RA - 180.0), 0.0, 5e-2);
    EXPECT_NEAR(equ.getDEC(), -kGC_DEC, 5e-2);
}

TEST(SkyCoo_Galactic, northCelestialPoleInGalacticCoordinates)
{
    // The third IAU-defining constant: the north celestial pole sits at
    // galactic longitude 122.93192 deg. Its galactic latitude is the
    // declination of the north galactic pole.
    EquatorialCoordinates ncp(0.0, 90.0);
    GalacticCoordinates gal;
    ncp.toGalactic(&gal);

    EXPECT_NEAR(lonDiff(gal.getGLON(), kNCP_GLON), 0.0, 5e-2);
    EXPECT_NEAR(gal.getGLAT(), kNGP_DEC, 5e-2);
}

TEST(SkyCoo_Galactic, roundTripEquatorialGalacticEquatorial)
{
    EquatorialCoordinates start(201.29825, -43.13111);  // arbitrary southern position
    GalacticCoordinates gal;
    start.toGalactic(&gal);

    EquatorialCoordinates back;
    gal.toEquatorial(&back);

    EXPECT_NEAR(lonDiff(back.getRA(), start.getRA()), 0.0, 1e-8);
    EXPECT_NEAR(back.getDEC(), start.getDEC(), 1e-8);
}

TEST(SkyCoo_Galactic, rotationPreservesAngularSeparation)
{
    // Equatorial -> galactic is a rigid rotation, so the angle between any two
    // directions must survive it unchanged.
    EquatorialCoordinates a(10.68470, 41.26875);   // M31
    EquatorialCoordinates b(83.82208, -5.39111);   // M42

    GalacticCoordinates ga, gb;
    a.toGalactic(&ga);
    b.toGalactic(&gb);

    EXPECT_NEAR(ga.angularSeparation(gb), a.angularSeparation(b), 1e-8);
}

#pragma endregion

#pragma region Galactic <-> Ecliptic - the two non-equatorial systems against each other

TEST(SkyCoo_GalacticEcliptic, galacticToEclipticMatchesTwoStepPath)
{
    // gal -> ecl must agree with gal -> equ -> ecl, whatever route the direct
    // conversion takes internally.
    GalacticCoordinates gal(121.17431951, -21.57291513);  // M31

    EclipticCoordinates direct;
    gal.toEcliptic(&direct);

    EquatorialCoordinates equ;
    gal.toEquatorial(&equ);
    EclipticCoordinates viaEquatorial;
    equ.toEcliptic(&viaEquatorial);

    EXPECT_NEAR(lonDiff(direct.getELON(), viaEquatorial.getELON()), 0.0, 1e-8);
    EXPECT_NEAR(direct.getELAT(), viaEquatorial.getELAT(), 1e-8);
}

TEST(SkyCoo_GalacticEcliptic, eclipticToGalacticMatchesTwoStepPath)
{
    EclipticCoordinates ecl(75.0, 5.0);

    GalacticCoordinates direct;
    ecl.toGalactic(&direct);

    EquatorialCoordinates equ;
    ecl.toEquatorial(&equ);
    GalacticCoordinates viaEquatorial;
    equ.toGalactic(&viaEquatorial);

    EXPECT_NEAR(lonDiff(direct.getGLON(), viaEquatorial.getGLON()), 0.0, 1e-8);
    EXPECT_NEAR(direct.getGLAT(), viaEquatorial.getGLAT(), 1e-8);
}

TEST(SkyCoo_GalacticEcliptic, roundTrip)
{
    GalacticCoordinates start(45.0, -12.0);

    EclipticCoordinates ecl;
    start.toEcliptic(&ecl);

    GalacticCoordinates back;
    ecl.toGalactic(&back);

    EXPECT_NEAR(lonDiff(back.getGLON(), start.getGLON()), 0.0, 1e-8);
    EXPECT_NEAR(back.getGLAT(), start.getGLAT(), 1e-8);
}

TEST(SkyCoo_GalacticEcliptic, polesAreSixtyDegreesApart)
{
    // Follows from the two definitions alone: with the NGP at
    // (192.85948, +27.12825) and the NEP at (270, 90 - 23.43929), the spherical
    // law of cosines gives 60.19 deg between them.
    GalacticCoordinates ngp(0.0, 90.0);
    EclipticCoordinates  nep(0.0, 90.0);

    EquatorialCoordinates ngpEqu, nepEqu;
    ngp.toEquatorial(&ngpEqu);
    nep.toEquatorial(&nepEqu);

    EXPECT_NEAR(ngpEqu.angularSeparation(nepEqu), 60.19, 0.1);
}

TEST(SkyCoo_GalacticEcliptic, fullSystemCycleReturnsToStart)
{
    // equ -> gal -> ecl -> equ closes on itself.
    EquatorialCoordinates start(310.35798, 45.28034);

    GalacticCoordinates gal;
    start.toGalactic(&gal);

    EclipticCoordinates ecl;
    gal.toEcliptic(&ecl);

    EquatorialCoordinates back;
    ecl.toEquatorial(&back);

    EXPECT_NEAR(lonDiff(back.getRA(), start.getRA()), 0.0, 1e-7);
    EXPECT_NEAR(back.getDEC(), start.getDEC(), 1e-7);
}

#pragma endregion

#pragma region FK5 <-> ICRS - frame tie is a fixed sub-arcsecond rotation

TEST(SkyCoo_Frames, FK5_ICRS_roundTrip)
{
    EquatorialCoordinates fk5(100.0, -20.0, 12.3, -4.5, CoordFrame::FK5);

    EquatorialCoordinates icrs;
    fk5.toICRS(&icrs);

    EquatorialCoordinates back;
    icrs.toFK5(&back);

    EXPECT_NEAR(back.getRA(),    fk5.getRA(),    1e-9);
    EXPECT_NEAR(back.getDEC(),   fk5.getDEC(),   1e-9);
    EXPECT_NEAR(back.getPMRA(),  fk5.getPMRA(),  1e-6);
    EXPECT_NEAR(back.getPMDEC(), fk5.getPMDEC(), 1e-6);
}

TEST(SkyCoo_Frames, FK5_ICRS_frameTieIsSubArcsecond)
{
    // The ICRS/dynamical-J2000 frame bias is a well documented, fixed
    // rotation of well under 1 arcsecond (see e.g. IERS/IAU frame-tie
    // reports); this is independent of how SuperNOVAS implements it.
    EquatorialCoordinates fk5(30.0, 10.0, 0.0, 0.0, CoordFrame::FK5);
    EquatorialCoordinates icrs;
    fk5.toICRS(&icrs);

    EXPECT_LT(fk5.angularSeparation(icrs), 1.0 * kArcsecToDeg);
    EXPECT_GT(fk5.angularSeparation(icrs), 0.0);
}

TEST(SkyCoo_Frames, FK5_ICRS_tieIsPositionIndependentInMagnitude)
{
    // A fixed rotation moves every direction by at most the rotation angle, so
    // the tie must stay sub-arcsecond everywhere on the sky, not just at one
    // convenient test point.
    const double positions[][2] = {{0.0, 0.0}, {90.0, 45.0}, {180.0, -60.0},
                                   {270.0, 80.0}, {45.0, -89.0}};

    for (const auto& p : positions)
    {
        EquatorialCoordinates fk5(p[0], p[1], 0.0, 0.0, CoordFrame::FK5);
        EquatorialCoordinates icrs;
        fk5.toICRS(&icrs);

        EXPECT_LT(fk5.angularSeparation(icrs), 1.0 * kArcsecToDeg)
            << "failed at (" << p[0] << ", " << p[1] << ")";
    }
}

TEST(SkyCoo_Frames, FK5_ICRS_tiePreservesProperMotionMagnitude)
{
    // The tie is a rigid rotation: it may redistribute proper motion between
    // the two components, but the total tangential rate is invariant.
    const double pmRA = 120.0, pmDEC = -45.0;
    EquatorialCoordinates fk5(45.0, 25.0, pmRA, pmDEC, CoordFrame::FK5);

    EquatorialCoordinates icrs;
    fk5.toICRS(&icrs);

    EXPECT_NEAR(std::hypot(icrs.getPMRA(), icrs.getPMDEC()),
                std::hypot(pmRA, pmDEC), 1e-2);
}

#pragma endregion

#pragma region FK4 (B1950) <-> FK5 (J2000) - precession over 50 years

TEST(SkyCoo_Frames, FK4_FK5_roundTrip)
{
    EquatorialCoordinates fk4(50.0, 30.0, 5.0, -2.0, CoordFrame::FK4);

    EquatorialCoordinates fk5;
    fk4.toFK5(&fk5);

    EquatorialCoordinates back;
    fk5.toFK4(&back);

    EXPECT_NEAR(back.getRA(),    fk4.getRA(),    1e-6);
    EXPECT_NEAR(back.getDEC(),   fk4.getDEC(),   1e-6);
    EXPECT_NEAR(back.getPMRA(),  fk4.getPMRA(),  1e-3);
    EXPECT_NEAR(back.getPMDEC(), fk4.getPMDEC(), 1e-3);
}

TEST(SkyCoo_Frames, FK4_FK5_shiftMatchesGeneralPrecession)
{
    // General (luni-solar + planetary) precession is a well known constant
    // of about 50.3 arcsec/year; over the 50 years between B1950.0 and
    // J2000.0 that amounts to roughly 2515 arcsec (~0.70 deg). The exact
    // shift depends on position on the sky, so only a generous bracket
    // around that figure is asserted here.
    EquatorialCoordinates fk4(60.0, 20.0, CoordFrame::FK4);
    EquatorialCoordinates fk5;
    fk4.toFK5(&fk5);

    const double sep = fk4.angularSeparation(fk5);
    EXPECT_GT(sep, 0.1);
    EXPECT_LT(sep, 2.0);
}

TEST(SkyCoo_Frames, FK4_ICRS_roundTrip)
{
    EquatorialCoordinates fk4(215.0, -8.0, 9.0, 14.0, CoordFrame::FK4);

    EquatorialCoordinates icrs;
    fk4.toICRS(&icrs);

    EquatorialCoordinates back;
    icrs.toFK4(&back);

    EXPECT_NEAR(back.getRA(),    fk4.getRA(),    1e-6);
    EXPECT_NEAR(back.getDEC(),   fk4.getDEC(),   1e-6);
    EXPECT_NEAR(back.getPMRA(),  fk4.getPMRA(),  1e-3);
    EXPECT_NEAR(back.getPMDEC(), fk4.getPMDEC(), 1e-3);
}

TEST(SkyCoo_Frames, FK4_to_ICRS_matchesFK4_to_FK5_to_ICRS)
{
    // convertFrame() pivots through the J2000 dynamical frame, so going
    // straight to ICRS and going via FK5 must be the same transformation.
    EquatorialCoordinates fk4(75.0, -15.0, 8.0, 2.5, CoordFrame::FK4);

    EquatorialCoordinates direct;
    fk4.toICRS(&direct);

    EquatorialCoordinates fk5, chained;
    fk4.toFK5(&fk5);
    fk5.toICRS(&chained);

    EXPECT_NEAR(lonDiff(chained.getRA(), direct.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(chained.getDEC(),   direct.getDEC(),   1e-9);
    EXPECT_NEAR(chained.getPMRA(),  direct.getPMRA(),  1e-6);
    EXPECT_NEAR(chained.getPMDEC(), direct.getPMDEC(), 1e-6);
}

TEST(SkyCoo_Frames, FK4_FK5_shiftIsMuchLargerThanTheICRSTie)
{
    // Sanity ordering of the two effects: 50 years of precession (~0.7 deg)
    // dwarfs the sub-arcsecond FK5/ICRS frame tie by a factor of ~2500.
    EquatorialCoordinates fk4(60.0, 20.0, CoordFrame::FK4);
    EquatorialCoordinates fk5, icrs;
    fk4.toFK5(&fk5);
    fk5.toICRS(&icrs);

    EXPECT_GT(fk4.angularSeparation(fk5), 1000.0 * fk5.angularSeparation(icrs));
}

#pragma endregion

#pragma region Frame conversion API - identity, dispatch and error handling

TEST(SkyCoo_FrameAPI, sameFrameIsIdentity)
{
    EquatorialCoordinates fk5(200.0, -35.0, 7.0, 3.0, CoordFrame::FK5);

    EquatorialCoordinates out;
    fk5.toFK5(&out);

    EXPECT_NEAR(out.getRA(),    fk5.getRA(),    1e-12);
    EXPECT_NEAR(out.getDEC(),   fk5.getDEC(),   1e-12);
    EXPECT_NEAR(out.getPMRA(),  fk5.getPMRA(),  1e-12);
    EXPECT_NEAR(out.getPMDEC(), fk5.getPMDEC(), 1e-12);
}

TEST(SkyCoo_FrameAPI, genericToEquatorialMatchesNamedHelpers)
{
    // toEquatorial(out, frame) and the toFK4()/toFK5()/toICRS() shorthands must
    // be the same transformation.
    EquatorialCoordinates fk5(88.0, 7.0, -3.0, 11.0, CoordFrame::FK5);

    EquatorialCoordinates viaGeneric, viaNamed;

    fk5.toEquatorial(&viaGeneric, CoordFrame::FK4);
    fk5.toFK4(&viaNamed);
    EXPECT_NEAR(viaGeneric.getRA(),  viaNamed.getRA(),  1e-12);
    EXPECT_NEAR(viaGeneric.getDEC(), viaNamed.getDEC(), 1e-12);

    fk5.toEquatorial(&viaGeneric, CoordFrame::ICRS);
    fk5.toICRS(&viaNamed);
    EXPECT_NEAR(viaGeneric.getRA(),  viaNamed.getRA(),  1e-12);
    EXPECT_NEAR(viaGeneric.getDEC(), viaNamed.getDEC(), 1e-12);
}

TEST(SkyCoo_FrameAPI, staticConvertFrameMatchesMemberConversion)
{
    // convertFrame() works in hours; the member conversions work in degrees.
    const double raDeg = 300.0, decDeg = 45.0, pmRA = 10.0, pmDEC = -3.0;

    double ra = 0.0, dec = 0.0, pmRAOut = 0.0, pmDECOut = 0.0;
    EquatorialCoordinates::convertFrame(raDeg / 15.0, decDeg, pmRA, pmDEC,
                                        CoordFrame::FK5, CoordFrame::ICRS,
                                        ra, dec, pmRAOut, pmDECOut);

    EquatorialCoordinates fk5(raDeg, decDeg, pmRA, pmDEC, CoordFrame::FK5);
    EquatorialCoordinates icrs;
    fk5.toICRS(&icrs);

    EXPECT_NEAR(icrs.getRA(),    ra * 15.0, 1e-12);
    EXPECT_NEAR(icrs.getDEC(),   dec,       1e-12);
    EXPECT_NEAR(icrs.getPMRA(),  pmRAOut,   1e-12);
    EXPECT_NEAR(icrs.getPMDEC(), pmDECOut,  1e-12);
}

TEST(SkyCoo_FrameAPI, undefinedSourceFrameThrows)
{
    EquatorialCoordinates undefinedFrame(10.0, 20.0, CoordFrame::UNDEFINED);
    EquatorialCoordinates out;

    EXPECT_THROW(undefinedFrame.toFK5(&out),  std::invalid_argument);
    EXPECT_THROW(undefinedFrame.toFK4(&out),  std::invalid_argument);
    EXPECT_THROW(undefinedFrame.toICRS(&out), std::invalid_argument);
}

TEST(SkyCoo_FrameAPI, undefinedTargetFrameThrows)
{
    double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0;

    EXPECT_THROW(EquatorialCoordinates::convertFrame(4.0, 20.0, 0.0, 0.0,
                                                     CoordFrame::FK5, CoordFrame::UNDEFINED,
                                                     ra, dec, pmRA, pmDEC),
                 std::invalid_argument);
}

TEST(SkyCoo_FrameAPI, nullOutputThrows)
{
    EquatorialCoordinates equ(10.0, 20.0);
    GalacticCoordinates   gal(10.0, 20.0);
    EclipticCoordinates   ecl(10.0, 20.0);

    EXPECT_THROW(equ.toEquatorial(nullptr), std::invalid_argument);
    EXPECT_THROW(equ.toGalactic(nullptr),   std::invalid_argument);
    EXPECT_THROW(equ.toEcliptic(nullptr),   std::invalid_argument);

    EXPECT_THROW(gal.toEquatorial(nullptr), std::invalid_argument);
    EXPECT_THROW(gal.toGalactic(nullptr),   std::invalid_argument);
    EXPECT_THROW(gal.toEcliptic(nullptr),   std::invalid_argument);

    EXPECT_THROW(ecl.toEquatorial(nullptr), std::invalid_argument);
    EXPECT_THROW(ecl.toGalactic(nullptr),   std::invalid_argument);
    EXPECT_THROW(ecl.toEcliptic(nullptr),   std::invalid_argument);

    EXPECT_THROW(equ.toICRS(nullptr), std::invalid_argument);
    EXPECT_THROW(equ.toFK5(nullptr),  std::invalid_argument);
    EXPECT_THROW(equ.toFK4(nullptr),  std::invalid_argument);
}

#pragma endregion

#pragma region System conversions requested in a non-native frame
// GalacticCoordinates and EclipticCoordinates are built in ICRS; asking them
// for another frame has to convert the *system* and the *frame*, in that order.

TEST(SkyCoo_FramedSystems, galacticToEclipticInFK5MatchesTwoStepPath)
{
    GalacticCoordinates gal(121.17431951, -21.57291513);  // M31

    EclipticCoordinates direct;
    gal.toEcliptic(&direct, CoordFrame::FK5);

    EquatorialCoordinates equ;
    gal.toEquatorial(&equ, CoordFrame::FK5);
    EclipticCoordinates viaEquatorial;
    equ.toEcliptic(&viaEquatorial, CoordFrame::FK5);

    EXPECT_NEAR(lonDiff(direct.getELON(), viaEquatorial.getELON()), 0.0, 1e-8);
    EXPECT_NEAR(direct.getELAT(), viaEquatorial.getELAT(), 1e-8);
}

TEST(SkyCoo_FramedSystems, galacticToEclipticInFK4MatchesTwoStepPath)
{
    // Same invariant across the much larger B1950 frame change, where a route
    // that forgot to convert the system first would be obvious.
    GalacticCoordinates gal(30.0, 15.0);

    EclipticCoordinates direct;
    gal.toEcliptic(&direct, CoordFrame::FK4);

    EquatorialCoordinates equ;
    gal.toEquatorial(&equ, CoordFrame::FK4);
    EclipticCoordinates viaEquatorial;
    equ.toEcliptic(&viaEquatorial, CoordFrame::FK4);

    EXPECT_NEAR(lonDiff(direct.getELON(), viaEquatorial.getELON()), 0.0, 1e-8);
    EXPECT_NEAR(direct.getELAT(), viaEquatorial.getELAT(), 1e-8);
}

TEST(SkyCoo_FramedSystems, galacticToEquatorialInFK4MatchesTwoStepPath)
{
    GalacticCoordinates gal(210.0, 33.0);

    EquatorialCoordinates direct;
    gal.toEquatorial(&direct, CoordFrame::FK4);

    EquatorialCoordinates icrs, chained;
    gal.toEquatorial(&icrs);  // native ICRS
    icrs.toFK4(&chained);

    EXPECT_NEAR(lonDiff(direct.getRA(), chained.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(direct.getDEC(), chained.getDEC(), 1e-9);
}

TEST(SkyCoo_FramedSystems, eclipticToEquatorialInFK5MatchesTwoStepPath)
{
    EclipticCoordinates ecl(75.0, 5.0);

    EquatorialCoordinates direct;
    ecl.toEquatorial(&direct, CoordFrame::FK5);

    EquatorialCoordinates icrs, chained;
    ecl.toEquatorial(&icrs);  // native ICRS
    icrs.toFK5(&chained);

    EXPECT_NEAR(lonDiff(direct.getRA(), chained.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(direct.getDEC(), chained.getDEC(), 1e-9);
}

TEST(SkyCoo_FramedSystems, eclipticToEquatorialInFK4MatchesTwoStepPath)
{
    EclipticCoordinates ecl(310.0, -22.0);

    EquatorialCoordinates direct;
    ecl.toEquatorial(&direct, CoordFrame::FK4);

    EquatorialCoordinates icrs, chained;
    ecl.toEquatorial(&icrs);
    icrs.toFK4(&chained);

    EXPECT_NEAR(lonDiff(direct.getRA(), chained.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(direct.getDEC(), chained.getDEC(), 1e-9);
}

TEST(SkyCoo_FramedSystems, eclipticFrameChangeIsInvertible)
{
    // The output object carries the frame it was written in, so asking it back
    // for ICRS must undo the change exactly.
    EclipticCoordinates start(200.0, -30.0);  // ICRS by construction

    EclipticCoordinates fk4;
    start.toEcliptic(&fk4, CoordFrame::FK4);

    EclipticCoordinates back;
    fk4.toEcliptic(&back, CoordFrame::ICRS);

    EXPECT_NEAR(lonDiff(back.getELON(), start.getELON()), 0.0, 1e-8);
    EXPECT_NEAR(back.getELAT(), start.getELAT(), 1e-8);
}

TEST(SkyCoo_FramedSystems, galacticFrameChangeIsInvertible)
{
    GalacticCoordinates start(45.0, -12.0);

    GalacticCoordinates fk4;
    start.toGalactic(&fk4, CoordFrame::FK4);

    GalacticCoordinates back;
    fk4.toGalactic(&back, CoordFrame::ICRS);

    EXPECT_NEAR(lonDiff(back.getGLON(), start.getGLON()), 0.0, 1e-8);
    EXPECT_NEAR(back.getGLAT(), start.getGLAT(), 1e-8);
}

TEST(SkyCoo_FramedSystems, eclipticFrameChangeIsAPrecessionInLongitude)
{
    // Because the same J2000 obliquity is used on both ends, the B1950 frame
    // change reduces to a rotation about the ecliptic pole: the longitude moves
    // by the general precession, the latitude barely moves at all.
    EclipticCoordinates icrs(120.0, 10.0);

    EclipticCoordinates fk4;
    icrs.toEcliptic(&fk4, CoordFrame::FK4);

    EXPECT_NEAR(std::fabs(lonDiff(fk4.getELON(), icrs.getELON())),
                kPrecessionB1950ToJ2000, 0.05);
    EXPECT_NEAR(fk4.getELAT(), icrs.getELAT(), 0.05);
}

TEST(SkyCoo_FramedSystems, requestingTheNativeFrameChangesNothing)
{
    // ICRS is the native frame of both non-equatorial classes, so the explicit
    // and implicit forms must agree bit for bit.
    GalacticCoordinates gal(88.0, -3.0);
    EclipticCoordinates implicitEcl, explicitEcl;

    gal.toEcliptic(&implicitEcl);
    gal.toEcliptic(&explicitEcl, CoordFrame::ICRS);

    EXPECT_DOUBLE_EQ(implicitEcl.getELON(), explicitEcl.getELON());
    EXPECT_DOUBLE_EQ(implicitEcl.getELAT(), explicitEcl.getELAT());
}

TEST(SkyCoo_FramedSystems, frameChangePreservesAngularSeparation)
{
    // Precession is a rigid rotation, so two directions carried into FK4
    // together keep their mutual angle.
    EquatorialCoordinates a(10.68470, 41.26875, CoordFrame::ICRS);
    EquatorialCoordinates b(83.82208, -5.39111, CoordFrame::ICRS);

    EquatorialCoordinates fa, fb;
    a.toFK4(&fa);
    b.toFK4(&fb);

    EXPECT_NEAR(fa.angularSeparation(fb), a.angularSeparation(b), 1e-6);
}

#pragma endregion

#pragma region Epochs - resolved through SuperNOVAS novas_epoch()

TEST(SkyCoo_Epoch, standardEpochOfEachFrame)
{
    // The Julian dates of the standard epochs are fixed definitions:
    // J2000.0 = JD 2451545.0, B1950.0 = JD 2433282.42345905 (the Besselian year
    // 1950.0), J1991.25 = JD 2448349.0625 (the Hipparcos catalogue epoch).
    EXPECT_NEAR(SkyCoord::epochOf(CoordFrame::FK5),       2451545.0,        1e-6);
    EXPECT_NEAR(SkyCoord::epochOf(CoordFrame::ICRS),      2451545.0,        1e-6);
    EXPECT_NEAR(SkyCoord::epochOf(CoordFrame::FK4),       2433282.42345905, 1e-6);
    EXPECT_NEAR(SkyCoord::epochOf(CoordFrame::HIPPARCOS), 2448349.0625,     1e-6);
}

TEST(SkyCoo_Epoch, undefinedFrameAssumesJ2000)
{
    EXPECT_NEAR(SkyCoord::epochOf(CoordFrame::UNDEFINED), 2451545.0, 1e-6);
}

TEST(SkyCoo_Epoch, epochStringsMatchTheFrameEpochs)
{
    EXPECT_NEAR(SkyCoord::epochOf("J2000.0"), SkyCoord::epochOf(CoordFrame::FK5),       1e-6);
    EXPECT_NEAR(SkyCoord::epochOf("B1950.0"), SkyCoord::epochOf(CoordFrame::FK4),       1e-6);
    EXPECT_NEAR(SkyCoord::epochOf("HIP"),     SkyCoord::epochOf(CoordFrame::HIPPARCOS), 1e-6);
}

TEST(SkyCoo_Epoch, julianYearsAreThreeSixtyFivePointTwoFiveDays)
{
    // A Julian year is 365.25 days by definition, so J2001.0 is exactly that
    // far from J2000.0.
    EXPECT_NEAR(SkyCoord::epochOf("J2001.0") - SkyCoord::epochOf("J2000.0"), 365.25, 1e-6);

    // ... and a Besselian year is 365.242198781 days.
    EXPECT_NEAR(SkyCoord::epochOf("B1951.0") - SkyCoord::epochOf("B1950.0"), 365.242198781, 1e-6);
}

TEST(SkyCoo_Epoch, unrecognisedEpochStringThrows)
{
    EXPECT_THROW(SkyCoord::epochOf("not an epoch"), std::invalid_argument);
}

TEST(SkyCoo_Epoch, currentEpochIsAPlausibleJulianDate)
{
    // Bracketed by 2020-01-01 and 2100-01-01: loose enough never to go stale by
    // surprise, tight enough to catch a clock conversion that lost its offset.
    const double now = SkyCoord::currentEpoch();
    EXPECT_GT(now, 2458849.5);
    EXPECT_LT(now, 2488069.5);
}

TEST(SkyCoo_Epoch, defaultEpochFollowsTheFrame)
{
    EXPECT_NEAR(EquatorialCoordinates(0.0, 0.0, CoordFrame::ICRS).getEpoch(),
                SkyCoord::epochOf(CoordFrame::ICRS), 1e-6);
    EXPECT_NEAR(EquatorialCoordinates(0.0, 0.0, CoordFrame::FK4).getEpoch(),
                SkyCoord::epochOf(CoordFrame::FK4), 1e-6);
    EXPECT_NEAR(EquatorialCoordinates(0.0, 0.0, CoordFrame::HIPPARCOS).getEpoch(),
                SkyCoord::epochOf(CoordFrame::HIPPARCOS), 1e-6);
}

#pragma endregion

#pragma region atEpoch - proper motion propagation

TEST(SkyCoo_atEpoch, sameEpochIsAFixedPoint)
{
    EquatorialCoordinates star(45.0, 20.0, 500.0, -300.0, 50.0, 10.0, CoordFrame::ICRS);

    EquatorialCoordinates same;
    star.atEpoch(star.getEpoch(), &same);

    EXPECT_NEAR(same.getRA(),  star.getRA(),  1e-9);
    EXPECT_NEAR(same.getDEC(), star.getDEC(), 1e-9);
}

TEST(SkyCoo_atEpoch, motionIsLinearInTimeForADistantSource)
{
    // With no parallax the propagation is purely angular, so the declination
    // displacement is exactly the proper motion times the elapsed years.
    const double pmDEC = 1000.0;  // mas/yr, i.e. 1 arcsec/yr
    EquatorialCoordinates star(45.0, 20.0, 0.0, pmDEC, 0.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates after;
    star.atEpoch(SkyCoord::epochOf("J2100.0"), &after);

    const double expectedShiftDeg = pmDEC * 100.0 / 1000.0 / 3600.0;  // 100 arcsec
    EXPECT_NEAR(after.getDEC() - star.getDEC(), expectedShiftDeg, 1e-6);
}

TEST(SkyCoo_atEpoch, raProperMotionIncludesTheCosDecFactor)
{
    // pmRA is given as mu_alpha* = mu_alpha cos(dec), so the coordinate change
    // in RA is the quoted rate divided by cos(dec).
    const double dec = 60.0, pmRA = 1000.0;  // mas/yr
    EquatorialCoordinates star(45.0, dec, pmRA, 0.0, 0.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates after;
    star.atEpoch(SkyCoord::epochOf("J2010.0"), &after);

    // Tolerance absorbs the second-order curvature of the great circle the star
    // actually follows, which is a couple of mas over this baseline.
    const double expectedShiftDeg =
        pmRA * 10.0 / 1000.0 / 3600.0 / std::cos(dec * M_PI / 180.0);
    EXPECT_NEAR(lonDiff(after.getRA(), star.getRA()), expectedShiftDeg, 1e-5);
}

TEST(SkyCoo_atEpoch, isInvertibleForASourceWithNoRadialMotion)
{
    // Straight-line space motion is exactly reversible, and with no radial velocity
    // SuperNOVAS reproduces that to well below a microarcsecond.
    EquatorialCoordinates star(210.0, -45.0, 250.0, 120.0, 20.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates forward, back;
    star.atEpoch(SkyCoord::epochOf("J2050.0"), &forward);
    forward.atEpoch(star.getEpoch(), &back);

    EXPECT_NEAR(lonDiff(back.getRA(), star.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(back.getDEC(), star.getDEC(), 1e-9);
    EXPECT_NEAR(back.getParallax(), star.getParallax(), 1e-6);
}

TEST(SkyCoo_atEpoch, radialMotionLeavesASmallRoundTripResidual)
{
    // With a radial velocity the there-and-back propagation no longer closes exactly.
    // transform_cat() computes its distance and Doppler scale factors (term1 and k in
    // target.c) from the *input* entry, before advancing the position, but reports the
    // *updated* parallax; the return leg then rebuilds the velocity on a slightly
    // different distance scale. The residual is first order in the fractional distance
    // change, i.e. of order mu * dt * (v_r * dt / d).
    //
    // The assertion is an upper bound, so it keeps passing if SuperNOVAS ever makes the
    // conversion self-consistent and the residual drops to rounding.
    const double pmRA = 250.0, pmDEC = 120.0;  // [mas/yr]
    const double parallaxMas = 20.0;           // [mas] -> 50 pc
    const double rvKmS = -30.0;                // [km/s]

    EquatorialCoordinates star(210.0, -45.0, pmRA, pmDEC, parallaxMas, rvKmS, CoordFrame::ICRS);

    EquatorialCoordinates forward, back;
    star.atEpoch(SkyCoord::epochOf("J2050.0"), &forward);
    forward.atEpoch(star.getEpoch(), &back);

    const double years              = 50.0;
    const double distanceKm         = (1000.0 / parallaxMas) * 3.0857e13;  // parsecs -> km
    const double fractionalApproach = std::fabs(rvKmS) * years * 3.15576e7 / distanceKm;
    const double travelMas          = std::hypot(pmRA, pmDEC) * years;
    const double boundDeg           = 1.5 * travelMas * fractionalApproach / 1000.0 / 3600.0;

    EXPECT_LT(star.angularSeparation(back), boundDeg);
}

TEST(SkyCoo_atEpoch, outputCarriesTheTargetEpochAndKeepsTheFrame)
{
    EquatorialCoordinates star(100.0, 30.0, 40.0, 15.0, 5.0, 0.0, CoordFrame::FK5);
    const double target = SkyCoord::epochOf("J2030.0");

    EquatorialCoordinates moved;
    star.atEpoch(target, &moved);

    EXPECT_NEAR(moved.getEpoch(), target, 1e-9);
    // The frame is orientation only: propagating in time must not rotate anything.
    EquatorialCoordinates roundTrip;
    moved.atEpoch(star.getEpoch(), &roundTrip);
    EXPECT_NEAR(lonDiff(roundTrip.getRA(), star.getRA()), 0.0, 1e-9);
}

TEST(SkyCoo_atEpoch, stepwisePropagationMatchesOneJump)
{
    // Radial velocity is left at zero on purpose: see
    // radialMotionLeavesASmallRoundTripResidual for why chaining is only exact without it.
    EquatorialCoordinates star(300.0, 10.0, 600.0, -200.0, 100.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates direct;
    star.atEpoch(SkyCoord::epochOf("J2040.0"), &direct);

    EquatorialCoordinates half, chained;
    star.atEpoch(SkyCoord::epochOf("J2020.0"), &half);
    half.atEpoch(SkyCoord::epochOf("J2040.0"), &chained);

    EXPECT_NEAR(lonDiff(chained.getRA(), direct.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(chained.getDEC(), direct.getDEC(), 1e-9);
}

TEST(SkyCoo_atEpoch, aStarWithoutProperMotionDoesNotMove)
{
    EquatorialCoordinates fixed(123.0, -4.0, 0.0, 0.0, 0.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates later;
    fixed.atEpoch(SkyCoord::epochOf("J2200.0"), &later);

    EXPECT_NEAR(lonDiff(later.getRA(), fixed.getRA()), 0.0, 1e-9);
    EXPECT_NEAR(later.getDEC(), fixed.getDEC(), 1e-9);
}

TEST(SkyCoo_atEpoch, frameOverloadTargetsThatFramesStandardEpoch)
{
    EquatorialCoordinates star(88.0, 12.0, 300.0, -100.0, 10.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates viaFrame, viaJD;
    star.atEpoch(CoordFrame::HIPPARCOS, &viaFrame);
    star.atEpoch(SkyCoord::epochOf(CoordFrame::HIPPARCOS), &viaJD);

    EXPECT_NEAR(viaFrame.getRA(),  viaJD.getRA(),  1e-12);
    EXPECT_NEAR(viaFrame.getDEC(), viaJD.getDEC(), 1e-12);
}

TEST(SkyCoo_atEpoch, stringOverloadMatchesTheJulianDateOverload)
{
    EquatorialCoordinates star(88.0, 12.0, 300.0, -100.0, 10.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates viaString, viaJD;
    star.atEpoch("J2026.7", &viaString);
    star.atEpoch(SkyCoord::epochOf("J2026.7"), &viaJD);

    EXPECT_NEAR(viaString.getRA(),  viaJD.getRA(),  1e-12);
    EXPECT_NEAR(viaString.getDEC(), viaJD.getDEC(), 1e-12);
}

TEST(SkyCoo_atEpoch, hipparcosEntryPropagatedToJ2000)
{
    // A Hipparcos position is given at J1991.25; moving it to J2000.0 advances it
    // by 8.75 Julian years of proper motion. Barnard's Star is the extreme case,
    // at 10.36 arcsec/yr, so it travels about 90.6 arcsec over that interval.
    EquatorialCoordinates barnard(269.45207, 4.66829,
                                  -798.58, 10328.12,   // [mas/yr]
                                  547.45, -110.51,     // [mas], [km/s]
                                  CoordFrame::HIPPARCOS);

    EXPECT_NEAR(barnard.getEpoch(), 2448349.0625, 1e-6);

    EquatorialCoordinates atJ2000;
    barnard.atEpoch("J2000.0", &atJ2000);

    const double elapsedYears = (2451545.0 - 2448349.0625) / 365.25;  // 8.75 yr
    const double expectedShiftDeg =
        std::hypot(-798.58, 10328.12) * elapsedYears / 1000.0 / 3600.0;

    EXPECT_NEAR(barnard.angularSeparation(atJ2000), expectedShiftDeg, 1e-3);
}

TEST(SkyCoo_atEpoch, nullOutputThrows)
{
    EquatorialCoordinates star(10.0, 20.0);

    EXPECT_THROW(star.atEpoch(2451545.0, nullptr),  std::invalid_argument);
    EXPECT_THROW(star.atEpoch("J2000.0", nullptr),  std::invalid_argument);
    EXPECT_THROW(star.atCurrentEpoch(nullptr),      std::invalid_argument);
}

#pragma endregion

#pragma region HIPPARCOS frame - ICRS axes at a different epoch

TEST(SkyCoo_Hipparcos, sharesTheICRSOrientation)
{
    // HIPPARCOS and ICRS are the same axes, so converting between them must not
    // move the position by even a microarcsecond. Only the epoch differs, and
    // that is atEpoch()'s business, not a rotation's.
    EquatorialCoordinates hip(150.0, -20.0, 100.0, 50.0, 25.0, 5.0, CoordFrame::HIPPARCOS);

    EquatorialCoordinates icrs;
    hip.toICRS(&icrs);

    EXPECT_NEAR(icrs.getRA(),  hip.getRA(),  1e-12);
    EXPECT_NEAR(icrs.getDEC(), hip.getDEC(), 1e-12);
}

TEST(SkyCoo_Hipparcos, frameChangeLeavesTheEpochAlone)
{
    // Rotating a Hipparcos entry into ICRS gives an ICRS-oriented position that
    // is still valid at J1991.25; it is not silently relabelled as J2000.
    EquatorialCoordinates hip(150.0, -20.0, 100.0, 50.0, 25.0, 5.0, CoordFrame::HIPPARCOS);

    EquatorialCoordinates icrs;
    hip.toICRS(&icrs);

    EXPECT_NEAR(icrs.getEpoch(), SkyCoord::epochOf(CoordFrame::HIPPARCOS), 1e-9);
}

TEST(SkyCoo_Hipparcos, convertingToFK5AndBackIsTheICRSTie)
{
    EquatorialCoordinates hip(150.0, -20.0, 0.0, 0.0, 0.0, 0.0, CoordFrame::HIPPARCOS);

    EquatorialCoordinates fk5;
    hip.toFK5(&fk5);

    EXPECT_LT(hip.angularSeparation(fk5), 1.0 * kArcsecToDeg);
    EXPECT_GT(hip.angularSeparation(fk5), 0.0);
}

TEST(SkyCoo_Hipparcos, anFK4RoundTripLandsOnTheFramesOwnEpochs)
{
    // The FK4 legs go through transform_cat(CHANGE_EPOCH, ...), which moves the
    // star in time as well as rotating it, so the epoch follows the frame there.
    EquatorialCoordinates icrs(150.0, -20.0, 0.0, 0.0, 0.0, 0.0, CoordFrame::ICRS);

    EquatorialCoordinates fk4;
    icrs.toFK4(&fk4);
    EXPECT_NEAR(fk4.getEpoch(), SkyCoord::epochOf(CoordFrame::FK4), 1e-9);

    EquatorialCoordinates back;
    fk4.toICRS(&back);
    EXPECT_NEAR(back.getEpoch(), SkyCoord::epochOf(CoordFrame::ICRS), 1e-9);
}

#pragma endregion

#pragma region Space motion carried through conversions

TEST(SkyCoo_SpaceMotion, parallaxAndVelocitySurviveASystemConversion)
{
    // The distance and the line-of-sight velocity are properties of the star,
    // not of the coordinate system used to point at it.
    EquatorialCoordinates star(83.82208, -5.39111, 0.0, 0.0, 130.23, -20.5, CoordFrame::ICRS);

    GalacticCoordinates gal;
    star.toGalactic(&gal);

    EXPECT_NEAR(gal.getParallax(),       130.23, 1e-9);
    EXPECT_NEAR(gal.getRadialVelocity(), -20.5,  1e-9);
    EXPECT_NEAR(gal.getEpoch(),          star.getEpoch(), 1e-9);
}

TEST(SkyCoo_SpaceMotion, parallaxSurvivesAFullSystemCycle)
{
    EquatorialCoordinates start(310.35798, 45.28034, 0.0, 0.0, 194.95, 8.0, CoordFrame::ICRS);

    GalacticCoordinates gal;
    start.toGalactic(&gal);

    EclipticCoordinates ecl;
    gal.toEcliptic(&ecl);

    EquatorialCoordinates back;
    ecl.toEquatorial(&back);

    EXPECT_NEAR(back.getParallax(),       start.getParallax(),       1e-9);
    EXPECT_NEAR(back.getRadialVelocity(), start.getRadialVelocity(), 1e-9);
}

TEST(SkyCoo_SpaceMotion, staticConvertFrameOverloadsAgreeWhenTheStarIsFarAway)
{
    // With no parallax and no radial velocity the wide overload must reduce to
    // the narrow one exactly.
    double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0;
    EquatorialCoordinates::convertFrame(4.0, 20.0, 30.0, -10.0,
                                        CoordFrame::FK4, CoordFrame::FK5,
                                        ra, dec, pmRA, pmDEC);

    double raW = 0.0, decW = 0.0, pmRAW = 0.0, pmDECW = 0.0, plxW = 0.0, rvW = 0.0;
    EquatorialCoordinates::convertFrame(4.0, 20.0, 30.0, -10.0, 0.0, 0.0,
                                        CoordFrame::FK4, CoordFrame::FK5,
                                        raW, decW, pmRAW, pmDECW, plxW, rvW);

    EXPECT_DOUBLE_EQ(raW,    ra);
    EXPECT_DOUBLE_EQ(decW,   dec);
    EXPECT_DOUBLE_EQ(pmRAW,  pmRA);
    EXPECT_DOUBLE_EQ(pmDECW, pmDEC);
}

#pragma endregion

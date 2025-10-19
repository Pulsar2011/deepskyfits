#include <gtest/gtest.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSimg.h>
#include <DSTfits/FITSexception.h>
#include <DSTfits/FITSmanager.h>

#include <string>
#include <limits>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <type_traits>
#include <cmath>
#include <filesystem>

using namespace DSL;


TEST(FITS_wcs, readFromHdu)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
    FITSmanager fm("build/testdata/DSS.fits", true);

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITShdu hdu_ref = FITShdu(*hdu);
    std::string hdu_str = hdu_ref.asString();

    FITSwcs wcs(hdu_ref);
 
    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 3);
    EXPECT_NEAR(wcs.CRPIX(1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(0,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(0,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(1,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(1,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(2,1), 446.000000,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2,2), 447.000000,1e-7);
}

TEST(FITS_wcs, readFromPHdu)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
    FITSmanager fm("build/testdata/DSS.fits", true);

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();

    std::string hdu_str = hdu->asString();
    
    FITSwcs wcs(hdu);

    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 3);
    EXPECT_NEAR(wcs.CRPIX(1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(0,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(0,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(1,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(1,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(2,1), 446.000000,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2,2), 447.000000,1e-7);
}

TEST(FITS_wcs, readFromFitfile)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
    FITSmanager fm("build/testdata/DSS.fits", true);
    int hdu_num = 0;

    fm.MoveToPrimary();
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    EXPECT_EQ(fits_get_hdu_num(hdu_ref.get(), &hdu_num),1);

    FITSwcs wcs(fm.CurrentHDU());

    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 3);
    EXPECT_NEAR(wcs.CRPIX(1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(0,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(0,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(1,1),  5009.62946648,1e-7);
    EXPECT_NEAR(wcs.CRPIX(1,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs.CRPIX(2,1), 446.000000,1e-7);
    EXPECT_NEAR(wcs.CRPIX(2,2), 447.000000,1e-7);
}
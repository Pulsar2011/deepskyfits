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
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

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
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

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
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif
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

TEST(FITS_wcs, ConvertBackToString)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif  

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs(hdu);

    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 3);

    std::string wcs_header = wcs.asString();
    EXPECT_TRUE(wcs_header.find("WCSAXES " )!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("WCSAXESA")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("WCSAXESB")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX1" )!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX1A")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX1B")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX2" )!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX2A")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRPIX2B")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL1" )!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL1A")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL1B")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL2" )!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL2A")!= std::string::npos);
    EXPECT_TRUE(wcs_header.find("CRVAL2B")!= std::string::npos);

    wcs_header.clear();
    wcs_header = wcs.asString(0);
    EXPECT_TRUE (wcs_header.find("WCSAXES ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("WCSAXESA")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("WCSAXESB")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX1 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1B")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX2 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2B")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL1 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1B")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL2 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2B")!= std::string::npos);

    wcs_header.clear();
    wcs_header = wcs.asString(1);
    EXPECT_FALSE(wcs_header.find("WCSAXES " )!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("WCSAXESA")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("WCSAXESB")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1 ")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX1A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2 ")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX2A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1 ")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL1A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2 ")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL2A")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2B")!= std::string::npos);

    wcs_header.clear();
    wcs_header = wcs.asString(2);
    EXPECT_FALSE(wcs_header.find("WCSAXES ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("WCSAXESA")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("WCSAXESB")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX1A")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX1B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRPIX2A")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRPIX2B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL1A")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL1B")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2 ")!= std::string::npos);
    EXPECT_FALSE(wcs_header.find("CRVAL2A")!= std::string::npos);
    EXPECT_TRUE (wcs_header.find("CRVAL2B")!= std::string::npos);
}

TEST(FITS_wcs, ConvertBackToHdu)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif  

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs(hdu);

    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 3);

    int idx = -1;
    while(idx < wcs.getNumberOfWCS())
    {
        FITShdu wcs_header = wcs.asFITShdu();
        for(FITSDictionary::const_iterator i = hdu->begin(); i != hdu->end(); ++i)
        {
            std::string key = i->first;
            if(hdu->Exists(key))
            {

                std::string val_ref = i->second.value();
                EXPECT_EQ(val_ref, hdu->GetEntry(key)->second.value()) << "Mismatch for key " << key <<" = " << val_ref<<" | "<< hdu->GetEntry(key)->second.value(); 
            }
            else
            {
                FAIL() << "Key " << key << " missing in WCS header";
            }
        }
        idx++;
    }
}
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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

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

TEST(FITS_wcs, InvalidWcsIndex)
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
    EXPECT_EQ(wcs.getNumberOfAxis(0), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(1), 2);
    EXPECT_EQ(wcs.getNumberOfAxis(2), 2);

    EXPECT_THROW(wcs.asString(3), WCSexception);
    EXPECT_THROW(wcs.asString(4), WCSexception);

    EXPECT_THROW(wcs.asFITShdu(3), WCSexception);
    EXPECT_THROW(wcs.asFITShdu(4), WCSexception);
}

TEST(FITS_wcs, UnsetWcs)
{
    verbose |= verboseLevel::VERBOSE_WCS;

    FITSwcs wcs;

    EXPECT_EQ(wcs.getStatus(), WCSERR_UNSET);
    EXPECT_EQ(wcs.getNumberOfWCS(), 0);

    EXPECT_THROW(wcs.asString(), WCSexception);
    EXPECT_THROW(wcs.asString(0), WCSexception);
    EXPECT_THROW(wcs.asFITShdu(), WCSexception);
    EXPECT_THROW(wcs.asFITShdu(0), WCSexception);
}

TEST(FITS_wcs, NoWcsFound)
{
    verbose |= verboseLevel::VERBOSE_WCS;
    verbose |= verboseLevel::VERBOSE_BASIC;

    FITSDictionary fakeHdu;
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("SIMPLE"),FITSkeyword("T","file does conform to FITS standard")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("BITPIX"),FITSkeyword("16","number of bits per data pixel")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS"),FITSkeyword("4","number of data axes")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS1"),FITSkeyword("100","length of data axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS2"),FITSkeyword("100","length of data axis 2")));

    FITShdu hdu(fakeHdu);
    FITSwcs wcs(hdu);
    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 1);
    EXPECT_EQ(wcs.getNumberOfAxis(0), 4);
    for(size_t i=1; i<=wcs.getNumberOfAxis(0); i++)
    {
        EXPECT_NEAR(wcs.CRPIX(0,i), 0.0,1e-7);
        EXPECT_NEAR(wcs.CRVAL(0,i), 0.0,1e-7);
        EXPECT_NEAR(wcs.CDELT(0,i), 1.0,1e-7);
    }
}

FITShdu buildFakeHDU()
{
    FITSDictionary fakeHdu;
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("SIMPLE"),FITSkeyword("T","file does conform to FITS standard")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("BITPIX"),FITSkeyword("16","number of bits per data pixel")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS"),FITSkeyword("3","number of data axes")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS1"),FITSkeyword("100","length of data axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS2"),FITSkeyword("100","length of data axis 2")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("NAXIS3"),FITSkeyword("100","length of data axis 2")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRPIX1"),FITSkeyword("90.","reference pixel for axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRPIX2"),FITSkeyword("90.","reference pixel for axis 2")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRPIX3"),FITSkeyword("1.0","reference pixel for axis 3")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CDELT1"),FITSkeyword("-1","pixel scale for axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CDELT2"),FITSkeyword("1","pixel scale for axis 2")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CDELT3"),FITSkeyword("19.68717093222","pixel scale for axis 3")));

    for(size_t i=1; i<=3; i++)
    {
        std::string si = std::to_string(i);
        for(size_t j=1; j<=3; j++)
        {
            std::string sj = std::to_string(j);
            std::string val = (i==j)?"1.0":"0.0";
            fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("PC"+si+"_"+sj),FITSkeyword(val,"coordinate transformation matrix")));
        }

        std::string unit = (i==3)?"KM/SEC":"ARCSEC";
        fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CUNIT"+si),FITSkeyword(unit,"units for axis "+si)));
    }

    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CTYPE1"),FITSkeyword("RA---NCP","coordinate type for axis 1")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CTYPE2"),FITSkeyword("DEC--NCP","coordinate type for axis 2")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CTYPE3"),FITSkeyword("FELO-HEL","coordinate type for axis 3")));
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << (192.2500 * 3600.0);
        fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRVAL1"), FITSkeyword(oss.str(), "coordinate type for axis 1")));
    }
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << (27.4000 * 3600.0);
        fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRVAL2"), FITSkeyword(oss.str(), "coordinate type for axis 2")));
    }
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << 5569.27104;
        fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("CRVAL3"), FITSkeyword(oss.str(), "coordinate type for axis 3")));
    }

    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("RESTFRQ"),FITSkeyword("1.42040575e9","rest frequency")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("RESTWAV"),FITSkeyword("0.0","rest wavelength")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("DATE-OBS"),FITSkeyword("1957/02/15 01:10:00","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("DATE-BEG"),FITSkeyword("1957/02/15 01:10:00","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("DATE-AVG"),FITSkeyword("1957/02/15 02:10:00","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("DATE-END"),FITSkeyword("1957/02/15 03:10:00","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("BEPOCH"),FITSkeyword("1957.124382563","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("MJD-BEG"),FITSkeyword("35884.048611","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("OBSGEO_L"),FITSkeyword("148.263510","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("OBSGEO_B"),FITSkeyword("-32.998406","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("OBSGEO_H"),FITSkeyword("411.793","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("EQUINOX"),FITSkeyword("1950.0","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("VELREF"),FITSkeyword("2","")));
    fakeHdu.insert(std::pair<key_code,FITSkeyword>(std::string("SPECSYS"),FITSkeyword("BARYCENT","")));

    FITShdu hdu(fakeHdu);

    return hdu;
}

TEST(FITS_wcs, changeReferencial)
{
    verbose &= verboseLevel::VERBOSE_WCS;
    verbose |= verboseLevel::VERBOSE_BASIC;

    FITSwcs wcs(buildFakeHDU());
    
    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs.getNumberOfWCS(), 1);
    EXPECT_EQ(wcs.getNumberOfAxis(0), 3);

    //wcs.Dump();

    // Transform equatorial B1950 to galactic coordinates.  The WCS has been
    // constructed with the galactic pole coincident with the native pole of
    // the projection in order to test the resolution of an indeterminacy.
    verbose |= verboseLevel::VERBOSE_DEBUG;

    EXPECT_NO_THROW(wcs.changeCelestialCorrds(0,{123.0,27.4},192.25,std::make_pair(std::string("GLON"),std::string("GLAT")),"G"));
    EXPECT_EQ(wcs.getStatus(), WCSERR_SUCCESS);
    EXPECT_TRUE(wcs.CTYPE(0)[0].find("GLON")!=std::string::npos);
    EXPECT_TRUE(wcs.CTYPE(0)[1].find("GLAT")!=std::string::npos);
    EXPECT_TRUE(wcs.CTYPE(0)[2].find("VOPT")!=std::string::npos);
    EXPECT_NEAR(wcs.CRVAL(0,1), 303, 1e-7);
    EXPECT_NEAR(wcs.CRVAL(0,2), 90, 1e-7);
    EXPECT_NEAR(wcs.CRVAL(0,3), 5569271.04, 1e-1);

    // Compute B1950 coordinates of a field point.
    pixelVectors pix({{25.0, 25.0, 1.0}});
    std::cout<< "Pixel coords: " << pix[0][0] << " " << pix[0][1] << " " << pix[0][2] << std::endl;
    
    worldVectors world = wcs.pixel2world(0, pix);
    std::cout<< "World coords: " << world[0][0] << " " << world[0][1] << " " << world[0][2] << std::endl;

    pixelVectors pix_back = wcs.world2pixel(0, world);
    std::cout<< "Back to Pixel coords: " << pix_back[0][0] << " " << pix_back[0][1] << " " << pix_back[0][2] << std::endl;

    EXPECT_NEAR(pix_back[0][0], pix[0][0], 1e-4);
    EXPECT_NEAR(pix_back[0][1], pix[0][1], 1e-4);
    EXPECT_NEAR(pix_back[0][2], pix[0][2], 1e-4);

    pixelVectors pix2({{25.0, 25.0, 1.0},{25.0, 25.0, 1.0},{25.0, 25.0, 1.0},{25.0, 25.0, 1.0},{25.0, 25.0, 1.0}});
    worldVectors world2 = wcs.pixel2world(0, pix2);
    pixelVectors pix2_back = wcs.world2pixel(0, world2);

    for(size_t i=0; i<pix2.size(); i++)
    {
        EXPECT_NEAR(world2[i][0], world[0][0], 1e-4);
        EXPECT_NEAR(world2[i][1], world[0][1], 1e-4);
        EXPECT_NEAR(world2[i][2], world[0][2], 1e-4);

        EXPECT_NEAR(pix2_back[i][0], pix2[i][0], 1e-4);
        EXPECT_NEAR(pix2_back[i][1], pix2[i][1], 1e-4);
        EXPECT_NEAR(pix2_back[i][2], pix2[i][2], 1e-4);
    }

    EXPECT_NO_THROW(wcs.~FITSwcs());
}

TEST(FITS_wcs, COPY_CONSTRUCTOR)
{
    verbose = verboseLevel::VERBOSE_NONE;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs1(hdu);
    EXPECT_EQ(wcs1.getStatus(), WCSERR_SUCCESS);

    FITSwcs wcs2(wcs1); // Copy constructor
    EXPECT_EQ(wcs2.getStatus(), WCSERR_SUCCESS);
    EXPECT_EQ(wcs2.getNumberOfWCS(), wcs1.getNumberOfWCS());

    for(size_t k = 0; k < wcs1.getNumberOfWCS(); k++)
    {
        EXPECT_EQ(wcs2.getNumberOfAxis(k), wcs1.getNumberOfAxis(k));
        for(size_t i=1; i<=wcs1.getNumberOfAxis(k); i++)
        {
            EXPECT_NEAR(wcs2.CRPIX(k,i), wcs1.CRPIX(k,i), 1e-10);
            EXPECT_NEAR(wcs2.CRVAL(k,i), wcs1.CRVAL(k,i), 1e-10);
            EXPECT_NEAR(wcs2.CDELT(k,i), wcs1.CDELT(k,i), 1e-10);
        }

        for(size_t ix = 0 ; ix < 100; ix+=5)
            for(size_t iy = 0 ; iy < 100; iy+=5)
            {
                pixelVectors pix({{static_cast<double>(ix), static_cast<double>(iy)}});
                worldVectors world1 = wcs1.pixel2world(k, pix);
                worldVectors world2 = wcs2.pixel2world(k, pix);

                EXPECT_NEAR(world2[0][0], world1[0][0], 1e-8);
                EXPECT_NEAR(world2[0][1], world1[0][1], 1e-8);

                pixelVectors pix2_back = wcs2.world2pixel(k, world2);

                EXPECT_NEAR(pix2_back[0][0], pix[0][0], 1e-8);
                EXPECT_NEAR(pix2_back[0][1], pix[0][1], 1e-8);
            }
    }
}

TEST(FITS_wcs, SUBCOPY_CONSTRUCTOR)
{
    verbose = verboseLevel::VERBOSE_NONE;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

    
    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs1(hdu);
    EXPECT_EQ(wcs1.getStatus(), WCSERR_SUCCESS);

    for(size_t k=0; k < wcs1.getNumberOfWCS(); k++)
    {
    
        FITSwcs wcs2(wcs1,k); // Copy constructor
        EXPECT_EQ(wcs2.getStatus(), WCSERR_SUCCESS);
        EXPECT_EQ(wcs2.getNumberOfWCS(), 1);

        EXPECT_EQ(wcs2.getNumberOfAxis(0), wcs1.getNumberOfAxis(k));
        for(size_t i=1; i<=wcs1.getNumberOfAxis(k); i++)
        {
            EXPECT_NEAR(wcs2.CRPIX(i), wcs1.CRPIX(k,i), 1e-10);
            EXPECT_NEAR(wcs2.CRVAL(i), wcs1.CRVAL(k,i), 1e-10);
            EXPECT_NEAR(wcs2.CDELT(i), wcs1.CDELT(k,i), 1e-10);
        }
        
        for(size_t ix = 0 ; ix < 100; ix+=5)
            for(size_t iy = 0 ; iy < 100; iy+=5)
            {
                pixelVectors pix({{static_cast<double>(ix), static_cast<double>(iy)}});
                worldVectors world1 = wcs1.pixel2world(k, pix);
                worldVectors world2 = wcs2.pixel2world(0, pix);

                EXPECT_NEAR(world2[0][0], world1[0][0], 1e-8);
                EXPECT_NEAR(world2[0][1], world1[0][1], 1e-8);

                pixelVectors pix2_back = wcs2.world2pixel(0, world2);

                EXPECT_NEAR(pix2_back[0][0], pix[0][0], 1e-8);
                EXPECT_NEAR(pix2_back[0][1], pix[0][1], 1e-8);
            }
        
    }
}

TEST(FITS_wcs, SUBFRAMECOPY_CONSTRUCTOR_INT)
{
    verbose = verboseLevel::VERBOSE_NONE;
    //verbose |= verboseLevel::VERBOSE_DEBUG;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

    
    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs1(hdu);
    EXPECT_EQ(wcs1.getStatus(), WCSERR_SUCCESS);

    for(size_t k=0; k < wcs1.getNumberOfWCS(); k++)
    {
    
        if(k==0)
        {
            EXPECT_ANY_THROW( FITSwcs wcs2(wcs1,k,std::vector<size_t>({10,50})); ); // Copy constructor with invalid offset size
            continue;
        }

        FITSwcs wcs2(wcs1,k,std::vector<size_t>({10,50})); // Copy constructor
        EXPECT_EQ(wcs2.getStatus(), WCSERR_SUCCESS);
        EXPECT_EQ(wcs2.getNumberOfWCS(), 1);

        EXPECT_EQ(wcs2.getNumberOfAxis(0), wcs1.getNumberOfAxis(k));
        for(size_t i=1; i<=wcs1.getNumberOfAxis(k); i++)
        {
            EXPECT_NEAR(wcs2.CRPIX(0,i), wcs1.CRPIX(k,i)-((i==1)?10:50), 1e-6);
            EXPECT_NEAR(wcs2.CRVAL(0,i), wcs1.CRVAL(k,i), 1e-6);
            EXPECT_NEAR(wcs2.CDELT(0,i), wcs1.CDELT(k,i), 1e-6);
        }
        
        for(size_t ix = 0 ; ix < 100; ix+=5)
            for(size_t iy = 0 ; iy < 100; iy+=5)
            {
                pixelVectors newpix({{static_cast<double>(ix), static_cast<double>(iy)}});
                pixelVectors oldpix({{static_cast<double>(ix+10), static_cast<double>(iy+50)}});
                worldVectors world1 = wcs1.pixel2world(k, oldpix);
                worldVectors world2 = wcs2.pixel2world(0, newpix);

                EXPECT_NEAR(world2[0][0], world1[0][0], 1e-8);
                EXPECT_NEAR(world2[0][1], world1[0][1], 1e-8);

                pixelVectors pix2_back = wcs2.world2pixel(0, world2);

                EXPECT_NEAR(pix2_back[0][0], newpix[0][0], 1e-8);
                EXPECT_NEAR(pix2_back[0][1], newpix[0][1], 1e-8);
            }
        
    }
}

TEST(FITS_wcs, SUBFRAMECOPY_CONSTRUCTOR_DBL)
{
    verbose = verboseLevel::VERBOSE_NONE;
    //verbose |= verboseLevel::VERBOSE_DEBUG;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

    
    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs1(hdu);
    EXPECT_EQ(wcs1.getStatus(), WCSERR_SUCCESS);

    for(size_t k=0; k < wcs1.getNumberOfWCS(); k++)
    {
    
        if(k==0)
        {
            EXPECT_ANY_THROW( FITSwcs wcs2(wcs1,k,{10.,50.}); ); // Copy constructor with invalid offset size
            continue;
        }

        FITSwcs wcs2(wcs1,k,{10.,50.}); // Copy constructor
        EXPECT_EQ(wcs2.getStatus(), WCSERR_SUCCESS);
        EXPECT_EQ(wcs2.getNumberOfWCS(), 1);

        EXPECT_EQ(wcs2.getNumberOfAxis(0), wcs1.getNumberOfAxis(k));
        for(size_t i=1; i<=wcs1.getNumberOfAxis(k); i++)
        {
            EXPECT_NEAR(wcs2.CRPIX(0,i), wcs1.CRPIX(k,i)-((i==1)?10:50), 1e-6);
            EXPECT_NEAR(wcs2.CRVAL(0,i), wcs1.CRVAL(k,i), 1e-6);
            EXPECT_NEAR(wcs2.CDELT(0,i), wcs1.CDELT(k,i), 1e-6);
        }
        
        for(size_t ix = 0 ; ix < 100; ix+=5)
            for(size_t iy = 0 ; iy < 100; iy+=5)
            {
                pixelVectors newpix({{static_cast<double>(ix), static_cast<double>(iy)}});
                pixelVectors oldpix({{static_cast<double>(ix+10), static_cast<double>(iy+50)}});
                worldVectors world1 = wcs1.pixel2world(k, oldpix);
                worldVectors world2 = wcs2.pixel2world(0, newpix);

                EXPECT_NEAR(world2[0][0], world1[0][0], 1e-8);
                EXPECT_NEAR(world2[0][1], world1[0][1], 1e-8);

                pixelVectors pix2_back = wcs2.world2pixel(0, world2);

                EXPECT_NEAR(pix2_back[0][0], newpix[0][0], 1e-8);
                EXPECT_NEAR(pix2_back[0][1], newpix[0][1], 1e-8);
            }
        
    }
}

TEST(FITS_wcs, swapWCS)
{
    verbose = verboseLevel::VERBOSE_NONE;

    // This is a placeholder test. Replace with actual tests for FITSwcs functionality.
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/DSS.fits", true);
#else
    FITSmanager fm("testdata/DSS.fits", true);
#endif

    std::shared_ptr<FITShdu> hdu = fm.GetPrimaryHeader();
    FITSwcs wcs1(hdu);
    EXPECT_EQ(wcs1.getStatus(), WCSERR_SUCCESS);

    FITSwcs wcs2(buildFakeHDU());
    EXPECT_EQ(wcs2.getStatus(), WCSERR_SUCCESS);

    FITSwcs::swap(wcs1, wcs2);

    EXPECT_EQ(wcs1.getNumberOfWCS(), 1);
    EXPECT_EQ(wcs1.getNumberOfAxis(0), 3);

    EXPECT_EQ(wcs2.getNumberOfWCS(), 3);
    EXPECT_EQ(wcs2.getNumberOfAxis(0), 2);

    EXPECT_NEAR(wcs1.CRPIX(0,1), 90.0, 1e-7);
    EXPECT_NEAR(wcs1.CRPIX(0,2), 90.0, 1e-7);
    EXPECT_NEAR(wcs1.CRPIX(0,3), 1., 1e-7);

    EXPECT_NEAR(wcs2.CRPIX(1,1), 5009.62946648,1e-7);
    EXPECT_NEAR(wcs2.CRPIX(1,2), -7580.52942578,1e-7);

    EXPECT_NEAR(wcs1.CRVAL(0,1), 192.2500, 1e-7);
    EXPECT_NEAR(wcs1.CRVAL(0,2), 27.4000, 1e-7);
    EXPECT_NEAR(wcs1.CRVAL(0,3), 5569271.04, 1e-7);

    EXPECT_NEAR(wcs2.CRVAL(1,1), 80.5671666667,1e-7);
    EXPECT_NEAR(wcs2.CRVAL(1,2), -14.953,1e-7);
}

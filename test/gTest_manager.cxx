#include <gtest/gtest.h>
#include <DSTfits/FITSmanager.h>

using namespace DSL;

#pragma region - Test verbose utilities

TEST(FITSmanager, verbose_operations)
{
    DSL::verbose = DSL::verboseLevel::VERBOSE_NONE;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x00);

    DSL::verbose = DSL::verboseLevel::VERBOSE_BASIC;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x01);

    DSL::verbose = DSL::verboseLevel::VERBOSE_DETAIL;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x0F);

    DSL::verbose = DSL::verboseLevel::VERBOSE_HDU;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x02);
    
    DSL::verbose = DSL::verboseLevel::VERBOSE_IMG;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x04);

    DSL::verbose = DSL::verboseLevel::VERBOSE_TBL;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x08);

    DSL::verbose = DSL::verboseLevel::VERBOSE_DEBUG;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0xFF);

    DSL::verbose = DSL::verboseLevel::VERBOSE_BASIC;
    DSL::verbose |= DSL::verboseLevel::VERBOSE_IMG;
    DSL::verbose |= DSL::verboseLevel::VERBOSE_TBL;
    DSL::verbose |= DSL::verboseLevel::VERBOSE_HDU;
    ASSERT_EQ  (static_cast<uint8_t>(DSL::verbose), static_cast<uint8_t>(DSL::verboseLevel::VERBOSE_DETAIL));
    ASSERT_EQ  (DSL::verbose|DSL::verboseLevel::VERBOSE_DETAIL, DSL::verboseLevel::VERBOSE_DETAIL);
    ASSERT_TRUE((DSL::verbose&DSL::verboseLevel::VERBOSE_IMG)==DSL::verboseLevel::VERBOSE_IMG);
    ASSERT_TRUE((DSL::verbose&DSL::verboseLevel::VERBOSE_TBL)==DSL::verboseLevel::VERBOSE_TBL);
    ASSERT_TRUE((DSL::verbose&DSL::verboseLevel::VERBOSE_HDU)==DSL::verboseLevel::VERBOSE_HDU);

    DSL::verbose &= DSL::verboseLevel::VERBOSE_IMG;
    ASSERT_EQ   (static_cast<uint8_t>(DSL::verbose), static_cast<uint8_t>(DSL::verboseLevel::VERBOSE_IMG));
    ASSERT_FALSE((DSL::verbose&DSL::verboseLevel::VERBOSE_TBL)==DSL::verboseLevel::VERBOSE_TBL);
    ASSERT_FALSE((DSL::verbose&DSL::verboseLevel::VERBOSE_HDU)==DSL::verboseLevel::VERBOSE_HDU);
    ASSERT_TRUE (static_cast<uint8_t>(DSL::verbose & DSL::verboseLevel::VERBOSE_IMG) > 0);

    DSL::verbose = ~DSL::verbose;
    ASSERT_FALSE(DSL::verbose==DSL::verboseLevel::VERBOSE_IMG);
    ASSERT_TRUE((DSL::verbose&DSL::verboseLevel::VERBOSE_TBL)==DSL::verboseLevel::VERBOSE_TBL);
    ASSERT_TRUE((DSL::verbose&DSL::verboseLevel::VERBOSE_HDU)==DSL::verboseLevel::VERBOSE_HDU);
    ASSERT_TRUE(static_cast<uint8_t>(DSL::verbose&(~DSL::verboseLevel::VERBOSE_DETAIL))==0xF0);

    DSL::verbose = DSL::verboseLevel::VERBOSE_NONE;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x00);
}

#pragma endregion
#pragma region - ctor/dtor

TEST(FITSmanager, default_ctor)
{
    FITSmanager fm;
    ASSERT_EQ(fm.NumberOfHeader(), 0);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName().size(), 0);
}

TEST(FITSmanager, ctor_file)
{
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    
    fm.Close();
    ASSERT_FALSE(fm.isOpen());

    ASSERT_EQ(hdu_ref.get(), nullptr);
    ASSERT_EQ(hdu_ref.use_count(), 0);

    ASSERT_ANY_THROW(FITSmanager("build/testdata/rosat_pspc_rdf2_3_bk1"));
}

TEST(FITSmanager, ctor_cpy)
{
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");

    FITSmanager fm_cpy(fm);
    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.NumberOfHeader(), 1);
    ASSERT_EQ(fm_cpy.Status(), 0);
    ASSERT_EQ(fm_cpy.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    const std::shared_ptr<fitsfile>& hdu_cpy = fm_cpy.CurrentHDU();
    ASSERT_EQ(hdu_ref.use_count(), 2); 
    ASSERT_EQ(hdu_cpy.use_count(), 2); 
    
    fm.Close();
    
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(hdu_ref.get(), nullptr);

    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_NE(fm_cpy.CurrentHDU().get(), nullptr);
    ASSERT_EQ(hdu_cpy.use_count(), 1); 

    fm_cpy.Close();
    ASSERT_FALSE(fm.isOpen());
    ASSERT_FALSE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.CurrentHDU().get(), nullptr);

    ASSERT_EQ(hdu_ref.use_count(), 0);
    ASSERT_EQ(hdu_cpy.use_count(), 0);
}

TEST(FITSmanager, ctor_fits)
{
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");

    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    FITSmanager fm_cpy(hdu_ref);
    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.NumberOfHeader(), 1);
    ASSERT_EQ(fm_cpy.Status(), 0);
    ASSERT_EQ(fm_cpy.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    
    const std::shared_ptr<fitsfile>& hdu_cpy = fm_cpy.CurrentHDU();
    ASSERT_EQ(hdu_ref.use_count(), 2); 
    ASSERT_EQ(hdu_cpy.use_count(), 2); 
    
    fm.Close();
    
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(hdu_ref.get(), nullptr);

    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_NE(fm_cpy.CurrentHDU().get(), nullptr);
    ASSERT_EQ(hdu_cpy.use_count(), 1); 

    fm_cpy.Close();
    ASSERT_FALSE(fm.isOpen());
    ASSERT_FALSE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.CurrentHDU().get(), nullptr);

    ASSERT_EQ(hdu_ref.use_count(), 0);
    ASSERT_EQ(hdu_cpy.use_count(), 0);
}

TEST(FITSmanager, ctor_cfitsio)
{
    fitsfile * fits = NULL;
    int fits_status = 0;
    EXPECT_EQ(fits_open_file(&fits, "build/testdata/rosat_pspc_rdf2_3_bk1.fits", true, &fits_status),0);
    EXPECT_EQ(fits_status, 0);

    FITSmanager fm(*fits);
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();

    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(fits), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(hdu_ref), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");

    ASSERT_EQ(hdu_ref.use_count(), 1);

    fm.Close();
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(hdu_ref.get(), nullptr);
    ASSERT_EQ(hdu_ref.use_count(), 0);
}

#pragma endregion
#pragma region - open/close

TEST(FITSmanager, open_close)
{
    FITSmanager fm;
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 0);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName().size(), 0);

    fm.OpenFile("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");

    fm.Close();
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 0);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName().size(), 0);

    FITSmanager fm2 = fm.Open("build/testdata/rosat_pspc_rdf2_3_im2.fits");
    ASSERT_TRUE(fm2.isOpen());
    ASSERT_EQ  (fm2.NumberOfHeader(), 3);
    ASSERT_EQ  (fm2.Status(), 0);
    ASSERT_EQ  (fm2.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_im2.fits");

    fm2.Close();
    ASSERT_FALSE(fm2.isOpen());
    ASSERT_EQ(fm2.NumberOfHeader(), 0);
    ASSERT_EQ(fm2.Status(), 0);
    ASSERT_EQ(fm2.GetFileName().size(), 0);
}

#pragma endregion
#pragma region - exception

TEST(FITSmanager, exception_manager)
{
    FITSmanager fm;
    ASSERT_ANY_THROW(fm.GetPrimaryHeader());
    ASSERT_ANY_THROW(fm.GetHeaderAtIndex(1));
    ASSERT_ANY_THROW(fm.GetImageAtIndex(1));
    ASSERT_ANY_THROW(fm.GetTableAtIndex(1));
    ASSERT_ANY_THROW(fm.GetTable("TABLE"));
    ASSERT_ANY_THROW(fm.MoveToHDU(1));
    ASSERT_ANY_THROW(fm.MoveToPrimary());
    ASSERT_ANY_THROW(fm.AppendKeyToHeader(1, "KEY", TSTRING, "VAL", "CMT"));
    ASSERT_ANY_THROW(fm.AppendKey("KEY", TSTRING, "VAL", "CMT"));

    fm.Open("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_ANY_THROW(fm.GetHeaderAtIndex(2));
    ASSERT_ANY_THROW(fm.GetImageAtIndex(2));
    ASSERT_ANY_THROW(fm.GetTableAtIndex(2));
    ASSERT_ANY_THROW(fm.MoveToHDU(2));

    fm.Close();
    ASSERT_ANY_THROW(fm.AppendKeyToHeader(1, "KEY", TSTRING, "VAL", "CMT"));
    ASSERT_ANY_THROW(fm.AppendKey("KEY", TSTRING, "VAL", "CMT"));
}

#pragma endregion
#pragma region - move between hdu

TEST(FITSmanager, move_between_hdu)
{
    int hdu_num=0;
    int status=0;
    FITSmanager ffm("build/testdata/rosat_pspc_rdf2_3_im2.fits");
    ASSERT_EQ(ffm.NumberOfHeader(), 3);
    
    const std::shared_ptr<fitsfile>& this_hdu = ffm.CurrentHDU();
    ASSERT_EQ(ffm.GetFileName(this_hdu), "build/testdata/rosat_pspc_rdf2_3_im2.fits");

    EXPECT_ANY_THROW(ffm.MoveToHDU(0));

    EXPECT_EQ(fits_get_hdu_num(this_hdu.get(), &hdu_num),1);
    EXPECT_EQ(hdu_num, 1);
    ffm.MoveToHDU(1);
    EXPECT_EQ(hdu_num, 1);
    
    ffm.MoveToHDU(2);
    EXPECT_EQ(fits_get_hdu_num(this_hdu.get(), &hdu_num),2);
    EXPECT_EQ(hdu_num, 2);

    ffm.MoveToHDU(3);
    EXPECT_EQ(fits_get_hdu_num(this_hdu.get(), &hdu_num),3);
    EXPECT_EQ(hdu_num, 3);

    EXPECT_ANY_THROW(ffm.MoveToHDU(4));

    ffm.MoveToPrimary();
    EXPECT_EQ(fits_get_hdu_num(this_hdu.get(), &hdu_num),1);
    EXPECT_EQ(hdu_num, 1);

    fits_movabs_hdu(this_hdu.get(), 2, NULL, &status);
    fits_get_hdu_num(this_hdu.get(), &hdu_num);
    EXPECT_EQ(hdu_num, 2);

    ffm.Close();
}

#pragma endregion
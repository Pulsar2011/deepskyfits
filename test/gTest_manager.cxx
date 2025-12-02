#include <gtest/gtest.h>
#include <DSTfits/FITSmanager.h>

#include <type_traits>
#include <string>
#include <limits>
#include <fstream>
#include <filesystem>

using namespace DSL;

#pragma region - Test verbose utilities

TEST(FITSmanager, verbose_operations)
{
    DSL::verbose = DSL::verboseLevel::VERBOSE_NONE;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x00);

    DSL::verbose = DSL::verboseLevel::VERBOSE_BASIC;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0x01);

    DSL::verbose = DSL::verboseLevel::VERBOSE_DETAIL;
    ASSERT_EQ(static_cast<uint8_t>(DSL::verbose), 0xF0);

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
    ASSERT_EQ  (static_cast<uint8_t>(DSL::verbose), static_cast<uint8_t>(DSL::verboseLevel::VERBOSE_ALL));
    ASSERT_EQ  (DSL::verbose|DSL::verboseLevel::VERBOSE_DETAIL, DSL::verboseLevel::VERBOSE_DEBUG);
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
    ASSERT_TRUE(static_cast<uint8_t>(DSL::verbose&(~DSL::verboseLevel::VERBOSE_DETAIL))==0x0B);

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
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    FITSmanager fm("testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);

#ifdef Darwinx86_64
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    
    fm.Close();
    ASSERT_FALSE(fm.isOpen());

    ASSERT_EQ(hdu_ref.get(), nullptr);
    ASSERT_EQ(hdu_ref.use_count(), 0);

#ifdef Darwinx86_64
    ASSERT_ANY_THROW(FITSmanager("build/testdata/rosat_pspc_rdf2_3_bk1"));
#else
    ASSERT_ANY_THROW(FITSmanager("testdata/rosat_pspc_rdf2_3_bk1"));
#endif
}

TEST(FITSmanager, ctor_cpy)
{
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    FITSmanager fm("testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
#ifdef Darwinx86_64
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif

    FITSmanager fm_cpy(fm);
    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.NumberOfHeader(), 1);
    ASSERT_EQ(fm_cpy.Status(), 0);
#ifdef Darwinx86_64
    ASSERT_EQ(fm_cpy.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm_cpy.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    
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
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    FITSmanager fm("testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
#ifdef Darwinx86_64
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif

    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();
    FITSmanager fm_cpy(hdu_ref);
    ASSERT_TRUE(fm_cpy.isOpen());
    ASSERT_EQ(fm_cpy.NumberOfHeader(), 1);
    ASSERT_EQ(fm_cpy.Status(), 0);

#ifdef Darwinx86_64
    ASSERT_EQ(fm_cpy.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm_cpy.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    
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
#ifdef Darwinx86_64
    EXPECT_EQ(fits_open_file(&fits, "build/testdata/rosat_pspc_rdf2_3_bk1.fits", true, &fits_status),0);
#else
    EXPECT_EQ(fits_open_file(&fits, "testdata/rosat_pspc_rdf2_3_bk1.fits", true, &fits_status),0);
#endif
    EXPECT_EQ(fits_status, 0);

    FITSmanager fm(*fits);
    const std::shared_ptr<fitsfile>& hdu_ref = fm.CurrentHDU();

    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    
#ifdef Darwinx86_64
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(fits), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(hdu_ref), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(fits), "testdata/rosat_pspc_rdf2_3_bk1.fits");
    ASSERT_EQ(fm.GetFileName(hdu_ref), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif
    ASSERT_NE(hdu_ref.get(), nullptr);
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

#ifdef Darwinx86_64
    fm.OpenFile("build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    fm.OpenFile("testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif  
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
#ifdef Darwinx86_64
    ASSERT_EQ(fm.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_bk1.fits");
#else
    ASSERT_EQ(fm.GetFileName(), "testdata/rosat_pspc_rdf2_3_bk1.fits");
#endif  

    fm.Close();
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 0);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName().size(), 0);

#ifdef Darwinx86_64
    FITSmanager fm2 = fm.Open("build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    FITSmanager fm2 = fm.Open("testdata/rosat_pspc_rdf2_3_im2.fits");
#endif  
    ASSERT_TRUE(fm2.isOpen());
    ASSERT_EQ  (fm2.NumberOfHeader(), 3);
    ASSERT_EQ  (fm2.Status(), 0);
#ifdef Darwinx86_64
    ASSERT_EQ  (fm2.GetFileName(), "build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    ASSERT_EQ  (fm2.GetFileName(), "testdata/rosat_pspc_rdf2_3_im2.fits");
#endif  

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

#ifdef Darwinx86_64
    fm.OpenFile("build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    fm.OpenFile("testdata/rosat_pspc_rdf2_3_im2.fits");
#endif
    ASSERT_TRUE(fm.isOpen());
    ASSERT_ANY_THROW(fm.GetHeaderAtIndex(4));
    ASSERT_ANY_THROW(fm.GetImageAtIndex(2));
    ASSERT_ANY_THROW(fm.GetTableAtIndex(4));
    ASSERT_ANY_THROW(fm.MoveToHDU(4));

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

#ifdef Darwinx86_64
    FITSmanager ffm("build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    FITSmanager ffm("testdata/rosat_pspc_rdf2_3_im2.fits");
#endif
    ASSERT_EQ(ffm.NumberOfHeader(), 3);
    
    const std::shared_ptr<fitsfile>& this_hdu = ffm.CurrentHDU();
#ifdef Darwinx86_64
    ASSERT_EQ(ffm.GetFileName(this_hdu), "build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    ASSERT_EQ(ffm.GetFileName(this_hdu), "testdata/rosat_pspc_rdf2_3_im2.fits");
#endif
    

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

TEST(FITSmanager, create_fitsfile)
{
#ifdef Darwinx86_64
    const std::string testfile = "build/testdata/test_create_fitsfile.fits";
#else
    const std::string testfile = "testdata/test_create_fitsfile.fits";
#endif
    std::remove(testfile.c_str());

    FITSmanager fm = FITSmanager::Create(testfile, false);
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), testfile);

    const std::shared_ptr<fitsfile>& this_hdu = fm.CurrentHDU();
    ASSERT_EQ(this_hdu.use_count(), 1);

    const std::shared_ptr<FITShdu> hdu_w = fm.GetPrimaryHeader();
    ASSERT_NE(hdu_w, nullptr);

    ASSERT_NE(hdu_w->GetValueForKey("COMMENT").find(DSF::gGIT::version()),std::string::npos);

    fm.Close();
    
    ASSERT_FALSE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 0);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName().size(), 0);
    ASSERT_EQ(this_hdu.use_count(), 0);

    // try to create again without replace
    ASSERT_ANY_THROW(FITSmanager::Create(testfile, false));
    ASSERT_FALSE(fm.isOpen());

    // create again with replace
    fm = FITSmanager::Create(testfile, true);
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), testfile);

    const std::shared_ptr<FITShdu> hdu_w2 = fm.GetPrimaryHeader();
    hdu_w2->ValueForKey("OBSERVER", "UnitTest", fChar, "Created by UnitTest");
    hdu_w2->Write(this_hdu);
    fm.Close();
    
    FITSmanager fo = FITSmanager(testfile);
    ASSERT_TRUE(fo.isOpen());
    ASSERT_EQ(fo.NumberOfHeader(), 1);
    ASSERT_EQ(fo.Status(), 0);

    const std::shared_ptr<FITShdu> hdu_r = fo.GetPrimaryHeader();
    ASSERT_TRUE(hdu_r->Exists("OBSERVER"));
    ASSERT_EQ(hdu_r->GetValueForKey("OBSERVER"), "UnitTest");

    fo.Close();
}

TEST(FITSmanager, ReadingImage)
{    
#ifdef Darwinx86_64
    std::string src = "build/testdata/testkeys.fits";
#else
    std::string src = "testdata/testkeys.fits";
#endif

    // Use the copied file for all operations
    FITSmanager ff(src);
    EXPECT_TRUE(ff.isOpen());

    std::shared_ptr<FITScube> img = ff.GetPrimary();
    EXPECT_NE(img, nullptr);
    EXPECT_EQ(img->Size(1), 300);
    EXPECT_EQ(img->Size(2), 300);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());         

}

template <typename T>
class FITSimgTest : public ::testing::Test
{
    protected:
        using value_type = T;
        static size_t N() { return 10; }
        
        std::string MakeFilename() const
        {
            std::ostringstream ss;
#ifdef Darwinx86_64
            ss << "build/testdata/test_" << typeid(T).name() << ".fits";
#else
            ss << "testdata/test_" << typeid(T).name() << ".fits";
#endif
            return ss.str();
        }
#ifdef Darwinx86_64
        void EnsureOutDir() const { std::filesystem::create_directories("build/testdata"); }
#else
        void EnsureOutDir() const { std::filesystem::create_directories("testdata"); }
#endif
};

using StatTypes = ::testing::Types<uint8_t,int8_t,int16_t,uint16_t,int32_t,uint32_t,int64_t,uint64_t,float,double>;
TYPED_TEST_SUITE(FITSimgTest, StatTypes);

TYPED_TEST(FITSimgTest, modif_fitsfile)
{
    std::remove(this->MakeFilename().c_str());

    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;

    this->EnsureOutDir();

    const size_t N = TestFixture::N();
    FITSimg<T> img1(2, {N, N});
    
    auto data = img1.template GetData<T>();
    ASSERT_NE(data, nullptr);

    // fill predictable pattern: value = (i % 256) + (j)
    for (size_t j = 0; j < data->size(); ++j)
        (*data)[j] = static_cast<T>(1);

    img1.Write(this->MakeFilename(), true);

    FITSmanager fm(this->MakeFilename(),false);
    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 1);
    ASSERT_EQ(fm.Status(), 0);
    ASSERT_EQ(fm.GetFileName(), this->MakeFilename());

    std::shared_ptr<FITScube> imgptr = fm.GetPrimary();
    ASSERT_NE(imgptr, nullptr);
    ASSERT_EQ(imgptr->Size(1), N);
    ASSERT_EQ(imgptr->Size(2), N);
    auto data_r = imgptr->template GetData<T>();
    ASSERT_NE(data_r, nullptr);
    ASSERT_NEAR(((*data_r)-(*data)).sum(), 0.0, 1e-12);

    FITSimg<T> img2(2, {N, N});
    img2.SetName("SECOND_IMAGE");
    auto data2 = img2.template GetData<T>();
    ASSERT_NE(data2, nullptr);

    for(size_t k = 0; k < data2->size(); k++)
        (*data2)[k] = static_cast<T>(2);

    fm.AppendImage(img2);
    fm.Close();

    FITSmanager fm2(this->MakeFilename());
    ASSERT_TRUE(fm2.isOpen());
    ASSERT_EQ(fm2.NumberOfHeader(), 2);
    ASSERT_EQ(fm2.Status(), 0);
    ASSERT_EQ(fm2.GetFileName(), this->MakeFilename());
    std::shared_ptr<FITScube> imgptr2 = fm2.GetImageAtIndex(2);
    ASSERT_NE(imgptr2, nullptr);
    ASSERT_EQ(imgptr2->Size(1), N);
    ASSERT_EQ(imgptr2->Size(2), N);
    auto data_r2 = imgptr2->template GetData<T>();
    ASSERT_NE(data_r2, nullptr);
    ASSERT_NEAR(((*data_r2)-(*data2)).sum(), 0.0, 1e-12);

    fm2.Close();

    std::remove(this->MakeFilename().c_str());

}

TEST(FITStable, openTableFromFile)
{
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/rosat_pspc_rdf2_3_im2.fits");
#else
    FITSmanager fm("testdata/rosat_pspc_rdf2_3_im2.fits");
#endif

    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 3);

    std::shared_ptr<FITStable> tbl = fm.GetTable("LS2");
    ASSERT_NE(tbl, nullptr);
    ASSERT_EQ(tbl->nrows(), 6);
    ASSERT_EQ(tbl->ncols(), 9);

    FITScolumn<int16_t>* src_nr = dynamic_cast<FITScolumn<int16_t>* >( tbl->getColumn("SRC_NR").get() );
    FITScolumn<float>* s_like   = dynamic_cast<FITScolumn<float>* >( tbl->getColumn("S_LIKE").get() );
    ASSERT_EQ(src_nr->size(), s_like->size());
    ASSERT_EQ(src_nr->size(), static_cast<size_t>(tbl->nrows()));

    for(size_t i=0; i<static_cast<size_t>(tbl->nrows()); i++)
    {
        ASSERT_EQ(src_nr->values<int16_t>().at(i), static_cast<int8_t>(i+1));
    }

    ASSERT_NEAR(s_like->values<float>().at(0),  437, 1e-6);
    ASSERT_NEAR(s_like->values<float>().at(1),  49, 1e-6);
    ASSERT_NEAR(s_like->values<float>().at(2),  9, 1e-6);
    ASSERT_NEAR(s_like->values<float>().at(3),  10, 1e-6);
    ASSERT_NEAR(s_like->values<float>().at(4),  8, 1e-6);
    ASSERT_NEAR(s_like->values<float>().at(5),  9, 1e-6);

    std::shared_ptr<FITStable> tbb = fm.GetTableAtIndex(3);
    ASSERT_NE(tbb, nullptr);
    ASSERT_EQ(tbb->nrows(), 17);
    ASSERT_EQ(tbb->ncols(), 9);
    ASSERT_EQ(tbb->GetName(), "MS2");
    FITScolumn<int16_t>* brc_nr = dynamic_cast<FITScolumn<int16_t>* > (tbb->getColumn("SRC_NR").get());

    for(size_t i=0; i<static_cast<size_t>(tbb->nrows()); i++)
    {
        ASSERT_EQ(brc_nr->values<int16_t>().at(i), static_cast<int8_t>(i+1));
    }

    tbb.reset();
    tbl.reset();
    fm.Close();
}

TEST(FITStable, UpdateTable)
{
    FITStable table;
    table.setName("UNIT_TEST_TABLE");

    // --- Scalar columns ---
    {
        std::cout<<"Inserting COL_SBYTE columns..."<<std::endl;
        FITScolumn<int8_t>    col_sbyte ("COL_SBYTE" , tsbyte   , "", 1);
        col_sbyte.push_back(int8_t(1)); col_sbyte.push_back(int8_t(2)); col_sbyte.push_back(int8_t(3));
        EXPECT_NO_THROW(table.InsertColumn(std::make_shared< FITScolumn<int8_t> >(col_sbyte)));
    }

    {
        CFITSIOGuard guard;   
#ifdef Darwinx86_64
        EXPECT_NO_THROW(table.write("build/testdata/test_fitstable.fits", 0, true));
#else
        EXPECT_NO_THROW(table.write("testdata/test_fitstable.fits", 0, true));
#endif 
    }

    CFITSIOGuard guard;   
#ifdef Darwinx86_64
    FITSmanager fm("build/testdata/test_fitstable.fits",false);
#else
    FITSmanager fm("testdata/test_fitstable.fits",false);
#endif

    ASSERT_TRUE(fm.isOpen());
    ASSERT_EQ(fm.NumberOfHeader(), 2);


    std::shared_ptr<FITStable> readTable = fm.GetTableAtIndex(2);
    ASSERT_NE(readTable, nullptr);
    ASSERT_EQ(readTable->nrows(), 3u);
    ASSERT_EQ(readTable->ncols(), 1u);

    {
        std::cout<<"Inserting COL_UINT64 columns..."<<std::endl;
        FITScolumn<uint64_t> col_uint64("COL_UINT64", tulonglong, "", 1);
        col_uint64.push_back(uint64_t(9)); col_uint64.push_back(uint64_t(10)); col_uint64.push_back(uint64_t(11));
        EXPECT_NO_THROW(readTable->InsertColumn(std::make_shared< FITScolumn<uint64_t> >(col_uint64)));
    }

    fm.UpdateTable(readTable);
    fm.Close();
    readTable.reset();
    
    ASSERT_EQ(readTable, nullptr);

#ifdef Darwinx86_64
    FITSmanager fm2("build/testdata/test_fitstable.fits");
#else
    FITSmanager fm2("testdata/test_fitstable.fits");
#endif

    readTable = fm2.GetTableAtIndex(2);
    ASSERT_NE(readTable, nullptr);
    ASSERT_EQ(readTable->nrows(), 3u);
    ASSERT_EQ(readTable->ncols(), 2u);

    auto* col_uint64 = dynamic_cast<FITScolumn<uint64_t>*>(readTable->getColumn("COL_UINT64").get());
    ASSERT_NE(col_uint64, nullptr);
    ASSERT_EQ(col_uint64->values<uint64_t>().size(), 3u);
    EXPECT_EQ(col_uint64->values<uint64_t>()[0], uint64_t(9));
    EXPECT_EQ(col_uint64->values<uint64_t>()[1], uint64_t(10));
    EXPECT_EQ(col_uint64->values<uint64_t>()[2], uint64_t(11));
    
    fm2.Close();

}

#pragma endregion
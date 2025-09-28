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

using namespace DSL;

TEST(FITSimg, Create_BYTE)
{
    return;
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<uint8_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<uint8_t>* data = img.GetData<uint8_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    uint8_t k = 0;
    uint8_t N2,NN2,NN;
    for(size_t j=0; j<img.Size(2); j++)
    {
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = static_cast<uint8_t>(k);
            if(i+j*img.Size(1) == N/2) N2 = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN = k;
        }
        k++;
    }

    EXPECT_EQ(img[N/2]    ,N2 );
    EXPECT_EQ(img[N*N/2]  ,NN2);
    EXPECT_EQ(img[N*N-1]  ,NN );

    img.Write("build/testdata/test_byte.fits", true);

    FITSmanager ff("build/testdata/test_byte.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), 256);
    EXPECT_EQ(rimg->Size(2), 256);

    std::valarray<uint8_t>* rdata = rimg->GetData<uint8_t>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    EXPECT_EQ((*rdata)[N/2],    N2 );
    EXPECT_EQ((*rdata)[N*N/2],  NN2);
    EXPECT_EQ((*rdata)[N*N-1]  ,NN );
    
    std::valarray<uint8_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_SHORT)
{
    return;
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<int16_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<int16_t>* data = img.GetData<int16_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    int16_t k = std::numeric_limits<int16_t>::min();
    int16_t N2,NN2,NN;
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2) N2    = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN  = k;

            k++;
        }

    EXPECT_EQ(img[N/2],    N2 );
    EXPECT_EQ(img[N*N/2],  NN2);
    EXPECT_EQ(img[N*N-1]  ,NN );

    img.Write("build/testdata/test_short.fits", true);

    FITSmanager ff("build/testdata/test_short.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<int16_t>* rdata = rimg->GetData<int16_t>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    EXPECT_EQ((*rdata)[N/2],    N2 );
    EXPECT_EQ((*rdata)[N*N/2],  NN2);
    EXPECT_EQ((*rdata)[N*N-1]  ,NN );
    
    std::valarray<int16_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_LONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<int32_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<int32_t>* data = img.GetData<int32_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    int32_t k = (int32_t) -65535;
    int32_t N2,NN2,NN;
    int32_t dN = static_cast<int32_t>(2.*65535. / static_cast<double>(N*N));

    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2)   N2  = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN  = k;
            k+=dN;
        }

    EXPECT_EQ(img[N/2]    , N2  );
    EXPECT_EQ(img[N*N/2]  , NN2 );
    EXPECT_EQ(img[N*N-1]  , NN  );

    img.Write("build/testdata/test_long.fits", true);

    FITSmanager ff("build/testdata/test_long.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<int32_t>* rdata = rimg->GetData<int32_t>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_EQ((*rdata)[N/2]    , N2  );
    EXPECT_EQ((*rdata)[N*N/2]  , NN2 );
    EXPECT_EQ((*rdata)[N*N-1]  , NN  );
    
    std::valarray<int32_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_LONGLONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<int64_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<int64_t>* data = img.GetData<int64_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    int64_t k = (int64_t) -65535;
    int64_t N2,NN2,NN;
    int64_t dN = static_cast<int64_t>(2.*65535. / static_cast<double>(N*N));

    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2) N2 = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN = k;
            k+=dN;
        }

    EXPECT_EQ(img[N/2],    N2 );
    EXPECT_EQ(img[N*N/2],  NN2);
    EXPECT_EQ(img[N*N-1]  ,NN );

    img.Write("build/testdata/test_longlong.fits", true);

    FITSmanager ff("build/testdata/test_longlong.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<int64_t>* rdata = rimg->GetData<int64_t>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_EQ((*rdata)[N/2],    N2 );
    EXPECT_EQ((*rdata)[N*N/2],  NN2);
    EXPECT_EQ((*rdata)[N*N-1]  ,NN );
    
    std::valarray<int64_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_FLOAT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<float> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<float>* data = img.GetData<float>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
            (*data)[i+j*img.Size(1)] = static_cast<float>(j+1);

    EXPECT_EQ(img[N/2],    (float) 1);
    EXPECT_EQ(img[N*N/2],  (float) 129);
    EXPECT_EQ(img[N*N-1]  ,(float) 256);

    img.Write("build/testdata/test_float.fits", true);

    FITSmanager ff("build/testdata/test_float.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);
    EXPECT_EQ(rimg->GetMean() > std::numeric_limits<double>::min(), true);

    std::valarray<float>* rdata = rimg->GetData<float>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    EXPECT_EQ((*rdata)[N/2],    (float) 1);
    EXPECT_EQ((*rdata)[N*N/2],  (float) 129);
    EXPECT_EQ((*rdata)[N*N-1]  ,(float) 256);
    
    std::valarray<float> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_DOUBLE)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = 256;
    FITSimg<double> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<double>* data = img.GetData<double>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
            (*data)[i+j*img.Size(1)] = static_cast<double>(j+1);

    EXPECT_EQ(img[N/2],    (double) 1);
    EXPECT_EQ(img[N*N/2],  (double) 129);
    EXPECT_EQ(img[N*N-1]  ,(double) 256);

    img.Write("build/testdata/test_double.fits", true);

    FITSmanager ff("build/testdata/test_double.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);
    EXPECT_EQ(rimg->GetMean() > std::numeric_limits<double>::min(), true);

    std::valarray<double>* rdata = rimg->GetData<double>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    EXPECT_EQ((*rdata)[N/2],    (double) 1);
    EXPECT_EQ((*rdata)[N*N/2],  (double) 129);
    EXPECT_EQ((*rdata)[N*N-1]  ,(double) 256);
    
    std::valarray<double> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Read_SHORT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    std::string src = "build/testdata/testkeys.fits";

    // Use the copied file for all operations

    FITSmanager ff(src);
    EXPECT_TRUE(ff.isOpen());

    std::shared_ptr<FITScube> img = ff.GetPrimary();
    EXPECT_NE(img, nullptr);
    EXPECT_EQ(img->Size(1), 300);
    EXPECT_EQ(img->Size(2), 300);

    size_t n = img->PixelIndex({150,150});
    EXPECT_EQ(n, 150 + 150*300);
    EXPECT_EQ(img->IntValueAtPixel({150,150}), 8801);
    EXPECT_EQ(img->IntValueAtPixel({0,0}), 3768);
    EXPECT_EQ(img->IntValueAtPixel({299,299}), 3965);
    EXPECT_EQ(img->IntValueAtPixel({59,290}), 6168);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Read_SHORT2FLOAT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    std::string src = "build/testdata/rosat_pspc_rdf2_3_bk1.fits";

    // Use the copied file for all operations

    FITSmanager ff(src);
    EXPECT_TRUE(ff.isOpen());

    std::shared_ptr<FITScube> img = ff.GetPrimary();
    EXPECT_NE(img, nullptr);
    EXPECT_EQ(img->Size(1), 512);
    EXPECT_EQ(img->Size(2), 512);

    size_t n = img->PixelIndex({256,256});
    EXPECT_EQ(n, 256 + 256*512);
    EXPECT_NEAR(img->FloatValueAtPixel(n), 0.143,1e-4);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, ReadAndWrite)
{
    verbose = verboseLevel::VERBOSE_NONE;

    std::string src = "build/testdata/vogtstar_awt.fits";
    std::string tmp = "build/testdata/vogtstar_awt_copy.fits";
    
    std::string cmd = "cp " + src + " " + tmp;
    int rc = std::system(cmd.c_str());
    if (rc != 0)
    {
        FAIL() << "cp failed with exit code " << rc;
    }

    // Use the copied file for all operations
    FITSmanager ff(tmp);
    EXPECT_TRUE(ff.isOpen());

    std::shared_ptr<FITScube> img = ff.GetPrimary();
    EXPECT_NE(img, nullptr);
    EXPECT_EQ(img->Size(1), 512);
    EXPECT_EQ(img->Size(2), 861);

    std::shared_ptr<FITScube> wimg = img->Window(35,215,25,34);
    EXPECT_NE(wimg, nullptr);
    EXPECT_EQ(wimg->Size(1), 25);
    EXPECT_EQ(wimg->Size(2), 34);    

    std::valarray<float>* src_img = img ->GetData<float>();
    std::valarray<float>* win_img = wimg->GetData<float>();
    ASSERT_NE(src_img, nullptr);
    ASSERT_NE(win_img, nullptr);

    std::valarray<float> expected = (*src_img)[std::gslice(35+215*861, {25,34}, {1,img->Size(1)})];
    expected -= (*win_img);
    EXPECT_EQ(expected.sum(), 0) << "Windowed image data does not match expected values";

    wimg->Write("build/testdata/vogtstar_awt_crop.fits", true);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}
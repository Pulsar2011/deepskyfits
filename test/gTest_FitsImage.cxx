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
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_EQ((*rdata)[N/2],    N2 );
    EXPECT_EQ((*rdata)[N*N/2],  NN2);
    EXPECT_EQ((*rdata)[N*N-1]  ,NN );
    
    std::valarray<uint8_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_SBYTE)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<int8_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<int8_t>* data = img.GetData<int8_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    int8_t k = -128;
    int8_t N2,NN2,NN;
    for(size_t j=0; j<img.Size(2); j++)
    {
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = static_cast<int8_t>(k);
            if(i+j*img.Size(1) == N/2) N2 = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN = k;
        }
        k++;
    }

    EXPECT_EQ(img[N/2]    ,N2 );
    EXPECT_EQ(img[N*N/2]  ,NN2);
    EXPECT_EQ(img[N*N-1]  ,NN );

    img.Write("build/testdata/test_sbyte.fits", true);

    FITSmanager ff("build/testdata/test_sbyte.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), 256);
    EXPECT_EQ(rimg->Size(2), 256);

    std::valarray<int8_t>* rdata = rimg->GetData<int8_t>();
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
    
    std::valarray<int8_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_SHORT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_EQ((*rdata)[N/2],    N2 );
    EXPECT_EQ((*rdata)[N*N/2],  NN2);
    EXPECT_EQ((*rdata)[N*N-1]  ,NN );
    
    std::valarray<int16_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_USHORT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<uint16_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);
    EXPECT_EQ(img.HDU().GetUInt16ValueForKey("BZERO"),32768);
    EXPECT_EQ(img.HDU().GetUInt16ValueForKey("BSCALE"),1);
    EXPECT_EQ(img.ReadBzero(), 32768);
    EXPECT_EQ(img.ReadBscale(), 1);

    std::valarray<uint16_t>* data = img.GetData<uint16_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    uint16_t k = 0;
    uint16_t N2,NN2,NN;
    uint16_t dN = static_cast<uint16_t>(static_cast<double>(std::numeric_limits<uint16_t>::max()) / static_cast<double>(N*N));
    if(dN < 1) dN = 1;
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2) N2    = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN  = k;

            k+=dN;

        }

    EXPECT_EQ(img[N/2],    N2 );
    EXPECT_EQ(img[N*N/2],  NN2);
    EXPECT_EQ(img[N*N-1]  ,NN );

    img.Write("build/testdata/test_ushort.fits", true);

    FITSmanager ff("build/testdata/test_ushort.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<uint16_t>* rdata = rimg->GetData<uint16_t>();
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
    
    std::valarray<uint16_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_LONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

TEST(FITSimg, Create_ULONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<uint32_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<uint32_t>* data = img.GetData<uint32_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    uint32_t k = 0;
    uint32_t N2,NN2,NN;
    uint32_t dN = dN = static_cast<uint16_t>(static_cast<double>(std::numeric_limits<uint32_t>::max()) / static_cast<double>(N*N));
    if(dN < 1) dN = 1;

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

    img.Write("build/testdata/test_ulong.fits", true);

    FITSmanager ff("build/testdata/test_ulong.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<uint32_t>* rdata = rimg->GetData<uint32_t>();
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
    
    std::valarray<uint32_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_LONGLONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

TEST(FITSimg, Create_ULONGLONG)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<uint64_t> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<uint64_t>* data = img.GetData<uint64_t>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    uint64_t k = 0;
    uint64_t N2,NN2,NN;
    uint64_t dN = static_cast<uint16_t>(static_cast<double>(std::numeric_limits<uint64_t>::max()) / static_cast<double>(N*N));
    if(dN < 1) dN = 1;

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

    img.Write("build/testdata/test_ulonglong.fits", true);

    FITSmanager ff("build/testdata/test_ulonglong.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<uint64_t>* rdata = rimg->GetData<uint64_t>();
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
    
    std::valarray<uint64_t> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_FLOAT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

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

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
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

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_EQ((*rdata)[N/2],    (double) 1);
    EXPECT_EQ((*rdata)[N*N/2],  (double) 129);
    EXPECT_EQ((*rdata)[N*N-1]  ,(double) 256);
    
    std::valarray<double> diff = (*data) - (*rdata);
    EXPECT_EQ(diff.sum(), 0) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_SHORT2FLOAT)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<float> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<float>* data = img.GetData<float>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    float k = -1250.;
    float N2,NN2,NN;
    float dN = (2*1260.)/static_cast<float>(N*N);
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2) N2    = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN  = k;

            k+=dN;
        }
    
    (*data) /= 1e4;
    N2/=1e4; NN2/=1e4; NN/=1e4;

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < data->size(); ++i) std::cout<<"["<<i<<"]"<<(*data)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_NEAR(img[N/2],    N2 , 1e-4);
    EXPECT_NEAR(img[N*N/2],  NN2, 1e-4);
    EXPECT_NEAR(img[N*N-1]  ,NN , 1e-4);

    img.Bscale((double)1e-4);
    img.Bzero( (double)0.0);
    img.BitPerPixel(SHORT_IMG,FLOAT_IMG);
    
    img.Write("build/testdata/test_short2float.fits", true);

    FITSmanager ff("build/testdata/test_short2float.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<float>* rdata = rimg->GetData<float>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_NEAR((*rdata)[N/2],    N2 ,1e-4);
    EXPECT_NEAR((*rdata)[N*N/2],  NN2,1e-4);
    EXPECT_NEAR((*rdata)[N*N-1]  ,NN ,1e-4);
    
    std::valarray<float> diff = (*data) - (*rdata);
    EXPECT_NEAR(diff.sum(), 0,1e-4) << "Image data does not match expected values";

    rimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Create_LONG2DOUBLE)
{
    verbose = verboseLevel::VERBOSE_NONE;

    size_t N = ((verbose&verboseLevel::VERBOSE_DEBUG)==verboseLevel::VERBOSE_DEBUG)?10:256;
    FITSimg<double> img(2,{N,N});
    EXPECT_EQ(img.Size(1), N);
    EXPECT_EQ(img.Size(2), N);

    std::valarray<double>* data = img.GetData<double>();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(img.Nelements(),data->size());
    
    double k = -1250.;
    double N2,NN2,NN;
    double dN = (2*1260.)/static_cast<double>(N*N);
    for(size_t j=0; j<img.Size(2); j++)
        for(size_t i=0; i<img.Size(1); i++)
        {
            (*data)[i+j*img.Size(1)] = k;
            if(i+j*img.Size(1) == N/2) N2    = k;
            if(i+j*img.Size(1) == N*N/2) NN2 = k;
            if(i+j*img.Size(1) == N*N-1) NN  = k;

            k+=dN;
        }
    
    (*data) /= 1e4;
    N2/=1e4; NN2/=1e4; NN/=1e4;

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < data->size(); ++i) std::cout<<"["<<i<<"]"<<(*data)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_NEAR(img[N/2],    N2 , 1e-4);
    EXPECT_NEAR(img[N*N/2],  NN2, 1e-4);
    EXPECT_NEAR(img[N*N-1]  ,NN , 1e-4);

    img.Bscale((double)1e-4);
    img.Bzero( (double)0.0);
    img.BitPerPixel(LONG_IMG,DOUBLE_IMG);
    
    img.Write("build/testdata/test_long2double.fits", true);

    FITSmanager ff("build/testdata/test_long2double.fits");
    EXPECT_TRUE(ff.isOpen());
    std::shared_ptr<FITScube> rimg = ff.GetPrimary();
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N);
    EXPECT_EQ(rimg->Size(2), N);

    std::valarray<double>* rdata = rimg->GetData<double>();
    ASSERT_NE(rdata, nullptr);
    EXPECT_EQ(rimg->Nelements(),rdata->size());

    if((verbose & verboseLevel::VERBOSE_DEBUG) == verboseLevel::VERBOSE_DEBUG)
    {
        for(size_t i=0; i < rdata->size(); ++i) std::cout<<"["<<i<<"]"<<(*rdata)[i]<<"   ";
        std::cout<<std::endl;
    }

    EXPECT_NEAR((*rdata)[N/2],    N2 ,1e-4);
    EXPECT_NEAR((*rdata)[N*N/2],  NN2,1e-4);
    EXPECT_NEAR((*rdata)[N*N-1]  ,NN ,1e-4);
    
    std::valarray<double> diff = (*data) - (*rdata);
    EXPECT_NEAR(diff.sum(), 0,1e-3) << "Image data does not match expected values";

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
    EXPECT_NEAR(img->FloatValueAtPixel(n), 0.141,1e-3);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}
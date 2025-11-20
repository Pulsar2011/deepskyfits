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

template<typename T>
static double eps_for_type()
{
    if constexpr (std::is_floating_point<T>::value) return 1e-6;
    return 0.0; // exact for integers (mean compared as double with no tolerance)
}

template <typename T>
class FITSimgStatsTest : public ::testing::Test
{
    protected:
        using value_type = T;
        static size_t N() { return 21; }
        
        std::string MakeFilename() const
        {
            std::ostringstream ss;
#ifdef Darwinx86_64
            ss << "build/testdata/stat_test_" << typeid(T).name() << ".fits";
#else
            ss << "testdata/stat_test_" << typeid(T).name() << ".fits";
#endif
            return ss.str();
        }
#ifdef Darwinx86_64
        void EnsureOutDir() const { std::filesystem::create_directories("build/testdata"); }
#else
        void EnsureOutDir() const { std::filesystem::create_directories("testdata"); }
#endif
};

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
    uint8_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_byte.fits", true);
    FITSmanager ff("build/testdata/test_byte.fits");
#else
    img.Write("testdata/test_byte.fits", true);
    FITSmanager ff("testdata/test_byte.fits");
#endif
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
    int8_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_sbyte.fits", true);
    FITSmanager ff("build/testdata/test_sbyte.fits");
#else
    img.Write("testdata/test_sbyte.fits", true);
    FITSmanager ff("testdata/test_sbyte.fits");
#endif
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
    int16_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_short.fits", true);
    FITSmanager ff("build/testdata/test_short.fits");
#else
    img.Write("testdata/test_short.fits", true);
    FITSmanager ff("testdata/test_short.fits");
#endif
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
    uint16_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_ushort.fits", true);
    FITSmanager ff("build/testdata/test_ushort.fits");
#else
    img.Write("testdata/test_ushort.fits", true);
    FITSmanager ff("testdata/test_ushort.fits");
#endif
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
    int32_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_long.fits", true);
    FITSmanager ff("build/testdata/test_long.fits");
#else
    img.Write("testdata/test_long.fits", true);
    FITSmanager ff("testdata/test_long.fits");
#endif
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
    uint32_t N2=0,NN2=0,NN=0;
    uint32_t dN = static_cast<uint16_t>(static_cast<double>(std::numeric_limits<uint32_t>::max()) / static_cast<double>(N*N));
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_ulong.fits", true);
    FITSmanager ff("build/testdata/test_ulong.fits");
#else
    img.Write("testdata/test_ulong.fits", true);
    FITSmanager ff("testdata/test_ulong.fits");
#endif
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
    int64_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_longlong.fits", true);
    FITSmanager ff("build/testdata/test_longlong.fits");
#else
    img.Write("testdata/test_longlong.fits", true);
    FITSmanager ff("testdata/test_longlong.fits");
#endif
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
    uint64_t N2=0,NN2=0,NN=0;
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_ulonglong.fits", true);
    FITSmanager ff("build/testdata/test_ulonglong.fits");
#else
    img.Write("testdata/test_ulonglong.fits", true);
    FITSmanager ff("testdata/test_ulonglong.fits");
#endif
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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_float.fits", true);
    FITSmanager ff("build/testdata/test_float.fits");
#else
    img.Write("testdata/test_float.fits", true);
    FITSmanager ff("testdata/test_float.fits");
#endif

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

#ifdef Darwinx86_64
    img.Write("build/testdata/test_double.fits", true);
    FITSmanager ff("build/testdata/test_double.fits");
#else
    img.Write("testdata/test_double.fits", true);
    FITSmanager ff("testdata/test_double.fits");
#endif
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
    float N2=0,NN2=0,NN=0;
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
    
#ifdef Darwinx86_64
    img.Write("build/testdata/test_short2float.fits", true);
    FITSmanager ff("build/testdata/test_short2float.fits");
#else
    img.Write("testdata/test_short2float.fits", true);
    FITSmanager ff("testdata/test_short2float.fits");
#endif

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
    double N2=0,NN2=0,NN=0;
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
    
#ifdef Darwinx86_64
    img.Write("build/testdata/test_long2double.fits", true);
    FITSmanager ff("build/testdata/test_long2double.fits");
#else
    img.Write("testdata/test_long2double.fits", true);
    FITSmanager ff("testdata/test_long2double.fits");
#endif
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

using StatTypes = ::testing::Types<uint8_t,int8_t,int16_t,uint16_t,int32_t,uint32_t,int64_t,uint64_t,float,double>;
TYPED_TEST_SUITE(FITSimgStatsTest, StatTypes);

TYPED_TEST(FITSimgStatsTest, arrayStatistics)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;

    this->EnsureOutDir();

    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});
    
    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    // fill predictable pattern: value = (i % 256) + (j)
    size_t k = 0;
    for (size_t j = 0; j < img.Size(2); ++j)
        for (size_t i = 0; i < img.Size(1); ++i)
        {
            if(k >= 100)
                img.MaskPixels({i + j * img.Size(1)});
            (*data)[i + j * img.Size(1)] = static_cast<T>(k);
            k++ ;
        }


    // compute expected stats
    std::valarray<bool> mask = img.GetMask();
    const double nelems = static_cast<double>(((std::valarray<bool>)mask[!mask]).size());
    
    EXPECT_NEAR(img.GetMinimum()      , (double)(0), 1e-6);
    EXPECT_NEAR(img.GetMaximum()      , (nelems-1.), 1e-12);
    EXPECT_NEAR(img.GetSum()          , (nelems * (nelems - 1.)) / 2.0, 1e-12);
    EXPECT_NEAR(img.GetMean()         , (nelems - 1.) / 2.0, 1e-12);
    EXPECT_NEAR(img.GetMedian()       , (nelems - 1.) / 2.0, 1e-12);
    EXPECT_NEAR(img.GetVariance()     , nelems * ( nelems +1 ) / 12.0, 1e-12);
    EXPECT_NEAR(img.GetStdDev()       , std::sqrt(nelems * ( nelems +1 ) / 12.0), 1e-12);
    EXPECT_NEAR(img.GetRMS()          , std::sqrt(((nelems - 1.) * (2 * nelems - 1)) / 6.0), 1e-12);
    EXPECT_NEAR(img.GetRMSE()         , std::sqrt(( nelems * nelems - 1. ) / 12.0), 1e-12);
    EXPECT_NEAR(img.Get5thpercentil() , (0.05 * (nelems - 1.)), 1e-12);
    EXPECT_NEAR(img.Get95thpercentil(), (0.95 * (nelems - 1.)), 1e-12);
    EXPECT_NEAR(img.Get25thpercentil(), (0.25 * (nelems - 1.)), 1e-12);
    EXPECT_NEAR(img.Get75thpercentil(), (0.75 * (nelems - 1.)), 1e-12);
    EXPECT_NEAR(img.GetQuadraticMean(), std::sqrt(((nelems - 1.) * (2 * nelems - 1)) / 6.0), 1e-5);
    EXPECT_NEAR(img.GetKurtosis()     , -6. * (nelems * nelems + 1.) / (5. * (nelems * nelems - 1.)), 5e-4);
    EXPECT_NEAR(img.GetSkewness()     , 0.0, 1e-5);
}

TEST(FITSimg, Read_SHORT)
{
    verbose = verboseLevel::VERBOSE_NONE;

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

    size_t n = img->PixelIndex({150,150});
    EXPECT_EQ(n, 150 + 150*300);
    EXPECT_EQ(img->Int16ValueAtPixel({150,150}), 8801);
    EXPECT_EQ(img->Int16ValueAtPixel({0,0}), 3768);
    EXPECT_EQ(img->Int16ValueAtPixel({299,299}), 3965);
    EXPECT_EQ(img->Int16ValueAtPixel({59,290}), 6168);

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Test_Crop)
{
    verbose = verboseLevel::VERBOSE_NONE;

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

    
    std::shared_ptr<FITScube> cimg = img->Window(50,25,60,75);
    EXPECT_NE(cimg, nullptr);
    EXPECT_EQ(cimg->Size(1), 60);
    EXPECT_EQ(cimg->Size(2), 75);

    for(size_t j=0; j<cimg->Size(2); j++)
        for(size_t i=0; i<cimg->Size(1); i++)
        {
            EXPECT_EQ(cimg->Int16ValueAtPixel({i,j}), img->Int16ValueAtPixel({i+50,j+25}));
            EXPECT_EQ(cimg->getNumberOfWCS(), img->getNumberOfWCS());
            EXPECT_EQ(cimg->getNumberOfWCS(), 1);

            worldCoords wcsrc = img->WorldCoordinates(img->PixelIndex({i+50,j+25}), 0);
            worldCoords wcscr = cimg->WorldCoordinates(cimg->PixelIndex({i,j}), 0);


            EXPECT_NEAR(wcsrc[0], wcscr[0], 1e-6);
            EXPECT_NEAR(wcsrc[1], wcscr[1], 1e-6);
        }

    std::shared_ptr<FITScube> cpy_img = ff.GetPrimary();
    
    cpy_img->Resize(std::pair<size_t,size_t>(50,60), std::pair<size_t,size_t>(25,75));
    EXPECT_EQ(cpy_img->Size(1), 60);
    EXPECT_EQ(cpy_img->Size(2), 75);

    for(size_t j=0; j<cpy_img->Size(2); j++)
        for(size_t i=0; i<cpy_img->Size(1); i++)
        {
            EXPECT_EQ(cpy_img->Int16ValueAtPixel({i,j}), img->Int16ValueAtPixel({i+50,j+25}));
            EXPECT_EQ(cpy_img->getNumberOfWCS(), img->getNumberOfWCS());
            EXPECT_EQ(cpy_img->getNumberOfWCS(), 1);

            worldCoords wcsrc = img->WorldCoordinates(img->PixelIndex({i+50,j+25}), 0);
            worldCoords wcscr = cpy_img->WorldCoordinates(cpy_img->PixelIndex({i,j}), 0);


            EXPECT_NEAR(wcsrc[0], wcscr[0], 1e-6);
            EXPECT_NEAR(wcsrc[1], wcscr[1], 1e-6);
        }

    img.reset();
    cpy_img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Test_RebinningWCS)
{
    verbose = verboseLevel::VERBOSE_NONE;

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
    //EXPECT_EQ(img->Size(1), 300);
    //EXPECT_EQ(img->Size(2), 300);

    std::shared_ptr<FITScube> cimg = img->Rebin({4,4}, false);
    EXPECT_NE(cimg, nullptr);
    EXPECT_EQ(cimg->Size(1), img->Size(1)/4);
    EXPECT_EQ(cimg->Size(2), img->Size(2)/4);

    for(size_t j=0; j<cimg->Size(2); j++)
        for(size_t i=0; i<cimg->Size(1); i++)
        {
            worldCoords wcsrc =  img->WorldCoordinates( img->PixelIndex({i*4+2,j*4+2}), 0);
            worldCoords wcscr = cimg->WorldCoordinates(cimg->PixelIndex({i    ,    j}), 0);
            EXPECT_NEAR(wcsrc[0], wcscr[0], 1e-1);
            EXPECT_NEAR(wcsrc[1], wcscr[1], 1e-1);
        }

    

    img.reset();
    cimg.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, Naming)
{
    verbose = verboseLevel::VERBOSE_NONE;

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

    EXPECT_EQ(img->GetName(), "NO NAME");
    img->SetName("MyImage");

    EXPECT_EQ(img->GetName(), "MyImage");
    

    img.reset();
    ff.Close();
    EXPECT_FALSE(ff.isOpen());
}

TYPED_TEST(FITSimgStatsTest, arrayRebinning)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;

    this->EnsureOutDir();

    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});
    
    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    // fill predictable pattern: value = (i % 256) + (j)
    for (size_t j = 0; j < img.Size(2); ++j)
        for (size_t i = 0; i < img.Size(1); ++i)
        {
            (*data)[i + j * img.Size(1)] = static_cast<T>(1);
        }

    std::shared_ptr<FITScube> rimg = img.Rebin({3,3},false);
    EXPECT_NE(rimg, nullptr);
    EXPECT_EQ(rimg->Size(1), N/3);
    EXPECT_EQ(rimg->Size(2), N/3);
    auto rdata = rimg->GetData<T>();
    for(size_t j=0; j<rdata->size(); j++)
        EXPECT_NEAR((*rdata)[j], (T)9, 1e-6);

    std::shared_ptr<FITScube> mimg = img.Rebin({3,3},true);
    EXPECT_NE(mimg, nullptr);
    EXPECT_EQ(mimg->Size(1), N/3);
    EXPECT_EQ(mimg->Size(2), N/3);
    auto mdata = mimg->GetData<T>();
    for(size_t j=0; j<mdata->size(); j++)
        EXPECT_NEAR((*mdata)[j], (T)1, 1e-6);
}

TYPED_TEST(FITSimgStatsTest, LayerParent)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;

    this->EnsureOutDir();

    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});
    
    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    // fill predictable pattern: value = (i % 256) + (j)
    size_t k = 0;
    for (size_t j = 0; j < img.Size(2); ++j)
        for (size_t i = 0; i < img.Size(1); ++i)
        {
            (*data)[i + j * img.Size(1)] = static_cast<T>(1);
            k++ ;
        }

    img.HDU().ValueForKey("CRPIX1", (double) 20.0);
    img.HDU().ValueForKey("CRPIX2", (double) 30.0);
    img.HDU().ValueForKey("CRVAL1", (double) 0.0);
    img.HDU().ValueForKey("CRVAL2", (double) 0.0);
    img.HDU().ValueForKey("CDELT1", (double) 0.5);
    img.HDU().ValueForKey("CDELT2", (double) 0.5);
    img.HDU().ValueForKey("CRPIX1A", (double) 0.0);
    img.HDU().ValueForKey("CRPIX2A", (double) 0.0);
    img.HDU().ValueForKey("CRVAL1A", (double) 10.0);
    img.HDU().ValueForKey("CRVAL2A", (double) 10.0);
    img.HDU().ValueForKey("CDELT1A", (double) 1);
    img.HDU().ValueForKey("CDELT2A", (double) 1);

    EXPECT_NO_THROW(img.reLoadWCS());
    EXPECT_EQ(img.getNumberOfWCS(), 2);
    
    
    for(size_t k = 2; k < 5; k++)
    {
        std::shared_ptr<FITScube> layer(new FITSimg<T>(2, {N,N}));
        auto ldata = layer->GetData<T>();
        ASSERT_NE(ldata, nullptr);
        
        for(size_t j=0; j<layer->Size(2); j++)
            for(size_t i=0; i<layer->Size(1); i++)
                (*ldata)[i + j * layer->Size(1)] = static_cast<T>(k);

        img.AddLayer((*layer.get()));
        EXPECT_EQ(img.Size(), (k)*N*N);
        EXPECT_EQ(img.Size(3), (k));

        EXPECT_EQ(img.getNumberOfWCS(), 2);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(0), 3);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(1), 3);
        EXPECT_NEAR(img.getWCS().CRPIX(0,3),0,1e-7);
        EXPECT_NEAR(img.getWCS().CRVAL(0,3),1,1e-7);
        EXPECT_NEAR(img.getWCS().CDELT(0,3),1,1e-7);
        
        for(size_t iz=0; iz<img.Size(3); iz++)
            for(size_t iy=0; iy<img.Size(2); iy++)
                for(size_t ix=0; ix<img.Size(1); ix++)
                        EXPECT_EQ(img[img.PixelIndex({ix,iy,iz})], static_cast<T>(iz+1));
    }

    for(size_t k=0; k < img.Size(3); k++)
    {
        std::shared_ptr<FITScube> layer = img.Layer(k);
        EXPECT_NE(layer, nullptr);
        EXPECT_EQ(layer->Size(1), N);
        EXPECT_EQ(layer->Size(2), N);

        EXPECT_EQ(img.getNumberOfWCS(), 2);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(0), 3);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(1), 3);

        auto ldata = layer->GetData<T>();
        EXPECT_NEAR(layer->getWCS().CRPIX(0,3),-1*static_cast<double>(k),1e-7);
        EXPECT_NEAR(layer->getWCS().CRVAL(0,3),1,1e-7);
        EXPECT_NEAR(layer->getWCS().CDELT(0,3),1,1e-7);

        EXPECT_ANY_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0}),0));
        EXPECT_ANY_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0}),1));

        EXPECT_NO_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),0));
        EXPECT_NO_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),1));

        EXPECT_NEAR(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),0)[2], 1.0 + static_cast<double>(k), 1e-7);
        EXPECT_NEAR(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),1)[2], 1.0 + static_cast<double>(k), 1e-7);

        for(size_t j=0; j<layer->Size(2); j++)
            for(size_t i=0; i<layer->Size(1); i++)
                EXPECT_EQ((*ldata)[layer->PixelIndex({i,j})], static_cast<T>(k+1));
    }

    EXPECT_ANY_THROW(img.Layer(img.Size(3)));

    FITSimg<T> img2(2, {N+5,N+5});
    auto idata = img2.template GetData<T>();
    ASSERT_NE(idata, nullptr);

    for(size_t k = 0; k < idata->size(); k++)
        (*idata)[k] = static_cast<T>(5);

    EXPECT_ANY_THROW(img.AddLayer(img2));

    FITSimg<T> img3(2, {N,N+5});
    EXPECT_ANY_THROW(img.AddLayer(img3));

    FITSimg<T> img4(2, {N+5,N});
    EXPECT_ANY_THROW(img.AddLayer(img4));

    FITSimg<T> cube(3, {N+5,N+5,2});
    auto cdata = cube.template GetData<T>();
    ASSERT_NE(cdata, nullptr);

    for(size_t k = 0; k < cdata->size(); k++)
        (*cdata)[k] = static_cast<T>(10);

    img2.AddLayer(cube);
    EXPECT_EQ(img2.Size(), (3)*(N+5)*(N+5));
    EXPECT_EQ(img2.Size(3), (3));

    for(k = 0; k < img2.Size(3); k++)
    {
        std::shared_ptr<FITScube> layer = img2.Layer(k);
        EXPECT_NE(layer, nullptr);
        EXPECT_EQ(layer->Size(1), N+5);
        EXPECT_EQ(layer->Size(2), N+5);

        auto ldata = layer->GetData<T>();

        for(size_t j=0; j<ldata->size(); j++)
            if(k < 1)
                EXPECT_EQ((*ldata)[j], static_cast<T>(5));
            else
                EXPECT_EQ((*ldata)[j], static_cast<T>(10));
    }
}

TYPED_TEST(FITSimgStatsTest, LayerChild)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;

    this->EnsureOutDir();

    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});
    
    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    // fill predictable pattern: value = (i % 256) + (j)
    size_t k = 0;
    for (size_t j = 0; j < img.Size(2); ++j)
        for (size_t i = 0; i < img.Size(1); ++i)
        {
            (*data)[i + j * img.Size(1)] = static_cast<T>(1);
            k++ ;
        }

    img.HDU().ValueForKey("CRPIX1", (double) 20.0);
    img.HDU().ValueForKey("CRPIX2", (double) 30.0);
    img.HDU().ValueForKey("CRVAL1", (double) 0.0);
    img.HDU().ValueForKey("CRVAL2", (double) 0.0);
    img.HDU().ValueForKey("CDELT1", (double) 0.5);
    img.HDU().ValueForKey("CDELT2", (double) 0.5);
    img.HDU().ValueForKey("CRPIX1A", (double) 0.0);
    img.HDU().ValueForKey("CRPIX2A", (double) 0.0);
    img.HDU().ValueForKey("CRVAL1A", (double) 10.0);
    img.HDU().ValueForKey("CRVAL2A", (double) 10.0);
    img.HDU().ValueForKey("CDELT1A", (double) 1);
    img.HDU().ValueForKey("CDELT2A", (double) 1);

    EXPECT_NO_THROW(img.reLoadWCS());
    EXPECT_EQ(img.getNumberOfWCS(), 2);
    
    
    for(size_t k = 2; k < 5; k++)
    {
        FITSimg<T> layer(2, {N,N});
        auto ldata = layer.template GetData<T>();
        ASSERT_NE(ldata, nullptr);
        
        for(size_t j=0; j<layer.Size(2); j++)
            for(size_t i=0; i<layer.Size(1); i++)
                (*ldata)[i + j * layer.Size(1)] = static_cast<T>(k);

        img.AddLayer(layer);
        EXPECT_EQ(img.Size(), (k)*N*N);
        EXPECT_EQ(img.Size(3), (k));

        EXPECT_EQ(img.getNumberOfWCS(), 2);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(0), 3);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(1), 3);
        EXPECT_NEAR(img.getWCS().CRPIX(0,3),0,1e-7);
        EXPECT_NEAR(img.getWCS().CRVAL(0,3),1,1e-7);
        EXPECT_NEAR(img.getWCS().CDELT(0,3),1,1e-7);
        
        for(size_t iz=0; iz<img.Size(3); iz++)
            for(size_t iy=0; iy<img.Size(2); iy++)
                for(size_t ix=0; ix<img.Size(1); ix++)
                        EXPECT_EQ(img[img.PixelIndex({ix,iy,iz})], static_cast<T>(iz+1));
    }

    for(size_t k=0; k < img.Size(3); k++)
    {
        std::shared_ptr<FITScube> layer = img.Layer(k);
        EXPECT_NE(layer, nullptr);
        EXPECT_EQ(layer->Size(1), N);
        EXPECT_EQ(layer->Size(2), N);

        EXPECT_EQ(img.getNumberOfWCS(), 2);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(0), 3);
        EXPECT_EQ(img.getWCS().getNumberOfAxis(1), 3);

        auto ldata = layer->GetData<T>();
        EXPECT_NEAR(layer->getWCS().CRPIX(0,3),-1*static_cast<double>(k),1e-7);
        EXPECT_NEAR(layer->getWCS().CRVAL(0,3),1,1e-7);
        EXPECT_NEAR(layer->getWCS().CDELT(0,3),1,1e-7);

        EXPECT_ANY_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0}),0));
        EXPECT_ANY_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0}),1));

        EXPECT_NO_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),0));
        EXPECT_NO_THROW(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),1));

        EXPECT_NEAR(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),0)[2], 1.0 + static_cast<double>(k), 1e-7);
        EXPECT_NEAR(layer->WorldCoordinates(std::vector<size_t>({0,0,0}),1)[2], 1.0 + static_cast<double>(k), 1e-7);

        for(size_t j=0; j<layer->Size(2); j++)
            for(size_t i=0; i<layer->Size(1); i++)
                EXPECT_EQ((*ldata)[layer->PixelIndex({i,j})], static_cast<T>(k+1));
    }

    EXPECT_ANY_THROW(img.Layer(img.Size(3)));

    FITSimg<T> img2(2, {N+5,N+5});
    auto idata = img2.template GetData<T>();
    ASSERT_NE(idata, nullptr);

    for(size_t k = 0; k < idata->size(); k++)
        (*idata)[k] = static_cast<T>(5);

    EXPECT_ANY_THROW(img.AddLayer(img2));

    FITSimg<T> img3(2, {N,N+5});
    EXPECT_ANY_THROW(img.AddLayer(img3));

    FITSimg<T> img4(2, {N+5,N});
    EXPECT_ANY_THROW(img.AddLayer(img4));

    FITSimg<T> cube(3, {N+5,N+5,2});
    auto cdata = cube.template GetData<T>();
    ASSERT_NE(cdata, nullptr);

    for(size_t k = 0; k < cdata->size(); k++)
        (*cdata)[k] = static_cast<T>(10);

    img2.AddLayer(cube);
    EXPECT_EQ(img2.Size(), (3)*(N+5)*(N+5));
    EXPECT_EQ(img2.Size(3), (3));

    for(k = 0; k < img2.Size(3); k++)
    {
        std::shared_ptr<FITScube> layer = img2.Layer(k);
        EXPECT_NE(layer, nullptr);
        EXPECT_EQ(layer->Size(1), N+5);
        EXPECT_EQ(layer->Size(2), N+5);

        auto ldata = layer->GetData<T>();

        for(size_t j=0; j<ldata->size(); j++)
            if(k < 1)
                EXPECT_EQ((*ldata)[j], static_cast<T>(5));
            else
                EXPECT_EQ((*ldata)[j], static_cast<T>(10));
    }
}

template<typename A, typename B>
struct TwoTypes { using T = A; using D = B; };

template<typename Pair>
class FITSimgMixTest : public ::testing::Test
{
    protected:
        using value_type = typename Pair::T;
        using other_type = typename Pair::D;
        static size_t N() { return 10; }
#ifdef Darwinx86_64
        void EnsureOutDir() const { std::filesystem::create_directories("build/testdata"); }
#else
        void EnsureOutDir() const { std::filesystem::create_directories("testdata");}
#endif
};

// instantiate the pair list: here all T are paired with double as D (adjust if you want other D per T)
using MixTypes = ::testing::Types<
    TwoTypes<uint8_t,double>,
    TwoTypes<uint8_t,float>,
    TwoTypes<uint8_t,uint64_t>,
    TwoTypes<uint8_t,int64_t>,
    TwoTypes<uint8_t,uint32_t>,
    TwoTypes<uint8_t,int32_t>,
    TwoTypes<uint8_t,uint16_t>,
    TwoTypes<uint8_t,int16_t>,
    TwoTypes<uint8_t,uint8_t>,
    TwoTypes<uint8_t,int8_t>,
    TwoTypes<int8_t,double>,
    TwoTypes<int8_t,float>,
    TwoTypes<int8_t,uint64_t>,
    TwoTypes<int8_t,int64_t>,
    TwoTypes<int8_t,uint32_t>,
    TwoTypes<int8_t,int32_t>,
    TwoTypes<int8_t,uint16_t>,
    TwoTypes<int8_t,int16_t>,
    TwoTypes<int8_t,uint8_t>,
    TwoTypes<int8_t,int8_t>,
    TwoTypes<uint16_t,double>,
    TwoTypes<uint16_t,float>,
    TwoTypes<uint16_t,uint64_t>,
    TwoTypes<uint16_t,int64_t>,
    TwoTypes<uint16_t,uint32_t>,
    TwoTypes<uint16_t,int32_t>,
    TwoTypes<uint16_t,uint16_t>,
    TwoTypes<uint16_t,int16_t>,
    TwoTypes<uint16_t,uint8_t>,
    TwoTypes<uint16_t,int8_t>,
    TwoTypes<int16_t,double>,
    TwoTypes<int16_t,float>,
    TwoTypes<int16_t,uint64_t>,
    TwoTypes<int16_t,int64_t>,
    TwoTypes<int16_t,uint32_t>,
    TwoTypes<int16_t,int32_t>,
    TwoTypes<int16_t,uint16_t>,
    TwoTypes<int16_t,int16_t>,
    TwoTypes<int16_t,uint8_t>,
    TwoTypes<int16_t,int8_t>,
    TwoTypes<uint32_t,double>,
    TwoTypes<uint32_t,float>,
    TwoTypes<uint32_t,uint64_t>,
    TwoTypes<uint32_t,int64_t>,
    TwoTypes<uint32_t,uint32_t>,
    TwoTypes<uint32_t,int32_t>,
    TwoTypes<uint32_t,uint16_t>,
    TwoTypes<uint32_t,int16_t>,
    TwoTypes<uint32_t,uint8_t>,
    TwoTypes<uint32_t,int8_t>,
    TwoTypes<int32_t,double>,
    TwoTypes<int32_t,float>,
    TwoTypes<int32_t,uint64_t>,
    TwoTypes<int32_t,int64_t>,
    TwoTypes<int32_t,uint32_t>,
    TwoTypes<int32_t,int32_t>,
    TwoTypes<int32_t,uint16_t>,
    TwoTypes<int32_t,int16_t>,
    TwoTypes<int32_t,uint8_t>,
    TwoTypes<int32_t,int8_t>,
    TwoTypes<uint64_t,double>,
    TwoTypes<uint64_t,float>,
    TwoTypes<uint64_t,uint64_t>,
    TwoTypes<uint64_t,int64_t>,
    TwoTypes<uint64_t,uint32_t>,
    TwoTypes<uint64_t,int32_t>,
    TwoTypes<uint64_t,uint16_t>,
    TwoTypes<uint64_t,int16_t>,
    TwoTypes<uint64_t,uint8_t>,
    TwoTypes<uint64_t,int8_t>,
    TwoTypes<int64_t,double>,
    TwoTypes<int64_t,float>,
    TwoTypes<int64_t,uint64_t>,
    TwoTypes<int64_t,int64_t>,
    TwoTypes<int64_t,uint32_t>,
    TwoTypes<int64_t,int32_t>,
    TwoTypes<int64_t,uint16_t>,
    TwoTypes<int64_t,int16_t>,
    TwoTypes<int64_t,uint8_t>,
    TwoTypes<int64_t,int8_t>,
    TwoTypes<float,double>,
    TwoTypes<float,float>,
    TwoTypes<float,uint64_t>,
    TwoTypes<float,int64_t>,
    TwoTypes<float,uint32_t>,
    TwoTypes<float,int32_t>,
    TwoTypes<float,uint16_t>,
    TwoTypes<float,int16_t>,
    TwoTypes<float,uint8_t>,
    TwoTypes<float,int8_t>,
    TwoTypes<double,double>,
    TwoTypes<double,float>,
    TwoTypes<double,uint64_t>,
    TwoTypes<double,int64_t>,
    TwoTypes<double,uint32_t>,
    TwoTypes<double,int32_t>,
    TwoTypes<double,uint16_t>,
    TwoTypes<double,int16_t>,
    TwoTypes<double,uint8_t>,
    TwoTypes<double,int8_t>
>;
TYPED_TEST_SUITE(FITSimgMixTest, MixTypes);

TYPED_TEST(FITSimgMixTest, opperators_scalar)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    using D = typename TestFixture::other_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    for(size_t k = 0; k < data->size(); k++)
        (*data)[k] = static_cast<T>(10);

    D val = 10;

    img += val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(20));

    img -= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(10));

    img *= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(100));

    img /= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(10));
}

TYPED_TEST(FITSimgMixTest, opperators_array)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    using D = typename TestFixture::other_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    for(size_t k = 0; k < data->size(); k++)
        (*data)[k] = static_cast<T>(10);

    typename std::valarray<D> val(data->size());
    val*=(D) 0;
    val+=(D) 10;

    img += val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(20));

    img -= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(10));

    img *= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(100));

    img /= val;
    for(size_t k = 0; k < data->size(); k++)
        EXPECT_EQ((*data)[k], static_cast<T>(10));
}

TYPED_TEST(FITSimgMixTest, opperators_img)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    using D = typename TestFixture::other_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    for(size_t k = 0; k < data->size(); k++)
    {
        (*data)[k] = static_cast<T>(10);
        if(!(k%5)) img.MaskPixels({k});
    }

    pxMask mask = pxMask(img.GetMask());
    

    FITSimg<D> dmg(2, {N, N});
    auto ddata = dmg.template GetData<D>();
    ASSERT_NE(ddata, nullptr);

    for(size_t k = 0; k < ddata->size(); k++)
    {
        (*ddata)[k] = static_cast<D>(10.0);
        if(!(k%2)) dmg.MaskPixels({k});
    }

    img += dmg;
    for(size_t k = 0; k < data->size(); k++)
    {
        EXPECT_EQ((*data)[k], img.Masked(k)?10:static_cast<T>(20));
        if(!(k%5))
            EXPECT_TRUE(img.Masked(k));
        else if(!(k%2))
            EXPECT_TRUE(img.Masked(k));
        else
        EXPECT_FALSE(img.Masked(k));
    }
    img.UnmaskPixels(img.GetMask()); // restore original mask
    img.MaskPixels(mask);

    img -= dmg;
    for(size_t k = 0; k < data->size(); k++)
    {
        EXPECT_EQ((*data)[k], img.Masked(k)?10:static_cast<T>(10));
        if(!(k%5))
            EXPECT_TRUE(img.Masked(k));
        else if(!(k%2))
            EXPECT_TRUE(img.Masked(k));
        else
        EXPECT_FALSE(img.Masked(k));
    }
    img.UnmaskPixels(img.GetMask()); // restore original mask
    img.MaskPixels(mask);

    img *= dmg;
    for(size_t k = 0; k < data->size(); k++)
    {
        EXPECT_EQ((*data)[k], img.Masked(k)?10:static_cast<T>(100));
        if(!(k%5))
            EXPECT_TRUE(img.Masked(k));
        else if(!(k%2))
            EXPECT_TRUE(img.Masked(k));
        else
        EXPECT_FALSE(img.Masked(k));
    }
    img.UnmaskPixels(img.GetMask()); // restore original mask
    img.MaskPixels(mask);

    img /= dmg;
    for(size_t k = 0; k < data->size(); k++)
    {
        EXPECT_EQ((*data)[k], img.Masked(k)?10:static_cast<T>(10));

        if(!(k%5))
            EXPECT_TRUE(img.Masked(k));
        else if(!(k%2))
            EXPECT_TRUE(img.Masked(k));
        else
        EXPECT_FALSE(img.Masked(k));
    }
    img.UnmaskPixels(img.GetMask()); // restore original mask
    img.MaskPixels(mask);
}


TYPED_TEST(FITSimgStatsTest, logical_scalar)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    T value = 10;
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    for(size_t k = 0; k < data->size(); k++)
    {
        (*data)[k] = static_cast<T>(value);
        if(!(k%5)) img.MaskPixels({k});
    }

    pxMask mask = pxMask(img.GetMask());
    
    std::vector<T> val({0,10,20});

    bool state = false;
    for(typename std::vector<T>::const_iterator it = val.begin(); it != val.end(); it++)
    {
        state = (value == *it);
        
        std::valarray<bool> res1 = (img == *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value != *it);
        res1 = (img != *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value >= *it);
        res1 = (img >= *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value > *it);
        res1 = (img > *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value <= *it);
        res1 = (img <= *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value < *it);
        res1 = (img < *it);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it == value);
        res1 = (*it == img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it != value);
        res1 = (*it != img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it >= value);
        res1 = (*it >= img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it > value);
        res1 = (*it > img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it <= value);
        res1 = (*it <= img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (*it < value);
        res1 = (*it < img);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }
    }
}

TYPED_TEST(FITSimgStatsTest, logical_img)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    T value = 10;
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);

    for(size_t k = 0; k < data->size(); k++)
    {
        (*data)[k] = static_cast<T>(value);
        if(!(k%5)) img.MaskPixels({k});
    }

    pxMask mask = pxMask(img.GetMask());
    
    std::vector<T> val({0,10,20});

    bool state = false;
    for(typename std::vector<T>::const_iterator it = val.begin(); it != val.end(); it++)
    {
        FITSimg<T> cmp(2, {N, N});
        auto cdata = cmp.template GetData<T>();
        ASSERT_NE(cdata, nullptr);
        for(size_t k = 0; k < cdata->size(); k++)
        {
            (*cdata)[k] = static_cast<T>(*it);
            if(!(k%3)) cmp.MaskPixels({k});
        }
        
        state = (value == *it);
        std::valarray<bool> res1 = (img == cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value != *it);
        res1 = (img != cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value >= *it);
        res1 = (img >= cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value > *it);
        res1 = (img > cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value <= *it);
        res1 = (img <= cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }

        state = (value < *it);
        res1 = (img < cmp);
        EXPECT_TRUE(res1.size() == img.Nelements());
        for(size_t k = 0; k < res1.size(); k++)
        {
            if(!(k%5) || !(k%3))
                EXPECT_FALSE(res1[k]);
            else
                EXPECT_EQ(res1[k],state);
        }
    }
}

TYPED_TEST(FITSimgStatsTest, modifier)
{
    verbose = verboseLevel::VERBOSE_NONE;
    using T = typename TestFixture::value_type;
    
    this->EnsureOutDir();
    const size_t N = TestFixture::N();
    FITSimg<T> img(2, {N, N});

    auto data = img.template GetData<T>();
    ASSERT_NE(data, nullptr);
    for(size_t k = 0; k < data->size(); k++)
        (*data)[k] = static_cast<T>(0);

    std::shared_ptr<FITScube> imgptr(&img, [](FITScube*){/* no delete */});
    EXPECT_NE(imgptr, nullptr);
    EXPECT_EQ(imgptr->Size(1), N);
    EXPECT_EQ(imgptr->Size(2), N);
    EXPECT_EQ(imgptr->Nelements(), N*N);

    uint8_t u8 = 25;
    int8_t i8 = 5;
    int16_t i16 = 10;
    uint16_t u16 = 15;
    int32_t i32 = 20;
    uint32_t u32 = 25;
    int64_t i64 = 30;
    uint64_t u64 = 35;
    float f = 40.5;
    double d = 45.5;

    imgptr->SetPixelValue(u8,{2,0});  EXPECT_EQ( imgptr->PixelIndex({2,0}), 2 + 0*N);
    imgptr->SetPixelValue(i8,{3,5});  EXPECT_EQ( imgptr->PixelIndex({3,5}), 3 + 5*N);
    imgptr->SetPixelValue(i16,{4,7}); EXPECT_EQ( imgptr->PixelIndex({4,7}), 4 + 7*N);
    imgptr->SetPixelValue(u16,{5,9}); EXPECT_EQ( imgptr->PixelIndex({5,9}), 5 + 9*N);
    imgptr->SetPixelValue(i32,{6,11});EXPECT_EQ( imgptr->PixelIndex({6,11}), 6 + 11*N);
    imgptr->SetPixelValue(u32,{7,13});EXPECT_EQ( imgptr->PixelIndex({7,13}), 7 + 13*N);
    imgptr->SetPixelValue(i64,{8,15});EXPECT_EQ( imgptr->PixelIndex({8,15}), 8 + 15*N);
    imgptr->SetPixelValue(u64,{9,17});EXPECT_EQ( imgptr->PixelIndex({9,17}), 9 + 17*N);
    imgptr->SetPixelValue(f,{0,19});  EXPECT_EQ( imgptr->PixelIndex({0,19}), 0 + 19*N);
    imgptr->SetPixelValue(d,{8,10});  EXPECT_EQ( imgptr->PixelIndex({8,10}), 8 + 10*N);

    EXPECT_EQ( (*data)[2 + 0*N], static_cast<T>(u8));
    EXPECT_EQ( (*data)[3 + 5*N], static_cast<T>(i8));
    EXPECT_EQ( (*data)[4 + 7*N], static_cast<T>(i16));
    EXPECT_EQ( (*data)[5 + 9*N], static_cast<T>(u16));
    EXPECT_EQ( (*data)[6 + 11*N],static_cast<T>(i32));
    EXPECT_EQ( (*data)[7 + 13*N],static_cast<T>(u32));
    EXPECT_EQ( (*data)[8 + 15*N],static_cast<T>(i64));
    EXPECT_EQ( (*data)[9 + 17*N],static_cast<T>(u64));
    
    if constexpr (std::is_integral_v<T>)
    {
        EXPECT_EQ( (*data)[0 + 19*N],static_cast<T>(f+0.5));
        EXPECT_EQ( (*data)[8 + 10*N],static_cast<T>(d+0.5));
    }
    else
    {
        EXPECT_NEAR( (*data)[0 + 19*N],static_cast<T>(f), 1e-5);
        EXPECT_NEAR( (*data)[8 + 10*N],static_cast<T>(d), 1e-5);
    }

    EXPECT_EQ(imgptr->UByteValueAtPixel(2+0*N), static_cast<uint8_t>(u8));
    EXPECT_EQ(imgptr->UByteValueAtPixel({2,0}), imgptr->UByteValueAtPixel(2+0*N));
    EXPECT_EQ(imgptr->UInt8ValueAtPixel({2,0}), imgptr->UByteValueAtPixel(2+0*N));

    EXPECT_EQ(imgptr->ByteValueAtPixel(3+5*N), static_cast<int8_t>(i8));
    EXPECT_EQ(imgptr->ByteValueAtPixel({3,5}), imgptr->ByteValueAtPixel(3+5*N));
    EXPECT_EQ(imgptr->Int8ValueAtPixel({3,5}), imgptr->ByteValueAtPixel(3+5*N));

    EXPECT_EQ(imgptr->UShortValueAtPixel(5+9*N), static_cast<uint16_t>(u16));
    EXPECT_EQ(imgptr->UShortValueAtPixel({5,9}), imgptr->UShortValueAtPixel(5+9*N));
    EXPECT_EQ(imgptr->UInt16ValueAtPixel({5,9}), imgptr->UShortValueAtPixel(5+9*N));

    EXPECT_EQ(imgptr->ShortValueAtPixel(4+7*N), static_cast<int16_t>(i16));
    EXPECT_EQ(imgptr->ShortValueAtPixel({4,7}), imgptr->ShortValueAtPixel(4+7*N));
    EXPECT_EQ(imgptr->Int16ValueAtPixel({4,7}), imgptr->ShortValueAtPixel(4+7*N));

    EXPECT_EQ(imgptr->ULongValueAtPixel(7+13*N), static_cast<uint32_t>(u32));
    EXPECT_EQ(imgptr->ULongValueAtPixel({7,13}), imgptr->ULongValueAtPixel(7+13*N));
    EXPECT_EQ(imgptr->UInt32ValueAtPixel({7,13}), imgptr->ULongValueAtPixel(7+13*N));

    EXPECT_EQ(imgptr->LongValueAtPixel(6+11*N), static_cast<int32_t>(i32));
    EXPECT_EQ(imgptr->LongValueAtPixel({6,11}), imgptr->LongValueAtPixel(6+11*N));
    EXPECT_EQ(imgptr->Int32ValueAtPixel({6,11}), imgptr->LongValueAtPixel(6+11*N));

    EXPECT_EQ(imgptr->ULongLongValueAtPixel(9+17*N), static_cast<uint64_t>(u64));
    EXPECT_EQ(imgptr->ULongLongValueAtPixel({9,17}), imgptr->ULongLongValueAtPixel(9+17*N));
    EXPECT_EQ(imgptr->UInt64ValueAtPixel({9,17}), imgptr->ULongLongValueAtPixel(9+17*N));

    EXPECT_EQ(imgptr->LongLongValueAtPixel(8+15*N), static_cast<int64_t>(i64));
    EXPECT_EQ(imgptr->LongLongValueAtPixel({8,15}), imgptr->LongLongValueAtPixel(8+15*N));
    EXPECT_EQ(imgptr->Int64ValueAtPixel({8,15}), imgptr->LongLongValueAtPixel(8+15*N));

    if constexpr (std::is_integral_v<T>)
    {
        EXPECT_EQ(imgptr->FloatValueAtPixel(0+19*N), static_cast<float>(static_cast<int>(f+0.5)));
        EXPECT_EQ(imgptr->FloatValueAtPixel({0,19}), imgptr->FloatValueAtPixel(0+19*N));

        EXPECT_EQ(imgptr->DoubleValueAtPixel(8+10*N), static_cast<double>(static_cast<int>(d+0.5)));
        EXPECT_EQ(imgptr->DoubleValueAtPixel({8,10}), imgptr->DoubleValueAtPixel(8+10*N));
    }
    else
    {
        EXPECT_NEAR(imgptr->FloatValueAtPixel(0+19*N), f, 1e-5);
        EXPECT_NEAR(imgptr->FloatValueAtPixel({0,19}), f, 1e-5);

        EXPECT_NEAR(imgptr->DoubleValueAtPixel(8+10*N), d, 1e-5);
        EXPECT_NEAR(imgptr->DoubleValueAtPixel({8,10}), d, 1e-5);
    }
}

TEST(FITSimg, WorldCoordinates)
{
    verbose = verboseLevel::VERBOSE_NONE;

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

    // test pixel to world and back
    size_t k = 0;
    for(size_t j=0; j< img->Size(2); j++)
    {
        for(size_t i=0; i< img->Size(1); i++)
        {
            EXPECT_EQ(k, img->PixelIndex({i,j}));
            EXPECT_EQ(i, img->PixelCoordinates(k)[0]);
            EXPECT_EQ(j, img->PixelCoordinates(k)[1]);
            worldCoords world0 = img->WorldCoordinates(k);

            worldCoords world1 = img->WorldCoordinates(std::vector<size_t>({i,j}));

            worldCoords world2 = img->WorldCoordinates(pixelCoords({static_cast<double>(i),static_cast<double>(j)}));
            EXPECT_NEAR(world0[0], world1[0], 5e-6);
            EXPECT_NEAR(world0[1], world1[1], 5e-6);
            EXPECT_NEAR(world0[0], world2[0], 5e-6);
            EXPECT_NEAR(world0[1], world2[1], 5e-6);

            pixelCoords pix = img->World2Pixel(world2);
            EXPECT_EQ(static_cast<size_t>(pix[0]+0.5), i);
            EXPECT_EQ(static_cast<size_t>(pix[1]+0.5), j);
            k++;
        }
    }
    
    EXPECT_NO_THROW(ff.Close());
    EXPECT_FALSE(ff.isOpen());
}

TEST(FITSimg, WorldCoordinatesMatrix)
{
    verbose = verboseLevel::VERBOSE_NONE;

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

    std::valarray<size_t> piX(img->Nelements());
    for(size_t k=0; k< img->Nelements(); k++)
        piX[k] = k;

    worldVectors world = img->WorldCoordinatesVector(piX);
    std::valarray<size_t> piY = img->World2PixelArray(world);

    for(size_t k=0; k< img->Nelements(); k++)
        EXPECT_EQ(piX[k], piY[k]);
    
    
    EXPECT_NO_THROW(ff.Close());
    EXPECT_FALSE(ff.isOpen());
}

#include <gtest/gtest.h>
#include <DSTfits/FITShdu.h>
#include <DSTfits/FITSimg.h>
#include <DSTfits/FITSexception.h>
#include <DSTfits/FITSmanager.h>
#include <DSTfits/DSF_version.h>

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

// Helper to decide if T is unsigned integral
template<typename T>
constexpr bool is_unsigned_integral_v = std::is_integral_v<T> && std::is_unsigned_v<T>;

// Build a 5x5x5 image and fill values per spec
template<typename T>
std::unique_ptr<FITSimg<T>> makeCubeAndFill()
{
    auto img = std::make_unique<FITSimg<T>>(std::vector<size_t>{5,5,5});
    // Fill: value(x,y,z) = z     for unsigned integral T
    //        z - 2.5             for signed integral or floating T
    for(size_t z=0; z<5; ++z)
    for(size_t y=0; y<5; ++y)
    for(size_t x=0; x<5; ++x)
    {
        const T val = is_unsigned_integral_v<T> ? static_cast<T>(z)
                                                : static_cast<T>(static_cast<double>(z) - 2.5);
        img->SetPixelValue(val, std::vector<size_t>{x,y,z});
    }
    return img;
}

template<typename T>
void mask(FITSimg<T>& img)
{
    // Mask entire layers z == 1 and z == 3
    const size_t nx = img.Size(1);
    const size_t ny = img.Size(2);
    const size_t nz = img.Size(3);

    std::valarray<bool> m(img.Nelements());
    m = false;

    for (size_t z = 0; z < nz; ++z)
    {
        if (z != 1 && z != 3) continue;
        for (size_t y = 0; y < ny; ++y)
        for (size_t x = 0; x < nx; ++x)
        {
            const size_t idx = x + nx * (y + ny * z);
            m[idx] = true;
        }
    }
    img.MaskPixels(m);
}

// Programmatically compute expected stats for the z-sequence (length 5) without masking
template<typename T>
void expected_unmasked(double& mean, double& sum, double& minv, double& maxv, double& median)
{
    if constexpr (is_unsigned_integral_v<T>)
    {
        // z in {0,1,2,3,4}
        mean = 2.0;
        sum  = 10.0;
        minv = 0.0;
        maxv = 4.0;
        median = 2.0;
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        // z-2.5 in {-2.5,-1.5,-0.5,0.5,1.5}
        mean = -0.5;
        sum  = -2.5;
        minv = -2.5;
        maxv = 1.5;
        median = -0.5;
    }
    else
    {
        // z-2.5 in {-2,-1,0,0,1}
        mean = 0.0;
        sum  = -2;
        minv = -2;
        maxv = 1;
        median = 0;
    }
}

// Expected values with masking z==0 and z==4
template<typename T>
void expected_masked(double& mean, double& sum, double& minv, double& maxv, double& median)
{
    if constexpr (is_unsigned_integral_v<T>)
    {
        // remaining z in {0,2,4}
        mean = 2.0;
        sum  = 6.0;
        minv = 0.0;
        maxv = 4.0;
        median = 2.0;
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        // remaining z-2.5 in {-2.5,-0.5,1.5}
        mean = -0.5;
        sum  = -1.5;
        minv = -2.5;
        maxv = 1.5;
        median = -0.5;
    }
    else
    {
        // signed integral truncation: remaining {-2,0,1}
        mean = 0;
        sum  = -1.0;
        minv = -2.0;
        maxv = 1.0;
        median = 0.0;
    }
}

static const char* overlayToString(FITScube::overlay m) {
    switch (m) {
        case FITScube::overlay::mean:   return "mean";
        case FITScube::overlay::sum:    return "sum";
        case FITScube::overlay::min:    return "min";
        case FITScube::overlay::max:    return "max";
        case FITScube::overlay::median: return "median";
        default: return "unknown";
    }
}

template<typename T>
void assertOverlayUnmasked(FITSimg<T>& img, FITScube::overlay method, double expected)
{
    const ::testing::ScopedTrace trace(__FILE__, __LINE__, std::string("Overlay method: ") + overlayToString(method) + " (unmasked)");

    auto resCube = img.Overlay(method);

    auto resImg  = dynamic_cast<FITSimg<T>*>(resCube.get());
    ASSERT_NE(resImg, nullptr) << "Overlay returned non-image for method=" << overlayToString(method);

    const auto* data = resImg->template GetData<T>();
    ASSERT_NE(data, nullptr) << "GetData returned null for method=" << overlayToString(method);
    ASSERT_EQ(data->size(), img.Size(1)*img.Size(2)) << "Unexpected output size for method=" << overlayToString(method);

    for (size_t i = 0; i < data->size(); ++i)
    {
        const double actual = static_cast<double>((*data)[i]);
        if constexpr (std::is_floating_point_v<T>)
        {
            EXPECT_NEAR(actual, expected, 1e-6)
                << "Unmasked overlay failed: method=" << overlayToString(method)
                << " index=" << i << " expected=" << expected << " actual=" << actual;
        }
        else
        {
            EXPECT_EQ(actual, expected)
                << "Unmasked overlay failed: method=" << overlayToString(method)
                << " index=" << i << " expected=" << expected << " actual=" << actual;
        }
    }
}

template<typename T>
void assertOverlayMasked(FITSimg<T>& img, FITScube::overlay method, double expected)
{
    const ::testing::ScopedTrace trace(__FILE__, __LINE__, std::string("Overlay method: ") + overlayToString(method) + " (masked)");

    auto resCube = img.Overlay(method);

    auto resImg  = dynamic_cast<FITSimg<T>*>(resCube.get());
    ASSERT_NE(resImg, nullptr) << "Overlay returned non-image for method=" << overlayToString(method);

    const auto* data = resImg->template GetData<T>();
    ASSERT_NE(data, nullptr) << "GetData returned null for method=" << overlayToString(method);
    ASSERT_EQ(data->size(), img.Size(1)*img.Size(2)) << "Unexpected output size for method=" << overlayToString(method);

    for (size_t i = 0; i < data->size(); ++i)
    {
        const double actual = static_cast<double>((*data)[i]);
        if constexpr (std::is_floating_point_v<T>)
        {
            EXPECT_NEAR(actual, expected, 1e-6)
                << "Unmasked overlay failed: method=" << overlayToString(method)
                << " index=" << i << " expected=" << expected << " actual=" << actual;
        }
        else
        {
            EXPECT_EQ(actual, expected)
                << "Unmasked overlay failed: method=" << overlayToString(method)
                << " index=" << i << " expected=" << expected << " actual=" << actual;
        }
    }
}

// Test suite for all supported types
template<typename T>
void runOverlayTestsForType()
{
    auto cube = makeCubeAndFill<T>();

    // Unmasked expectations
    double mean,sum,minv,maxv,median;
    expected_unmasked<T>(mean,sum,minv,maxv,median);

    assertOverlayUnmasked<T>(*cube, FITScube::overlay::mean,   mean);
    assertOverlayUnmasked<T>(*cube, FITScube::overlay::sum,    sum);
    assertOverlayUnmasked<T>(*cube, FITScube::overlay::min,    minv);
    assertOverlayUnmasked<T>(*cube, FITScube::overlay::max,    maxv);
    assertOverlayUnmasked<T>(*cube, FITScube::overlay::median, median);

    // Mask and re-test using detailed per-pixel computation
    mask<T>(*cube);

    expected_masked<T>(mean,sum,minv,maxv,median);
    assertOverlayMasked<T>(*cube, FITScube::overlay::mean, mean);
    assertOverlayMasked<T>(*cube, FITScube::overlay::sum , sum);
    assertOverlayMasked<T>(*cube, FITScube::overlay::min , minv);
    assertOverlayMasked<T>(*cube, FITScube::overlay::max , maxv);
    assertOverlayMasked<T>(*cube, FITScube::overlay::median, median);
}

// Define one test per type
TEST(FITSimgOverlay, UInt8)      { runOverlayTestsForType<uint8_t>(); }
TEST(FITSimgOverlay, Int8)       { runOverlayTestsForType<int8_t>(); }
TEST(FITSimgOverlay, UInt16)     { runOverlayTestsForType<uint16_t>(); }
TEST(FITSimgOverlay, Int16)      { runOverlayTestsForType<int16_t>(); }
TEST(FITSimgOverlay, UInt32)     { runOverlayTestsForType<uint32_t>(); }
TEST(FITSimgOverlay, Int32)      { runOverlayTestsForType<int32_t>(); }
TEST(FITSimgOverlay, UInt64)     { runOverlayTestsForType<uint64_t>(); }
TEST(FITSimgOverlay, Int64)      { runOverlayTestsForType<int64_t>(); }
#ifdef Darwinx86_64
TEST(FITSimgOverlay, SizeT)      { runOverlayTestsForType<size_t>(); }
#endif
TEST(FITSimgOverlay, Float)      { runOverlayTestsForType<float>(); }
TEST(FITSimgOverlay, Double)     { runOverlayTestsForType<double>(); }

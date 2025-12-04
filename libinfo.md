# <img src="DeepSkyFitsLogo.png"> DeepSkyFits

A C++ library for reading, writing, and manipulating FITS images and tables, built on CFITSIO and WCSLIB, with optional GTest-based tests and Doxygen documentation.

- Repo: https://gitlab.in2p3.fr/deepskyproject/deepskyfits.git
- C++ Standard: prefers C++23/20, falls back to C++17 (as configured by CMake)
- Platforms: Linux and macOS

--------------------------------------------------------------------------------

## Get the project

Clone with submodules (required: googletest, cfitsio, Minuit2, doxygen-awesome-css):

- Recommended:

  `git clone --recurse-submodules https://gitlab.in2p3.fr/deepskyproject/deepskyfits.git`
- If already cloned:

  `git submodule update --init --recursive`

--------------------------------------------------------------------------------

## Build and install

### Prerequisites:
- CMake ≥ 3.5, a C++17+ compiler, Make or Ninja
- System libs used by externals: zlib, bzip2, curl
- GSL (CMake finds it as REQUIRED)
- Internet access (to download wcslib tarball during build)

### Notes about dependencies:
- *CFITSIO* builds from the submodule via ExternalProject_Add
- *WCSLIB* is downloaded and built via ExternalProject_Add (static)
- *Minuit2* and *googletest* come from submodules
- *Doxygen* is optional (target: doc)

### Basic build:
```
mkdir -p build
cmake -S . -B build
  -DWITH_DEBUG=ON                        # optional: debug flags, ASan
  -DCMAKE_INSTALL_PREFIX=/desired/prefix # optional, default printed by CMake
cmake --build build -j
```


### Install (optional):
```
cmake --build build --target install
```

What CMake does:
- Picks the highest supported C++ standard (23→20→17)
- **Builds** internal static *CFITSIO* and *WCSLIB* (no system CFITSIO needed)
- Produces the shared library **libDSTfits** and test executables
- Generates helper scripts to copy CFITSIO/Minuit2 headers for install

--------------------------------------------------------------------------------

## Test the installation

Tests are based on GoogleTest and come with automated test data download.

- Build tests (already done with the regular build)
- Run tests:
  - From build dir: `ctest` or: `ctest -V for verbose`
  - With Makefiles: `make test`
- Notes:
  - A custom target download_testdata fetches sample FITS files; test executables depend on it
  - The wcslib *tofits* utility is built and used by a test dependency

--------------------------------------------------------------------------------

## Library
DeepSkyFits provides a modern C++ interface to CFITSIO and WCSLIB with a strong focus on safety, performance, and ergonomic data manipulation. The key building blocks are FITSmanager (I/O and HDU navigation), FITShdu (header keywords and metadata), FITScube and FITSimg<T> (image/cube data with statistics and masking), and FITStable (binary tables with typed columns, filtering, and transformations). This section introduces their purposes, core design choices, and typical usage patterns.

### FITSmanager: purpose and responsibilities
FITSmanager centralizes file-level operations:
- Open/close FITS files and report status, filename, and HDU count.
- Navigate HDUs (primary image, images, and tables) and expose the current fitsfile handle via shared_ptr for safe ownership.
- Retrieve typed objects: primary header (FITShdu), primary image/cubes (FITScube), images by index, and tables by name/index.
- Create new FITS files and append images/tables, coordinating header updates and metadata persistence.

It acts as the "session" that ensures CFITSIO calls are checked, resources are consistently owned, and HDU transitions are explicit and predictable. When you request image or table objects, FITSmanager provides high-level wrappers (FITScube, FITStable) that combine typed storage, metadata, and utilities.

```c++
// Open an existing file and fetch primary image and its header
FITSmanager fm("testdata/testkeys.fits");
if (!fm.isOpen()) throw std::runtime_error("cannot open FITS");

auto img = fm.GetPrimary();           // std::shared_ptr<FITScube>
auto hdr = fm.GetPrimaryHeader();     // std::shared_ptr<FITShdu>

std::cout << "Image size: " << img->Size(1) << " x " << img->Size(2) << "\n";
std::cout << "BITPIX: " << hdr->GetInt16ValueForKey("BITPIX") << "\n";

fm.Close();
```

### FITShdu: header keywords and metadata
FITShdu encapsulates keyword storage and typed accessors for reading/writing header content. Internally, keywords are kept in a dictionary-like container (FITSDictionary), mapping key_code (string) to FITSkeyword (value, comment, and type). The class:
- Detects and preserves keyword types (e.g., fChar, fInt, fFloat, fDouble, logical).
- Provides typed getters (GetInt16ValueForKey, GetDoubleValueForKey, GetValueForKey, GetBoolValueForKey, etc.) with robust error signaling when keys are missing or type mismatches occur.
- Offers setters that enforce type consistency while allowing comments and overwrites when appropriate.
- Serializes to/from the canonical header card format and writes back to CFITSIO handles.

Type safety matters: once a keyword exists with a given type, subsequent writes respect and validate that type, preventing accidental corruption. Common dimensionality keywords (NAXISn), scaling keywords (BSCALE, BZERO), and table descriptors (TTYPEn, TFORMn, TSCALn, TZEROn) are straightforward to inspect or update via FITShdu.

```c++
// Create or modify header keywords with enforced types
FITShdu hdu;
hdu.ValueForKey("OBJECT", std::string("M31"), "Target name");
hdu.ValueForKey("BSCALE", 1.0, "Scaling factor");
hdu.ValueForKey("BZERO",  0.0, "Zero offset");
hdu.ValueForKey("NAXIS",  2);
hdu.ValueForKey("NAXIS1", 300);
hdu.ValueForKey("NAXIS2", 300);

// Access with typed getters
double bscale = hdu.GetDoubleValueForKey("BSCALE");
std::string obj = hdu.GetValueForKey("OBJECT");

// Serialize to FITS header block string (2880-char padded)
std::string cards = hdu.asString();
```

### FITScube and FITSimg<T>: valarray-backed data and masking
Images and cubes are represented by FITScube (generic) and FITSimg<T> (typed image helper). Both rely on std::valarray for:
- Efficient bulk arithmetic and elementwise operations.
- Contiguous storage with well-defined semantics for sum, mean, variance, etc.
- Clear expression of transformations on entire arrays or masked subsets.

A per-pixel mask is maintained alongside the data using a valarray<bool>, enabling:
- Mask-aware statistics (minimum, maximum, sum, mean, median, variance, stddev, RMS, RMSE, quantiles, kurtosis, skewness).
- Logical queries and selection over values while honoring masked pixels.
- Operators that propagate masks when combining arrays or images.

FITSimg<T> highlights:
- Compile-time data type binding (T ∈ {integer and floating types}) and correct handling of BSCALE/BZERO for unsigned types or quantized conversions.
- Convenience arithmetic operators for scalar, array, and image-to-image operations with mask merging.
- WCS handling via FITScube APIs: world-to-pixel transforms, rebinning with WCS adjustments, cropping/windows, and multi-layer cubes.

```c++
// Build a 2D typed image, fill, mask, compute statistics, and write to FITS
FITSimg<float> img(2, {256,256});

// Access storage and fill predictable values
auto data = img.GetData<float>();
for (size_t j=0; j<img.Size(2); ++j)
    for (size_t i=0; i<img.Size(1); ++i)
        (*data)[i + j*img.Size(1)] = static_cast<float>(j+1);

// Mask out a region
img.MaskPixels({ img.PixelIndex({128,128}), img.PixelIndex({129,128}) });

// Arithmetic with scalars and arrays
img += 1.0f;                 // add scalar
std::valarray<float> bias(data->size()); bias = 0.5f;
img -= bias;                 // subtract array elementwise

// Statistics (mask-aware)
double mean   = img.GetMean();
double median = img.GetMedian();
double stddev = img.GetStdDev();

// WCS: convert pixel to sky and back (when WCS loaded)
auto world = img->WorldCoordinates(std::vector<size_t>{100, 120}); // RA/Dec etc.
auto pix   = img->World2Pixel(world);

// Save to disk
img.Write("testdata/my_image.fits", /*replace=*/true);
```

```c++
// Rebinning (sum or mean) and crop/window preserving WCS
auto src = FITSimg<double>(2, {300,300});
auto rebinned_sum  = src.Rebin({3,3}, /*asMean=*/false);
auto rebinned_mean = src.Rebin({3,3}, /*asMean=*/true);

auto window = src.Window(/*x0=*/50, /*y0=*/25, /*nx=*/60, /*ny=*/75);

// Layered cubes: add planes and keep the 3rd axis WCS consistent
FITSimg<float> base(2, {64,64});
FITSimg<float> layer1(2, {64,64});
FITSimg<float> layer2(2, {64,64});

base.AddLayer(layer1);
base.AddLayer(layer2);

auto z0 = base.Layer(0); // shared_ptr<FITScube> view of the first plane
```

### FITStable: TFORM, typed columns, and data manipulation helpers
FITS binary tables are exposed through FITStable. Each column is a typed FITScolumn<T> bound to a CFITSIO TFORM (e.g., "E" for float, "D" for double, "J" for 32-bit int, "1B" for packed bits, "A" for strings). The library:
- Maps CFITSIO TFORM to the corresponding C++ type and owns column data in vectors (scalar cells) or vector-of-vectors (vector cells).
- Preserves TSCALE/TZERO keywords for unsigned and quantized formats, ensuring round-trip correctness.
- Provides a fluent selection/filter API (RowSet builder and ColumnFilterExpr) operating on typed columns, returning immutable row-index sets.
- Offers ColumnView and ColumnHandle utilities to apply arithmetic or user-defined transforms over selected rows, and stable sorting that preserves inter-column alignment.
- Validates reorder operations and enforces type-correct column access.

```c++
// Build a table with typed columns and write to disk
FITStable tbl;
auto c_i32 = std::make_shared<FITScolumn<int32_t>>("COL_INT", tint, "", 1);
c_i32->push_back(-3); c_i32->push_back(0); c_i32->push_back(5);
tbl.InsertColumn(c_i32);

auto c_dbl = std::make_shared<FITScolumn<double>>("COL_DBL", tdouble, "", 1);
c_dbl->push_back(0.5); c_dbl->push_back(1.5); c_dbl->push_back(3.0);
tbl.InsertColumn(c_dbl);

// Filter: rows where COL_INT >= 0 and <= 5
RowSet rows = tbl.select<int32_t>("COL_INT").between(0,5).build();

// Manipulate only selected rows of double column
tbl.column<double>("COL_DBL").on(rows).add(2.0).mul(10.0);

// Sort by COL_INT ascending, preserving row alignment across columns
tbl.column<int32_t>("COL_INT").sortAscending();

// Write table
tbl.write("testdata/my_table.fits", /*hduIndex=*/0, /*replace=*/true);
```

```c++
// Combine symbolic filters and apply across columns
FITStable t = /*...load or build...*/;

// Symbolic filter chain
RowSet negatives = t.filter<int32_t>("COL_INT") < 0;
RowSet smallDbl  = t.filter<double>("COL_DBL") <= 1.0;

// Apply a transformation to COL_DBL where both conditions hold
RowSet target = negatives && smallDbl;
t.column<double>("COL_DBL").on(target).apply([](double& v, size_t){ v = v - 0.25; });

// Chain across columns via ColumnHandle
t["COL_DBL"].where<int32_t>("COL_INT").ge(3).set<double>(99.0);
```

### FITSexception: error handling and CFITSIO integration
FITSexception centralizes error reporting across the library and provides a uniform way to propagate CFITSIO failures with human-readable messages. Whenever a CFITSIO call returns a non-zero status, DeepSkyFits converts that status into a FITSexception. The exception:
- Stores the CFITSIO status code and a formatted message using CFITSIO's error stack (fits_get_errstatus / fits_report_error).
- Preserves contextual information from the call site (e.g., operation detail) to aid debugging.
- Is thrown consistently by high-level APIs (FITSmanager, FITShdu, FITSimg/FITScube, FITStable) so user code can rely on try/catch blocks.

Typical usage: wrap file operations, header edits, and I/O in try/catch. On catch, you can log the message, inspect the status code, and decide whether to retry, skip, or abort. This approach keeps error handling explicit and predictable, while still leveraging CFITSIO's informative diagnostics.

```c++
// Example: catching CFITSIO errors via FITSexception
try
{
    FITSmanager fm("testdata/does_not_exist.fits");
    // CFITSIO open fails => FITSexception thrown by FITSmanager
}
catch(const FITSexception& ex)
{
    // ex.what() includes CFITSIO status and decoded message
    std::cerr << "FITS error: " << ex.what() << "\n";
    // Optionally inspect code if available (implementation provides status retrieval)
    // int code = ex.status(); // if exposed
}
```

DeepSkyFits also ensures CFITSIO's internal error stack is cleared between calls where appropriate, preventing stale messages from leaking. When writing headers or appending HDUs, failures (e.g., invalid TFORM, type mismatch, out-of-range values) are converted into FITSexception with precise details, aligning with the library's type-safe philosophy.

### CFITSIOGuard and error stack hygiene
CFITSIO maintains a thread-local error stack and requires disciplined status handling. CFITSIOGuard is a small RAII helper used internally to:
- Initialize and scope a CFITSIO status variable (int status = 0).
- Clear the CFITSIO error stack when a scope ends or before sensitive sequences, avoiding stale messages.
- Ensure consistent propagation of non-zero status via FITSexception when failures occur mid-sequence.

This complements FITSexception: guards keep CFITSIO's state clean, exceptions carry the decoded message and status up to callers.

```c++
// Typical pattern (simplified)
void writeHeader(const std::shared_ptr<fitsfile>& fptr, const FITShdu& hdu) {
    CFITSIOGuard guard; // status=0; stack is clean in this scope
    int status = 0;
    // CFITSIO calls...
    // if (status) throw FITSexception(status, "writing header");
}
```

### Verbose flags and diagnostics
DeepSkyFits exposes lightweight, compile-time-safe verbose flags to help diagnose workflows without a full logger:
- VERBOSE_NONE: silent (default in tests)
- VERBOSE_BASIC, VERBOSE_HDU, VERBOSE_IMG, VERBOSE_TBL: targeted reporting
- VERBOSE_DETAIL: extended details
- VERBOSE_DEBUG: enables all

Combine flags with bitwise operations. Library code uses these flags to emit concise, contextual messages around I/O, HDU transitions, and WCS operations.

```c++
// Example: enable HDU and IMG messages
DSL::verbose = DSL::verboseLevel::VERBOSE_BASIC;
DSL::verbose |= DSL::verboseLevel::VERBOSE_HDU;
DSL::verbose |= DSL::verboseLevel::VERBOSE_IMG;
```

### Note on FITSstatistic
Mask-aware statistics exposed by FITScube/FITSimg<T> rely on utilities in FITSstatistic. These functions:
- Respect per-pixel masks and numeric types (integer vs floating point).
- Provide robust implementations of min/max, sum/mean/variance/stddev, RMS/RMSE, quantiles/percentiles, skewness, and kurtosis.
- Integrate cleanly with valarray-backed storage to keep operations predictable and fast on large 2D/3D datasets.

--------------------------------------------------------------------------------

DeepSkyFits Library 

Authors :
- William GILLARD, Aix-Marseille Université, CNRS, CPPM, France

[<img src="by-nc.png" style="width:10%">](licence)
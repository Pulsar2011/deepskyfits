# DeepSkyFits

A C++ library for reading, writing, and manipulating FITS images and tables, built on CFITSIO and WCSLIB, with optional GTest-based tests and Doxygen documentation.

- Repo: https://gitlab.in2p3.fr/deepskyproject/deepskyfits.git
- C++ Standard: prefers C++23/20, falls back to C++17 (as configured by CMake)
- Platforms: Linux and macOS

--------------------------------------------------------------------------------

## Get the project

Clone with submodules (required: googletest, cfitsio, Minuit2, doxygen-awesome-css):

- Recommended:
  git clone --recurse-submodules https://gitlab.in2p3.fr/deepskyproject/deepskyfits.git
- If already cloned:
  git submodule update --init --recursive

--------------------------------------------------------------------------------

## Build and install

### Prerequisites:
- CMake ≥ 3.5, a C++17+ compiler, Make or Ninja
- System libs used by externals: zlib, bzip2, curl
- GSL (CMake finds it as REQUIRED)
- Internet access (to download wcslib tarball during build)

### Notes about dependencies:
- CFITSIO builds from the submodule via ExternalProject_Add
- WCSLIB is downloaded and built via ExternalProject_Add (static)
- Minuit2 and googletest come from submodules
- Doxygen is optional (target: doc)

### Basic build:
- mkdir -p build
- cmake -S . -B build
  -DWITH_DEBUG=ON                        # optional: debug flags, ASan
  -DCMAKE_INSTALL_PREFIX=/desired/prefix # optional, default printed by CMake
- cmake --build build -j

### Install (optional):
- cmake --build build --target install

What CMake does:
- Picks the highest supported C++ standard (23→20→17)
- Builds internal static CFITSIO and WCSLIB (no system CFITSIO needed)
- Produces the shared library libDSTfits and test executables
- Generates helper scripts to copy CFITSIO/Minuit2 headers for install

--------------------------------------------------------------------------------

## Test the installation

Tests are based on GoogleTest and come with automated test data download.

- Build tests (already done with the regular build)
- Run tests:
  - From build dir: ctest          # or: ctest -V for verbose
  - With Makefiles: make test
- Notes:
  - A custom target download_testdata fetches sample FITS files; test executables depend on it
  - The wcslib "tofits" utility is built and used by a test dependency

--------------------------------------------------------------------------------

## Library overview (major classes)

### FITSmanager
DeepSkyFits centers around a single manager that opens, inspects, and updates FITS files safely. FITSmanager abstracts CFITSIO handles behind shared ownership and locks, so you can move between HDUs, fetch headers, images, and tables without worrying about thread-safety or lifetime. In practice you construct a manager with a path, navigate to the HDU you need, operate on images or tables in memory, then write changes back. It also exposes simple helpers to append images or update keywords, making it your main entry point to the library.
- High-level manager for a FITS file (open/create/close/write)
- Thread-safe shared_ptr<fitsfile> with shared/exclusive locking helpers
- HDU navigation: MoveToHDU, MoveToPrimary, GetPrimaryHeader, GetHeaderAtIndex
- Image access: GetPrimary, GetImageAtIndex; append images
- Table access: GetTableAtIndex, GetTable by EXTNAME; InsertTable, UpdateTable
- File-level helpers: GetFileName, Write, Status
- Internals discover structure via explore()

### FITScube (abstract) and FITSimg<T> (typed image)
At the core of image handling are FITScube and its typed implementation FITSimg<T>. FITScube provides a consistent N‑D image abstraction with header, WCS, data buffer, and a pixel mask. FITSimg<T> specializes it with concrete storage and typed arithmetic. Data and mask are stored as flattened valarrays, so operations like cropping, rebinning, or layer extraction use fast slicing rather than manual loops. Most algorithms automatically respect the mask: masked pixels are treated as invalid and skipped from statistics and transforms. This mirrors how astronomers think about image processing—work on the valid data, keep the header and WCS coherent, and make bulk operations concise.
- FITScube: N-D FITS image base class with:
  - FITShdu header, polymorphic data storage, pixel mask, WCS support
  - Accessors: sizes, pixel indexing, mask handling
  - Operations: Layer extraction, Window (crop), Rebin, Resize
  - Statistics: sum/mean/variance/stddev/percentiles/kurtosis/skewness/min/max
  - Pixel-wise arithmetic and image-to-image arithmetic with mask propagation
  - Pixel mask: operations treat pixels with mask=true (or 1) as invalid; they are skipped in statistics, arithmetic, and transforms.
- FITSimg<T>: typed implementation
  - Manages BSCALE/BZERO/BLANK and BITPIX/equivalent BITPIX
  - Read/write image data (via CFITSIO), safe typed data access
  - GetData<T>() for direct valarray access; WithTypedData helpers
  - WCS updates when resizing/layering (best-effort)

#### Storage model and rationale
Using flattened std::valarray for both pixels and masks gives predictable performance and expressive slicing. It's easy to address N‑D data through 1‑D indices, and operations like Window, Layer, and Rebin rely on std::gslice to copy contiguous blocks efficiently. Although std::valarray is less commonly used than std::vector, its element‑wise semantics and slicing make it well‑suited to numerical image processing.

### FITStable (tables)
Tables follow a similar philosophy: load once, work in memory with typed columns, then write back. A FITStable discovers columns and their metadata (names, types, units), provides typed views for safe access, and lets you build RowSet selections by chaining simple predicates. Sorting and reordering apply globally, ensuring all columns stay aligned. This design aims to make common catalog tasks—filtering, adding derived columns, exporting—straightforward while preserving FITS conventions like BSCALE/BZERO for pseudo‑unsigned types.
- Abstraction for ASCII/BINARY table HDUs with typed columns
- FITSform: column descriptor (name/type/unit/scale/zero/repeat/width/position)
  - Handles pseudo-unsigned scaling (BSCALE/BZERO) and type mapping
  - In-memory storage via type-erased columnData
- FITScolumn<T>: typed column, push_back, internal type-safe storage, write support
- Reading/writing:
  - Read scalar/vector columns with CFITSIO (null handling, BSCALE/BZERO)
  - Write columns with correct CFITSIO storage type mapping
- Introspection and access:
  - nrows/ncols, listColumns, getColumn by name/index
  - ColumnHandle and ColumnView<T> (typed access)
  - RowSet and filter/builders (selection and reordering)
- Utilities:
  - boolVector encoding/decoding helpers for bit-packed columns
  - reorderRows applies a consistent permutation across all columns

#### Logic behind selection/filtering
Working with tables usually starts by choosing a column, building a set of matching rows, and then operating on those rows. A RowSet is simply a list of row indices that satisfy your predicates (for example, "MAG < 20 and FLAG == 0"). You create it with select<T>("COL") and chain comparisons; combining sets with && and || lets you intersect or union selections naturally. ColumnView<T> then gives you typed access to a column's in‑memory data, and can be restricted to a RowSet to read or update only the selected rows. If you need to stay type‑erased until the last moment, ColumnHandle provides a fluent entry point to obtain typed views and apply selections. Finally, when you sort or reorder rows, reorderRows applies one global permutation to all columns, keeping the table consistent.
- ColumnView<T> gives typed, read/write access to a column's in-memory data (std::vector<T>) with optional RowSet restriction. It throws if the column's dtype doesn't match T.
- RowSet represents a set of row indices. You build it with select<T>("COL") and chain predicates; sets support && (intersection) and || (union).
- ColumnHandle provides a fluent entry point to create typed views, apply RowSet selections, and perform updates without exposing the underlying storage.
- reorderRows enforces a single global permutation across all columns. It validates:
  - size(order) == nrows,
  - indices are within [0..nrows-1],
  - no duplicates,
  - all columns share the same row count.

#### Examples: creating, manipulating, and exporting FITS images
- Construct and inspect an image:
  // FITSmanager fm("path/to/file.fits");
  // auto cube = fm.GetPrimary(); // shared_ptr<FITScube>
  // auto imgD = std::dynamic_pointer_cast<FITSimg<double>>(cube); // typed image
  // size_t nelem = imgD->Nelements();
  // int bitpix = imgD->GetBitPerPixel();

- Access typed data and pixels:
  // const std::valarray<double>* data = imgD->GetData<double>();
  // double p00 = imgD->DoubleValueAtPixel({0,0}); // by coordinates
  // double pIdx = imgD->DoubleValueAtPixel(42);   // by linear index

- Arithmetic on the whole image:
  // (*imgD) += 10.0;       // shift all unmasked pixels
  // (*imgD) *= 2.0;        // scale
  // (*imgD) /= 3.0;        // divide (checks for zero)
  // std::valarray<double> v(nelem); v = 1.0;
  // (*imgD) += v;          // element-wise add
  // (*imgD) *= v;          // element-wise multiply

- Masking and statistics:
  // imgD->MaskPixels({0,1,2});     // mask first three pixels
  // bool isMasked = imgD->Masked(1);
  // double mean = imgD->GetMean(); // computed on unmasked pixels
  // double p95  = imgD->Get95thpercentil();

- Resize and crop:
  // // Crop a 2D image from (xMin=10,yMin=20) with width=100,height=80
  // imgD->Resize(10, 20, 100, 80);
  // // Or get a new cropped image without modifying the original:
  // auto win = imgD->Window(10, 20, 100, 80);

- Layer extraction (3D cubes):
  // // Extract layer k into a new 2D image
  // auto layer2D = imgD->Layer(3); // returns shared_ptr<FITScube>

- Rebinning:
  // // Downsample X by 2 and Y by 2; do mean aggregation
  // auto rebinned = imgD->Rebin({2,2}, /*doMean*/true);

- WCS usage:
  // auto wc = imgD->WorldCoordinates({50,25}); // world coords at pixel (50,25)
  // auto px = imgD->World2Pixel(wc);           // inverse transform

- Write table back:
  // fm.UpdateTable(tbl); // update existing HDU, or
  // tbl->write("!updated.fits", /*start*/1, /*replace*/true);


Notes
- Pseudo-unsigned types (tsbyte, tushort, tuint, tulong, tulonglong) are stored with signed CFITSIO codes and use BSCALE/BZERO for exact value preservation. Typed access remains in the target unsigned type.
- For bit-packed flags, use toBoolVector/fromBoolVector helpers to decode/encode scalar masks.

--------------------------------------------------------------------------------

# C++ API reference

## Doxygen website:
- https://deepskyfits-4be303.pages.in2p3.fr/

--------------------------------------------------------------------------------

# License
CC BY-NC 4.0 (non-commercial, attribution)
https://creativecommons.org/licenses/by-nc/4.0/

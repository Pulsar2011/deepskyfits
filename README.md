# DeepSkyFits

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
- *superNOVAS* is downloaded and built via ExternalProject_Add  (static)

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

#### Examples: creating, manipulating, and exporting FITS images
- Construct and inspect an image:
```c++
   FITSmanager fm("path/to/file.fits");
   auto cube = fm.GetPrimary(); // shared_ptr<FITScube>
   auto imgD = std::dynamic_pointer_cast<FITSimg<double>>(cube); // typed image
   size_t nelem = imgD->Nelements();
   int bitpix = imgD->GetBitPerPixel();
```

- Access typed data and pixels:
```c++
   const std::valarray<double>* data = imgD->GetData<double>();
   double p00 = imgD->DoubleValueAtPixel({0,0}); // by coordinates
   double pIdx = imgD->DoubleValueAtPixel(42);   // by linear index
```

- Arithmetic on the whole image:
```c++
 (*imgD) += 10.0;       // shift all unmasked pixels
 (*imgD) *= 2.0;        // scale
 (*imgD) /= 3.0;        // divide (checks for zero)
 std::valarray<double> v(nelem); v = 1.0;
 (*imgD) += v;          // element-wise add
 (*imgD) *= v;          // element-wise multiply
```

- Masking and statistics:
```c++
   imgD->MaskPixels({0,1,2});     // mask first three pixels
   bool isMasked = imgD->Masked(1);
   double mean = imgD->GetMean(); // computed on unmasked pixels
   double p95  = imgD->Get95thpercentil();
```

- Resize and crop:
```c++
   // Crop a 2D image from (xMin=10,yMin=20) with width=100,height=80
   imgD->Resize(10, 20, 100, 80);
   // Or get a new cropped image without modifying the original:
   auto win = imgD->Window(10, 20, 100, 80);
```

- Layer extraction (3D cubes):
```c++
  // Extract layer k into a new 2D image
  auto layer2D = imgD->Layer(3); // returns shared_ptr<FITScube>
  ```

- Rebinning:
```c++
   // Downsample X by 2 and Y by 2; do mean aggregation
   auto rebinned = imgD->Rebin({2,2}, /*doMean*/true);
```

- WCS usage:
```c++
   auto wc = imgD->WorldCoordinates({50,25}); // world coords at pixel (50,25)
   auto px = imgD->World2Pixel(wc);           // inverse transform
```

- Write back to FITS:
```c++
   fm.UpdateTable(/*tables*/);           // if you modified tables
   imgD->Write("!out.fits", /*replace*/true);
```

#### Notes
- Arithmetic and SetPixelValue perform safe casting between scalar types and storage type T; masked pixels are skipped.
- Layer and Window return new images with updated headers and best-effort WCS updates; complex WCS may require recalibration.

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
Working with tables usually starts by choosing a column, building a set of matching rows, and then operating on those rows. A RowSet is simply a list of row indices that satisfy your predicates (for example, “MAG < 20 and FLAG == 0”). You create it with select<T>("COL") and chain comparisons; combining sets with && and || lets you intersect or union selections naturally. ColumnView<T> then gives you typed access to a column’s in‑memory data, and can be restricted to a RowSet to read or update only the selected rows. If you need to stay type‑erased until the last moment, ColumnHandle provides a fluent entry point to obtain typed views and apply selections. Finally, when you sort or reorder rows, reorderRows applies one global permutation to all columns, keeping the table consistent.
- ColumnView<T> gives typed, read/write access to a column's in-memory data (std::vector<T>) with optional RowSet restriction. It throws if the column's dtype doesn't match T.
- RowSet represents a set of row indices. You build it with select<T>("COL") and chain predicates; sets support && (intersection) and || (union).
- ColumnHandle provides a fluent entry point to create typed views, apply RowSet selections, and perform updates without exposing the underlying storage.
- reorderRows enforces a single global permutation across all columns. It validates:
  - size(order) == nrows,
  - indices are within [0..nrows-1],
  - no duplicates,
  - all columns share the same row count.

#### Examples: filtering, selecting, and manipulating table rows
- Open a table and list columns:
```c++
   FITSmanager fm("path/to/file.fits");
   auto tbl = fm.GetTable("MYTABLE");
   auto meta = tbl->listColumns(); // each entry is {name, TFORM, unit}
```

- Typed column access and filtering:
```c++
   // Get a typed view by column name, e.g. double column "MAG"
   auto mag = tbl->column<double>("MAG");
   Build a row selection where MAG < 20 and "FLAG" == 0:
   auto rows = tbl->select<double>("MAG") /* builder for MAG */
                 /* chain predicates, e.g. .lt(20) && tbl->select<int>("FLAG").eq(0) */
                 /* finalize to RowSet */;
   Use the selection with a handle:
   auto h = (*tbl)["MAG"]; // ColumnHandle
   auto v = h.view<double>(rows); // ColumnView<double> restricted to rows
```

- Sort or reorder rows globally:
```c++
   // Suppose you computed a custom permutation "order"
   std::vector<size_t> order = { /* new-to-old indices, size == nrows */ };
   tbl->reorderRows(order); // applies to all columns; validates permutation
```

- Append/update columns:
```c++
   // Insert a new empty column:
   tbl->InsertColumn("SNR", dtype::tfloat, ""); // define type/unit
  // Insert a populated column:
   auto snr = std::make_shared<FITScolumn<float>>("SNR", dtype::tfloat);
   snr->push_back(std::any(12.3f)); // repeat for each row
   tbl->InsertColumn(snr);
```

- Write table back:
```c++
   fm.UpdateTable(tbl); // update existing HDU, or
   tbl->write("!updated.fits", /*start*/1, /*replace*/true);
```

Logic behind selection/filtering
- ColumnView<T> gives typed, read/write access to a column's in-memory data (std::vector<T>) with optional RowSet restriction. It throws if the column's dtype doesn't match T.
- RowSet represents a set of row indices. You build it with select<T>("COL") and chain predicates; sets support && (intersection) and || (union).
- ColumnHandle provides a fluent entry point to create typed views, apply RowSet selections, and perform updates without exposing the underlying storage.
- reorderRows enforces a single global permutation across all columns. It validates:
  - size(order) == nrows,
  - indices are within [0..nrows-1],
  - no duplicates,
  - all columns share the same row count.

Notes
- Pseudo-unsigned types (tsbyte, tushort, tuint, tulong, tulonglong) are stored with signed CFITSIO codes and use BSCALE/BZERO for exact value preservation. Typed access remains in the target unsigned type.
- For bit-packed flags, use toBoolVector/fromBoolVector helpers to decode/encode scalar masks..

### SkyCoordinates (celestial coordinates)
Alongside the WCS wrapper, DeepSkyFits ships a small hierarchy for the positions themselves: where an object is on the sky, how fast it is moving, and when it was measured. `SkyCoordinates` is the abstract base; `EquatorialCoordinates`, `GalacticCoordinates` and `EclipticCoordinates` each fix one spherical system and name the angles the way astronomers do. Everything underneath is SuperNOVAS — the library owns every constant and every rotation matrix, and nothing is reimplemented here. The design goal is that you should never have to remember whether a given routine wants hours or degrees, or which of precession and proper motion a "convert to B1950" actually applied.

- `SkyCoordinates` (abstract): the two angles, proper motion, parallax, radial velocity, epoch, system and frame
  - `angularSeparation()` — great-circle angle, haversine below 90°, law of cosines above
  - `assign()` — populates any concrete subclass, which is how a conversion fills a sibling type
  - `toEquatorial()` / `toGalactic()` / `toEcliptic()` — one-argument (keep the frame) and two-argument (rotate into one) forms
  - `epochOf()`, `currentEpoch()` — epoch lookups, forwarded to SuperNOVAS
- `EquatorialCoordinates`: right ascension and declination; the pivot every other system routes through
  - `getRA()`, `getDEC()`, `getPMRA()`, `getPMDEC()`
  - `toICRS()`, `toFK5()`, `toFK4()` — named shorthands for the frame rotations
  - `atEpoch()`, `atCurrentEpoch()` — transport the star along its own space motion
  - `convertFrame()` — static, for callers holding loose numbers rather than objects
- `GalacticCoordinates`: galactic longitude and latitude, on the IAU/Hipparcos definition
- `EclipticCoordinates`: ecliptic longitude and latitude, tilted from the equator by the obliquity

#### System, frame, and epoch
Three independent things describe a position, and keeping them apart is what the hierarchy is organised around. Mixing them up is the usual source of arcsecond-scale confusion, so the API makes each one a separate operation.

| | What it fixes | How you change it | What moves |
|---|---|---|---|
| **System** (`CoordSystem`) | Which pole and origin the angles are measured from | `toEquatorial()`, `toGalactic()`, `toEcliptic()` | Nothing physical — the same direction, different numbers |
| **Frame** (`CoordFrame`) | The orientation of the axes: FK4, FK5, ICRS, HIPPARCOS | the two-argument conversions, or `toFK4()` / `toFK5()` / `toICRS()` | The axes rotate under the star |
| **Epoch** (`getEpoch()`) | The date the position is valid at | `atEpoch()`, `atCurrentEpoch()` | The star moves along its proper motion |

Angles are in degrees everywhere in the public interface, proper motions in mas/yr, parallax in mas, radial velocity in km/s, and epochs are Julian dates on the TT scale.

#### Examples: converting between systems and frames
- Convert between systems (the destination is an output parameter, not a return value):
```c++
   using namespace DSL;

   EquatorialCoordinates m31(10.68470, 41.26875);   // ICRS by default
   GalacticCoordinates   gal;
   EclipticCoordinates   ecl;

   m31.toGalactic(&gal);    // gal.getGLON() == 121.174..., gal.getGLAT() == -21.573...
   m31.toEcliptic(&ecl);
```

- Rotate into another reference frame:
```c++
   EquatorialCoordinates b1950;
   m31.toFK4(&b1950);                              // named shorthand
   m31.toEquatorial(&b1950, CoordFrame::FK4);      // identical, generic form

   // Or ask for a system and a frame in one step:
   m31.toEcliptic(&ecl, CoordFrame::FK4);
```

- Measure an angle on the sky:
```c++
   EquatorialCoordinates m42(83.82208, -5.39111);
   double sep = m31.angularSeparation(m42);        // [deg]
```

#### Examples: proper motion and epochs
- A full catalogue entry, moved to tonight:
```c++
   // Barnard's Star, as published by Hipparcos at epoch J1991.25
   EquatorialCoordinates barnard(269.45207, 4.66829,   // [deg]    RA, Dec
                                 -798.58, 10328.12,    // [mas/yr] pmRA*, pmDec
                                 547.45, -110.51,      // [mas] parallax, [km/s] v_rad
                                 CoordFrame::HIPPARCOS);

   EquatorialCoordinates today;
   barnard.atCurrentEpoch(&today);

   std::cout << barnard.angularSeparation(today) << " deg since J1991.25\n";
```

- Name a date however is convenient:
```c++
   EquatorialCoordinates at2000, at2030, atHip;

   barnard.atEpoch("J2000.0", &at2000);                  // epoch string
   barnard.atEpoch(SkyCoord::epochOf("J2030.5"), &at2030); // explicit Julian date
   barnard.atEpoch(CoordFrame::HIPPARCOS, &atHip);       // a frame's standard epoch
```

- Frame and epoch compose in either order, and are genuinely independent:
```c++
   EquatorialCoordinates rotated, moved;
   barnard.toFK5(&rotated);              // axes rotate, date untouched
   barnard.atEpoch("J2000.0", &moved);   // star moves, axes untouched
```

- Static form, when you have loose numbers rather than objects:
```c++
   double ra = 0.0, dec = 0.0, pmRA = 0.0, pmDEC = 0.0;
   // NOTE: this is the one place right ascension is in HOURS, not degrees.
   EquatorialCoordinates::convertFrame(4.0, 20.0, 30.0, -10.0,
                                       CoordFrame::FK4, CoordFrame::FK5,
                                       ra, dec, pmRA, pmDEC);
```

#### Notes
- Conversions fill a destination you supply rather than returning a new object, so one call site can target any concrete subclass. Every conversion throws `std::invalid_argument` on a null destination, and `convertFrame` throws if either frame is `UNDEFINED`.
- Right ascension is in degrees everywhere except the static `convertFrame()`, which takes hours to match the SuperNOVAS catalogue convention. The doc comment says so at each overload.
- Proper motion is not carried into or out of the galactic and ecliptic systems — the rate would have to be re-projected onto different axes, which the classes do not do, so those results come back with zero proper motion. Parallax, radial velocity and epoch do travel through: they are properties of the star, not of the axes used to point at it.
- HIPPARCOS and ICRS name the same axes. Converting between them is a no-op; what separates them is the J1991.25 epoch, which is `atEpoch()`'s business. Rotating a Hipparcos entry to ICRS therefore leaves it at J1991.25 rather than silently relabelling it J2000.
- FK4 is the exception to "a frame change is only a rotation". Converting into or out of it goes through a NOVAS `CHANGE_EPOCH` transformation, which transports the star over the 50 years between B1950.0 and J2000.0 as well as rotating it, so the epoch follows the frame there. `getEpoch()` reports where you actually ended up.
- `CoordSystem::UNDEFINED` and `CoordSystem::EQUATORIAL` are both `0`, so an unset system reads as equatorial. Treat `UNDEFINED` as a default, not as a detectable error.
- Ecliptic conversions use the J2000.0 obliquity on both ends of a frame change. That makes the change a clean rotation about the ecliptic pole — longitude precesses, latitude does not — and makes it exactly invertible. It is not the mean obliquity of the target epoch.
- Propagating a star to another epoch and back does not close exactly when the radial velocity is non-zero. The residual is of order `μ · Δt · (v_r · Δt / d)` — about 0.4 mas for a 50 pc star at 30 km/s over 50 years — and comes from SuperNOVAS `transform_cat()` scaling the returned proper motion by the input distance while reporting the updated parallax. It vanishes for zero radial velocity.

--------------------------------------------------------------------------------

# C++ API reference

## Doxygen website:
- https://deepskyfits-4be303.pages.in2p3.fr/

--------------------------------------------------------------------------------

# License
Licensed under [Non-Commercial Polyform](https://polyformproject.org/licenses/noncommercial/1.0.0/).
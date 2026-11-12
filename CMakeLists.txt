#-------------------- CMAKE Configuration
# 3.24: required by the SuperNOVAS submodule
cmake_minimum_required(VERSION 3.24)

# -Wno-dev by default (developer and deprecation warnings, mostly from the submodules): no need to
# pass it, nor to alias cmake with it, which breaks 'cmake --build' and 'cmake --install'.
# 'cmake -Wdev ...' shows them again.
if(NOT DEFINED CACHE{CMAKE_SUPPRESS_DEVELOPER_WARNINGS})
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "Suppress developer warnings (-Wno-dev)")
endif()

# Everything is generated in a build directory, never in the sources: from the project directory,
# 'cmake --preset default' (or 'cmake -B build'); from a build directory, 'cmake <path to project>'.
get_filename_component(_src "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(_bin "${CMAKE_BINARY_DIR}" REALPATH)
if(_src STREQUAL _bin)
    message(FATAL_ERROR
        "In-source builds are not allowed. Remove what this run (and any former in-source build) created:\n"
        "    cmake -P cmake/CleanInSourceBuild.cmake\n"
        "then, from the project directory:\n"
        "    cmake --preset default        (or: cmake -B build)\n"
        "    cmake --build build -j\n"
        "    cmake --build build --target install\n"
        "Other presets: debug (build-debug/), asan (build-asan/).\n"
        "Or from a build directory: mkdir build && cd build && cmake .. && make")
endif()

project(DeepSkyFits LANGUAGES C CXX)

if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif()

message("${BoldCyan}----------- DeepSkyFits (DSTfits) -----------${ColourReset}")
message("${Green}-- Project source directory :${ColourReset} ${CMAKE_SOURCE_DIR}")
message("${Green}-- Project build directory  :${ColourReset} ${CMAKE_BINARY_DIR}")

if(DEFINED ENV{DeepSkyROOT})
    set(DeepSkyROOT $ENV{DeepSkyROOT})
else()
    set(DeepSkyROOT ${CMAKE_SOURCE_DIR}/..)
endif()
get_filename_component(DeepSkyROOT "${DeepSkyROOT}" ABSOLUTE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${DeepSkyROOT} CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

#-------------------- Options
option(WITH_ASAN   "Build DSTfits and its tests with AddressSanitizer" OFF)
option(BUILD_TESTS "Build the GoogleTest suite" ON)
option(CI_CIWORKER "Build for the GitLab CI runner" OFF)
option(WITH_DEBUG  "Deprecated: same as CMAKE_BUILD_TYPE=Debug and WITH_ASAN=ON (use the asan preset)" OFF)

if(WITH_DEBUG)
    message(WARNING "WITH_DEBUG is deprecated: use 'cmake --preset asan' (Debug + AddressSanitizer, in build-asan/)")
    set(CMAKE_BUILD_TYPE Debug)
    set(WITH_ASAN ON)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CI_CIWORKER)
    add_compile_definitions(CI_CIWORKER)
endif()

# Products of this build: executables in bin/, libraries in lib/ (objects stay in CMakeFiles/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# The highest standard the compiler supports, from C++23 down
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
# Static parts (cfitsio, wcslib, SuperNOVAS) end up inside the shared libDSTfits
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(ProcessorCount)
ProcessorCount(NCORES)
if(NCORES EQUAL 0)
    set(NCORES 4)
endif()

#-------------------- COMPILER
# These flags apply to every C++ target of this directory, the submodules included (Minuit2, gtest).
set(_release "$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>")
set(_debug   "$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(OperatingSystem "Linux")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wall;-pipe;-pthread;-fexceptions;-Wno-unknown-pragmas;-Wmisleading-indentation;-mtune=native;-ffp-contract=on>")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-m64>")
        add_compile_definitions(Linuxx86_64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        add_compile_definitions(Linuxaarch64)
    endif()
    add_compile_options("$<${_release}:-flto>")
    add_link_options(-pthread "$<$<CONFIG:Release>:-flto>")
    # Interprocedural optimization (LTO) of the release build, when supported
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(OperatingSystem "Mac OS X")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wall;-mcpu=native;-mtune=native;-ffp-contract=on>")
    add_compile_definitions(Darwin)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        add_compile_definitions(Darwinarm64)
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-m64>")
        add_compile_definitions(Darwinx86_64)
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
    add_compile_options("$<${_release}:-flto=thin>")
    add_link_options("$<$<CONFIG:Release>:-flto=thin>")
endif()

add_compile_options("$<${_release}:-O3;-fomit-frame-pointer>")
# No LTO in debug: on macOS it replaces the .o files with LLVM bitcode, leaving the debug map with
# empty OSO entries that LLDB cannot follow.
add_compile_options("$<${_debug}:-O0;-g>")
add_compile_definitions("$<$<CONFIG:Debug>:__DEBUG__>")

if(WITH_ASAN)
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address;-fno-omit-frame-pointer;-g>")
    add_link_options(-fsanitize=address)
endif()

message("${BoldYellow}-- CMAKE INSTALL PREFIX : ${ColourReset}${CMAKE_INSTALL_PREFIX}")
message("${Green}-- DeepSkyROOT :${ColourReset} ${DeepSkyROOT}")
message("${Green}-- BUILD TYPE :${ColourReset} ${CMAKE_BUILD_TYPE}")
message("${Green}-- CXX COMPILER :${ColourReset} ${CMAKE_CXX_COMPILER}")
message("${Green}-- USE CXX STANDARD :${ColourReset} ${CMAKE_CXX_STANDARD}")
if(WITH_ASAN)
    message("${Blue}-- AddressSanitizer ACTIVATED${ColourReset}")
endif()

#-------------------- VERSION FROM THE GIT TAG
find_package(Git)
include(GetGitRevisionDescription)
git_describe(VERSION --always --tags --long)

string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+.(-*)" "\\1" VERSION_SHA1 "${VERSION}")
set(VERSION_SHORT "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message("-- PKG VERSION : ${VERSION} (${VERSION_SHORT}, sha1 ${VERSION_SHA1})")

set(CMAKE_PROJECT_VERSION       "${VERSION}")
set(CMAKE_PROJECT_VERSION_MAJOR "${VERSION_MAJOR}")
set(CMAKE_PROJECT_VERSION_MINOR "${VERSION_MINOR}")
set(CMAKE_PROJECT_VERSION_PATCH "${VERSION_PATCH}")

# A clone without a reachable tag (e.g. a shallow CI clone) has no numeric version: the library
# stays unversioned and no CMake package is installed.
if(VERSION MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+")
    set(DSTfits_NUMERIC_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(PROJECT_VERSION "${DSTfits_NUMERIC_VERSION}")
else()
    message(WARNING "No version tag found (git describe gave '${VERSION}'): DSTfits will not be versioned")
endif()

# Generated sources stay in the build directory
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
configure_file(${CMAKE_SOURCE_DIR}/modules/DSF_version.cxx.in ${GENERATED_DIR}/src/DSF_version.cxx)
configure_file(${CMAKE_SOURCE_DIR}/modules/gTest_version.h.in ${GENERATED_DIR}/test/gTest_version.h)

#-------------------- GSL
message("${Red}----------- SEARCH GSL -----------${ColourReset}")
find_package(GSL REQUIRED)
message("${Blue}-- GSL INC DIR ${ColourReset} ${GSL_INCLUDE_DIRS}")
message("${Blue}-- GSL LIB ${ColourReset} ${GSL_LIBRARIES}")

#-------------------- Dependencies built from the submodules
message("${Red}----------- SUBMODULES : cfitsio, wcslib, Minuit2, SuperNOVAS -----------${ColourReset}")
set(_submodules cfitsio/configure Minuit2/CMakeLists.txt SuperNOVAS/CMakeLists.txt)
if(BUILD_TESTS)
    list(APPEND _submodules googletest/CMakeLists.txt)
endif()
foreach(sub IN LISTS _submodules)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${sub})
        message(FATAL_ERROR "${BoldRed}${sub} is missing: run 'git submodule update --init'${ColourReset}")
    endif()
endforeach()

include(ExternalProject)
set(EXTERNAL_DIR ${CMAKE_BINARY_DIR}/external)

# The autotools builds need make, whatever the generator (CMAKE_MAKE_PROGRAM is ninja with Ninja)
find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

#---------- cfitsio: static, absorbed into libDSTfits; headers used straight from the submodule
set(CFITSIO_SRC_DIR     ${CMAKE_SOURCE_DIR}/cfitsio)
set(CFITSIO_INCLUDE_DIR ${CFITSIO_SRC_DIR})
set(CFITSIO_LIBRARY     ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/libcfitsio.a)
ExternalProject_Add(cfitsio_external
    PREFIX            ${EXTERNAL_DIR}/cfitsio
    SOURCE_DIR        ${CFITSIO_SRC_DIR}
    BINARY_DIR        ${EXTERNAL_DIR}/cfitsio-build
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env CC=${CMAKE_C_COMPILER} CFLAGS=-fPIC
                      ${CFITSIO_SRC_DIR}/configure --enable-static --disable-shared
                      --prefix=${EXTERNAL_DIR}/cfitsio-install --with-gnu-ld --with-bzip2
    BUILD_COMMAND     ${MAKE_EXECUTABLE} -j ${NCORES} libcfitsio.la
    INSTALL_COMMAND   ${CMAKE_COMMAND} -E copy_if_different <BINARY_DIR>/.libs/libcfitsio.a ${CFITSIO_LIBRARY}
    BUILD_BYPRODUCTS  ${CFITSIO_LIBRARY}
)
add_library(cfitsio_static STATIC IMPORTED)
set_target_properties(cfitsio_static PROPERTIES
    IMPORTED_LOCATION ${CFITSIO_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CFITSIO_INCLUDE_DIR}
)
add_dependencies(cfitsio_static cfitsio_external)
message("${Blue}-- cfitsio  ${ColourReset} static, from ${CFITSIO_SRC_DIR}")

#---------- wcslib: downloaded, static, absorbed into libDSTfits
set(WCSLIB_VERSION 8.5)
set(WCSLIB_SHA256 f1fd1b78fbfdbabda363f8045e0c59e32735eca45482a5302191e56fe062eace)
set(WCSLIB_TARBALL "" CACHE FILEPATH "Local wcslib-${WCSLIB_VERSION}.tar.bz2, to build without downloading it")
if(WCSLIB_TARBALL)
    set(WCSLIB_URLS ${WCSLIB_TARBALL})
else()
    set(WCSLIB_URLS
        "https://www.atnf.csiro.au/people/mcalabre/WCS/wcslib-${WCSLIB_VERSION}.tar.bz2"
        "https://www.atnf.csiro.au/computing/software/wcs/WCS/wcslib-${WCSLIB_VERSION}.tar.bz2"
        "https://www.atnf.csiro.au/computing/software/wcs/wcslib-releases/wcslib-${WCSLIB_VERSION}.tar.bz2"
    )
endif()
set(WCSLIB_SRC_DIR     ${EXTERNAL_DIR}/wcslib-src)
set(WCSLIB_INSTALL_DIR ${EXTERNAL_DIR}/wcslib-install)
set(WCSLIB_INCLUDE_DIR ${WCSLIB_INSTALL_DIR}/include)
set(WCSLIB_LIBRARY     ${WCSLIB_INSTALL_DIR}/lib/libwcs-${WCSLIB_VERSION}.a)
set(WCSLIB_TOFITS      ${WCSLIB_INSTALL_DIR}/bin/tofits)
# An imported target needs its include directory to exist when the project is generated
file(MAKE_DIRECTORY ${WCSLIB_INCLUDE_DIR})
ExternalProject_Add(wcslib_external
    DEPENDS           cfitsio_external
    PREFIX            ${EXTERNAL_DIR}/wcslib
    URL               ${WCSLIB_URLS}
    URL_HASH          SHA256=${WCSLIB_SHA256}
    DOWNLOAD_DIR      ${EXTERNAL_DIR}/download
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_DIR        ${WCSLIB_SRC_DIR}
    BUILD_IN_SOURCE   1
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env CC=${CMAKE_C_COMPILER} CFLAGS=-fPIC
                      ./configure --prefix=${WCSLIB_INSTALL_DIR}
                      --with-cfitsioinc=${CFITSIO_INCLUDE_DIR} --with-cfitsiolib=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
                      --disable-shared
    BUILD_COMMAND     ${MAKE_EXECUTABLE} -j ${NCORES}
    INSTALL_COMMAND   ${MAKE_EXECUTABLE} install
    BUILD_BYPRODUCTS  ${WCSLIB_LIBRARY} ${WCSLIB_TOFITS}
)
add_library(wcslib_static STATIC IMPORTED)
set_target_properties(wcslib_static PROPERTIES
    IMPORTED_LOCATION ${WCSLIB_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${WCSLIB_INCLUDE_DIR}
)
add_dependencies(wcslib_static wcslib_external)
message("${Blue}-- wcslib   ${ColourReset} ${WCSLIB_VERSION}, static, downloaded into ${EXTERNAL_DIR}/download")

#---------- Minuit2: shared, installed (the other DeepSky projects link it)
# Minuit2 asks for CMake 3.1, which CMake 4 no longer accepts
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(Minuit2 EXCLUDE_FROM_ALL)
unset(CMAKE_POLICY_VERSION_MINIMUM)
message("${Blue}-- Minuit2  ${ColourReset} shared, from ${CMAKE_SOURCE_DIR}/Minuit2")

#---------- SuperNOVAS: static + PIC, absorbed into libDSTfits
# Plain variables: SuperNOVAS' option() calls honour them (CMP0077), without touching the cache.
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING     OFF)
set(BUILD_EXAMPLES    OFF)
set(BUILD_BENCHMARK   OFF)
set(BUILD_DOC         OFF)
# SuperNOVAS ships a C++ API (namespace `supernovas`, include <supernovas.h>) in a library of
# its own, libsupernovas++, gated behind ENABLE_CPP. DSTfits still calls the C99 API: the C++ one
# has no catalog-epoch propagation (no transform_cat(PROPER_MOTION, ...) equivalent) and its
# Equatorial::to_system() carries only a direction vector, so it would drop proper motion,
# parallax and radial velocity. Building it here keeps it available for when that gap closes.
set(ENABLE_CPP        ON)
add_subdirectory(SuperNOVAS EXCLUDE_FROM_ALL)
unset(BUILD_TESTING)
# SuperNOVAS/src/cpp/CMakeLists.txt pins its target to C++11, but supernovas.h includes
# <optional>, which is C++17. Override it here rather than patching the vendored tree, so
# the fix survives re-vendoring the dependency.
if(TARGET cpp)
    set_property(TARGET cpp PROPERTY CXX_STANDARD 17)
    set_property(TARGET cpp PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
message("${Blue}-- SuperNOVAS${ColourReset} static, from ${CMAKE_SOURCE_DIR}/SuperNOVAS")

#-------------------- libDSTfits
message("${Red}----------- libDSTfits -----------${ColourReset}")
set(DSTFITS_SOURCES
    src/FITSexception.cxx
    src/FITShdu.cxx
    src/FITSimg.cxx
    src/FITSmanager.cxx
    src/FITSnumeric.cxx
    src/FITSstatistic.cxx
    src/FITStable.cxx
    src/FITSunit.cxx
    src/FITSwcs.cxx
    ${GENERATED_DIR}/src/DSF_version.cxx
)
file(GLOB DSTFITS_PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/DSTfits/*.h)

add_library(DSTfits SHARED ${DSTFITS_SOURCES} ${DSTFITS_PUBLIC_HEADERS})
if(DSTfits_NUMERIC_VERSION)
    set_target_properties(DSTfits PROPERTIES
        VERSION   "${DSTfits_NUMERIC_VERSION}"
        SOVERSION "${VERSION_MAJOR}"
    )
endif()
target_include_directories(DSTfits
    PUBLIC  ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/DSTfits
            ${CFITSIO_INCLUDE_DIR} ${WCSLIB_INCLUDE_DIR}
    PRIVATE ${GSL_INCLUDE_DIRS}
)
target_link_libraries(DSTfits
    PUBLIC  Minuit2
    PRIVATE cfitsio_static wcslib_static supernovas::core bz2 curl z GSL::gsl GSL::gslcblas
)
if(TARGET cpp)
    add_dependencies(DSTfits cpp)
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set_target_properties(DSTfits PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH}")

#-------------------- Tests
if(BUILD_TESTS)
    message("${Red}----------- GoogleTest -----------${ColourReset}")
    set(DSTFITS_TESTS ON)
    enable_testing()
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
    include(GoogleTest)

    # Test data: sample files downloaded once, and DSS.fits written by wcslib's tofits.
    # The tests run from the build directory and read/write testdata/ there.
    set(TESTDATA_DIR ${CMAKE_BINARY_DIR}/testdata)
    file(MAKE_DIRECTORY ${TESTDATA_DIR})
    set(SAMPLE_FITS_URLS
        "https://heasarc.gsfc.nasa.gov/FTP/fits_info/sample_files/images/rosat_pspc_rdf2_3_bk1.fits"
        "https://heasarc.gsfc.nasa.gov/FTP/fits_info/sample_files/images/rosat_pspc_rdf2_3_im2.fits"
        "http://fits.gsfc.nasa.gov/samples/testkeys.fits"
    )
    set(TESTDATA_FILES "")
    foreach(url IN LISTS SAMPLE_FITS_URLS)
        get_filename_component(fname ${url} NAME)
        list(APPEND TESTDATA_FILES ${TESTDATA_DIR}/${fname})
        add_custom_command(
            OUTPUT  ${TESTDATA_DIR}/${fname}
            COMMAND ${CMAKE_COMMAND} -DURL=${url} -DOUT=${TESTDATA_DIR}/${fname} -P ${CMAKE_SOURCE_DIR}/cmake/DownloadFile.cmake
            COMMENT "Downloading sample FITS file: ${fname}"
            VERBATIM
        )
    endforeach()
    add_custom_command(
        OUTPUT  ${TESTDATA_DIR}/DSS.fits
        COMMAND ${CMAKE_COMMAND} -E rm -f ${TESTDATA_DIR}/DSS.fits
        COMMAND sh -c "\"${WCSLIB_TOFITS}\" < \"${WCSLIB_SRC_DIR}/C/test/DSS.keyrec\" > \"${TESTDATA_DIR}/DSS.fits\""
        DEPENDS wcslib_external
        COMMENT "Writing DSS.fits with wcslib tofits"
        VERBATIM
    )
    list(APPEND TESTDATA_FILES ${TESTDATA_DIR}/DSS.fits)
    add_custom_target(testdata DEPENDS ${TESTDATA_FILES})

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/test/gTest_*.cxx)
    set(TEST_NAMES "")
    foreach(test_source IN LISTS TEST_SOURCES)
        get_filename_component(tname ${test_source} NAME_WE)
        list(APPEND TEST_NAMES ${tname})
        add_executable(${tname} ${test_source})
        target_link_libraries(${tname} PRIVATE DSTfits gtest gtest_main)
        target_include_directories(${tname} PRIVATE ${GENERATED_DIR}/test)
        add_dependencies(${tname} testdata)
        gtest_discover_tests(${tname} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} PROPERTIES TIMEOUT 300)
    endforeach()
    message("${Blue}-- Test executables ${ColourReset} ${TEST_NAMES} (run with ctest)")
else()
    set(DSTFITS_TESTS OFF)
endif()

#-------------------- Doxygen documentation (target: doc, written in build/doc)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    # The Doxyfile paths are relative to the sources: run there, but write in the build directory
    file(READ ${CMAKE_SOURCE_DIR}/DSL.Doxyfile _doxyfile)
    string(APPEND _doxyfile "\nOUTPUT_DIRECTORY = ${CMAKE_BINARY_DIR}/doc\n")
    file(WRITE ${GENERATED_DIR}/DSL.Doxyfile "${_doxyfile}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/DSL.Doxyfile)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${GENERATED_DIR}/DSL.Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen in ${CMAKE_BINARY_DIR}/doc"
        VERBATIM
    )
endif()

#-------------------- Installation
# Headers of the submodules come from their sources, so that they are known at the first configure;
# wcslib's are only produced by its build, so they are listed at install time (install(DIRECTORY)).
file(GLOB CFITSIO_HEADERS    ${CFITSIO_SRC_DIR}/*.h)
file(GLOB SUPERNOVAS_HEADERS ${CMAKE_SOURCE_DIR}/SuperNOVAS/include/*.h)

install(FILES ${DSTFITS_PUBLIC_HEADERS} DESTINATION include/DSTfits)
install(FILES ${CFITSIO_HEADERS}        DESTINATION include/cfitsio)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Minuit2/inc/ DESTINATION include/Minuit2)
install(FILES ${SUPERNOVAS_HEADERS}     DESTINATION include/supernovas)
install(DIRECTORY ${WCSLIB_INCLUDE_DIR}/wcslib-${WCSLIB_VERSION}/ DESTINATION include/wcslib
    FILES_MATCHING PATTERN "*.h")

install(TARGETS DSTfits
    RUNTIME DESTINATION exe
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(TARGETS Minuit2 Minuit2Math
    RUNTIME DESTINATION exe
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(TARGETS core ARCHIVE DESTINATION lib)
install(FILES ${CFITSIO_LIBRARY} ${WCSLIB_LIBRARY} DESTINATION lib)

if(DSTfits_NUMERIC_VERSION)
    # CMake package, so that dependent projects can call find_package(DSTfits <version>)
    include(CMakePackageConfigHelpers)
    set(DSTfits_CMAKE_DIR lib/cmake/DSTfits)
    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/modules/DSTfitsConfig.cmake.in
        ${GENERATED_DIR}/DSTfitsConfig.cmake
        INSTALL_DESTINATION ${DSTfits_CMAKE_DIR}
    )
    write_basic_package_version_file(
        ${GENERATED_DIR}/DSTfitsConfigVersion.cmake
        VERSION ${DSTfits_NUMERIC_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(FILES
        ${GENERATED_DIR}/DSTfitsConfig.cmake
        ${GENERATED_DIR}/DSTfitsConfigVersion.cmake
        DESTINATION ${DSTfits_CMAKE_DIR}
    )

    # CMake never removes files from the install directory: drop the versioned libraries
    # (libDSTfits.1.5.0.dylib, libDSTfits.so.1.5.0, ...) left behind by previous installs.
    install(CODE "set(DSTfits_KEEP_VERSIONS \"${DSTfits_NUMERIC_VERSION};${VERSION_MAJOR}\")")
    install(CODE [[
        file(GLOB _dsf_libs "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libDSTfits.*")
        foreach(_dsf_lib ${_dsf_libs})
            get_filename_component(_dsf_name "${_dsf_lib}" NAME)
            if(_dsf_name MATCHES "^libDSTfits\\.(so\\.)?([0-9.]*[0-9])(\\.dylib)?$")
                list(FIND DSTfits_KEEP_VERSIONS "${CMAKE_MATCH_2}" _dsf_keep)
                if(_dsf_keep EQUAL -1)
                    message(STATUS "Removing old library: ${_dsf_lib}")
                    file(REMOVE "${_dsf_lib}")
                endif()
            endif()
        endforeach()
    ]])
endif()

#-------------------- Summary
message("${BoldGreen}----------- DeepSkyFits ${VERSION} configured -----------${ColourReset}")
message("${Green}-- Build directory  :${ColourReset} ${CMAKE_BINARY_DIR}")
message("${Green}-- Build type       :${ColourReset} ${CMAKE_BUILD_TYPE}")
message("${Green}-- AddressSanitizer :${ColourReset} ${WITH_ASAN}")
message("${Green}-- Tests            :${ColourReset} ${DSTFITS_TESTS}")
message("${Green}-- DSTfits          :${ColourReset} ${DSTfits_NUMERIC_VERSION}")
message("${Green}-- cfitsio          :${ColourReset} submodule, static")
message("${Green}-- wcslib           :${ColourReset} ${WCSLIB_VERSION}, static")
message("${Green}-- Minuit2          :${ColourReset} submodule, shared")
message("${Green}-- SuperNOVAS       :${ColourReset} submodule, static")
message("${Green}-- Install prefix   :${ColourReset} ${CMAKE_INSTALL_PREFIX}")
if(WITH_ASAN)
    message("${Yellow}--                    AddressSanitizer build: installing it replaces the release libDSTfits${ColourReset}")
endif()
message("${BoldGreen}-------------------------------------------------${ColourReset}")

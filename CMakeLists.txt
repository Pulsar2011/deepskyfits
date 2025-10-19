#-------------------- Configuration du projet
#-------------------- CMAKE Configuration
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(DeepSkyProject CXX)
message("${Green}-- Project source directory :" ${CMAKE_SOURCE_DIR} "${ColourReset}")

set(CMAKE_BUILD_DIRECTORY ./build)

if(DEFINED ENV{DeepSkyROOT})
    set(DeepSkyROOT $ENV{DeepSkyROOT})
endif()

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

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../. CACHE PATH "default install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(EXECUTABLE_OUTPUT_PATH ./build/exe/)
set(LIBRARY_OUTPUT_PATH ./build/lib/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

message("${BoldYellow}-- CMAKE INSTALL PREFIX : ${ColourReset}" ${CMAKE_INSTALL_PREFIX})

#-------------------- COMPILER
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++23" COMPILER_SUPPORTS_CXX23)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX23)
    message("-- USE CXX23 STANDARD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
    set(CMAKE_CXX_STANDARD "23")
elseif(COMPILER_SUPPORTS_CXX20)
    message("-- USE CXX20 STANDARD")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    set(CMAKE_CXX_STANDARD "20")
elseif(COMPILER_SUPPORTS_CXX17)
    message("-- USE CXX17 STANDARD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_CXX_STANDARD "17")
elseif(COMPILER_SUPPORTS_CXX11)
    message("-- USE CXX11 STANDARD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_STANDARD "11")
elseif(COMPILER_SUPPORTS_CXX0X)
    message("-- USE CX0X STANDARD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++23, C++20, C++17 nor C++11 nor C++0x support. Please use a different C++ compiler.")
endif()

set(BUILD_SHARED_LIBS "YES")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")

#-------------------- GCC specifics
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Linux specific code
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -pipe -fexceptions -fPIC -Wno-unknown-pragmas -Wmisleading-indentation")

	set(OperatingSystem "Linux")
	ADD_DEFINITIONS(-DLinuxx86_64)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#-------------------- DARWIN specifics
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Mac OS X specific code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -m64 -fPIC")

	set(OperatingSystem "Mac OS X")
	ADD_DEFINITIONS(-DDarwinx86_64)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

option(WITH_DEBUG "Activate debug mode" OFF)
if(WITH_DEBUG)
	message("${Blue}-- DEBUG MODE ACTIVATED${ColourReset}")
	add_definitions(-D__DEBUG__)
	set(CMAKE_BUILD_TYPE DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else(WITH_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif(WITH_DEBUG)

message("${Green}-- CXX COMPILER:${ColourReset} ${CMAKE_CXX_COMPILER}")
message("${Green}-- DEFAULT CXX FLAGES:${ColourReset} ${CMAKE_CXX_FLAGS}")

#-------------------- APPEND GIT VERSION TAG
# Make a version file containing the current version from git.
find_package(Git)
include(GetGitRevisionDescription)
git_describe(VERSION --always --tags --long)

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+.(-*)" "\\1" VERSION_SHA1 "${VERSION}")
set(VERSION_SHORT "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message("-- PKG VERSION : " ${VERSION})
message("-- PKG VERSION : " ${VERSION_SHORT})
message("-- PKG MAJOR   : " ${VERSION_MAJOR})
message("-- PKG MINOR   : " ${VERSION_MINOR})
message("-- PKG PATCH   : " ${VERSION_PATCH})
message("-- PKG SHA1    : " ${VERSION_SHA1})

set(CMAKE_PROJECT_VERSION "${VERSION}")
set(CMAKE_PROJECT_VERSION_MAJOR "${VERSION_MAJOR}")
set(CMAKE_PROJECT_VERSION_MINOR "${VERSION_MINOR}")
set(CMAKE_PROJECT_VERSION_PATCH "${VERSION_PATCH}")

configure_file(${CMAKE_MODULE_PATH}/DSF_version.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/src/DSF_version.cxx)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/src/DSF_version.cxx")

configure_file(${CMAKE_MODULE_PATH}/gTest_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/test/gTest_version.h)

#-------------------- Inclusion des en-têtes publics
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/test ./include ./include/DSTfits)

message("-- PROJECT DIR: " ${CMAKE_SOURCE_DIR})
message("-- PROJECT BINARY DIR: " ${PROJECT_BINARY_DIR})
message("-- CURRENT BINARY DIR: " ${CMAKE_CURRENT_BINARY_DIR})
message("-- CMAKE SOURCE DIR  : " ${CMAKE_SOURCE_DIR})

#-------------------- Find GSL dependencies
find_package(GSL REQUIRED)
include_directories(AFTER ${GSL_INCLUDE_DIRS})
link_directories(${GSL_LIBRARY})
if(${GSL_FOUND})
    message("${Blue}-- GSL INC DIR ${ColourReset} ${GSL_INCLUDE_DIRS}")
    message("${Blue}-- GSL LIBRARY DIR ${ColourReset} ${GSL_LIBRARY}")
    message("${Blue}-- GSL LIB ${ColourReset} ${GSL_LIBRARIES}")
endif(${GSL_FOUND})

#-------------------- Add gtest testsuite
set(CMAKE_LIBRARY_PATH "${PROJECT_BINARY_DIR}/gtest/lib" ${CMAKE_LIBRARY_PATH})

message("-- CMAKE INCLUDE DIR  : " ${CMAKE_INCLUDE_PATH})
message("-- CMAKE LIBRARY DIR  : " ${CMAKE_LIBRARY_PATH})

#-------------------- Find Boost dependencies
#message("${Red}----------- SEARCH BOOST Framework -----------${ColourReset}")
#find_package(Boost REQUIRED COMPONENTS filesystem chrono)
#if(${Boost_FOUND})
#    message("${Blue}-- Boost INC DIR ${ColourReset} ${Boost_INCLUDE_DIRS}")
#    message("${Blue}-- Boost LIBRARY DIR ${ColourReset} ${Boost_LIBRARY_DIRS}")
#    message("${Blue}-- Boost LIB ${ColourReset} ${Boost_LIBRARIES}")
#endif(${Boost_FOUND})
#set(Boost_USE_MULTITHREADED ON)

#-------------------- LIST SOURCE FILE

file(GLOB CORE_SRC "src/*.cxx")
file(GLOB CORE_HEADER "include/DSTfits/*.h")
file(GLOB TEST_SRC "test/*.cxx")
file(GLOB FITSIO_HEADER "cfitsio/*.h")

set(HEADER_FILES ${CORE_HEADER})
file(GLOB FITS_HEADER "include/cfitsio/*.h")
file(GLOB MINUIT_HEADER "include/Minuit2/*.h")
file(GLOB MINUIT2_HEADER "include/Minuit2/Minuit2/*.h")
file(GLOB MINUITF_HEADER "include/Minuit2/Fit/*.h")
file(GLOB MINUITM_HEADER "include/Minuit2/Math/*.h")


#-------------------- Configuration de la bibliothèque

include(ExternalProject)

set(CFITSIO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/cfitsio)

ExternalProject_Add(
    cfitsio_external
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/cfitsio
    BINARY_DIR ${CMAKE_SOURCE_DIR}/build/cfitsio 
    CONFIGURE_COMMAND CC=clang ${CMAKE_SOURCE_DIR}/cfitsio/configure --enable-static --disable-shared --prefix=${CMAKE_SOURCE_DIR}/build --with-gnu-ld --with-bzip2
    BUILD_COMMAND make -j 4 libcfitsio.la
    INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${CFITSIO_INCLUDE_DIR}
        && ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/build/cfitsio/.libs/libcfitsio.a ${CMAKE_SOURCE_DIR}/build/lib/libcfitsio.a
        # Copy each header file individually
        && cmake -P ${CMAKE_SOURCE_DIR}/cmake/copy_cfitsio_headers.cmake
    BUILD_BYPRODUCTS ${CMAKE_SOURCE_DIR}/build/lib/libcfitsio.a
)

#-------------------- Configuration de la bibliothèque WCSFIT

set(WCSLIB_VERSION 8.4)
set(WCSLIB_URL "https://www.atnf.csiro.au/computing/software/wcs/WCS/wcslib-${WCSLIB_VERSION}.tar.bz2")
set(WCSLIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/build/wcslib)
set(WCSLIB_SRC_DIR ${CMAKE_SOURCE_DIR}/wcslib/wcslib-${WCSLIB_VERSION})

# Ensure download dir exists and add a download target using wget (fallback to file(DOWNLOAD) if wget missing)
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/wcslib")
file(MAKE_DIRECTORY "${WCSLIB_INSTALL_DIR}")
set(WCSLIB_TARBALL "${CMAKE_SOURCE_DIR}/wcslib-${WCSLIB_VERSION}.tar.bz2")

# Custom target to download wcslib tarball (only download if missing)
add_custom_target(download_wcslib
    COMMAND /usr/bin/env bash -c "if [ ! -f \"${WCSLIB_TARBALL}\" ]; then echo 'Downloading wcslib to ${WCSLIB_TARBALL}...' && wget -O \"${WCSLIB_TARBALL}\" \"${WCSLIB_URL}\"; else echo 'wcslib tarball already exists: ${WCSLIB_TARBALL}'; fi"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/wcslib"
    COMMENT "Download wcslib tarball if missing"
    VERBATIM
)

add_custom_target(extract_wcslib
  COMMAND ${CMAKE_COMMAND} -E echo "Extracting ${WCSLIB_TARBALL} to ${CMAKE_SOURCE_DIR}/wcslib"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/wcslib"
  COMMAND /usr/bin/env tar -xjf "${WCSLIB_TARBALL}" -C "${CMAKE_SOURCE_DIR}/wcslib"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/wcslib"
  VERBATIM
)

# ExternalProject to configure/build/install wcslib from the extracted source
ExternalProject_Add(
  wcslib_external
  SOURCE_DIR      ${WCSLIB_SRC_DIR}
  PREFIX          ${CMAKE_SOURCE_DIR}/build/wcslib
  DOWNLOAD_COMMAND ""                         # download handled by download_wcslib
  UPDATE_COMMAND   ""                         # no update step
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir ${WCSLIB_SRC_DIR} ./configure --prefix=${WCSLIB_INSTALL_DIR}  --with-cfitsioinc=${CFITSIO_INCLUDE_DIR} --with-cfitsiolib=${CMAKE_SOURCE_DIR}/build/lib --disable-shared
  BUILD_COMMAND     ${CMAKE_MAKE_PROGRAM} -j ${CMAKE_BUILD_PARALLEL_LEVEL}
  INSTALL_COMMAND   ${CMAKE_MAKE_PROGRAM} install
  BUILD_IN_SOURCE   1
  BUILD_BYPRODUCTS  ${WCSLIB_INSTALL_DIR}/lib/libwcs.a ${WCSLIB_INSTALL_DIR}/bin/tofits
)

add_custom_target(tofits
    COMMAND /usr/bin/env bash -c "cat \"${CMAKE_SOURCE_DIR}/wcslib/wcslib-8.4/C/test/DSS.keyrec\" | \"${CMAKE_SOURCE_DIR}/build/wcslib/bin/tofits\" > \"${CMAKE_SOURCE_DIR}/build/testdata/DSS.fits\""
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run tofit: ${CMAKE_SOURCE_DIR}/build/wcslib/bin/tofits <${CMAKE_SOURCE_DIR}/wcslib/wcslib-8.4/C/test/DSS.keyrec> ${CMAKE_SOURCE_DIR}/build/testdata/DSS.fits"
    VERBATIM
)

add_dependencies(tofits wcslib_external)

# extraction must run before building wcslib

# expose include/lib paths for targets
set(WCSLIB_INCLUDE_DIR "${WCSLIB_INSTALL_DIR}/include" CACHE PATH "wcslib include dir")
set(WCSLIB_LIBRARY     "${WCSLIB_INSTALL_DIR}/lib/libwcs-${WCSLIB_VERSION}.a" CACHE FILEPATH "wcslib static library")

# ensure DSTfits waits for wcslib to be built and links to it

add_dependencies(wcslib_external cfitsio_external extract_wcslib)
add_dependencies(extract_wcslib download_wcslib)


# Provide explicit cfitsio static library path variable for later linking
set(CFITSIO_OUTPUT_LIB "${CMAKE_SOURCE_DIR}/build/lib/libcfitsio.a" CACHE FILEPATH "cfitsio static archive produced by external build")
file(GLOB WCSLIB_HEADER "build/wcslib/include/wcslib-${WCSLIB_VERSION}/*.h")

#-------------------- Configuration de la bibliothèque

add_library(
    DSTfits
    SHARED
    ${CORE_SRC}
    ${CORE_HEADER}
    ${version_file}
)

# target-level configuration must come after the target is created
add_dependencies(DSTfits cfitsio_external wcslib_external)
target_include_directories(DSTfits PRIVATE ${WCSLIB_INCLUDE_DIR})
target_include_directories(DSTfits PRIVATE ${CFITSIO_INCLUDE_DIR})

include_directories(include/cfitsio)
include_directories(build/wcslib/include)

#-------------------- Configuration des test
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)

add_subdirectory(Minuit2   EXCLUDE_FROM_ALL)
include_directories(Minuit2/inc)

add_dependencies(DSTfits Minuit2)


#-------------------- Configuration des test
add_subdirectory(googletest   EXCLUDE_FROM_ALL)
include_directories(${PARENT_PATH_OF_GTEST}/googltest/googltest/include)
enable_testing()

target_link_libraries(DSTfits
    PRIVATE ${CFITSIO_OUTPUT_LIB}
    PRIVATE Minuit2
    PRIVATE bz2
    PRIVATE curl
    PRIVATE z
    PRIVATE ${WCSLIB_LIBRARY}
)

#-------------------- Download sample FITS files for testing
set(TESTDATA_DIR ${CMAKE_SOURCE_DIR}/build/testdata)
file(MAKE_DIRECTORY ${TESTDATA_DIR})

set(SAMPLE_FITS_URL
    "https://heasarc.gsfc.nasa.gov/FTP/fits_info/sample_files/images/rosat_pspc_rdf2_3_bk1.fits"
    "https://heasarc.gsfc.nasa.gov/FTP/fits_info/sample_files/images/rosat_pspc_rdf2_3_im2.fits"
    "http://fits.gsfc.nasa.gov/samples/testkeys.fits"
)

set(SAMPLE_FITS_FILE "")
foreach(url ${SAMPLE_FITS_URL})
    get_filename_component(fname ${url} NAME)
    list(APPEND SAMPLE_FITS_FILE "${TESTDATA_DIR}/${fname}")
    add_custom_command(
        OUTPUT "${TESTDATA_DIR}/${fname}"
        COMMAND wget -O "${TESTDATA_DIR}/${fname}" "${url}"
        COMMENT "Downloading sample FITS file: ${fname}"
    )
endforeach()

add_custom_target(download_testdata
    DEPENDS ${SAMPLE_FITS_FILE}
)

foreach( test_source_file ${TEST_SRC} )
	get_filename_component( tSRC ${test_source_file} NAME_WE)
	add_executable(
		${tSRC}
		${test_source_file}
	)
    # Ensure test data is downloaded before running tests
    add_dependencies(${tSRC} download_testdata)

    if(COMPILER_SUPPORTS_CXX17)
        target_link_libraries( ${tSRC} DSTfits Minuit2 gtest gtest_main)
    else(COMPILER_SUPPORTS_CXX17)
        target_link_libraries( ${tSRC} DSTfits Minuit2 ${Boost_LIBRARIES} gtest gtest_main)
    endif(COMPILER_SUPPORTS_CXX17)


endforeach( test_source_file ${TEST_SRC} )

include(GoogleTest)

foreach( test_source_file ${TEST_SRC} )
    get_filename_component( tSRC ${test_source_file} NAME_WE)
    gtest_discover_tests(${tSRC})
endforeach( test_source_file ${TEST_SRC} )

add_dependencies(gTest_wcs tofits)

#-------------------- CONFIGURATION OF DOXYGEN DOC GENERATION

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DSL.Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/DSL.Doxyfile @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DSL.Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)



#-------------------- ADD DISTCLEAN CUSTOM TARGET

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
set(DISTCLEANED
cmake.depends
cmake.check_depends
CMakeCache.txt
cmake.check_cache
CMakeTmp
CMakeDoxyfile.in
${version_file}
Makefile
CMakeFiles
*.cmake
./build
./doc
./wcslib*
./googletest/CMakeFiles
./googletest/Makefile
./googletest/*.cmake
./lib
./bin
./Testing
./cfitsio_external-prefix
./include/cfitsio
./include/Minuit2
)

ADD_CUSTOM_COMMAND(
DEPENDS clean
COMMENT "distribution clean"
COMMAND rm ARGS    -Rf ${DISTCLEANED}
COMMAND test -e install_manifest.txt || xargs ARGS rm -f < install_manifest.txt
COMMAND test -e install_manifest.txt || rm ARGS -f install_manifest.txt
TARGET  distclean
)


#-------------------- Installation proceedure
if(CMAKE_INSTALL_PREFIX)
    INSTALL(FILES ${CORE_HEADER} DESTINATION include/DSTfits)
    INSTALL(FILES ${FITS_HEADER} DESTINATION include/cfitsio)
    INSTALL(FILES ${MINUIT_HEADER} DESTINATION include/Minuit2)
    INSTALL(FILES ${MINUIT2_HEADER} DESTINATION include/Minuit2/Minuit2)
    INSTALL(FILES ${MINUITF_HEADER} DESTINATION include/Minuit2/Fit)
    INSTALL(FILES ${MINUITM_HEADER} DESTINATION include/Minuit2/Math)
    INSTALL(FILES ${WCSLIB_HEADER} DESTINATION include/wcslib)

    # Core library built by this project
    INSTALL(TARGETS DSTfits
        RUNTIME DESTINATION exe
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    # cfitsio is built via ExternalProject -> not a CMake target; install the produced archive directly
    set(CFITSIO_OUTPUT_LIB ${CMAKE_SOURCE_DIR}/build/lib/libcfitsio.a)
    INSTALL(FILES ${CFITSIO_OUTPUT_LIB} DESTINATION lib)

    set(WCSLIB_OUTPUT_LIB ${CMAKE_SOURCE_DIR}/build/wcslib/lib/libwcs-${WCSLIB_VERSION}.a)
    INSTALL(FILES ${WCSLIB_OUTPUT_LIB} DESTINATION lib)

    # Minuit2 libraries: install if targets exist, otherwise fallback to copying built files
    if(TARGET Minuit2)
        INSTALL(TARGETS Minuit2
            RUNTIME DESTINATION exe
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
        )
    endif()

    if(TARGET Minuit2Math)
        INSTALL(TARGETS Minuit2Math
            RUNTIME DESTINATION exe
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
        )
    endif()

    if(NOT (TARGET Minuit2 OR TARGET Minuit2Math))
        # Fallback: copy any libMinuit2* libraries found in the build lib dir
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib/ DESTINATION lib
            FILES_MATCHING
            PATTERN "libMinuit2*.a"
            PATTERN "libMinuit2*.so"
            PATTERN "libMinuit2*.dylib"
        )
    endif()
endif(CMAKE_INSTALL_PREFIX)

# At the end of your CMakeLists.txt, generate a helper script for cfitsio headers:
file(WRITE ${CMAKE_SOURCE_DIR}/cmake/copy_cfitsio_headers.cmake
"file(GLOB FITSIO_HEADER \"${CMAKE_SOURCE_DIR}/cfitsio/*.h\")\n"
"file(MAKE_DIRECTORY \"${CFITSIO_INCLUDE_DIR}\")\n"
"foreach(header \${FITSIO_HEADER})\n"
"  file(COPY \${header} DESTINATION \"${CFITSIO_INCLUDE_DIR}\")\n"
"endforeach()\n"
)

# Generate a helper script for Minuit2 headers:
file(WRITE ${CMAKE_SOURCE_DIR}/cmake/copy_minuit2_headers.cmake
"file(GLOB MINUIT2_HEADER \"${CMAKE_SOURCE_DIR}/Minuit2/inc/*\")\n"
"file(MAKE_DIRECTORY \"${CMAKE_SOURCE_DIR}/include/Minuit2\")\n"
"foreach(header \${MINUIT2_HEADER})\n"
"  file(COPY \${header} DESTINATION \"${CMAKE_SOURCE_DIR}/include/Minuit2\")\n"
"endforeach()\n"
)

# Add a post-build custom command to copy Minuit2 headers
add_custom_command(
    TARGET DSTfits POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/copy_minuit2_headers.cmake
    COMMENT "Copying Minuit2 headers to include/Minuit2"
)

# Removes what the former in-source builds left in the source tree (deepskyfits used to be
# configured with 'cmake .' in the project directory). Run once, from anywhere:
#     cmake -P cmake/CleanInSourceBuild.cmake
# then build out of source: 'cmake --preset default && cmake --build build -j'.
# Files tracked by git that the in-source build overwrote (SuperNOVAS ships its own Makefiles)
# are restored, not removed.

get_filename_component(SRC "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
find_package(Git QUIET)

set(_removed 0)
function(_remove path)
    if(EXISTS "${path}" OR IS_SYMLINK "${path}")
        file(REMOVE_RECURSE "${path}")
        file(RELATIVE_PATH _rel "${SRC}" "${path}")
        message(STATUS "removed   ${_rel}")
        math(EXPR _n "${_removed} + 1")
        set(_removed ${_n} PARENT_SCOPE)
    endif()
endfunction()

# Restores a file tracked by the git repository <repo>, removes it otherwise
function(_restore_or_remove repo path)
    if(GIT_FOUND)
        file(RELATIVE_PATH _in_repo "${repo}" "${path}")
        execute_process(COMMAND ${GIT_EXECUTABLE} -C "${repo}" ls-files --error-unmatch -- "${_in_repo}"
            RESULT_VARIABLE _untracked OUTPUT_QUIET ERROR_QUIET)
        if(_untracked EQUAL 0)
            execute_process(COMMAND ${GIT_EXECUTABLE} -C "${repo}" checkout -- "${_in_repo}" OUTPUT_QUIET ERROR_QUIET)
            file(RELATIVE_PATH _rel "${SRC}" "${path}")
            message(STATUS "restored  ${_rel}")
            return()
        endif()
    endif()
    _remove("${path}")
    set(_removed ${_removed} PARENT_SCOPE)
endfunction()

#---------- Project directory
foreach(p
        CMakeCache.txt CMakeFiles Makefile cmake_install.cmake CTestTestfile.cmake
        CMakeDoxyfile.in CMakeDoxygenDefaults.cmake CPackConfig.cmake CPackSourceConfig.cmake
        install_manifest.txt Testing bin lib exe doc cfitsio_external-prefix
        include/cfitsio include/Minuit2 include/supernovas
        src/DSF_version.cxx test/gTest_version.h
        wcslib wcslib-8.5.tar.bz2
        cmake/copy_cfitsio_headers.cmake cmake/copy_minuit2_headers.cmake cmake/copy_supernovas_headers.cmake
        build)
    _remove("${SRC}/${p}")
endforeach()
file(GLOB _junk "${SRC}/gTest_*.cmake" "${SRC}/cmake_test_discovery_*.json")
foreach(p IN LISTS _junk)
    _remove("${p}")
endforeach()

#---------- Submodules: CMake files generated in their sources by the in-source build
foreach(sub googletest Minuit2 SuperNOVAS)
    set(_dir "${SRC}/${sub}")
    if(NOT IS_DIRECTORY "${_dir}")
        continue()
    endif()
    file(GLOB_RECURSE _files LIST_DIRECTORIES false
        "${_dir}/Makefile" "${_dir}/cmake_install.cmake" "${_dir}/CTestTestfile.cmake")
    foreach(f IN LISTS _files)
        if(NOT f MATCHES "/CMakeFiles/")
            _restore_or_remove("${_dir}" "${f}")
        endif()
    endforeach()
    file(GLOB_RECURSE _dirs LIST_DIRECTORIES true "${_dir}/*CMakeFiles")
    foreach(d IN LISTS _dirs)
        if(IS_DIRECTORY "${d}" AND d MATCHES "/CMakeFiles$" AND NOT d MATCHES "/CMakeFiles/")
            _remove("${d}")
        endif()
    endforeach()
endforeach()
foreach(p
        googletest/googletest/generated
        Minuit2/Minuit2Config.cmake Minuit2/Minuit2ConfigVersion.cmake Minuit2/Minuit2Targets.cmake
        SuperNOVAS/cmake_uninstall.cmake SuperNOVAS/supernovas.pc
        SuperNOVAS/supernovasConfig.cmake SuperNOVAS/supernovasConfigVersion.cmake SuperNOVAS/supernovasTargets.cmake)
    get_filename_component(_parent "${SRC}/${p}" DIRECTORY)
    _restore_or_remove("${_parent}" "${SRC}/${p}")
endforeach()

message(STATUS "${_removed} in-source build product(s) removed from ${SRC}")
message(STATUS "Now: cmake --preset default && cmake --build build -j && cmake --build build --target install")

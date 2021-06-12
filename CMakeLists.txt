cmake_minimum_required (VERSION 3.1.0)

project(base-askapparallel VERSION 0.0.1 LANGUAGES C CXX)


if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/askap-cmake)
    execute_process(COMMAND git pull 
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/askap-cmake)
else ()	
    execute_process(COMMAND git clone https://bitbucket.csiro.au/scm/askapsdp/askap-cmake.git
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/askap-cmake")

configure_file(askap_askapparallel.h.in askap_askapparallel.h)

option (CXX11 "Compile as C++11 if possible" YES)
option (ENABLE_SHARED "Build shared libraries" YES)
option (ENABLE_RPATH "Include rpath in executables and shared libraries" YES)

# YandaSoft dependencies .. in order
include(install_yanda_depends)

set (EXTERNAL_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
find_package(LofarBlob)
if (NOT LofarBlob_FOUND)
	install_yanda_depends(lofar-blob)
	find_package(LofarBlob REQUIRED)
endif ()

find_package(Askap)
if (NOT ASKAP_FOUND)
	install_yanda_depends(base-askap)
	find_package(Askap REQUIRED)
endif ()

find_package(CPPUnit)


# find packages

find_package(log4cxx REQUIRED)
find_package(Casacore REQUIRED COMPONENTS  ms images mirlib coordinates fits lattices measures scimath scimath_f tables casa)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

if (ENABLE_SHARED)
option (BUILD_SHARED_LIBS "" YES)
    if (ENABLE_RPATH)
        # Set RPATH to use for installed targets; append linker search path
        set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" )
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    endif (ENABLE_RPATH)
endif(ENABLE_SHARED)


find_package(MPI)
include_directories(${MPI_INCLUDE_PATH})

if (MPI_FOUND)
	add_definitions(-DHAVE_MPI)
endif()

# include directories
include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories (${Boost_INCLUDE_DIRS})
add_definitions(-DHAVE_BOOST)
include_directories(${log4cxx_INCLUDE_DIRS})
add_definitions(-DHAVE_LOG4CXX)
include_directories(${ASKAP_INCLUDE_DIRS})
include_directories(${LofarBlob_INCLUDE_DIRS})
include_directories(${CASACORE_INCLUDE_DIRS})
add_definitions(-DHAVE_AIPSPP)
add_definitions(-Dcasa=casacore)

if (CASACORE3 OR CXX11)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set (_modules askapparallel)

# add some more tests and sub-directories

foreach (module ${_modules})
	add_subdirectory(askap/${module})
endforeach (module)

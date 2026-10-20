set(CMAKE_CXX_STANDARD 17)

# define build type
IF( DEFINED CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ELSE()
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ENDIF()

# In CMake 4, 3.10 is deprecated and 3.5 has been removed.
# use 3.1...3.10 so all of these versions are acceptable as min. version.
# https://cmake.org/cmake/help/latest/command/cmake_minimum_required.html
cmake_minimum_required(VERSION 3.1...3.10)

project(CSXCAD)

if (NOT WIN32)
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# default
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 7)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION_STRING ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})

set(VERSION "v${LIB_VERSION_STRING}")

IF(EXISTS ${PROJECT_SOURCE_DIR}/localConfig.cmake)
   include(${PROJECT_SOURCE_DIR}/localConfig.cmake)
ENDIF()

# add git revision
IF(EXISTS ${PROJECT_SOURCE_DIR}/.git )
  FIND_PACKAGE(Git)
  # Get the latest abbreviated commit hash of the working branch
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GITREV
  )
  set(VERSION ${GITREV})
  string(STRIP ${VERSION} VERSION)
  message(STATUS "Found Git repository, ${PROJECT_NAME} version tag: ${VERSION}")
ENDIF()

ADD_DEFINITIONS(-DGIT_VERSION=\"${VERSION}\")

if (WIN32)
    ADD_DEFINITIONS( -DBUILD_CSXCAD_LIB )
endif (WIN32)

#
# Set up RPATH for the project
#
option(ENABLE_RPATH "Enable rpath support on Linux and Mac" ON)
if(NOT CMAKE_INSTALL_RPATH)
  # the RPATH to be used when installing, but only if it's not a system directory
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
  ENDIF("${isSystemDir}" STREQUAL "-1")
endif()
if(APPLE AND NOT CMAKE_INSTALL_NAME_DIR)
  set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
endif()
if(UNIX AND ENABLE_RPATH)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Set locations of CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CSXCAD_SOURCE_DIR}/cmake/Modules/")

# fparser
# $ cmake -D FPARSER_ROOT_DIR=~/opt/openEMS .
IF(NOT EXISTS ${FPARSER_ROOT_DIR})
    SET(FPARSER_ROOT_DIR /usr)
ENDIF()
message(STATUS "FPARSER_ROOT_DIR: ${FPARSER_ROOT_DIR}" )
find_library(fparser_LIBRARIES
  NAMES fparser
  HINTS ${FPARSER_ROOT_DIR}/lib${LIB_SUFFIX}
  NO_CMAKE_FIND_ROOT_PATH
message(STATUS "fparser_LIBRARIES: ${fparser_LIBRARIES}" )
)
find_path(fparser_INCLUDE_DIR
  NAMES fparser.hh
  HINTS ${FPARSER_ROOT_DIR}/include ${FPARSER_ROOT_DIR}/include/fparser
  NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "fparser_INCLUDE_DIR: ${fparser_INCLUDE_DIR}" )

#TODO test if fparser was found
INCLUDE_DIRECTORIES( ${fparser_INCLUDE_DIR} ) 

message(STATUS "TinyXML_ROOT_DIR ${TinyXML_ROOT_DIR}")
# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS( -DTIXML_USE_STL )
message(STATUS "TinyXML_INCLUDE_DIR: ${TinyXML_INCLUDE_DIR}")
message(STATUS "TinyXML_LIBRARY: ${TinyXML_LIBRARY}")

INCLUDE_DIRECTORIES( ${TinyXML_INCLUDE_DIR} )

find_package(HDF5 COMPONENTS C HL REQUIRED)

INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIRS})
link_directories(${HDF5_LIBRARY_DIRS})
# hdf5 compat
ADD_DEFINITIONS( -DH5_USE_16_API )
#ADD_DEFINITIONS( -DH5_BUILT_AS_DYNAMIC_LIB )

# message(status "hdf5 all libs: ${HDF5_LIBRARIES}")

# depend on fparser.hh
ADD_SUBDIRECTORY( src )

# C++ unit tests. Off by default: they are of no use to somebody installing
# CSXCAD, and building them needs nothing the library does not need anyway.
# Enable with -DCSXCAD_BUILD_TESTS=ON, then run them with `ctest`.
option(CSXCAD_BUILD_TESTS "Build the C++ unit tests in tests/" OFF)
if (CSXCAD_BUILD_TESTS)
  enable_testing()
  ADD_SUBDIRECTORY( tests )
endif()

INSTALL(DIRECTORY matlab DESTINATION share/CSXCAD)

#TODO tarball, debug, release, doxygen

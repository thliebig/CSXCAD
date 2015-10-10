
# define build type
IF( DEFINED CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ELSE()
  SET( CMAKE_BUILD_TYPE Release CACHE STRING "Set to either \"Release\" or \"Debug\"" )
ENDIF()

PROJECT(CSXCAD CXX)

cmake_minimum_required(VERSION 2.8)

# default
set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 6)
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
    COMMAND ${GIT_EXECUTABLE} describe --tags
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
find_library(fparser_LIBRARIES 
  NAMES fparser
  HINTS  ${FPARSER_ROOT_DIR}/lib
  NO_CMAKE_FIND_ROOT_PATH
)
message(STATUS "fparser: ${fparser_LIBRARIES}" )
#TODO test if fparser was found
INCLUDE_DIRECTORIES( ${FPARSER_ROOT_DIR}/include ) 

# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS(  -DTIXML_USE_STL )

find_package(HDF5 1.8 COMPONENTS C HL REQUIRED)
INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})
link_directories(${HDF5_LIBRARY_DIRS})
# hdf5 compat
ADD_DEFINITIONS( -DH5_USE_16_API )

# message(status "hdf5 all libs: ${HDF5_LIBRARIES}")


find_package(CGAL REQUIRED)
INCLUDE_DIRECTORIES (${CGAL_INCLUDE_DIR})

# cgal needs, fail o Linux otherwise
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-frounding-math" )
endif()

# TODO what are the needed libs?
find_package(Boost 1.46 COMPONENTS
  thread
  system
  date_time
  serialization
  chrono
)

# vtk
if (WIN32)
    find_package(VTK 6.1 REQUIRED)
else()
    # prefer >=6.1, fallback to >=5.4
    find_package(VTK REQUIRED)
endif()

message(STATUS "Found package VTK. Using version " ${VTK_VERSION})
include(${VTK_USE_FILE})
INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIR})
if("${VTK_MAJOR_VERSION}" EQUAL 6)
    set( vtk_LIBS
        vtkCommonCore
        vtkCommonDataModel
        vtkIOLegacy
        vtkIOXML
        vtkIOGeometry
        vtkIOPLY
        vtksys
        vtkIOCore
    )
else()
    set( vtk_LIBS
        vtkCommon
        vtkFiltering
        vtkGraphics
        vtkHybrid
        vtkIO
        vtkRendering
        vtkWidgets
        QVTK
    )    
endif()
# depend on fparser.hh
ADD_SUBDIRECTORY( src )

INSTALL(DIRECTORY matlab DESTINATION share/CSXCAD)

#TODO tarball, debug, release, doxygen

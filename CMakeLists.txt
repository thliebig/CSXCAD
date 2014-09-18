
PROJECT(CSXCAD CXX)

cmake_minimum_required(VERSION 2.8)

# default
set(VERSION "v0.5.2")

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


# Set locations of CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CSXCAD_SOURCE_DIR}/cmake/Modules/")

# TinyXML module from https://github.com/ros/cmake_modules
find_package(TinyXML REQUIRED)
ADD_DEFINITIONS(  -DTIXML_USE_STL )

find_package(HDF5 1.8 REQUIRED)
INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})
# hdf5 compat
ADD_DEFINITIONS( -DH5_USE_16_API )

#message(status "hdf5 all libs: ${HDF5_LIBRARIES}")

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

find_package(VTK REQUIRED)

if("${VTK_MAJOR_VERSION}" EQUAL 5)
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
endif()

if(VTK_FOUND)
	message(STATUS "Found package VTK. Using " ${VTK_USE_FILE})
    include(${VTK_USE_FILE})
else(VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please set VTK_DIR.")
endif(VTK_FOUND)

INCLUDE_DIRECTORIES (${VTK_INCLUDE_DIR})

#message(status "VTK selected: ${vtk_LIBS}")


INCLUDE_DIRECTORIES( ${fparser_SOURCE_DIR} )


# depend on fparser.hh
ADD_SUBDIRECTORY( src )

INSTALL(DIRECTORY matlab DESTINATION share/CSXCAD/)

#TODO tarball, debug, release, doxygen

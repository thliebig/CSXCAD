#
#	Copyright (C) 2010,2011 Thorsten Liebig (Thorsten.Liebig@gmx.de)
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU Lesser General Public License as published
#	by the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CONFIG -= qt
CONFIG += rtti exceptions shared
TEMPLATE = lib
INCLUDEPATH += .
OBJECTS_DIR = obj

VERSION = 0.6.0

# add git revision
GITREV = $$system(git describe --tags)
isEmpty(GITREV):GITREV=$$VERSION
DEFINES += GIT_VERSION=\\\"$$GITREV\\\"

# remove unnecessary webkit define
DEFINES -= QT_WEBKIT

exists(localPaths.pri) {
    include(localPaths.pri)
}

unix:!macx { 
    LIBS += -ltinyxml
    DEFINES += TIXML_USE_STL
    LIBS += -lhdf5_hl -lhdf5
    LIBS += -lCGAL

    #vtk
    isEmpty(VTK_INCLUDEPATH) {
        INCLUDEPATH += /usr/include/vtk-5.2 \
        /usr/include/vtk-5.4 \
        /usr/include/vtk-5.6 \
        /usr/include/vtk-5.8 \
        /usr/include/vtk-5.10 \
        /usr/include/vtk-6.0 \
        /usr/include/vtk-6.1 \
        /usr/include/vtk-6.2 \
        /usr/include/vtk-6.3 \
        /usr/include/vtk
    } else {
        INCLUDEPATH += $$VTK_INCLUDEPATH
    }

    #hdf5
    !isEmpty(HDF5_ROOT) {LIBS += -L$$HDF5_ROOT}
    INCLUDEPATH += /usr/include/hdf5/serial
    INCLUDEPATH += /usr/include/mpi

    #fparser
    isEmpty(FPARSER_ROOT) {
        FPARSER_ROOT = /usr
    } else {
        INCLUDEPATH += $$FPARSER_ROOT/include
        LIBS += -L$$FPARSER_ROOT/lib
        QMAKE_LFLAGS += \'-Wl,-rpath,$$FPARSER_ROOT/lib\'
    }
    LIBS += -lfparser
}

win32 { 
    DEFINES += BUILD_CSXCAD_LIB

    isEmpty(WIN32_LIB_ROOT) {
        WIN32_LIB_ROOT = ..
    }

    # #3rd party libraries#
    # fparser
    INCLUDEPATH += $$WIN32_LIB_ROOT/fparser/include
    LIBS += -L$$WIN32_LIB_ROOT/fparser/bin -lfparser4
    # tinyxml
    INCLUDEPATH += $$WIN32_LIB_ROOT/tinyxml/include
    LIBS += -L$$WIN32_LIB_ROOT/tinyxml/bin -ltinyxml2
    DEFINES += TIXML_USE_STL
    # hdf5
    INCLUDEPATH += $$WIN32_LIB_ROOT/hdf5/include
    LIBS += -L$$WIN32_LIB_ROOT/hdf5/lib -lhdf5_hl -lhdf5
    # zlib
    LIBS += -L$$WIN32_LIB_ROOT/zlib/lib -lz
    # vtk
    INCLUDEPATH += $$WIN32_LIB_ROOT/vtk/include/vtk-5.10
    LIBS += -L$$WIN32_LIB_ROOT/vtk/bin -lvtkCommon -lvtkIO -lvtkFiltering
    #boost, needed by cgal
    INCLUDEPATH += $$WIN32_LIB_ROOT/boost/include
    LIBS += -L$$WIN32_LIB_ROOT/boost/lib -lboost_thread -lboost_system -lboost_date_time -lboost_serialization
    #cgal
    INCLUDEPATH += $$WIN32_LIB_ROOT/cgal/include
    LIBS += -L$$WIN32_LIB_ROOT/cgal/bin -lCGAL
}

macx {
    LIBS += -ltinyxml
    DEFINES += TIXML_USE_STL
    LIBS += -lhdf5_hl -lhdf5
    LIBS += -lCGAL

    #vtk5 provided with Homebrew
    isEmpty(VTK_INCLUDEPATH) {
        INCLUDEPATH += \
          /usr/local/opt/vtk5/include  \
          /usr/local/opt/vtk5/include/vtk-5.10

    } else {
        INCLUDEPATH += $$VTK_INCLUDEPATH
    }

    LIBS += -L/usr/local/opt/vtk5/lib/vtk-5.10  -lvtkCommon  -lvtkIO -lvtkFiltering
    LIBS += -L/usr/local/lib -lboost_thread-mt -lboost_system -lboost_date_time -lboost_serialization

    #hdf5 (is this MPI?)
    INCLUDEPATH += /usr/local/opt/hd5/include

    #fparser
    #isEmpty(FPARSER_ROOT) {
    #    FPARSER_ROOT = /usr
    #} else {
        INCLUDEPATH += $$FPARSER_ROOT/include
        LIBS += -L$$FPARSER_ROOT/lib
        QMAKE_LFLAGS += \'-Wl,-rpath,$$FPARSER_ROOT/lib\'
    #}
    LIBS += -lfparser
}

# vtk includes deprecated header files; silence the corresponding warning
QMAKE_CXXFLAGS += -Wno-deprecated -frounding-math

# enable this flag to use with valgrind! valgrind emulates a buggy FPU
# QMAKE_CXXFLAGS += -DCGAL_DISABLE_ROUNDING_MATH_CHECK=ON

# Input
PUB_HEADERS += src/ContinuousStructure.h \
    src/CSPrimitives.h \
    src/CSProperties.h \
    src/CSRectGrid.h \
    src/CSXCAD_Global.h \
    src/ParameterObjects.h \
    src/CSFunctionParser.h \
    src/CSUseful.h \
    src/ParameterCoord.h \
    src/CSTransform.h \
    src/CSBackgroundMaterial.h \
    src/CSPrimPoint.h \
    src/CSPrimBox.h \
    src/CSPrimMultiBox.h \
    src/CSPrimSphere.h \
    src/CSPrimSphericalShell.h \
    src/CSPrimCylinder.h \
    src/CSPrimCylindricalShell.h \
    src/CSPrimPolygon.h \
    src/CSPrimLinPoly.h \
    src/CSPrimRotPoly.h \
    src/CSPrimPolyhedron.h \
    src/CSPrimPolyhedronReader.h \
    src/CSPrimCurve.h \
    src/CSPrimWire.h \
    src/CSPrimUserDefined.h \
    src/CSPropUnknown.h \
    src/CSPropMaterial.h \
    src/CSPropDispersiveMaterial.h \
    src/CSPropLorentzMaterial.h \
    src/CSPropDebyeMaterial.h \
    src/CSPropDiscMaterial.h \
    src/CSPropLumpedElement.h \
    src/CSPropMetal.h \
    src/CSPropConductingSheet.h \
    src/CSPropExcitation.h \
    src/CSPropProbeBox.h \
    src/CSPropDumpBox.h \
    src/CSPropResBox.h

HEADERS += $$PUB_HEADERS \
    src/CSPrimPolyhedron_p.h

SOURCES += src/ContinuousStructure.cpp \
    src/CSPrimitives.cpp \
    src/CSProperties.cpp \
    src/CSRectGrid.cpp \
    src/ParameterObjects.cpp \
    src/CSFunctionParser.cpp \
    src/CSUseful.cpp \
    src/ParameterCoord.cpp \
    src/CSTransform.cpp \
    src/CSPrimPoint.cpp \
    src/CSPrimBox.cpp \
    src/CSPrimMultiBox.cpp \
    src/CSPrimSphere.cpp \
    src/CSPrimSphericalShell.cpp \
    src/CSPrimCylinder.cpp \
    src/CSPrimCylindricalShell.cpp \
    src/CSPrimPolygon.cpp \
    src/CSPrimLinPoly.cpp \
    src/CSPrimRotPoly.cpp \
    src/CSPrimPolyhedron.cpp \
    src/CSPrimPolyhedronReader.cpp \
    src/CSPrimCurve.cpp \
    src/CSPrimWire.cpp \
    src/CSPrimUserDefined.cpp \
    src/CSPropUnknown.cpp \
    src/CSPropMaterial.cpp \
    src/CSPropDispersiveMaterial.cpp \
    src/CSPropLorentzMaterial.cpp \
    src/CSPropDebyeMaterial.cpp \
    src/CSPropDiscMaterial.cpp \
    src/CSPropLumpedElement.cpp \
    src/CSPropMetal.cpp \
    src/CSPropConductingSheet.cpp \
    src/CSPropExcitation.cpp \
    src/CSPropProbeBox.cpp \
    src/CSPropDumpBox.cpp \
    src/CSPropResBox.cpp \
    src/CSBackgroundMaterial.cpp

#
# create tar file
#
tarball.target = tarball
tarball.commands = git archive --format=tar --prefix=CSXCAD-$$VERSION/ HEAD | bzip2 > CSXCAD-$${VERSION}.tar.bz2
QMAKE_EXTRA_TARGETS += tarball

#
# INSTALL
#
isEmpty(PREFIX) {
    PREFIX = /usr/local
}
install.target = install
install.commands = mkdir -p \"$$PREFIX/lib$$LIB_SUFFIX\"
install.commands += && mkdir -p \"$$PREFIX/include/CSXCAD\"
install.commands += && mkdir -p \"$$PREFIX/share/CSXCAD/matlab\"
install.commands += && cp $$PUB_HEADERS \"$$PREFIX/include/CSXCAD/\"
unix:!macx:install.commands += && cp -at \"$$PREFIX/lib$$LIB_SUFFIX/\" libCSXCAD.so*
win32:install.commands += && cp -at \"$$PREFIX/lib$$LIB_SUFFIX/\" release/CSXCAD0.dll
macx:install.commands += && cp  libCSXCAD*.dylib \"$$PREFIX/lib$$LIB_SUFFIX/\"
install.commands += && cp -r matlab/* \"$$PREFIX/share/CSXCAD/matlab\"
QMAKE_EXTRA_TARGETS += install

#
# create .PHONY target
#
phony.target = .PHONY
phony.depends = $$QMAKE_EXTRA_TARGETS
QMAKE_EXTRA_TARGETS += phony

#
# create documentation
# use
#  make docs
# to start doxygen. It will generate html, latex and compressed help for QtCreator
#
include(doc.pri)

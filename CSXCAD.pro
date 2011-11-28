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
TEMPLATE = lib
INCLUDEPATH += .
INCLUDEPATH += ../tinyxml
INCLUDEPATH += ../fparser
OBJECTS_DIR = obj

CONFIG-=qt

# add git revision
QMAKE_CXXFLAGS += -DGIT_VERSION=\\\"`git describe --tags`\\\"

unix { 
	VERSION = 0.2.4
    LIBS += -L../fparser \
        -lfparser
    LIBS += -L../tinyxml \
        -ltinyxml
    LIBS += -lhdf5_cpp -lhdf5

#vtk
	INCLUDEPATH += /usr/include/vtk-5.2 \
		/usr/include/vtk-5.4 \
		/usr/include/vtk-5.6
}
win32 { 
    DEFINES = BUILD_CSXCAD_LIB
    INCLUDEPATH += ../tinyxml
    INCLUDEPATH += ../fparser
	LIBS += ..\fparser\release\libfparser4.a \
		..\tinyxml\release\libtinyxml2.a
	INCLUDEPATH += ../hdf5/include ../hdf5/include/cpp ../boost/include/boost-1_42
	LIBS +=  ../hdf5/lib/hdf5_cpp.lib ../hdf5/lib/hdf5.lib
### vtk ###
	 INCLUDEPATH += ..\vtk ..\vtk\Common
	 LIBS +=..\vtk\bin\libvtkCommon.dll
}

# Input
HEADERS += ContinuousStructure.h \
    CSPrimitives.h \
    CSProperties.h \
    CSRectGrid.h \
    CSXCAD_Global.h \
    ParameterObjects.h \
    CSFunctionParser.h \
    CSUseful.h \
    ParameterCoord.h \
    CSTransform.h
SOURCES += ContinuousStructure.cpp \
    CSPrimitives.cpp \
    CSProperties.cpp \
    CSRectGrid.cpp \
    ParameterObjects.cpp \
    CSFunctionParser.cpp \
    CSUseful.cpp \
    ParameterCoord.cpp \
    CSTransform.cpp

ABI2 {
        CONFIG-=debug debug_and_release
        CONFIG+=release
        QMAKE_CFLAGS_RELEASE=-O2 -fabi-version=2
        QMAKE_CXXFLAGS_RELEASE=-O2 -fabi-version=2
        QMAKE_CC = apgcc
        QMAKE_CXX = apg++
        QMAKE_LINK = apg++
        QMAKE_LINK_SHLIB = apg++
        QMAKE_LFLAGS_RPATH =
}

bits64 {
        QMAKE_CXXFLAGS_RELEASE+=-m64 -march=athlon64
        QMAKE_LFLAGS_RELEASE+=-m64 -march=athlon64
        OBJECTS_DIR = ABI2-64
	LIBS = ../fparser/ABI2-64/libfparser.so
	LIBS += ../tinyxml/ABI2-64/libtinyxml.so
	LIBS += ../hdf5-64/lib/libhdf5.so
	LIBS += ../hdf5-64/lib/libhdf5_cpp.so
	INCLUDEPATH += ../hdf5-64/include
}

bits32 {
        QMAKE_CXXFLAGS_RELEASE+=-m32 -march=i686
        QMAKE_LFLAGS_RELEASE+=-m32 -march=i686
        OBJECTS_DIR = ABI2-32
	LIBS = ../fparser/ABI2-32/libfparser.so
	LIBS += ../tinyxml/ABI2-32/libtinyxml.so
	LIBS += ../hdf5-32/lib/libhdf5.so
	LIBS += ../hdf5-32/lib/libhdf5_cpp.so
	INCLUDEPATH += ../hdf5-32/include
}

ABI2 {
        DESTDIR = $$OBJECTS_DIR
        MOC_DIR = $$OBJECTS_DIR
        UI_DIR = $$OBJECTS_DIR
        RCC_DIR = $$OBJECTS_DIR
}

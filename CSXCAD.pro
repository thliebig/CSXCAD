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
INCLUDEPATH += ../fparser
OBJECTS_DIR = obj

# add git revision
GITREV = $$system(git describe --tags)
DEFINES += GIT_VERSION=\\\"$$GITREV\\\"

VERSION = 0.2.5

unix { 
    LIBS += -L../fparser \
        -lfparser
    LIBS += -ltinyxml
    LIBS += -lhdf5_cpp -lhdf5

#vtk
	INCLUDEPATH += /usr/include/vtk-5.2 \
		/usr/include/vtk-5.4 \
		/usr/include/vtk-5.6 \
		/usr/include/vtk-5.8 \
		/usr/include/vtk
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






#
# create tar file
#
tarball.target = tarball
tarball.commands = git archive --format=tar --prefix=CSXCAD-$$VERSION/ HEAD | bzip2 > CSXCAD-$${VERSION}.tar.bz2
QMAKE_EXTRA_TARGETS += tarball


#
# INSTALL
#
install.target = install
install.commands = mkdir -p \"$(INSTALL_ROOT)/usr/lib$$LIB_SUFFIX\"
install.commands += && mkdir -p \"$(INSTALL_ROOT)/usr/include/CSXCAD\"
install.commands += && mkdir -p \"$(INSTALL_ROOT)/usr/share/CSXCAD/matlab\"
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/include/CSXCAD/\" $$HEADERS
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/lib$$LIB_SUFFIX/\" libCSXCAD.so*
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/share/CSXCAD/matlab/\" matlab/*.m
QMAKE_EXTRA_TARGETS += install


#
# create .PHONY target
#
phony.target = .PHONY
phony.depends = $$QMAKE_EXTRA_TARGETS
QMAKE_EXTRA_TARGETS += phony

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
OBJECTS_DIR = obj

VERSION = 0.3.0

# add git revision
GITREV = $$system(git describe --tags)
isEmpty(GITREV):GITREV=$$VERSION
DEFINES += GIT_VERSION=\\\"$$GITREV\\\"

unix { 
    LIBS += -lfparser
    LIBS += -ltinyxml
    LIBS += -lhdf5_cpp -lhdf5

#vtk
    INCLUDEPATH += /usr/include/vtk-5.2 \
        /usr/include/vtk-5.4 \
        /usr/include/vtk-5.6 \
        /usr/include/vtk-5.8 \
        /usr/include/vtk-5.10 \
        /usr/include/vtk
}

win32 { 
    DEFINES += BUILD_CSXCAD_LIB

    # #3rd party libraries#
    WIN32_LIB_ROOT = ..
    # fparser
    INCLUDEPATH += $$WIN32_LIB_ROOT/fparser/include
    LIBS += -L$$WIN32_LIB_ROOT/fparser/bin -lfparser4
    # tinyxml
    INCLUDEPATH += $$WIN32_LIB_ROOT/tinyxml/include
    LIBS += -L$$WIN32_LIB_ROOT/tinyxml/bin -ltinyxml2
    # hdf5
    INCLUDEPATH += $$WIN32_LIB_ROOT/hdf5/include
    LIBS += -L$$WIN32_LIB_ROOT/hdf5/lib -lhdf5_cpp -lhdf5
    # vtk
    INCLUDEPATH += $$WIN32_LIB_ROOT/vtk/include/vtk-5.10
    LIBS += -L$$WIN32_LIB_ROOT/vtk/bin -lvtkCommon
}

# vtk includes deprecated header files; silence the corresponding warning
QMAKE_CXXFLAGS += -Wno-deprecated


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
install.commands += && cp -at \"$(INSTALL_ROOT)/usr/share/CSXCAD/\" matlab/
QMAKE_EXTRA_TARGETS += install

#
# create .PHONY target
#
phony.target = .PHONY
phony.depends = $$QMAKE_EXTRA_TARGETS
QMAKE_EXTRA_TARGETS += phony

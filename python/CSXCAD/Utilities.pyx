# -*- coding: utf-8 -*-
#
# Copyright (C) 2015,20016 Thorsten Liebig (Thorsten.Liebig@gmx.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

def CheckNyDir(ny):
    """ CheckNyDir(ny)

    Translate directions like 'x'/'y' or 'z' into 0/1 or 2.
    Raise an assertion error otherwise.

    :param ny: int or str
    :returns: int -- direction as 0/1/2
    """
    if ny in [0, 1, 2]:
        return ny
    elif ny in ['x','y','z']:
        return ['x','y','z'].index(ny)
    elif ny in ['r','a','z']:
        return ['r','a','z'].index(ny)
    else:
        raise Exception('CheckNyDir: invalid direction: "{}"'.format(ny))

def GetMultiDirs(dirs):
    assert type(dirs)==str, 'GetMultiDirs: dirs must be of type str'
    dirs = ''.join(sorted(dirs))
    if dirs == 'all' or dirs == 'xyz' or dirs == 'raz':
        return [0, 1, 2]
    elif dirs == 'x' or dirs == 'r':
        return [0,]
    elif dirs == 'y' or dirs == 'a':
        return [1,]
    elif dirs == 'z':
        return [2,]
    elif dirs == 'xy' or dirs == 'ra':
        return [0, 1]
    elif dirs == 'yz' or dirs == 'az':
        return [1, 2]
    elif dirs == 'xz' or dirs == 'rz':
        return [0, 2]

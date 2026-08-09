# -*- coding: utf-8 -*-
#
# Copyright (C) 2026 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

# Destruction notification for CSXCAD objects, see CSXCAD/CSObject.h
#
# The wrapper classes here only hold a borrowed pointer, the C++ instance is
# owned by e.g. the ContinuousStructure. Every wrapper therefore registers a
# callback, which clears its `thisptr` once the C++ instance is gone, and keeps
# a reference to the wrapper of its owner for as long as C++ says it owns it.

cdef extern from "CSXCAD/CSObject.h":
    # no `noexcept`: that is cython 3 syntax, and this has to build with the
    # cython 0.28 of the oldest distributions in CI. The callback cannot raise
    # anyway, it touches nothing but a pointer. \sa wrapper_destroyed
    ctypedef void (*CSDestructionCallback)(void* obj, void* user_data) nogil

    cdef enum ObjectKind "CSObject::ObjectKind":
        STRUCTURE          "CSObject::STRUCTURE"
        PROPERTY           "CSObject::PROPERTY"
        PRIMITIVE          "CSObject::PRIMITIVE"
        TRANSFORM          "CSObject::TRANSFORM"
        GRID               "CSObject::GRID"
        PARAMETERSET       "CSObject::PARAMETERSET"
        BACKGROUNDMATERIAL "CSObject::BACKGROUNDMATERIAL"

    cdef cppclass _CSObject "CSObject":
        void SetDestructionCallback(CSDestructionCallback cb, void* user_data)
        ObjectKind GetObjectKind()
        _CSObject* GetOwner()
        void SetOwner(_CSObject* owner)
        bint IsOwned()

from libc.stdint cimport uintptr_t

cdef inline void wrapper_destroyed(void* obj, void* thisptr_slot) nogil:
    """ Callback for SetDestructionCallback(), clearing a wrapper's `thisptr`.

    `thisptr_slot` is the address of the wrapper's `thisptr` field, passed as the
    `user_data` of SetDestructionCallback(). Storing NULL there is all this does,
    and deliberately so: it needs no GIL, no python object and no module state,
    because it runs whenever C++ destroys the object -- on any thread, and during
    interpreter shutdown, when python state is being torn down around us and
    anything more ambitious would silently fail and leave a dangling wrapper.

    The slot stays valid because a wrapper unregisters in its own __dealloc__,
    so this cannot fire for a wrapper that is gone. The `_instances` entry is
    left to the weak reference, which drops it when the wrapper dies; until then
    the entry holds an invalidated wrapper that fromPtr() must not hand out.
    \\sa CSProperties.fromPtr
    """
    (<void**>thisptr_slot)[0] = NULL

cdef inline object resolve_owner(_CSObject* obj):
    """ Wrapper of whoever owns `obj`, or None if nobody does.

    The owner and its kind come from C++, so no accessor handing out an object
    has to know or pass who owns it. \sa CSObject::GetOwner
    """
    cdef _CSObject* owner = obj.GetOwner()
    if owner == NULL:
        return None
    from CSXCAD.Utilities import WrapperFromAddress
    return WrapperFromAddress(<uintptr_t>owner, <int>owner.GetObjectKind())

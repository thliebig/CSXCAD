/*
*	Copyright (C) 2026 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CSOBJECT_H
#define CSOBJECT_H

#include <stddef.h>
#include "CSXCAD_Global.h"

//! Callback type for CSObject::SetDestructionCallback
typedef void (*CSDestructionCallback)(void* obj, void* user_data);

//! Base class for CSXCAD objects whose lifetime is coordinated with an external owner.
/*!
  Most CSXCAD objects are owned by another object: primitives by their property,
  properties, grid and background material by the ContinuousStructure. The owner
  may destroy them at any time, while something outside still holds a borrowed
  pointer to them -- typically a language binding, which keeps one wrapper object
  per C++ instance. A destruction callback lets whoever holds such a pointer be
  told, whichever code path destroys the object, including a plain delete in a
  C++ application.

  ~CSObject() invokes the callback by default, so no derived class can forget
  about it. A destructor that tears down other objects which notify as well may
  call NotifyDestruction() itself, to be notified before its content is gone; the
  callback runs at most once, so the default call then does nothing.

  For the same reason CSObject knows whether somebody else will destroy it, and
  who: the owner. A binding needs that to keep the owner's wrapper alive as long
  as it hands out wrappers for the owned objects, and to know whether it may
  destroy an object itself. Reconstructing that on the binding side is guesswork.
  GetOwner() is NULL for an object nobody owns yet.

  Keep this class limited to lifetime coordination: naming, serialisation, IDs
  and the like belong to the individual classes, not here.

  Notes
  -----
  - Not thread safe per object, like the rest of CSXCAD. Independent objects are
    unaffected, the state is per object.
  - Registering a second callback replaces the first, passing NULL unregisters.
  - A callback must not delete the object it is called for, it is already being
    destroyed.
  - Copies inherit neither the callback nor the ownership: a copy is a different
    object, which neither the owner of the callback nor the owner knows about.
*/
class CSXCAD_EXPORT CSObject
{
public:
	typedef CSDestructionCallback Callback;

	//! Set the callback invoked at the start of this object's destructor.
	/*!
	  \param cb        callback to invoke, or NULL to unregister
	  \param user_data passed back to the callback unchanged
	*/
	void SetDestructionCallback(CSDestructionCallback cb, void* user_data)
	{
		m_destr_cb = cb;
		m_destr_user = user_data;
	}

	//! Which family of CSXCAD object this is. \sa GetObjectKind
	/*!
	  A discriminator, not a domain type and not a name: it exists so that an
	  owner obtained through GetOwner() can be resolved to the right wrapper in a
	  language binding. The domain type of a property or primitive is its own
	  GetType(). Pure virtual, so a new class deriving from CSObject cannot
	  forget to identify itself.
	*/
	enum ObjectKind
	{
		STRUCTURE, PROPERTY, PRIMITIVE, TRANSFORM, GRID, PARAMETERSET, BACKGROUNDMATERIAL
	};

	//! Which family of CSXCAD object this is, \sa ObjectKind
	virtual ObjectKind GetObjectKind() const = 0;

	//! Get the object that owns, and will destroy, this one. NULL if unowned.
	CSObject* GetOwner() const {return m_owner;}
	//! Set the owner, NULL when ownership is given up again. \sa GetOwner
	void SetOwner(CSObject* owner) {m_owner = owner;}

	//! True if somebody else will destroy this object. \sa SetOwnedExternally
	/*!
	  Either a CSXCAD owner, or code outside CSXCAD that claimed it. A language
	  binding must not destroy an object for which this is true, even one it
	  created itself.
	*/
	bool IsOwned() const {return (m_owner != NULL) || m_owned_externally;}

	//! Claim ownership from outside CSXCAD, e.g. a solver taking over a structure.
	/*!
	  Nobody can verify the claim: whoever sets it is responsible for destroying
	  the object. Claim it where ownership is actually taken, then every binding
	  sees it without having to be told separately. \sa IsOwned
	*/
	void SetOwnedExternally(bool val) {m_owned_externally = val;}

protected:
	CSObject() : m_destr_cb(NULL), m_destr_user(NULL), m_owner(NULL), m_owned_externally(false) {}
	//! Copies inherit neither the callback nor the ownership, see class documentation
	CSObject(const CSObject&) : m_destr_cb(NULL), m_destr_user(NULL), m_owner(NULL), m_owned_externally(false) {}
	CSObject& operator=(const CSObject&) {return *this;}
	//! Protected: never delete an object through a CSObject pointer.
	//! Virtual and defined out of line on purpose: a key function has to be
	//! virtual, non-pure and non-inline, and being one anchors the vtable and
	//! typeinfo in CSObject.cpp instead of emitting them into every translation
	//! unit that uses a derived class. What keeps CSObject the primary base, so
	//! that \a this is the canonical address, is that the class is polymorphic at
	//! all, which the pure virtual GetObjectKind() already guarantees.
	virtual ~CSObject();

	//! Invoke the registered callback, if it was not invoked already.
	/*! Always pass plain \a this. \sa class documentation */
	void NotifyDestruction(void* obj);

private:
	CSDestructionCallback m_destr_cb;
	void*                 m_destr_user;
	CSObject*             m_owner;
	bool                  m_owned_externally;
};

#endif // CSOBJECT_H

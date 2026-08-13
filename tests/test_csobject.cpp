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

/*
  Tests for CSObject destruction notification.

  These cover what the python test suite cannot reach: destruction initiated by
  plain C++ (which is why the hook lives in the destructors in the first place),
  unregistering, re-registering, and the object layout invariant the whole
  mechanism depends on.

  Build with -DCSXCAD_BUILD_TESTS=ON and run it through ctest:

    cmake -DCSXCAD_BUILD_TESTS=ON <the usual options> ..
    make && ctest --output-on-failure

  Exits non-zero and prints "FAIL: ..." per failed check.
*/

#include "ContinuousStructure.h"
#include "CSPropMaterial.h"
#include "CSPropExcitation.h"
#include "CSPrimBox.h"
#include "CSObject.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::cout << "FAIL: " << msg << "\n"; ++fails; } } while (0)

//! Notified objects, by the name they were registered with
static std::set<std::string> g_live;
static std::vector<std::string> g_order;
static int g_notifications = 0;

static void gone(void* obj, void* user_data)
{
	UNUSED(obj);
	const char* name = (const char*)user_data;
	g_live.erase(name);
	g_order.push_back(name);
	++g_notifications;
}

static void expect_all_notified(const char* what)
{
	for (std::set<std::string>::iterator it=g_live.begin(); it!=g_live.end(); ++it)
	{
		std::cout << "FAIL: " << what << ": never notified: " << *it << "\n";
		++fails;
	}
}

//! The CSObject subobject must be at offset 0, i.e. `this` in ~CSObject() must be
//! the address the object is known by outside. Only a polymorphic base gives that,
//! and a binding looking up its wrapper by address depends on it. Any virtual
//! method is enough to keep it so, GetObjectKind() among them.
template<class T> static void check_offset(const char* name, T* p)
{
	long delta = (long)((char*)static_cast<CSObject*>(p) - (char*)p);
	CHECK(delta == 0, "CSObject subobject of " << name << " is at offset " << delta
	                  << ", expected 0 -- is CSObject still polymorphic?");
}

int main()
{
	// ---- 1. layout invariant, for every class deriving from CSObject
	{
		ContinuousStructure csx;
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		CSPrimBox* box = new CSPrimBox(csx.GetParameterSet(), mat);
		CSTransform tr;
		CSRectGrid grid;
		check_offset("CSPropMaterial",       mat);
		check_offset("CSPrimBox",            box);
		check_offset("CSTransform",          &tr);
		check_offset("CSRectGrid",           &grid);
		check_offset("ContinuousStructure",  &csx);
		check_offset("ParameterSet",         csx.GetParameterSet());
		check_offset("CSBackgroundMaterial", csx.GetBackgroundMaterial());
	}

	// ---- 2. DeleteProperty() notifies the property, its primitives and their transform
	{
		g_live.clear(); g_order.clear();
		ContinuousStructure csx;
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		CSPrimBox* box = new CSPrimBox(csx.GetParameterSet(), mat);
		CSTransform* tr = box->GetTransform();
		const char* names[] = {"prop", "prim", "transform"};
		for (int n=0;n<3;++n) g_live.insert(names[n]);
		mat->SetDestructionCallback(gone, (void*)"prop");
		box->SetDestructionCallback(gone, (void*)"prim");
		tr->SetDestructionCallback(gone, (void*)"transform");

		csx.DeleteProperty(mat);
		expect_all_notified("DeleteProperty");
		CHECK(!g_order.empty() && g_order[0]=="prop",
		      "the property must be notified before the primitives it deletes");
	}

	// ---- 3. destroying the structure notifies everything it owns, itself first
	{
		g_live.clear(); g_order.clear();
		{
			ContinuousStructure csx;
			CSPropExcitation* exc = new CSPropExcitation(csx.GetParameterSet());
			csx.AddProperty(exc);
			CSPrimBox* box = new CSPrimBox(csx.GetParameterSet(), exc);
			const char* names[] = {"csx", "prop", "prim", "paraset", "bgmat", "grid"};
			for (int n=0;n<6;++n) g_live.insert(names[n]);
			csx.SetDestructionCallback(gone, (void*)"csx");
			csx.GetGrid()->SetDestructionCallback(gone, (void*)"grid");
			csx.GetParameterSet()->SetDestructionCallback(gone, (void*)"paraset");
			csx.GetBackgroundMaterial()->SetDestructionCallback(gone, (void*)"bgmat");
			exc->SetDestructionCallback(gone, (void*)"prop");
			box->SetDestructionCallback(gone, (void*)"prim");
		}
		expect_all_notified("structure teardown");
		CHECK(!g_order.empty() && g_order[0]=="csx",
		      "the structure must be notified before its content");
	}

	// ---- 4. a heap structure deleted by whoever owns it
	{
		g_live.clear(); g_live.insert("heap_csx");
		ContinuousStructure* csx = new ContinuousStructure();
		csx->SetDestructionCallback(gone, (void*)"heap_csx");
		delete csx;
		expect_all_notified("heap structure");
	}

	// ---- 5. passing NULL unregisters
	{
		ContinuousStructure csx;
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		mat->SetDestructionCallback(gone, (void*)"unregistered");
		mat->SetDestructionCallback(NULL, NULL);
		int before = g_notifications;
		csx.DeleteProperty(mat);
		CHECK(g_notifications==before, "callback was invoked after being unregistered");
	}

	// ---- 6. registering again replaces the previous callback
	{
		g_live.clear(); g_live.insert("second");
		ContinuousStructure csx;
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		mat->SetDestructionCallback(gone, (void*)"first");
		mat->SetDestructionCallback(gone, (void*)"second");
		csx.DeleteProperty(mat);
		expect_all_notified("re-registration");
	}

	// ---- 7. ParameterSet::clear() destroys parameters, not the set itself
	{
		g_live.clear(); g_live.insert("paraset");
		{
			ParameterSet pset;
			pset.SetDestructionCallback(gone, (void*)"paraset");
			pset.AddParameter(new Parameter("x", 1.0));
			pset.clear();
			CHECK(g_live.count("paraset")==1,
			      "ParameterSet::clear() must not notify the set itself");
		}
		expect_all_notified("parameter set out of scope");
	}

	// ---- 8. a standalone grid, as CSRectGrid::Clone() hands out
	{
		g_live.clear(); g_live.insert("clone");
		CSRectGrid* grid = new CSRectGrid();
		grid->SetDestructionCallback(gone, (void*)"clone");
		delete grid;
		expect_all_notified("standalone grid");
	}

	// ---- 9. ReadFromXML() clears the structure C++ side, which must notify
	{
		g_live.clear();
		ContinuousStructure csx;
		const char* fn = "test_csobject_empty.xml";
		CHECK(csx.Write2XML(fn), "could not write " << fn);
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		g_live.insert("reread");
		mat->SetDestructionCallback(gone, (void*)"reread");
		csx.ReadFromXML(fn);
		expect_all_notified("ReadFromXML");
		remove(fn);
	}

	// ---- 10. ownership: who will destroy this object
	{
		ContinuousStructure csx;
		CHECK(csx.GetGrid()->GetOwner()              == &csx, "the grid's owner must be the structure");
		CHECK(csx.GetParameterSet()->GetOwner()      == &csx, "the parameter set's owner must be the structure");
		CHECK(csx.GetBackgroundMaterial()->GetOwner()== &csx, "the background material's owner must be the structure");

		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		CHECK(mat->GetOwner() == NULL, "a property nobody added must have no owner");
		CHECK(!mat->IsOwned(),         "a property nobody added must not be owned");
		csx.AddProperty(mat);
		CHECK(mat->GetOwner() == &csx, "AddProperty must make the structure the owner");
		CHECK(mat->IsOwned(),          "an added property must be owned");

		// a structure has no CSXCAD owner, but a solver can claim it
		CHECK(!csx.IsOwned(), "a structure nobody claimed must not be owned");
		csx.SetOwnedExternally(true);
		CHECK(csx.IsOwned(),  "SetOwnedExternally must make the structure owned");
		CHECK(csx.GetOwner() == NULL, "an external claim is not a CSXCAD owner");
		csx.SetOwnedExternally(false);
		CHECK(!csx.IsOwned(), "the claim must be revocable");

		CSPrimBox* box = new CSPrimBox(csx.GetParameterSet(), mat);
		CHECK(box->GetOwner() == mat, "a primitive's owner must be its property");
		CHECK(box->GetTransform()->GetOwner() == box, "a transform's owner must be its primitive");

		CSPrimitives* taken = mat->TakePrimitive(0);
		CHECK(taken == box, "TakePrimitive returned the wrong primitive");
		CHECK(box->GetOwner() == NULL, "TakePrimitive must give ownership back");
		mat->AddPrimitive(box);
		CHECK(box->GetOwner() == mat, "AddPrimitive must take ownership again");

		csx.RemoveProperty(mat);
		CHECK(mat->GetOwner() == NULL, "RemoveProperty must give ownership back");
		delete mat;   // we own it again, and it takes the box with it
	}

	// ---- 11. a copy is a different object, it inherits neither hook nor owner
	{
		g_live.clear(); g_live.insert("original");
		{
			ContinuousStructure csx;
			CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
			csx.AddProperty(mat);
			mat->SetDestructionCallback(gone, (void*)"original");
			CSProperties* copy = mat->GetCopy(false);
			CHECK(copy->GetOwner() == NULL, "a copy must not inherit the owner");
			int before = g_notifications;
			delete copy;
			CHECK(g_notifications == before, "a copy must not inherit the destruction callback");
		}
		// the original is only notified once its structure is gone
		expect_all_notified("original after its structure is gone");
	}

	// ---- 12. every family identifies its kind, so an owner can be resolved
	{
		ContinuousStructure csx;
		CSPropMaterial* mat = new CSPropMaterial(csx.GetParameterSet());
		csx.AddProperty(mat);
		CSPrimBox* box = new CSPrimBox(csx.GetParameterSet(), mat);
		CHECK(csx.GetObjectKind()                       == CSObject::STRUCTURE,   "wrong kind for ContinuousStructure");
		CHECK(mat->GetObjectKind()                      == CSObject::PROPERTY,    "wrong kind for CSProperties");
		CHECK(box->GetObjectKind()                      == CSObject::PRIMITIVE,   "wrong kind for CSPrimitives");
		CHECK(box->GetTransform()->GetObjectKind()      == CSObject::TRANSFORM,   "wrong kind for CSTransform");
		CHECK(csx.GetGrid()->GetObjectKind()            == CSObject::GRID,        "wrong kind for CSRectGrid");
		CHECK(csx.GetParameterSet()->GetObjectKind()    == CSObject::PARAMETERSET,"wrong kind for ParameterSet");
		CHECK(csx.GetBackgroundMaterial()->GetObjectKind()==CSObject::BACKGROUNDMATERIAL, "wrong kind for CSBackgroundMaterial");

		// the kind is what makes an owner resolvable without knowing its static type
		CSObject* owner = box->GetOwner();
		CHECK(owner != NULL && owner->GetObjectKind() == CSObject::PROPERTY,
		      "a primitive's owner must identify itself as a property");
	}

	std::cout << (fails ? "FAILED" : "all CSObject tests passed")
	          << " (" << g_notifications << " notifications)" << std::endl;
	return fails != 0;
}

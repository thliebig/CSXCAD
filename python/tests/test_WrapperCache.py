# -*- coding: utf-8 -*-
"""Regression tests for the python wrapper cache (issue #83).

`CSProperties.fromPtr` and `CSPrimitives.fromPtr` cache their python wrapper
objects in a process global dict keyed on the raw C++ address. Once the C++
object behind such an address is destroyed (e.g. by deleting its property or
by destroying the owning structure) the cache entry becomes stale. If the
allocator hands the very same address out again for an object of a *different*
type, the cache returns the old wrapper: the C++ methods still dispatch
correctly (`GetTypeString()`, `GetName()`), but the python class is wrong and
any type specific method raises `AttributeError`.

The tests below churn wrapped objects, free them, and then let the C++ side
allocate new objects of a different type by reading a model from XML.
"""

import gc
import os
import tempfile
import unittest

from CSXCAD.CSXCAD import ContinuousStructure


# python class expected for each property type string
PROP_CLASS = {
    'Material':        'CSPropMaterial',
    'Metal':           'CSPropMetal',
    'ConductingSheet': 'CSPropConductingSheet',
    'Excitation':      'CSPropExcitation',
    'ProbeBox':        'CSPropProbeBox',
    'DumpBox':         'CSPropDumpBox',
    'LumpedElement':   'CSPropLumpedElement',
}


class Test_WrapperCache(unittest.TestCase):
    def setUp(self):
        self.fn = os.path.join(tempfile.mkdtemp(), 'wrapper_cache.xml')

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    # ------------------------------------------------------------------
    def _churn_properties(self, n_iter=4, n_props=20):
        """Create python wrapped properties and free their C++ objects again."""
        for _ in range(n_iter):
            csx = ContinuousStructure()
            props = [csx.AddMaterial('churn_%d' % n, epsilon=2.0) for n in range(n_props)]
            for prop in props:
                csx.DeleteProperty(prop)   # frees the C++ property
            del props, csx
            gc.collect()

    def _churn_primitives(self, n_iter=4, n_props=10, n_prims=5):
        """Create python wrapped primitives and free their C++ objects again.

        Uses primitive types other than a box, so that a stale wrapper reused
        for a box shows up as a wrong python class.
        """
        for _ in range(n_iter):
            csx = ContinuousStructure()
            props = [csx.AddMaterial('churn_%d' % n, epsilon=2.0) for n in range(n_props)]
            for prop in props:
                for _n in range(n_prims):
                    prop.AddSphere([0, 0, 0], 1.0)
                    prop.AddPoint([0, 0, 0])
                    prop.AddWire([[0, 1], [0, 1], [0, 1]], 0.1)
                    prop.AddPolygon([[0, 1, 1, 0], [0, 0, 1, 1]], norm_dir='z', elevation=0)
            for prop in props:
                csx.DeleteProperty(prop)   # frees the property and its primitives
            del props, csx
            gc.collect()

    # ==================================================================
    def test_property_wrapper_class_after_realloc(self):
        """GetProperty() must not return a wrapper of the wrong python class."""
        csx = ContinuousStructure()
        for n in range(20):
            csx.AddExcitation('exc_%d' % n, exc_type=0, exc_val=[1, 0, 0])
        self.assertTrue(csx.Write2XML(self.fn))
        del csx
        gc.collect()

        self._churn_properties()

        csx2 = ContinuousStructure()
        self.assertEqual(csx2.ReadFromXML(self.fn), '')
        self.assertEqual(csx2.GetQtyProperties(), 20)
        for n in range(csx2.GetQtyProperties()):
            prop = csx2.GetProperty(n)
            type_str = prop.GetTypeString()
            self.assertEqual(
                type(prop).__name__, PROP_CLASS[type_str],
                'property %d (%r) is a live %s but was returned as a %s'
                % (n, prop.GetName(), type_str, type(prop).__name__))
            # type specific method, raises AttributeError on a stale wrapper
            prop.GetExcitation()

    def test_primitive_wrapper_class_after_realloc(self):
        """GetPrimitive() must not return a wrapper of the wrong python class."""
        csx = ContinuousStructure()
        mat = csx.AddMaterial('boxes', epsilon=2.0)
        for n in range(20):
            mat.AddBox([0, 0, 0], [1, 1, 1])
        self.assertTrue(csx.Write2XML(self.fn))
        del mat, csx
        gc.collect()

        self._churn_primitives()

        csx2 = ContinuousStructure()
        self.assertEqual(csx2.ReadFromXML(self.fn), '')
        prop = csx2.GetPropertiesByName('boxes')[0]
        self.assertEqual(prop.GetQtyPrimitives(), 20)
        for n in range(prop.GetQtyPrimitives()):
            prim = prop.GetPrimitive(n)
            self.assertEqual(
                type(prim).__name__, 'CSPrimBox',
                'primitive %d is a live box but was returned as a %s'
                % (n, type(prim).__name__))
            # type specific method, raises AttributeError on a stale wrapper
            prim.GetStart()

    def test_stale_wrapper_raises_instead_of_crashing(self):
        """A wrapper whose C++ object was destroyed must raise, not segfault."""
        self.assertTrue(ContinuousStructure().Write2XML(self.fn))
        csx = ContinuousStructure()
        prop = csx.AddMaterial('mat', epsilon=2.0)
        prim = prop.AddBox([0, 0, 0], [1, 1, 1])
        transform = prim.GetTransform()
        grid = csx.GetGrid()
        bg = csx.GetBackgroundMaterial()
        pset = csx.GetParameterSet()

        # ReadFromXML clears the structure C++ side, i.e. destroys the property
        # and its primitive behind the back of the wrappers above
        self.assertEqual(csx.ReadFromXML(self.fn), '')

        for wrapper, call in ((prop, lambda: prop.GetMaterialProperty('epsilon')),
                              (prim, lambda: prim.GetStart()),
                              (transform, lambda: transform.GetMatrix())):
            with self.assertRaises(RuntimeError) as raised:
                call()
            self.assertIn('has been deleted', str(raised.exception))
            self.assertIn(type(wrapper).__name__, str(raised.exception))

        # the grid, background material and parameter set belong to the
        # structure itself, which is still alive, so they must keep working
        grid.AddLine('x', [0, 1, 2])
        self.assertEqual(bg.GetEpsilon(), 1.0)
        self.assertIsNotNone(pset)

    def test_structure_owned_parts_survive_a_property_purge(self):
        """The grid, background material and parameter set outlive the properties.

        They are members of the ContinuousStructure, not of a property, so
        deleting every property must not invalidate them.

        NOTE: there is no ContinuousStructure.__dealloc__ yet -- the C++
        structure is leaked instead of deleted, so no python-reachable path
        destroys these three. Once it is deleted, this test should be extended
        to check that they are invalidated with it.
        """
        csx = ContinuousStructure()
        grid = csx.GetGrid()
        bg = csx.GetBackgroundMaterial()
        pset = csx.GetParameterSet()
        for n in range(5):
            csx.AddMetal('metal_%d' % n).AddBox([0, 0, 0], [1, 1, 1])
        for prop in csx.GetAllProperties():
            csx.DeleteProperty(prop)
        gc.collect()

        self.assertEqual(csx.GetQtyProperties(), 0)
        grid.AddLine('x', [0, 1, 2])
        self.assertEqual(grid.GetQtyLines('x'), 3)
        self.assertEqual(bg.GetEpsilon(), 1.0)
        self.assertIs(csx.GetGrid(), grid)
        self.assertIs(csx.GetBackgroundMaterial(), bg)
        self.assertIs(csx.GetParameterSet(), pset)

    def test_identity_is_still_preserved(self):
        """Eviction must not break the wrapper cache for live objects."""
        csx = ContinuousStructure()
        prop = csx.AddMaterial('mat', epsilon=2.0)
        prim = prop.AddBox([0, 0, 0], [1, 1, 1])
        self.assertIs(csx.GetProperty(0), prop)
        self.assertIs(csx.GetProperty(0).GetPrimitive(0), prim)
        self.assertIs(prim.GetTransform(), prim.GetTransform())
        self.assertIs(csx.GetGrid(), csx.GetGrid())
        self.assertIs(csx.GetBackgroundMaterial(), csx.GetBackgroundMaterial())
        self.assertIs(csx.GetParameterSet(), csx.GetParameterSet())

    def test_owner_reference_follows_cpp_ownership(self):
        """A wrapper keeps its owner alive exactly as long as C++ says it owns it.

        `_owner` is a snapshot of CSObject::GetOwner() taken when the wrapper is
        created, so every operation that changes ownership afterwards has to
        refresh it -- otherwise a wrapper either pins a structure that no longer
        owns it, or fails to pin the one that does.
        """
        from CSXCAD.CSXCAD import ContinuousStructure as CSX
        from CSXCAD.CSTransform import CSTransform

        # RemoveProperty hands ownership back, the pin has to go
        csx = ContinuousStructure()
        prop = csx.AddMetal('metal')
        csx.RemoveProperty(prop)
        n_before = len(CSX._instances)
        del csx
        gc.collect()
        self.assertLess(len(CSX._instances), n_before,
                        'a removed property must not keep its former structure alive')
        self.assertEqual(prop.GetName(), 'metal')

        # a wrapper whose C++ object was destroyed releases its owner once it is
        # dropped. Not earlier: invalidation is a plain pointer store that cannot
        # run python code at all, see wrapper_destroyed() in CSObject.pxd
        csx = ContinuousStructure()
        prop = csx.AddMetal('metal')
        csx.DeleteProperty(prop)
        n_before = len(CSX._instances)
        del csx, prop
        gc.collect()
        self.assertLess(len(CSX._instances), n_before,
                        'dropping an invalidated wrapper must release its owner')

        # SetTransform transfers the transform to the property, which must pin it
        csx = ContinuousStructure()
        disc = csx.AddDiscMaterial('disc')
        transform = CSTransform()
        transform.Translate([2, 0, 0])
        disc.SetTransform(transform)
        del csx, disc
        gc.collect()
        self.assertEqual(transform.GetMatrix()[0][3], 2.0,
                         'a transform must keep the property that owns it alive')

    def test_wrapper_cache_is_evicted_exactly(self):
        """Destroying a C++ object must drop its wrapper from the cache, at once."""
        from CSXCAD.CSProperties import CSProperties
        from CSXCAD.CSPrimitives import CSPrimitives

        gc.collect()
        n_prop0 = len(CSProperties._instances)
        n_prim0 = len(CSPrimitives._instances)

        self._churn_properties(n_iter=2, n_props=50)
        self._churn_primitives(n_iter=2, n_props=10, n_prims=5)
        gc.collect()

        # the churn helpers create and destroy 100 properties and 400 primitives,
        # every one of them has to be gone from the cache again
        self.assertEqual(len(CSProperties._instances), n_prop0)
        self.assertEqual(len(CSPrimitives._instances), n_prim0)


if __name__ == '__main__':
    unittest.main()

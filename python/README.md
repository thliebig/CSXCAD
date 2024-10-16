# CSXCAD python interface

## Install
If openEMS was installed into `~/opt/openEMS`, then install this package with:

```python
python setup.py build_ext -I ~/opt/openEMS/include -L ~/opt/openEMS/lib -R ~/opt/openEMS/lib"
pyhton setup.py install
```

Otherwise, replace `~/opt/openEMS` with the path to the place where it was installed.

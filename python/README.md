# CSXCAD python interface

## Install
If CSXCAD was installed into `~/opt/CSXCAD`, then install this package with:

```bash
python setup.py build_ext -I ~/opt/CSXCAD/include -L ~/opt/CSXCAD/lib -R ~/opt/CSXCAD/lib
python setup.py install
```

Otherwise, replace `~/opt/CSXCAD` with the path to the place where it was installed.

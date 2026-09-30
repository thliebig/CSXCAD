.. _install:

Install
=======

Instructions how to install the CSXCAD python interface.

Linux
-----

* Make sure CSXCAD was built and installed correctly

* Simple version:

.. code-block:: console

    pip install .

* Extended options, e.g. for a custom install path at */opt*:

.. code-block:: console

    CSXCAD_INSTALL_PATH=/opt pip install .

**Note:** Installing into the system Python may require root; prefer a
virtual environment, or add ``--user`` to install to *~/.local*.

Windows
-------

Windows is supported; the openEMS Windows package ships the Python
interface. See the openEMS documentation for the install instructions.

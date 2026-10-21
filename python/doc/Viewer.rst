.. _csxcad_viewer:

Viewer
------

``CSXCAD.Viewer`` shows a structure with VTK, on screen or off screen. The
core needs ``vtk`` and ``numpy`` (``pip install CSXCAD[viewer]``), the GUI
additionally ``PySide6`` (``pip install CSXCAD[gui]``).

.. code-block:: python

    from CSXCAD.Viewer import Show, Viewer

    Show(CSX, render_mode='solid+edges')        # window, blocks until closed

    v = Viewer(CSX, offscreen=True, size=(1200, 800))
    v.SetView('iso')
    v.Screenshot('structure.png')
    v.RenderViews('all_sides.png')
    print(v.Describe('text'))

The command ``csxcad-view`` (also ``python -m CSXCAD.Viewer``) opens a
structure file, or writes images, exports and a summary without a window.
The view comes from a file, from single keys, or both:

.. code-block:: bash

    csxcad-view model.xml --describe                   # properties, tree paths, mesh statistics
    csxcad-view model.xml --views all_sides.png        # all sides and iso in one image
    csxcad-view model.xml --png feed.png --set view=iso --set focus=port --set elevation=20
    csxcad-view model.xml --png cut.png --set view=+y --set 'clip={"normal": "y", "position": 0}'
    csxcad-view --list-view-keys                       # every view key, its default and values
    csxcad-view model.xml --print-view --set view=+z > view.json   # a view to edit, for --view
    csxcad-view sim/openEMS_setup.xml --file sim/PEC_dump.vtp --set isolate=type:Metal --png pec.png

``--set KEY=VALUE`` takes the value as JSON (plain words as text). A wrong
key, value or property name is reported with the valid ones.

``--file`` (:meth:`~CSXCAD.Viewer.Core.Viewer.AddFile`) shows a VTK file next
to the structure, as the item ``files/<name>``: ``.vtp``, ``.vtu``, ``.vtr``,
``.vts``, ``.vti``, legacy ``.vtk`` or ``.vtkhdf``. Its coordinates are in
metres unless its field data ``LengthUnit`` gives the metres per unit (the
VTP export writes it). The section plane cuts it like the structure. E.g. the
metal edges of openEMS's ``--debug-PEC`` (``PEC_dump.vtp``) over the metals,
``color_by='direction'`` colouring x, y and z edges like the axes::

    v.AddFile('sim/PEC_dump.vtp', color_by='direction')
    v.Isolate('type:Metal')

The GUI embeds VTK's native X11 window, on Wayland through XWayland. A script
that shows the GUI after matplotlib figures imports ``CSXCAD.Viewer`` before
the first figure, so that matplotlib starts Qt that way too (with PySide6).

View state
""""""""""

The whole view is a dict of keyword arguments, accepted by ``Show()``,
``Viewer()`` and :meth:`~CSXCAD.Viewer.Core.Viewer.SetState`, and returned by
:meth:`~CSXCAD.Viewer.Core.Viewer.GetState`; ``csxcad-view --list-view-keys``
lists them. Each key has a method of the viewer (e.g. ``render_mode``:
``SetRenderMode()``); the colour, opacity, visibility and render mode of a property
are not part of it, they are set on the property itself
(``SetColor()``, ``SetVisibility()``, ``SetRenderMode()``) and saved with the
XML file.

=========================  ===================================================================
Key                        Meaning (default)
=========================  ===================================================================
``size``                   render area: ``(fraction, aspect)`` of the screen diagonal and
                           width / height, the same share of any screen (offscreen of
                           1920 x 1080), or ``(width, height)`` in pixels,
                           ``(0.454, 1.333)`` (800 x 600 offscreen)
``window_position``        top-left corner of the window in fractions of the available
                           screen area (GUI and VTK window), ``None``
``window_mode``            GUI: the 3D view and the figures as ``'tabs'`` or as sub-windows
                           side by side, ``'windows'``; ``'tabs'``
``background``             colour, or a pair for a gradient, ``'white'``
``camera``                 ``position``, ``focal_point``, ``view_up``, ``view_angle``,
                           ``parallel_scale``, ``parallel_projection``; None fits the view
``view``                   named direction applied before ``camera``: ``'+x'`` ... ``'-z'``,
                           ``'xy'``, ``'yz'``, ``'xz'``, ``'iso'``, ``'iso_x'``, ``'iso_y'``,
                           ``'iso_z'``
``azimuth``                degrees to turn the camera about the view up (z for the iso
                           views), after ``view``
``elevation``              degrees to tilt the camera up (negative: down), after
                           ``azimuth``
``focus``                  fit on a property, a list of them, ``(name, index)`` of a
                           primitive or a box, after the turns; small targets keep a tenth
                           of the structure around them. Without it the view is fitted on
                           everything shown (cut by ``clip``). ``view``, ``azimuth``,
                           ``elevation`` and ``focus`` only move the camera, ``GetState()``
                           returns the result as ``camera``
``mode_2d``                snapped axis view with 2D interaction, ``False``
``render_mode``            ``'solid'``, ``'edges'`` or ``'solid+edges'`` for properties without
                           their own, ``'solid'``
``transparency``           cap the opacity of all properties (0..1), ``None``
``edge_color``             one edge colour for all properties, ``None``: their EdgeColor
                           with ``'solid+edges'``, their fill colour with ``'edges'``
``edge_angle``             feature-edge angle in degrees, ``30``
``axes``                   orientation marker, ``True``
``labels``                 property names, ``False``
``grid``                   ``visible``, ``color``, ``opacity``, ``planes`` (per direction
                           ``visible`` and ``index`` or ``position``), ``outline``,
                           ``quality`` (``'ratio'`` or ``'size'``), ``quality_threshold``
``clip``                   section plane ``{normal, position, invert}``, ``None``
``arc_resolution``         segments of a full circle, ``32``
``isolate``                only show these properties, ``'type:Metal'`` for all of a type
                           (``GetTypeString()``, any case), ``None``
``highlight``              outline properties or primitives: ``(name, index)`` or
                           ``{prop, primitive}``, the index being the position in
                           ``prop.GetAllPrimitives()`` (stable across sessions, unlike the
                           primitive's ID), ``None``
``visible``                ``{tree path: bool}`` for items and groups
``items``                  ``{item path: {setting: value}}``, see
                           :meth:`~CSXCAD.Viewer.Core.Viewer.GetItemParameters`
``legends``                legends dragged out of the layout: ``{item path: (x, y)}``
                           (lower left, fractions of the view), ``{}``
``disc_material``          show discrete-material models, ``True``
``antialias``              FXAA, ``True``
``interactive_quality``    ``'auto'``, ``'full'`` or ``'fast'`` while moving, ``'auto'``
``animation``              ``{step, fps, playing}`` of the animation clock
``max_peels``              depth-peeling cap for transparency, ``100``
=========================  ===================================================================

Without a display, set ``CSXCAD_VIEWER_OUTPUT=<dir>`` and every ``Show()``
writes ``<name>_view.png``, ``<name>_views.png``, ``<name>_describe.json`` and
``<name>_<figure>.png`` per shown figure there instead of opening a window. The offscreen backend (EGL, OSMesa or the
native window system) is chosen automatically; ``CSXCAD_VIEWER_BACKEND``
overrides it.

Animations (of items, e.g. field results) are rendered with
:meth:`~CSXCAD.Viewer.Core.Viewer.ExportAnimation` or *File → Export
animation...*: a GIF (needs Pillow), PNG frames, or a video (``.ogv``,
``.mp4``) where the VTK build has the writer; the pip wheels have Ogg Theora
but no FFMPEG.

Figures
^^^^^^^

Matplotlib figures (results, e.g.) are shown next to the 3D view with
:meth:`~CSXCAD.Viewer.Core.Viewer.AddFigure`: in the GUI as further tabs, or
sub-windows (``window_mode``), each figure's matplotlib toolbar in the main
toolbar area while it is in front. The tree lists them below *Figures*, with
a row per curve to show or hide; selecting a row brings its figure to the
front and draws the selected curves thicker
(:meth:`~CSXCAD.Viewer.Core.Viewer.HighlightCurves`), selecting the
structure brings back the 3D view. Closing a figure only
hides it, the 3D view cannot be closed. The plain VTK window does not show
figures.

.. code-block:: python

    from matplotlib.figure import Figure

    fig = Figure()
    ax = fig.add_subplot()
    ax.plot(f, s11_dB, label='S11')
    ax.plot(f, s21_dB, label='S21')
    v = Viewer(CSX)
    v.AddFigure(fig, 'S-parameters')                       # curves: the labelled lines
    v.SetVisible('figures/S-parameters/S21', False)
    v.Show()

Extending
^^^^^^^^^

An application building on the viewer adds these kinds of things:

* **Actions** go to the viewer with :meth:`~CSXCAD.Viewer.Core.Viewer.AddAction`:
  they get a key binding in the plain VTK window and the GUI, a GUI menu entry
  named after their group and, if asked, a toolbar button.
* **Items** (:class:`~CSXCAD.Viewer.Scene.ViewItem` subclasses) describe their
  settings in ``GetParameters()``; the GUI shows them as a form in its
  Settings dock while the item is selected, the view state stores them (key
  ``items``, also for items a script adds later), and scripts use
  :meth:`~CSXCAD.Viewer.Core.Viewer.SetItemParameter`. Every item also has an
  ``opacity``. ``GetDescription()`` fills the tree's second column,
  ``GetValueText()`` the value under the mouse, ``GetActions()`` the item's
  context menu and ``CreateWidget()`` an own Qt widget. ``GetParts()`` lists
  parts to show and hide on their own, e.g. the planes of a cut: tree rows
  below the item, their visibility kept in the view state. Colour legends go
  through :meth:`~CSXCAD.Viewer.Core.Viewer.AddLegend`, which lays them out;
  the mouse drags them elsewhere, a double click puts them back.
* **Docks, menus and toolbars** come from a
  :class:`~CSXCAD.Viewer.GUI.MainWindow.MainWindow` subclass, passed to
  ``Show(window_class=...)``.
* **An own window** (embedding the viewer elsewhere, with
  ``Viewer(render_window=..., interactor=...)``) implements the parts of
  :class:`~CSXCAD.Viewer.Core.ViewerHost` it supports, e.g. its render size,
  status messages or clicks, and registers with
  :meth:`~CSXCAD.Viewer.Core.Viewer.SetHost`.

.. code-block:: python

    from CSXCAD.Viewer import Viewer, ViewItem
    from CSXCAD.Viewer.Actions import Action
    from CSXCAD.Viewer.GUI.MainWindow import MainWindow

    class Slice(ViewItem):
        def __init__(self, name, actor):
            super().__init__(name, actor)
            self.level = 0.5
        def GetParameters(self):
            return [dict(name='level', label='Level', type='float', value=self.level, min=0, max=1)]
        def SetParameter(self, name, value):
            self.level = value
            ...                              # update the actor, do not render

    class AppWindow(MainWindow):
        TITLE = 'My application'
        SETTINGS = ('MyOrg', 'my-app')       # its own saved window layout
        def Customize(self):
            self.AddDock('control', 'Control', ControlPanel(self))

    v = Viewer(CSX)
    v.AddAction(Action('start', 'Start', lambda v: start(), shortcut='Ctrl+R', group='Run'), toolbar='Run')
    v.AddItem(Slice('slice', actor), group='results')
    v.Show(window_class=AppWindow)

.. autofunction:: CSXCAD.Viewer.Core.Show

.. autoclass:: CSXCAD.Viewer.Core.Viewer
    :members:

.. autoclass:: CSXCAD.Viewer.Scene.ViewItem
    :members:

.. autoclass:: CSXCAD.Viewer.Figures.ViewFigure

.. automodule:: CSXCAD.Viewer.Files
    :members: FileItem, ReadFile, FileUnit, LENGTH_UNIT, DIRECTION
    :members:

.. automodule:: CSXCAD.Viewer.Actions
    :members: Action

.. autoclass:: CSXCAD.Viewer.Core.ViewerHost
    :members:

.. autoclass:: CSXCAD.Viewer.GUI.MainWindow.MainWindow
    :members: Customize, AddDock, Menu, Toolbar, TITLE, SETTINGS

.. automodule:: CSXCAD.Viewer.Export
    :members: Export, ExportCSX, RegisterExporter

.. automodule:: CSXCAD.Viewer.Geometry
    :members: PrimitivePolyData, PropertyPolyData, DiscMaterialPolyData, CSXPolyData

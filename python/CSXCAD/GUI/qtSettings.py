import os

USE_PYSIDE_VER = os.environ.get('USE_PYSIDE_VER', 'AUTO')

PYSIDE_VER = None
if USE_PYSIDE_VER == 'AUTO':
    try:
        from PySide6.QtCore import Qt
        PYSIDE_VER = '6'
    except:
        pass

    try:
        from PySide2.QtCore import Qt
        PYSIDE_VER = '2'
    except:
        pass
else:
    PYSIDE_VER = USE_PYSIDE_VER

if PYSIDE_VER=='6':
    from PySide6.QtWidgets import QApplication, QMainWindow
elif PYSIDE_VER=='2':
    from PySide2.QtWidgets import QApplication, QMainWindow
else:
    raise Exception('Unable to import PySide 2 or 6')


def createApp(argv):
    app = QApplication(argv)
    return app

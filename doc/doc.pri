OTHER_FILES += $$PWD/doxydoc

docs.target = docs
docs.commands = CSXCAD_VERSION=$$VERSION doxygen $$PWD/doxydoc

QMAKE_EXTRA_TARGETS += docs

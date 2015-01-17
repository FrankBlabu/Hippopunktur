TEMPLATE = app
TARGET = hippopunktur

QT += quick
QT += widgets
QT += xml
QT += quickwidgets
QT += qml

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    database/hip_database.cpp \
    core/hip_exception.cpp \
    database/hip_database_model.cpp \
    explorer/hip_sort_filter_proxy_model.cpp \
    gui/hip_main_window.cpp \
    explorer/hip_explorer.cpp \
    core/hip_tools.cpp \
    explorer/hip_explorer_tagselector.cpp

RESOURCES += \
    hippopunktur.qrc

OTHER_FILES += \
    main.qml \
    core/*.qml \
    explorer/*.qml

HEADERS += \
    database/HIPDatabase.h \
    core/HIPException.h \
    database/HIPDatabaseModel.h \
    explorer/HIPSortFilterProxyModel.h \
    gui/HIPMainWindow.h \
    explorer/HIPExplorer.h \
    core/HIPTools.h \
    explorer/HIPExplorerTagSelector.h

FORMS += \
    gui/hip_main_window.ui \
    explorer/hip_explorer_tagselector.ui

DISTFILES += \
    assets/*


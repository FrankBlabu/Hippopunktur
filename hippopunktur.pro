TEMPLATE = app
TARGET = hippopunktur

QT += quick
QT += widgets
QT += xml
QT += quickwidgets
QT += qml
QT += concurrent

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    database/hip_database.cpp \
    core/hip_exception.cpp \
    database/hip_database_model.cpp \
    explorer/hip_sort_filter_proxy_model.cpp \
    core/hip_tools.cpp \
    explorer/hip_explorer_tagselector.cpp \
    image/hip_image_imageview.cpp \
    core/hip_image_loader.cpp \
    gui/hip_gui_point_editor.cpp \
    gui/hip_gui_main_window.cpp \
    core/hip_status_bar.cpp \
    explorer/hip_qml_explorer.cpp

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
    core/HIPTools.h \
    explorer/HIPExplorerTagSelector.h \
    image/HIPImageImageView.h \
    core/HIPImageLoader.h \
    gui/HIPGuiMainWindow.h \
    gui/HIPGuiPointEditor.h \
    core/HIPStatusBar.h \
    explorer/HIPQMLExplorer.h

FORMS += \
    explorer/hip_explorer_tagselector.ui \
    image/hip_image_imageview.ui \
    gui/hip_gui_pointeditor.ui \
    gui/hip_gui_main_window.ui

DISTFILES += \
    assets/*


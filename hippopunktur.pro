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
    core/hip_exception.cpp \
    core/hip_image_loader.cpp \
    core/hip_status_bar.cpp \
    core/hip_tools.cpp \
    database/hip_database.cpp \
    database/hip_database_model.cpp \
    explorer/hip_explorer_tagselector.cpp \
    explorer/hip_explorer_view.cpp \
    gl/hip_gl_view.cpp \
    gui/hip_gui_point_editor.cpp \
    gui/hip_gui_main_window.cpp \
    image/hip_image_imageview.cpp

RESOURCES += \
    hippopunktur.qrc

OTHER_FILES += \
    main.qml \
    core/*.qml \
    explorer/*.qml

HEADERS += \
    core/HIPException.h \
    core/HIPImageLoader.h \
    core/HIPStatusBar.h \
    core/HIPTools.h \
    database/HIPDatabase.h \
    database/HIPDatabaseModel.h \
    explorer/HIPExplorerTagSelector.h \
    explorer/HIPExplorerView.h \
    gui/HIPGuiMainWindow.h \
    gui/HIPGuiPointEditor.h \
    gl/HIPGLView.h \
    image/HIPImageImageView.h

FORMS += \
    explorer/hip_explorer_tagselector.ui \
    explorer/hip_explorer_view.ui \
    gui/hip_gui_pointeditor.ui \
    gui/hip_gui_main_window.ui \
    gl/hip_gl_view.ui \
    image/hip_image_imageview.ui

DISTFILES += \
    assets/* \
    gl/View.qml


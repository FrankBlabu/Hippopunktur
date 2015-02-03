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
    gl/hip_gl_view.cpp \
    gui/hip_gui_main_window.cpp \
    explorer/hip_point_explorer_view.cpp \
    explorer/hip_explorer_tagselector.cpp \
    explorer/hip_anatomy_explorer_view.cpp \
    gl/hip_gl_data.cpp

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
    gui/HIPGuiMainWindow.h \
    gl/HIPGLView.h \
    core/HIPVersion.h \
    explorer/HIPPointExplorerView.h \
    explorer/HIPExplorerTagSelector.h \
    explorer/HIPAnatomyExplorerView.h \
    gl/HIPGLData.h

FORMS += \
    explorer/hip_explorer_tagselector.ui \
    gui/hip_gui_main_window.ui \
    gl/hip_gl_view.ui \
    explorer/hip_point_explorer_view.ui \
    explorer/hip_anatomy_explorer_view.ui

DISTFILES += \
    assets/* \
    gl/View.qml \
    gl/FragmentShader.glsl \
    gl/VertexShader.glsl


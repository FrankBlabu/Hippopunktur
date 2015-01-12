TEMPLATE = app
TARGET = hippopunktur
QT += quick
QT += widgets

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp

RESOURCES += \
    hippopunktur.qrc

OTHER_FILES += \
    main.qml \
    core/*.qml \
    explorer/*.qml


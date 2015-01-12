/****************************************************************************
** main.cpp - Main file for the Hippopunktur application
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

//#include <QtWidgets/QApplication>
#include <QtGui/QGuiApplication>

#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QSurfaceFormat>

int main (int argc, char *argv[])
{
  QGuiApplication app (argc, argv);

  if (QCoreApplication::arguments ().contains (QLatin1String ("--coreprofile")))
  {
    QSurfaceFormat fmt;
    fmt.setVersion (4, 4);
    fmt.setProfile (QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat (fmt);
  }

  QQmlApplicationEngine engine (QUrl ("qrc:/main.qml"));
  return app.exec ();
}

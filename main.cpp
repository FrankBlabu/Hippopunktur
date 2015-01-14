/****************************************************************************
** main.cpp - Main file for the Hippopunktur application
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QSurfaceFormat>

#include "core/HIPException.h"
#include "database/HIPDatabase.h"

int main (int argc, char *argv[])
{
  bool ok = true;

  QGuiApplication app (argc, argv);

  if (QCoreApplication::arguments ().contains (QLatin1String ("--coreprofile")))
  {
    QSurfaceFormat fmt;
    fmt.setVersion (4, 4);
    fmt.setProfile (QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat (fmt);
  }

  try
  {
    HIP::Database::Database database (":/data/points.xml");

    QQmlApplicationEngine engine (QUrl ("qrc:/main.qml"));
    ok = app.exec ();
  }
  catch (const HIP::Exception& exception)
  {
    QString message = QObject::tr ("ERROR: %1").arg (exception.getText ());
    qFatal (qUtf8Printable (message));
    ok = false;
  }

  return ok ? 0 : -1;
}

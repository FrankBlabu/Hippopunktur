/****************************************************************************
** main.cpp - Main file for the Hippopunktur application
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QtQml>
#include <QFile>

#include "core/HIPException.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"
#include "gui/HIPGuiMainWindow.h"

namespace {

  static const char* const DATABASE_FILE = ":/data/database.xml";

}

/*
 * MAIN
 */
int main (int argc, char* argv[])
{
  bool ok = true;

  QApplication app (argc, argv);

  app.setApplicationName (QObject::tr ("Hippopunktur"));
  app.setApplicationVersion (QObject::tr ("0.1"));
  app.setStyleSheet (HIP::Tools::loadResource<QString> (":/assets/style/hippopunktur.css"));

  qmlRegisterType<HIP::Database::Database> ("com.blankenburg.hippopunktur", 1, 0, "Database");

  try
  {
    HIP::Database::Database database;
    database.load (HIP::Tools::loadResource<QString> (DATABASE_FILE));

    HIP::Gui::MainWindow main_win (&database);
    main_win.resize (800, 600);
    main_win.show ();

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

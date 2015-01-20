/****************************************************************************
** main.cpp - Main file for the Hippopunktur application
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

#include <QApplication>
#include <QtQml>
#include <QDebug>

#include "core/HIPException.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"
#include "gui/HIPGuiMainWindow.h"

int main (int argc, char* argv[])
{
  bool ok = true;

  QApplication app (argc, argv);

  app.setStyleSheet (HIP::Tools::loadResource<QString> (":/assets/style/hippopunktur.css"));

  qmlRegisterType<HIP::Database::Database> ("com.blankenburg.hippopunktur", 1, 0, "Database");

  try
  {
    HIP::Database::Database database (":/data/database.xml");

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

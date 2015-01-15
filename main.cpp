/****************************************************************************
** main.cpp - Main file for the Hippopunktur application
**
** Frank Blankenburg, Jan. 2014
****************************************************************************/

#include <QApplication>
#include <QtQml>

#include "core/HIPException.h"
#include "database/HIPDatabase.h"
#include "database/HIPDatabaseModel.h"
#include "explorer/HIPSortFilterProxyModel.h"
#include "gui/HIPMainWindow.h"

int main (int argc, char* argv[])
{
  bool ok = true;

  QApplication app (argc, argv);

  qmlRegisterType<HIP::Explorer::SortFilterProxyModel> ("com.blankenburg.hippopunktur", 1, 0, "SortFilterProxyModel");

  try
  {
    HIP::Database::Database database (":/data/points.xml");
    HIP::Database::DatabaseModel model (&database);

    HIP::Gui::MainWindow main_win;
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

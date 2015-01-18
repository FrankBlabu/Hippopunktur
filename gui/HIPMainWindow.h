/*
 * HIPMainWindow.h - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPMainWindow_h__
#define __HIPMainWindow_h__

#include <QMainWindow>

namespace Ui {
  class HIP_Gui_MainWindow;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Gui {

    /*
     * Main window for the hippopunktur application
     */
    class MainWindow : public QMainWindow
    {
      Q_OBJECT

    public:
      explicit MainWindow (Database::Database* database, QWidget* parent=0);
      virtual ~MainWindow ();

    private slots:
      void onTagChanged (const QString& tag);
      void onAbout ();

    private:
      Ui::HIP_Gui_MainWindow* _ui;
      Database::Database* _database;
    };

  } // namespace Gui
} // namespace HIP

#endif

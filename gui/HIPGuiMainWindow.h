/*
 * HIPGuiMainWindow.h - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGuiMainWindow_h__
#define __HIPGuiMainWindow_h__

#include <database/HIPDatabase.h>

#include <QMainWindow>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;


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

      virtual void setVisible (bool visible);

    protected:
      virtual void dragEnterEvent (QDragEnterEvent* event);
      virtual void dragMoveEvent (QDragMoveEvent* event);
      virtual void dropEvent (QDropEvent* event);

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);
      void onCurrentTabChanged (int index);
      void onExportDatabase ();
      void onAbout ();

    private:
      Ui::HIP_Gui_MainWindow* _ui;
      Database::Database* _database;
    };

  } // namespace Gui
} // namespace HIP

#endif

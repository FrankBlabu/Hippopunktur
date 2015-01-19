/*
 * HIPGuiMainWindow.h - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGuiMainWindow_h__
#define __HIPGuiMainWindow_h__

#include <QMainWindow>

namespace Ui {
  class HIP_Gui_MainWindow;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Gui {

    class PointEditor;

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

    private slots:
      void onTagChanged (const QString& tag);
      void onImageSelected (const QString& id);
      void onCurrentTabChanged (int index);

      void onAbout ();

    private:
      Ui::HIP_Gui_MainWindow* _ui;
      Database::Database* _database;

      PointEditor* _point_editor;
    };

  } // namespace Gui
} // namespace HIP

#endif

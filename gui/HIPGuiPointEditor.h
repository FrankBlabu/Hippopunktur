/*
 * HIPGuiPointEditor.h - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGuiPointEditor_h__
#define __HIPGuiPointEditor_h__

#include <QWidget>

namespace Ui {
  class HIP_Gui_PointEditor;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Gui {

    /*!
     * Editor for the point database
     */
    class PointEditor : public QWidget
    {
      Q_OBJECT

    public:
      PointEditor (Database::Database* database, QWidget* parent);
      virtual ~PointEditor ();

    private:
      Ui::HIP_Gui_PointEditor* _ui;
      Database::Database* _database;
    };

  }
}

#endif

/*
 * HIPTagSelector.h - Widget for choosing tag filter options
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorerTagSelector_h__
#define __HIPExplorerTagSelector_h__

#include <QWidget>

namespace Ui {
  class HIP_Explorer_TagSelector;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Explorer {

    /*
     * Widget for choosing tag filter options
     */
    class TagSelector : public QWidget
    {
      Q_OBJECT

    public:
      explicit TagSelector (Database::Database* database, QWidget* parent);
      virtual ~TagSelector ();

    signals:
      void tagChanged (const QString& tag);

    private slots:
      void onActivated (int index);
      void onClear ();

    private:
      Ui::HIP_Explorer_TagSelector* _ui;
      Database::Database* _database;
    };


  }
}

#endif

/*
 * HIPExplorerView.h - Point list explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorerView_h__
#define __HIPExplorerView_h__

#include <QWidget>

#include "database/HIPDatabase.h"

class QItemSelection;

namespace Ui {
  class HIP_Explorer_ExplorerView;
}

namespace HIP {

  namespace Database {
    class Database;
    class DatabaseModel;
    class DatabaseFilterProxyModel;
  }

  namespace Explorer {

    /*
     * Point list explorer
     */
    class ExplorerView : public QWidget
    {
      Q_OBJECT

    public:
      ExplorerView (Database::Database* database, QWidget* parent);
      virtual ~ExplorerView ();

    private slots:
      void onSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
      void onDatabaseChanged (Database::Database::Reason_t reason, const QString& id);

    private:
      Ui::HIP_Explorer_ExplorerView* _ui;

      Database::Database* _database;
      Database::DatabaseModel* _model;
      Database::DatabaseFilterProxyModel* _filter;
    };

  }
}

#endif

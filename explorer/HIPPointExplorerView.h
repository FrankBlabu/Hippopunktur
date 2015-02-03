/*
 * HIPPointExplorerView.h - Point list explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPPointExplorerView_h__
#define __HIPPointExplorerView_h__

#include <QWidget>

#include "database/HIPDatabase.h"

class QItemSelection;

namespace Ui {
  class HIP_Explorer_PointExplorerView;
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
    class PointExplorerView : public QWidget
    {
      Q_OBJECT

    public:
      PointExplorerView (Database::Database* database, QWidget* parent);
      virtual ~PointExplorerView ();

    private slots:
      void onSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);

    private:
      Ui::HIP_Explorer_PointExplorerView* _ui;

      Database::Database* _database;
      Database::DatabaseModel* _model;
      Database::DatabaseFilterProxyModel* _filter;

      bool _update_in_progress;
    };

  }
}

#endif

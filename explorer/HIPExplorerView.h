/*
 * HIPExplorerView.h - Point list explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorerView_h__
#define __HIPExplorerView_h__

#include <QWidget>

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

    public slots:
      void onTagChanged (const QString& tag);
      void onPointChanged (const QString& id);
      void onDataChanged ();

    private slots:
      void onExplorerSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);
      void onDatabaseSelectionChanged (const QString& id);

    private:
      Ui::HIP_Explorer_ExplorerView* _ui;

      Database::Database* _database;
      Database::DatabaseModel* _model;
      Database::DatabaseFilterProxyModel* _filter;
    };

  }
}

#endif

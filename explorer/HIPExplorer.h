/*
 * HIPExplorer.h - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorer_h__
#define __HIPExplorer_h__

#include <QWidget>

namespace HIP {

  namespace Database {
    class Database;
    class DatabaseModel;
    class DatabaseFilterProxyModel;
  }

  namespace Explorer {

    /*
     * Point explorer
     */
    class Explorer : public QWidget
    {
      Q_OBJECT

    public:
      Explorer (Database::Database* database, QWidget* parent);
      virtual ~Explorer ();

    public slots:
      void onTagChanged (const QString& tag);

    private:
      Database::Database* _database;
      Database::DatabaseModel* _model;
      Database::DatabaseFilterProxyModel* _filter;

      static const char* const QML_MODEL_NAME;
    };

  }
}

#endif

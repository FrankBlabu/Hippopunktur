/*
 * HIPDatabaseModel.h - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPDatabaseModel_h__
#define __HIPDatabaseModel_h__

#include <QAbstractItemModel>

namespace HIP {
  namespace Database {

    class Database;

    class DatabaseModel : public QAbstractItemModel
    {
    public:
      DatabaseModel (Database* database);
      virtual ~DatabaseModel ();

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private:
      Database* _database;
    };

  }
}

#endif

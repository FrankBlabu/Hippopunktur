/*
 * HIPDatabaseModel.h - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPDatabaseModel_h__
#define __HIPDatabaseModel_h__

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "database/HIPDatabase.h"

namespace HIP {
  namespace Database {

    class Database;

    /*
     * Proxy model for filtered database access
     */
    class DatabaseFilterProxyModel : public QSortFilterProxyModel
    {
      Q_OBJECT

    public:
      DatabaseFilterProxyModel (const Database* database, QObject* parent);
      virtual ~DatabaseFilterProxyModel ();

    protected:
      virtual bool	filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

    private slots:
      void onDatabaseChanged (Database::Reason_t reason, const QVariant& data);

    private:
      QString _tag;
    };

    /*
     * Model for database access
     */
    class DatabaseModel : public QAbstractItemModel
    {
      Q_OBJECT

    public:
      struct Role { enum Type_t {
          ID = Qt::UserRole + 1,
          NAME,
          DESCRIPTION,
          SELECTED,
          POINT
        }; };
      typedef Role::Type_t Role_t;

    public:
      DatabaseModel (Database* database, QObject* parent);
      virtual ~DatabaseModel ();

      QModelIndex getIndex (const QString& id) const;

      virtual QHash<int, QByteArray> roleNames () const;

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private slots:
      void onDatabaseChanged (Database::Reason_t reason, const QVariant& data);

    private:
      Database* _database;
    };

  }
}

#endif

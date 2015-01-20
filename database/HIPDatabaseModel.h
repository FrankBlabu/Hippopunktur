/*
 * HIPDatabaseModel.h - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPDatabaseModel_h__
#define __HIPDatabaseModel_h__

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace HIP {
  namespace Database {

    class Database;

    /*
     * Proxy model for filtered database access
     */
    class DatabaseFilterProxyModel : public QSortFilterProxyModel
    {
    public:
      DatabaseFilterProxyModel (QObject* parent);
      virtual ~DatabaseFilterProxyModel ();

      void setTag (const QString& tag);

    protected:
      virtual bool	filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

    private:
      QString _tag;
    };


    /*
     * Model for database access
     */
    class DatabaseModel : public QAbstractItemModel
    {
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
      void reset ();
      void onChanged (const QString& id);

      virtual QHash<int, QByteArray> roleNames () const;

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual bool setData (const QModelIndex& index, const QVariant& value, int role);
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private:
      Database* _database;
    };

  }
}

#endif

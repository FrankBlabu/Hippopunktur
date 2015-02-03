/*
 * hip_database_model.cpp - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPDatabaseModel.h"

#include <QDebug>

namespace HIP {
  namespace Database {

    //#**********************************************************************
    // CLASS HIP::Database::DatabaseFilterProxyModel
    //#**********************************************************************

    /* Constructor */
    DatabaseFilterProxyModel::DatabaseFilterProxyModel (const Database* database, QObject* parent)
      : QSortFilterProxyModel (parent),
        _tag ()
    {
      connect (database, &Database::databaseChanged, this, &DatabaseFilterProxyModel::onDatabaseChanged);
    }

    /* Destructor */
    DatabaseFilterProxyModel::~DatabaseFilterProxyModel ()
    {
    }

    /* Check if row is filtered */
    bool DatabaseFilterProxyModel::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const
    {
      QModelIndex index = sourceModel ()->index (source_row, 0, source_parent);
      return sourceModel ()->data (index, DatabaseModel::Role::POINT).value<Point> ().matches (_tag);
    }

    /* Database change listener */
    void DatabaseFilterProxyModel::onDatabaseChanged (Database::Reason_t reason, const QVariant& data)
    {
      if (reason == Database::Reason::FILTER)
        {
          Q_ASSERT (data.type () == QVariant::String);

          beginResetModel ();
          _tag = data.toString ();
          endResetModel ();
        }
    }


    //#**********************************************************************
    // CLASS HIP::Database::DatabaseModel
    //#**********************************************************************

    /* Constructor */
    DatabaseModel::DatabaseModel (Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database (database)
    {
      connect (database, &Database::databaseChanged, this, &DatabaseModel::onDatabaseChanged);
    }

    /*! Destructor */
    DatabaseModel::~DatabaseModel ()
    {
    }

    QModelIndex DatabaseModel::getIndex (const QString& id) const
    {
      QModelIndex index;

      for (int i=0; i < _database->getPoints ().size () && !index.isValid (); ++i)
        if (_database->getPoints ()[i].getId () == id)
          index = this->index (i, 0, QModelIndex ());

      return index;
    }

    /*! Custom role names for QML interaction */
    QHash<int, QByteArray> DatabaseModel::roleNames () const
    {
      QHash<int, QByteArray> roles;

      roles[Role::ID]          = "id";
      roles[Role::NAME]        = "name";
      roles[Role::DESCRIPTION] = "description";
      roles[Role::SELECTED]    = "selected";
      roles[Role::POINT]       = "point";

      return roles;
    }

    int DatabaseModel::columnCount (const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return 1;
    }

    int DatabaseModel::rowCount (const QModelIndex& parent) const
    {
      return !parent.isValid () ? _database->getPoints ().size () : 0;
    }

    QModelIndex DatabaseModel::index (int row, int column, const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return createIndex (row, column);
    }

    QModelIndex DatabaseModel::parent (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return QModelIndex ();
    }

    Qt::ItemFlags DatabaseModel::flags (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant DatabaseModel::data (const QModelIndex& index, int role) const
    {
      QVariant result;

      const QList<Point>& points = _database->getPoints ();

      if (index.isValid () && index.row () < points.size ())
        {
          const Point& point = points[index.row ()];

          switch (role)
            {
            case Qt::DisplayRole:
            case Role::ID:
            case Role::NAME:
              result = qVariantFromValue (point.getId ());
              break;

            case Role::DESCRIPTION:
              result = qVariantFromValue (point.getDescription ());
              break;

            case Role::SELECTED:
              result = qVariantFromValue (point.getSelected ());
              break;

            case Role::POINT:
              result = qVariantFromValue (point);
              break;
            }
        }

      return result;
    }

    QVariant DatabaseModel::headerData (int section, Qt::Orientation orientation, int role) const
    {
      QVariant data;

      Q_UNUSED (orientation);

      if (role == Qt::DisplayRole && section == 0)
        data = QVariant (tr ("points"));

      return data;
    }

    void DatabaseModel::onDatabaseChanged (Database::Reason_t reason, const QVariant& data)
    {
      QModelIndex index;

      if (data.type () == QVariant::String)
        {
          for (int i=0; i < _database->getPoints ().size (); ++i)
            if (_database->getPoints ()[i].getId () == data.toString ())
              index = this->index (i, 0, QModelIndex ());
        }

      switch (reason)
        {
        case Database::Reason::DATA:
          Q_ASSERT (!data.isValid ());
          beginResetModel ();
          endResetModel ();
          break;

        case Database::Reason::POINT:
          Q_ASSERT (index.isValid ());
          emit dataChanged (index, index);
          break;

        case Database::Reason::SELECTION:
          Q_ASSERT (index.isValid ());
          emit dataChanged (index, index);
          break;

        case Database::Reason::FILTER:
          break;
        }
    }


  }
}



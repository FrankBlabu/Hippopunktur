/*
 * hip_database_model.cpp - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPDatabaseModel.h"
#include "HIPDatabase.h"

#include <QDebug>

namespace HIP {
  namespace Database {

    //#**********************************************************************
    // CLASS HIP::Database::DatabaseFilterProxyModel
    //#**********************************************************************

    /* Constructor */
    DatabaseFilterProxyModel::DatabaseFilterProxyModel (QObject* parent)
      : QSortFilterProxyModel (parent),
        _tag ()
    {
    }

    /* Destructor */
    DatabaseFilterProxyModel::~DatabaseFilterProxyModel ()
    {
    }

    /*! Set filter tag */
    void DatabaseFilterProxyModel::setTag (const QString& tag)
    {
      beginResetModel ();
      _tag = tag;
      endResetModel ();
    }

    /* Check if row is filtered */
    bool DatabaseFilterProxyModel::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const
    {
      QModelIndex index = sourceModel ()->index (source_row, 0, source_parent);
      return sourceModel ()->data (index, DatabaseModel::Role::POINT).value<Point> ().matches (_tag);
    }


    //#**********************************************************************
    // CLASS HIP::Database::DatabaseModel
    //#**********************************************************************

    /* Constructor */
    DatabaseModel::DatabaseModel (Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database (database)
    {
    }

    /*! Destructor */
    DatabaseModel::~DatabaseModel ()
    {
    }

    /*! Compute model matching the given point id */
    QModelIndex DatabaseModel::getIndex (const QString& id) const
    {
      int row = -1;
      for (int i=0; i < _database->getPoints ().size () && row == -1; ++i)
        if (_database->getPoints ()[i].getId () == id)
          row = i;

      return index (row, 0, QModelIndex ());
    }

    /*! Reset whole model */
    void DatabaseModel::reset ()
    {
      beginResetModel ();
      endResetModel ();
    }

    /*! Notify change in a single row */
    void DatabaseModel::onChanged (const QString &id)
    {
      QModelIndex index = getIndex (id);
      if (index.isValid ())
        emit dataChanged (index, index);
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

    bool DatabaseModel::setData (const QModelIndex& index, const QVariant& value, int role)
    {
      bool modified = false;

      const QList<Point>& points = _database->getPoints ();

      if (index.isValid () && index.row () < points.size ())
        {
          const Point& point = points[index.row ()];

          switch (role)
            {
            case Role::SELECTED:
              {
                _database->setSelected (point.getId (), value.toBool () ? Database::SelectionMode::SELECT : Database::SelectionMode::DESELECT) ;
                emit dataChanged (index, index, QVector<int> (1, Role::SELECTED));
              }
              break;

            case Role::POINT:
              {
                Point p = value.value<Point> ();
                _database->setPoint (p.getId (), p);
                emit dataChanged (index, index, QVector<int> (1, Role::SELECTED));
              }
              break;

            default:
              qWarning ("Setting role not allowed: " + roleNames ().value (role));
              break;
            }
        }

      return modified;
    }

    QVariant DatabaseModel::headerData (int section, Qt::Orientation orientation, int role) const
    {
      QVariant data;

      Q_UNUSED (orientation);

      if (role == Qt::DisplayRole && section == 0)
        data = QVariant (tr ("points"));

      return data;
    }


  }
}



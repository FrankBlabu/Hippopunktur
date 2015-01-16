/*
 * hip_database_model.cpp - Model accessing the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPDatabaseModel.h"
#include "HIPDatabase.h"

namespace HIP {
  namespace Database {

    /* Constructor */
    DatabaseModel::DatabaseModel (Database* database)
      : _database (database)
    {
    }

    /*! Destructor */
    DatabaseModel::~DatabaseModel ()
    {
    }

    /*! Custom role names for QML interaction */
    QHash<int, QByteArray> DatabaseModel::roleNames () const
    {
      QHash<int, QByteArray> roles;
      roles[Role::NAME]        = "name";
      roles[Role::DESCRIPTION] = "description";
      roles[Role::SELECTED]    = "selected";

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
            case Role::NAME:
              result = qVariantFromValue (point.getId ());
              break;

            case Role::DESCRIPTION:
              result = qVariantFromValue (point.getDescription ());
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


  }
}



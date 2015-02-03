/*
 * hip_gl_data_model.cpp - Model to access the GL data
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLDataModel.h"

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::DataModel
    //#**********************************************************************

    /*! Constructor */
    DataModel::DataModel (Database::Database* database, QObject* parent)
      : QAbstractItemModel (parent)
    {
      connect (database, &Database::Database::databaseChanged, this, &DataModel::onDatabaseChanged);
    }

    /*! Destructor */
    DataModel::~DataModel ()
    {
    }

    int DataModel::columnCount (const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return 2;
    }

    int DataModel::rowCount (const QModelIndex& parent) const
    {
      return !parent.isValid () ? 0 : 0;
    }

    QModelIndex DataModel::index (int row, int column, const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return createIndex (row, column);
    }

    QModelIndex DataModel::parent (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return QModelIndex ();
    }

    Qt::ItemFlags DataModel::flags (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant DataModel::data (const QModelIndex& index, int role) const
    {
      QVariant result;

      if (index.isValid ())
        {

          switch (role)
            {
            case Qt::DisplayRole:
              break;
            }
        }

      return result;
    }

    QVariant DataModel::headerData (int section, Qt::Orientation orientation, int role) const
    {
      QVariant data;

      Q_UNUSED (orientation);

      if (role == Qt::DisplayRole)
        {
          if (section == Section::PART)
            data = QVariant (tr ("part"));
          else if (section == Section::VISIBLE)
            data = QVariant (tr ("part"));
        }

      return data;
    }

    void DataModel::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      switch (reason)
        {
        case Database::Database::Reason::DATA:
          Q_ASSERT (!data.isValid ());
          beginResetModel ();
          endResetModel ();
          break;

        case Database::Database::Reason::POINT:
        case Database::Database::Reason::SELECTION:
        case Database::Database::Reason::FILTER:
          break;
        }
    }



  }
}

/*
 * HIPGLDataModel.h - Model to access the GL data
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPGLDataModel_h__
#define __HIPGLDataModel_h__

#include <QAbstractItemModel>
#include "database/HIPDatabase.h"

namespace HIP {
  namespace GL {

    /*!
     * Model to access the GL data
     */
    class DataModel : public QAbstractItemModel
    {
      Q_OBJECT

    public:
      struct Section { enum Type_t { PART, VISIBLE }; };
      typedef Section::Type_t Section_t;

    public:
      DataModel (Database::Database* database, QObject* parent);
      virtual ~DataModel ();

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);

    private:
      Database::Database* _database;
    };

  }
}

#endif

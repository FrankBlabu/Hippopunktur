/*
 * HIPTagSelector.h - Widget for choosing tag filter options
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorerTagSelector_h__
#define __HIPExplorerTagSelector_h__

#include <QAbstractItemModel>
#include <QWidget>

#include "database/HIPDatabase.h"

namespace Ui {
  class HIP_Explorer_TagSelector;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Explorer {

    /*
     * Model for the tag selector
     */
    class TagSelectorModel : public QAbstractItemModel
    {
      Q_OBJECT

    public:
      TagSelectorModel (Database::Database* database, QObject* parent);
      virtual ~TagSelectorModel ();

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QString& id);

    private:
      Database::Database* _database;
    };

    /*
     * Widget for choosing tag filter options
     */
    class TagSelector : public QWidget
    {
      Q_OBJECT

    public:
      explicit TagSelector (Database::Database* database, QWidget* parent);
      virtual ~TagSelector ();

    private slots:
      void onTextChanged (const QString& text);
      void onActivated (int index);
      void onClear ();

    private:
      Ui::HIP_Explorer_TagSelector* _ui;
      Database::Database* _database;
    };


  }
}

#endif

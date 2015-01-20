/*
 * hip_explorer_view.cpp - Point list explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorerView.h"
#include "ui_hip_explorer_view.h"

#include "database/HIPDatabase.h"
#include "database/HIPDatabaseModel.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QSignalBlocker>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::ExplorerView
    //#**********************************************************************

    /*! Constructor */
    ExplorerView::ExplorerView (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _ui       (new Ui::HIP_Explorer_ExplorerView),
        _database (database),
        _model    (new Database::DatabaseModel (database, this)),
        _filter   (new Database::DatabaseFilterProxyModel (this))
    {
      _ui->setupUi (this);

      _filter->setSourceModel (_model);
      _ui->_tree_w->setModel (_filter);

      connect (_ui->_tree_w->selectionModel (),
               SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
               SLOT (onExplorerSelectionChanged (const QItemSelection&, const QItemSelection&)));
      connect (_database, SIGNAL (selectionChanged (const QString&)),
               SLOT (onDatabaseSelectionChanged (const QString&)));

    }

    ExplorerView::~ExplorerView ()
    {
      delete _ui;
    }

    void ExplorerView::onTagChanged (const QString& tag)
    {
      _database->clearSelection ();
      _filter->setTag (tag);
    }

    void ExplorerView::onPointChanged (const QString& id)
    {
      _model->onChanged (id);
    }

    void ExplorerView::onDataChanged ()
    {
      _model->reset ();
    }

    void ExplorerView::onExplorerSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected)
    {
      foreach (QModelIndex index, selected.indexes ())
        _database->setSelected (_model->data (index, Database::DatabaseModel::Role::ID).toString (),
                                Database::Database::SELECT);

      foreach (QModelIndex index, deselected.indexes ())
        _database->setSelected (_model->data (index, Database::DatabaseModel::Role::ID).toString (),
                                Database::Database::DESELECT);
    }


    void ExplorerView::onDatabaseSelectionChanged (const QString& id)
    {
      QSignalBlocker blocker (this);

      if (_database->getPoint (id).getSelected ())
        _ui->_tree_w->selectionModel ()->select (_model->getIndex (id), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
      else
        _ui->_tree_w->selectionModel ()->select (_model->getIndex (id), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }

  }
}

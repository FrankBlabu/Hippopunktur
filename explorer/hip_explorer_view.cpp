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
#include <QLabel>
#include <QPainter>
#include <QSharedPointer>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QTextDocument>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::ExplorerViewDelegate
    //#**********************************************************************

    /*
     * Delegate for painting the explorer items
     */
    class ExplorerViewDelegate : public QStyledItemDelegate
    {
    public:
      ExplorerViewDelegate (const Database::Database* database, QObject* parent);
      virtual ~ExplorerViewDelegate ();

      virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      virtual QSize	sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
      void setupLabel (const Database::Point& point, const QSize& proposed_size) const;

    private:
      const Database::Database* _database;
      mutable QLabel* _label;
    };

    /* Constructor */
    ExplorerViewDelegate::ExplorerViewDelegate (const Database::Database* database, QObject* parent)
      : QStyledItemDelegate (parent),
        _database (database),
        _label    (new QLabel)
    {
      _label->setObjectName ("HIP_ExplorerViewDelegate_ItemLabel");
    }

    /* Destructor */
    ExplorerViewDelegate::~ExplorerViewDelegate ()
    {
      delete _label;
    }

    /* Paint item */
    void ExplorerViewDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
      const Database::Point& point = _database->getPoint (index.model ()->data (index, Database::DatabaseModel::Role::ID).toString ());
      setupLabel (point, option.rect.size ());

      if (point.getSelected ())
        {
          painter->save ();
          painter->translate (option.rect.topLeft ());
          _label->render (painter);
          painter->restore ();
        }
      else
        QStyledItemDelegate::paint (painter, option, index);
    }

    /* Compute size hint */
    QSize ExplorerViewDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
      QSize size = QStyledItemDelegate::sizeHint (option, index);

      const Database::Point& point = _database->getPoint (index.model ()->data (index, Database::DatabaseModel::Role::ID).toString ());
      setupLabel (point, size);

      if (point.getSelected ())
        size = _label->size ();

      return size;
    }

    /* Setup label to display the text belonging to the given point */
    void ExplorerViewDelegate::setupLabel (const Database::Point& point, const QSize& proposed_size) const
    {
      _label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Preferred);
      _label->setMinimumWidth (proposed_size.width ());
      _label->setMaximumWidth (proposed_size.width ());

      _label->setText (QString ("<b>%1</b><p>%2")
                       .arg (point.getId ())
                       .arg (point.getDescription ()));

      _label->adjustSize ();
      _label->update ();
    }



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
      _ui->_tree_w->setItemDelegate (new ExplorerViewDelegate (database, this));

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

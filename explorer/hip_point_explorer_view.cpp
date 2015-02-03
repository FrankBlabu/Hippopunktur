/*
 * hip_point_explorer_view.cpp - Point list explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPPointExplorerView.h"
#include "ui_hip_point_explorer_view.h"

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
    // CLASS HIP::Explorer::PointExplorerViewDelegate
    //#**********************************************************************

    /*
     * Delegate for painting the explorer items
     */
    class PointExplorerViewDelegate : public QStyledItemDelegate
    {
    public:
      PointExplorerViewDelegate (const Database::Database* database, QObject* parent);
      virtual ~PointExplorerViewDelegate ();

      virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      virtual QSize	sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
      void setupLabel (const Database::Point& point, const QSize& proposed_size) const;

    private:
      const Database::Database* _database;
      mutable QLabel* _label;
    };

    /* Constructor */
    PointExplorerViewDelegate::PointExplorerViewDelegate (const Database::Database* database, QObject* parent)
      : QStyledItemDelegate (parent),
        _database (database),
        _label    (new QLabel)
    {
      _label->setObjectName ("HIP_PointExplorerViewDelegate_ItemLabel");
    }

    /* Destructor */
    PointExplorerViewDelegate::~PointExplorerViewDelegate ()
    {
      delete _label;
    }

    /* Paint item */
    void PointExplorerViewDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
    QSize PointExplorerViewDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
      QSize size = QStyledItemDelegate::sizeHint (option, index);

      const Database::Point& point = _database->getPoint (index.model ()->data (index, Database::DatabaseModel::Role::ID).toString ());
      setupLabel (point, size);

      if (point.getSelected ())
        size = _label->size ();

      return size;
    }

    /* Setup label to display the text belonging to the given point */
    void PointExplorerViewDelegate::setupLabel (const Database::Point& point, const QSize& proposed_size) const
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
    // CLASS HIP::Explorer::PointExplorerView
    //#**********************************************************************

    /*! Constructor */
    PointExplorerView::PointExplorerView (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _ui                 (new Ui::HIP_Explorer_PointExplorerView),
        _database           (database),
        _model              (new Database::DatabaseModel (database, this)),
        _filter             (new Database::DatabaseFilterProxyModel (database, this)),
        _update_in_progress (false)
    {
      _ui->setupUi (this);

      _filter->setSourceModel (_model);
      _ui->_tree_w->setModel (_filter);
      _ui->_tree_w->setItemDelegate (new PointExplorerViewDelegate (database, this));

      connect (_ui->_tree_w->selectionModel (),
               SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
               SLOT (onSelectionChanged (const QItemSelection&, const QItemSelection&)));
      connect (database, &Database::Database::databaseChanged, this, &PointExplorerView::onDatabaseChanged);
    }

    PointExplorerView::~PointExplorerView ()
    {
      delete _ui;
    }

    void PointExplorerView::onSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected)
    {
      if (!_update_in_progress)
        {
          _update_in_progress = true;

          foreach (QModelIndex index, selected.indexes ())
            {
              QString id = _filter->data (index, Database::DatabaseModel::Role::ID).toString ();

              if (!_database->getPoint (id).getSelected ())
                _database->select (id);
            }

          foreach (QModelIndex index, deselected.indexes ())
            {
              QString id = _filter->data (index, Database::DatabaseModel::Role::ID).toString ();

              if (_database->getPoint (id).getSelected ())
                _database->deselect (id);
            }

          _update_in_progress = false;
        }
    }

    void PointExplorerView::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      if (!_update_in_progress)
        {
          _update_in_progress = true;

          switch (reason)
            {
            case Database::Database::Reason::SELECTION:
              {
                Q_ASSERT (data.type () == QVariant::String);

                QString id = data.toString ();
                Q_ASSERT (!id.isEmpty ());

                QModelIndex index = _filter->mapFromSource (_model->getIndex (id));
                Q_ASSERT (index.isValid ());

                if (_database->getPoint (id).getSelected ())
                  _ui->_tree_w->selectionModel ()->select (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                else
                  _ui->_tree_w->selectionModel ()->select (index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
              }
              break;

            case Database::Database::Reason::DATA:
            case Database::Database::Reason::POINT:
            case Database::Database::Reason::FILTER:
              break;
            }

          _update_in_progress = false;
        }
    }

  }
}

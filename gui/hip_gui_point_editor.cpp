/*
 * hip_gui_point_editor.cpp - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiPointEditor.h"
#include "ui_hip_gui_pointeditor.h"

#include "database/HIPDatabase.h"

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QDebug>

namespace HIP {
  namespace Gui {

    //#**********************************************************************
    // CLASS HIP::Gui::PointEditor::ImageViewInterface
    //#**********************************************************************

    /*! Constructor */
    PointEditor::ImageViewInterface::ImageViewInterface (QObject* parent)
      : QObject (parent)
    {
    }

    /*! Destructor */
    PointEditor::ImageViewInterface::~ImageViewInterface ()
    {
    }


    //#**********************************************************************
    // CLASS HIP::Gui::PointEditorModel
    //#**********************************************************************

    /*
     * Model for the point editor positions
     */
    class PointEditorModel : public QAbstractItemModel
    {
    public:
      struct Column { enum Type_t { IMAGE, COORDINATE }; };
      typedef Column::Type_t Columm_t;

      struct Role { enum Type_t { IMAGE_ID=Qt::UserRole + 1, COORDINATE }; };
      typedef Role::Type_t Role_t;

    public:
      PointEditorModel (Database::Database* database, QObject* parent);
      virtual ~PointEditorModel ();

      void setPoint (const Database::Point& point);
      QModelIndex getImageIndex (const QString& id) const;

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual bool setData (const QModelIndex& index, const QVariant& value, int role);
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private:
      Database::Database* _database;
      QString _id;
      QList<Database::Position> _positions;
    };


    /* Constructor */
    PointEditorModel::PointEditorModel (Database::Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database  (database),
        _id        (),
        _positions ()
    {
    }

    /*! Destructor */
    PointEditorModel::~PointEditorModel ()
    {
    }

    /*! Set point to be displayed */
    void PointEditorModel::setPoint (const Database::Point& point)
    {
      beginResetModel ();

      _id = point.getId ();
      _positions = point.getPositions ();

      endResetModel ();
    }

    /*! Get the model index of the given image */
    QModelIndex PointEditorModel::getImageIndex (const QString& id) const
    {
      QModelIndex result;

      for (int i=0; i < _positions.size () && !result.isValid (); ++i)
        if (_positions[i].getImage () == id)
          result = index (i, 0, QModelIndex ());

      return result;
    }

    int PointEditorModel::columnCount (const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return 2;
    }

    int PointEditorModel::rowCount (const QModelIndex& parent) const
    {
      return !parent.isValid () ? _positions.size () : 0;
    }

    QModelIndex PointEditorModel::index (int row, int column, const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return createIndex (row, column);
    }

    QModelIndex PointEditorModel::parent (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return QModelIndex ();
    }

    Qt::ItemFlags PointEditorModel::flags (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    QVariant PointEditorModel::data (const QModelIndex& index, int role) const
    {
      QVariant result;

      if (index.isValid () && index.row () < _positions.size ())
        {
          const Database::Position& position = _positions[index.row ()];

          switch (role)
            {
            case Qt::DisplayRole:
              {
                if (index.column () == Column::IMAGE)
                  result = qVariantFromValue (_database->getImage (position.getImage ()).getTitle ());
                else if (index.column () == Column::COORDINATE)
                  result = qVariantFromValue (QString ("(%1, %2)")
                                              .arg (static_cast<int> (position.getCoordinate ().x ()))
                                              .arg (static_cast<int> (position.getCoordinate ().y ())));
              }
              break;

            case Role::IMAGE_ID:
              result = qVariantFromValue (position.getImage ());
              break;
            }
        }

      return result;
    }

    bool PointEditorModel::setData (const QModelIndex& index, const QVariant& value, int role)
    {
      bool modified = false;

      Q_UNUSED (value);
      Q_UNUSED (role);

      if (index.isValid () && index.row () < _positions.size ())
        {
          Database::Position position = _positions[index.row ()];
          Q_UNUSED (position);

          if (index.column () == Column::IMAGE)
            {
            }

          if (role == Role::COORDINATE)
            {
              position.setCoordinate (value.value<QPointF> ());
              _database->setPosition (_id, position);
              emit dataChanged (this->index (index.row (), Column::IMAGE, QModelIndex ()),
                                this->index (index.row (), Column::COORDINATE, QModelIndex ()));
            }
        }

      return modified;
    }

    QVariant PointEditorModel::headerData (int section, Qt::Orientation orientation, int role) const
    {
      QVariant data;

      Q_UNUSED (orientation);

      if (role == Qt::DisplayRole)
        {
          if (section == Column::IMAGE)
            data = QVariant (tr ("Image"));
          else if (section == Column::COORDINATE)
            data = QVariant (tr ("Coordinate"));
        }

      return data;
    }


    //#**********************************************************************
    // CLASS HIP::Gui::PointEditor
    //#**********************************************************************

    /*! Constructor */
    PointEditor::PointEditor (Database::Database* database, QWidget* parent)
      : QWidget(parent),
        _ui       (new Ui::HIP_Gui_PointEditor),
        _database (database),
        _model    (new PointEditorModel (database, this)),
        _iv       (0)
    {
      _ui->setupUi (this);
      _ui->_positions_w->setModel (_model);

      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::IMAGE, QHeaderView::ResizeToContents);
      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::COORDINATE, QHeaderView::Stretch);

      connect (_ui->_add_w, SIGNAL (clicked (bool)), SLOT (onAdd ()));
      connect (_ui->_remove_w, SIGNAL (clicked (bool)), SLOT (onRemove ()));
      connect (_ui->_edit_w, SIGNAL (clicked (bool)), SLOT (onEdit ()));

      connect (_ui->_positions_w->selectionModel (),
               SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
               SLOT (onPositionSelectionChanged (const QItemSelection&)));

      updateSensitivity ();
    }

    /*! Destructor */
    PointEditor::~PointEditor ()
    {
      delete _ui;
    }

    /*! Set image view interface */
    void PointEditor::setImageViewInterface (ImageViewInterface* iv)
    {
      _iv = iv;
    }

    /*! Called when a new point should be added */
    void PointEditor::onAdd ()
    {
      updateSensitivity ();
    }

    /*! Called when the current point shall be removed */
    void PointEditor::onRemove ()
    {
      updateSensitivity ();
    }

    /*! Called when the coordinates of the current point should be edited */
    void PointEditor::onEdit ()
    {
      QModelIndexList indices = _ui->_positions_w->selectionModel ()->selectedRows ();
      Q_ASSERT (indices.size () == 1);
      Q_ASSERT (_iv != 0);

      QModelIndex index = indices.front ();

      QPointF coordinate;
      if (_iv->selectCoordinate (_model->data (index, PointEditorModel::Role::IMAGE_ID).toString (), &coordinate))
        _model->setData (index, qVariantFromValue (coordinate), PointEditorModel::Role::COORDINATE);

      updateSensitivity ();
    }

    /*! Update editor content on point selection changes */
    void PointEditor::onPointSelectionChanged (const QString& id)
    {
      const Database::Point& point = _database->getPoint (id);

      _ui->_name_w->setText (point.getId ());
      _ui->_description_w->setText (point.getDescription ());

      QString tags;
      QString separator;
      foreach (const QString& tag, point.getTags ())
        {
          tags += separator + tag;
          separator = ",";
        }

      _ui->_tags_w->setText (tags);

      _model->setPoint (point);

      updateSensitivity ();
    }

    /*! Called when the selection in the positions list changed */
    void PointEditor::onPositionSelectionChanged (const QItemSelection& selected)
    {
      foreach (const QModelIndex& index, selected.indexes ())
        emit imageSelected (_model->data (index, PointEditorModel::Role::IMAGE_ID).toString ());

      updateSensitivity ();
    }

    /*! Called when the currently displayed image changed */
    void PointEditor::onCurrentImageChanged (const QString& id)
    {
      QModelIndex index = _model->getImageIndex (id);
      if (index.isValid ())
        _ui->_positions_w->selectionModel ()->select (index, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
      else
        _ui->_positions_w->selectionModel ()->clear ();
    }

    void PointEditor::updateSensitivity ()
    {
      bool selected = !_ui->_positions_w->selectionModel ()->selectedRows ().isEmpty ();
      _ui->_remove_w->setEnabled (selected);
      _ui->_edit_w->setEnabled (selected);
    }

  }
}
/*
 * hip_gui_point_editor.cpp - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiPointEditor.h"
#include "ui_hip_gui_pointeditor.h"

#include "database/HIPDatabase.h"

#include <QAbstractItemModel>
#include <QColorDialog>
#include <QItemSelectionModel>
#include <QFont>
#include <QFontMetrics>
#include <QMessageBox>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QSignalBlocker>
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

      void reset ();

      const Database::Point& getPoint () const;
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

    /*! Reset model */
    void PointEditorModel::reset ()
    {
      beginResetModel ();
      endResetModel ();
    }

    /*! Get edited point */
    const Database::Point& PointEditorModel::getPoint () const
    {
      return _database->getPoint (_id);
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

      connect (_ui->_name_w, SIGNAL (textChanged (const QString&)), SLOT (onCommit ()));
      connect (_ui->_description_w, SIGNAL (textChanged (const QString&)), SLOT (onCommit ()));
      connect (_ui->_tags_w, SIGNAL (textChanged (const QString&)), SLOT (onCommit ()));
      connect (_ui->_color_w, SIGNAL (clicked (bool)), SLOT (onSelectColor ()));

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

    /*! Commit changes to database */
    void PointEditor::onCommit ()
    {
      Database::Point point = _model->getPoint ();

      point.setDescription (_ui->_description_w->text ());

      QStringList tags = _ui->_tags_w->text ().split (',');
      for (int i=0; i < tags.size (); ++i)
        tags[i] = tags[i].trimmed ();

      point.setTags (tags);

      _database->setPoint (point.getId (), point);
    }

    /*! Called when a new point should be added */
    void PointEditor::onAdd ()
    {
      QMessageBox::warning (this, tr ("Not implemented"), tr ("Adding image locations interactively is not implemented yet.<p>Export and edit database instead."));
      updateSensitivity ();
    }

    /*! Called when the current point shall be removed */
    void PointEditor::onRemove ()
    {
      QSet<QString> ids;
      foreach (const QModelIndex& index, _ui->_positions_w->selectionModel ()->selectedRows ())
        ids.insert (_model->data (index, PointEditorModel::Role::IMAGE_ID).toString ());

      Database::Point point = _model->getPoint ();

      QList<Database::Position> positions;
      foreach (const Database::Position& position, point.getPositions ())
        if (!ids.contains (position.getImage ()))
          positions.push_back (position);

      point.setPositions (positions);
      _database->setPoint (point.getId (), point);
      _model->reset ();

      onPointSelectionChanged (point.getId ());
    }

    /*! Called when the coordinates of the current point should be edited */
    void PointEditor::onEdit ()
    {
      QModelIndexList indices = _ui->_positions_w->selectionModel ()->selectedRows ();
      Q_ASSERT (indices.size () == 1);
      Q_ASSERT (_iv != 0);

      QModelIndex index = indices.front ();

      setEnabled (false);

      QPointF coordinate;
      if (_iv->selectCoordinate (_model->data (index, PointEditorModel::Role::IMAGE_ID).toString (), &coordinate))
        {
          _model->setData (index, qVariantFromValue (coordinate), PointEditorModel::Role::COORDINATE);
          _database->setSelected (_model->getPoint ().getId (), Database::Database::SelectionMode::EXCLUSIV);
        }

      setEnabled (true);

      updateSensitivity ();
    }

    /*! Select point color */
    void PointEditor::onSelectColor ()
    {
      Database::Point point = _model->getPoint ();

      QColor color = QColorDialog::getColor (_model->getPoint ().getColor (), this, tr ("Select point color"));
      if (color.isValid ())
        {
          point.setColor (color);
          _database->setPoint (point.getId (), point);
          updateColorButton ();
        }
    }

    /*! Update editor content on point selection changes */
    void PointEditor::onPointSelectionChanged (const QString& id)
    {
      QSignalBlocker blocker (this);

      const Database::Point& point = _database->getPoint (id);

      QSignalBlocker name_blocker (_ui->_name_w);
      _ui->_name_w->setText (point.getId ());

      QSignalBlocker description_blocker (_ui->_description_w);
      _ui->_description_w->setText (point.getDescription ());

      QString tags;
      QString separator;
      foreach (const QString& tag, point.getTags ())
        {
          tags += separator + tag;
          separator = ",";
        }

      QSignalBlocker tags_blocker (_ui->_tags_w);
      _ui->_tags_w->setText (tags);

      _model->setPoint (point);

      updateColorButton ();
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

    /*! Update color displayed in the color selection button */
    void PointEditor::updateColorButton ()
    {
      QFontMetrics metrics (_ui->_color_w->font ());
      QPixmap icon (metrics.height (), metrics.height ());

      {
        QPainter painter (&icon);
        painter.fillRect (QRectF (0, 0, icon.width (), icon.height ()), _model->getPoint ().getColor ());
      }

      _ui->_color_w->setIcon (icon);
    }

    /*! Update widget sensitivities */
    void PointEditor::updateSensitivity ()
    {
      bool position_selected = !_ui->_positions_w->selectionModel ()->selectedRows ().isEmpty ();

      bool point_selected = false;
      foreach (const Database::Point& point, _database->getPoints ())
        if (point.getSelected ())
          point_selected = true;

      _ui->_color_w->setEnabled (point_selected);
      _ui->_remove_w->setEnabled (position_selected);
      _ui->_edit_w->setEnabled (position_selected);
    }

  }
}

/* * hip_gui_point_editor.cpp - Editor for the point database
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


    /* Constructor */
    PointEditorModel::PointEditorModel (Database::Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database  (database),
        _point     ()
    {
      connect (_database, &Database::Database::databaseChanged, this, &PointEditorModel::onDatabaseChanged);
    }

    /*! Destructor */
    PointEditorModel::~PointEditorModel ()
    {
    }

    QModelIndex PointEditorModel::getIndex (const QString& image_id) const
    {
      QModelIndex index;

      for (int i=0; i < _point.getPositions ().size () && !index.isValid (); ++i)
        if (_point.getPositions ()[i].getImage () == image_id)
          index = this->index (i, 0, QModelIndex ());

      return index;
    }

    int PointEditorModel::columnCount (const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return 2;
    }

    int PointEditorModel::rowCount (const QModelIndex& parent) const
    {
      return _point.isValid () && !parent.isValid () ? _point.getPositions ().size () : 0;
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

      if (index.isValid () && index.row () < _point.getPositions ().size ())
        {
          const Database::Position& position = _point.getPositions ()[index.row ()];

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

    void PointEditorModel::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      Q_UNUSED (data);

      switch (reason)
        {
        case Database::Database::Reason::POINT:
        case Database::Database::Reason::SELECTION:
        case Database::Database::Reason::DATA:
          {
            beginResetModel ();

            QList<Database::Point> points;
            foreach (const Database::Point& point, _database->getPoints ())
              if (point.getSelected ())
                points.push_back (point);

            _point = points.size () == 1 ? points.front () : Database::Point ();

            endResetModel ();
          }
          break;

        case Database::Database::Reason::FILTER:
        case Database::Database::Reason::VISIBLE_IMAGE:
          break;
        }
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
        _iv       (0),
        _point    ()
    {
      _ui->setupUi (this);

      _ui->_positions_w->setModel (_model);

      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::IMAGE, QHeaderView::ResizeToContents);
      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::COORDINATE, QHeaderView::Stretch);

      connect (_ui->_description_w, SIGNAL (textChanged (const QString&)), SLOT (onValueChanged ()));
      connect (_ui->_tags_w, SIGNAL (textChanged (const QString&)), SLOT (onValueChanged ()));
      connect (_ui->_color_w, SIGNAL (clicked (bool)), SLOT (onSelectColor ()));

      connect (_ui->_add_w, SIGNAL (clicked (bool)), SLOT (onAdd ()));
      connect (_ui->_remove_w, SIGNAL (clicked (bool)), SLOT (onRemove ()));
      connect (_ui->_edit_w, SIGNAL (clicked (bool)), SLOT (onEdit ()));

      connect (_database, &Database::Database::databaseChanged, this, &PointEditor::onDatabaseChanged);
      connect (_ui->_positions_w->selectionModel (), &QItemSelectionModel::selectionChanged, this, &PointEditor::onSelectionChanged);

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
    void PointEditor::onValueChanged ()
    {
      _point.setDescription (_ui->_description_w->text ());

      QStringList tags = _ui->_tags_w->text ().split (',');
      for (int i=0; i < tags.size (); ++i)
        tags[i] = tags[i].trimmed ();

      _point.setTags (tags);
      _database->setPoint (_point);
    }

    /*! Called when a new point should be added */
    void PointEditor::onAdd ()
    {
      QString image_id = _database->getVisibleImage ();

      QPointF coordinate;
      if (_iv->selectCoordinate (image_id, &coordinate))
        {
          QList<Database::Position> positions = _point.getPositions ();

          Database::Position position;
          position.setImage (image_id);
          position.setCoordinate (coordinate);

          positions.push_back (position);

          _point.setPositions (positions);

          QString id = _point.getId ();
          _database->setPoint (_point);

          _database->clearSelection ();
          _database->select (id);
          _database->setVisibleImage (image_id);
        }

      updateSensitivity ();
    }

    /*! Called when the current point shall be removed */
    void PointEditor::onRemove ()
    {
      QSet<QString> ids;
      foreach (const QModelIndex& index, _ui->_positions_w->selectionModel ()->selectedRows ())
        ids.insert (_model->data (index, PointEditorModel::Role::IMAGE_ID).toString ());

      QList<Database::Position> positions;
      foreach (const Database::Position& position, _point.getPositions ())
        if (!ids.contains (position.getImage ()))
          positions.push_back (position);

      _point.setPositions (positions);
      _database->setPoint (_point);
    }

    /*! Called when the coordinates of the current point should be edited */
    void PointEditor::onEdit ()
    {
      QModelIndexList indices = _ui->_positions_w->selectionModel ()->selectedRows ();
      Q_ASSERT (indices.size () == 1);
      Q_ASSERT (_iv != 0);

      QModelIndex index = indices.front ();
      QString image_id = _model->data (index, PointEditorModel::Role::IMAGE_ID).toString ();

      setEnabled (false);

      QPointF coordinate;
      if (_iv->selectCoordinate (image_id, &coordinate))
        {
          QList<Database::Position> positions = _point.getPositions ();

          for (int i=0; i < positions.size (); ++i)
            if (positions[i].getImage () == image_id)
              positions[i].setCoordinate (coordinate);

          _point.setPositions (positions);

          QString id = _point.getId ();
          _database->setPoint (_point);

          _database->clearSelection ();
          _database->select (id);
          _database->setVisibleImage (image_id);
        }

      setEnabled (true);
      updateSensitivity ();
    }

    /*! Select point color */
    void PointEditor::onSelectColor ()
    {
      QColor color = QColorDialog::getColor (_point.getColor (), this, tr ("Select point color"));
      if (color.isValid ())
        {
          _point.setColor (color);
          _database->setPoint (_point);
          updateColorButton ();
        }
    }

    void PointEditor::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      Q_UNUSED (data);

      switch (reason)
        {
        case Database::Database::Reason::DATA:
        case Database::Database::Reason::SELECTION:
        case Database::Database::Reason::POINT:
          {
            QList<Database::Point> points;

            foreach (const Database::Point& point, _database->getPoints ())
              if (point.getSelected ())
                points.push_back (point);

            _point = points.size () == 1 ? points.front () : Database::Point ();
          }
          break;

        case Database::Database::Reason::FILTER:
          break;

        case Database::Database::Reason::VISIBLE_IMAGE:
          {
            Q_ASSERT (data.type () == QVariant::String);

            QModelIndex index =  _model->getIndex (data.toString ());
            if (index.isValid ())
              _ui->_positions_w->selectionModel ()->select (index, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            else
              _ui->_positions_w->selectionModel ()->select (index, QItemSelectionModel::Clear | QItemSelectionModel::Rows);
          }
          break;
        }

      QSignalBlocker description_blocker (_ui->_description_w);
      QSignalBlocker tags_blocker (_ui->_tags_w);

      if (_point.isValid ())
        {
          _ui->_name_w->setText (_point.getId ());
          _ui->_description_w->setText (_point.getDescription ());
          _ui->_tags_w->setText (QStringList (_point.getTags ()).join (","));
        }
      else
        {
          _ui->_name_w->clear ();
          _ui->_description_w->clear ();
          _ui->_tags_w->clear ();
        }

      updateColorButton ();
      updateSensitivity ();
    }

    /*! Called when the selection in the positions list changed */
    void PointEditor::onSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected)
    {
      Q_UNUSED (selected);
      Q_UNUSED (deselected);

      if (_ui->_positions_w->selectionModel ()->selectedRows ().size () == 1)
        {
          _database->setVisibleImage (_model->data (_ui->_positions_w->selectionModel ()->selectedRows ().front (),
                                                    PointEditorModel::Role::IMAGE_ID).toString ());
          updateSensitivity ();
        }
    }

    /*! Update color displayed in the color selection button */
    void PointEditor::updateColorButton ()
    {
      QFontMetrics metrics (_ui->_color_w->font ());
      QPixmap icon (metrics.height (), metrics.height ());

      {
        QPainter painter (&icon);
        painter.fillRect (QRectF (0, 0, icon.width (), icon.height ()), _point.getColor ());
      }

      QSignalBlocker blocker (_ui->_color_w);
      _ui->_color_w->setIcon (icon);
    }

    /*! Update widget sensitivities */
    void PointEditor::updateSensitivity ()
    {
      bool selected = !_ui->_positions_w->selectionModel ()->selectedRows ().isEmpty ();
      bool current_image_present = false;

      foreach (const Database::Position& position, _point.getPositions ())
        if (position.getImage () == _database->getVisibleImage ())
          current_image_present = true;

      _ui->_positions_w->setEnabled (_point.isValid ());
      _ui->_color_w->setEnabled (_point.isValid ());
      _ui->_add_w->setEnabled (_point.isValid () && !current_image_present);
      _ui->_remove_w->setEnabled (selected);
      _ui->_edit_w->setEnabled (selected);
    }

  }
}

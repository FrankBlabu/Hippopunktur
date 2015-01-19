/*
 * hip_gui_point_editor.cpp - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiPointEditor.h"
#include "ui_hip_gui_pointeditor.h"

#include "database/HIPDatabase.h"

#include <QAbstractItemModel>
#include <QLineEdit>
#include <QDebug>

namespace HIP {
  namespace Gui {

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

    public:
      PointEditorModel (QObject* parent);
      virtual ~PointEditorModel ();

      void setPoint (const Database::Point& point);

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;
      virtual bool setData (const QModelIndex& index, const QVariant& value, int role);
      virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

    private:
      QString _id;
      QList<Database::Position> _positions;
    };


    /* Constructor */
    PointEditorModel::PointEditorModel (QObject* parent)
      : QAbstractItemModel (parent),
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
                  result = qVariantFromValue (position.getImage ());
                else if (index.column () == Column::COORDINATE)
                  {}
              }
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

          if (index.column () == Column::IMAGE)
            {
            }
          else if (index.column () == Column::COORDINATE)
            {
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
        _model    (new PointEditorModel (this))
    {
      _ui->setupUi (this);
      _ui->_positions_w->setModel (_model);

      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::IMAGE, QHeaderView::ResizeToContents);
      _ui->_positions_w->header ()->setSectionResizeMode (PointEditorModel::Column::COORDINATE, QHeaderView::Stretch);
    }

    /*! Destructor */
    PointEditor::~PointEditor ()
    {
      delete _ui;
    }

    /*! Update editor content on point selection changes */
    void PointEditor::onSelectionChanged (const QString& id)
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
    }

  }
}

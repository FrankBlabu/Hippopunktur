/*
 * HIPGuiPointEditor.h - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGuiPointEditor_h__
#define __HIPGuiPointEditor_h__

#include <QWidget>
#include <QPointF>
#include <QAbstractItemModel>

#include "database/HIPDatabase.h"

class QItemSelection;

namespace Ui {
  class HIP_Gui_PointEditor;
}

namespace HIP {
  namespace Gui {

    /*
     * Model for the point editor positions
     */
    class PointEditorModel : public QAbstractItemModel
    {
      Q_OBJECT

    public:
      struct Column { enum Type_t { IMAGE, COORDINATE }; };
      typedef Column::Type_t Columm_t;

      struct Role { enum Type_t { IMAGE_ID=Qt::UserRole + 1, COORDINATE }; };
      typedef Role::Type_t Role_t;

    public:
      PointEditorModel (Database::Database* database, QObject* parent);
      virtual ~PointEditorModel ();

      QModelIndex getIndex (const QString& image_id) const;

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
      Database::Point _point;
    };

    /*!
     * Editor for the point database
     */
    class PointEditor : public QWidget
    {
      Q_OBJECT

    public:
      class ImageViewInterface : public QObject
      {
      public:
        ImageViewInterface (QObject* parent);
        virtual ~ImageViewInterface ();

        virtual QString getActiveImage () const = 0;
        virtual bool selectCoordinate (const QString& id, QPointF* coordinate) const = 0;
      };

    public:
      PointEditor (Database::Database* database, QWidget* parent);
      virtual ~PointEditor ();

      void setImageViewInterface (ImageViewInterface* iv);

    public slots:
      void onAdd ();
      void onRemove ();
      void onEdit ();
      void onSelectColor ();

      void onValueChanged ();
      void onSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);

    signals:
      void imageSelected (const QString& id);

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);

    private:
      void updateColorButton ();
      void updateSensitivity ();

    private:
      Ui::HIP_Gui_PointEditor* _ui;
      Database::Database* _database;
      PointEditorModel* _model;
      ImageViewInterface* _iv;

      Database::Point _point;
    };

  }
}

#endif

/*
 * HIPGuiPointEditor.h - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGuiPointEditor_h__
#define __HIPGuiPointEditor_h__

#include <QWidget>
#include <QPointF>

class QItemSelection;

namespace Ui {
  class HIP_Gui_PointEditor;
}

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace Gui {

    class PointEditorModel;

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

      void onPointSelectionChanged (const QString& id);
      void onPositionSelectionChanged (const QItemSelection& selected);
      void onCurrentImageChanged (const QString& id);

    signals:
      void imageSelected (const QString& id);

    private:
      void updateSensitivity ();

    private:
      Ui::HIP_Gui_PointEditor* _ui;
      Database::Database* _database;
      PointEditorModel* _model;
      ImageViewInterface* _iv;
    };

  }
}

#endif

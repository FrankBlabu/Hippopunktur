/*
 * HIPImageImageView.h - View displaying an acupunctur image
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPImageImageView_h__
#define __HIPImageImageView_h__

#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QPointF>

#include "database/HIPDatabase.h"

class QEventLoop;

namespace Ui {
  class HIP_Image_ImageView;
}

namespace HIP {

  namespace Tools {
    class ImageLoader;
  }

  namespace Image {

    /*!
     * Image drawing widget
     */
    class ImageWidget : public QWidget
    {
      Q_OBJECT;

    public:
      ImageWidget (Database::Database* database, const Database::Image& image, QWidget* parent);
      virtual ~ImageWidget ();

      bool selectCoordinate (QPointF* coordinate);
      void resetZoom ();

    protected:
      QPointF toPixmapPoint (const QPointF& widget_point) const;
      QPointF toWidgetPoint (const QPointF& pixmap_point) const;

      virtual bool event (QEvent* event);

      virtual void paintEvent (QPaintEvent* event);
      virtual void resizeEvent (QResizeEvent* event);

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private slots:
      void onImageLoaded ();
      void onDatabaseChanged (Database::Database::Reason_t reason, const QString& id);

    private:
      QString getPointAt (const QPointF& pos) const;
      void ensureBounds ();

      QRectF computeDefaultViewport () const;
      QString computeToolTipText (const QString& id) const;

    private:
      Database::Database* _database;
      Database::Image _image;
      QString _tag;

      QPixmap _pixmap; // Source pixmap
      Tools::ImageLoader* _loader;

      QRectF _viewport; // Source viewport in pixmap coordinates

      QPointF _clicked_point; // Origin for dragging movements in widget coordinates
      QRectF _dragged;

      QEventLoop* _selection;
      QPointF _selected_coordinate;

      static const double POINT_RADIUS;
    };

    /*
     * View displaying an acupunctur image
     */
    class ImageView : public QWidget
    {
      Q_OBJECT

    public:
      ImageView (Database::Database* database, const Database::Image& image, QWidget* parent);
      virtual ~ImageView ();

      const Database::Image& getImage () const { return _image; }

      bool selectCoordinate (QPointF* coordinate);

    private slots:
      void onResetZoom ();

    private:
      Ui::HIP_Image_ImageView* _ui;
      Database::Database* _database;
      Database::Image _image;
      ImageWidget* _widget;
    };

  }
}

#endif

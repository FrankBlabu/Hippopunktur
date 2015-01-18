/*
 * hip_image_image_view.cpp - View displaying an acupunctur image
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPImageImageView.h"
#include "ui_hip_image_imageview.h"

#include "core/HIPException.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"

#include <QImage>
#include <QPainter>
#include <QScrollArea>
#include <QWheelEvent>
#include <QDebug>

namespace HIP {
  namespace Image {

    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*!
     * Image drawing widget
     */
    class ImageWidget : public QWidget
    {
    public:
      ImageWidget (const Database::Image& image, QWidget* parent);
      virtual ~ImageWidget ();

      void resetZoom ();

    protected:
      virtual void paintEvent (QPaintEvent* event);
      virtual void resizeEvent (QResizeEvent* event);

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      QRectF computeDefaultViewport () const;
      QPointF toPixmapPoint (const QPointF& widget_point) const;
      void ensureBounds ();

    private:
      QPixmap _pixmap;  // Source pixmap
      QRectF _viewport; // Source viewport in pixmap coordinates

      QPointF _clicked_point; // Origin for dragging movements in widget coordinates
      QRectF _dragged;
    };

    /*! Constructor */
    ImageWidget::ImageWidget (const Database::Image& image, QWidget* parent)
      : QWidget (parent),
        _pixmap        (image.getPath ()),
        _viewport      (),
        _clicked_point (),
        _dragged       ()
    {
      if (_pixmap.isNull ())
        throw Exception (tr ("Unable to load image '%1'").arg (image.getPath ()));

      qDebug () << "Pixmap size: " << _pixmap.size ();

      setMouseTracking (true);
    }

    /*! Destructor */
    ImageWidget::~ImageWidget ()
    {
    }

    /*! Computes the default maximum viewport size */
    QRectF ImageWidget::computeDefaultViewport () const
    {
      QSizeF pixmap_size (_pixmap.size ());
      QSizeF widget_size (size ());

      double scaling = 1.0;
      if (pixmap_size.width () / pixmap_size.height () < widget_size.width () / widget_size.height ())
        scaling = pixmap_size.height () / widget_size.height ();
      else
        scaling = pixmap_size.width () / widget_size.width ();

      QRectF r = QRectF (0, 0, width () * scaling, height () * scaling);
      r.moveCenter (_pixmap.rect ().center ());

      return r;
    }

    /* Reset zoom to original pixmap scale */
    void ImageWidget::resetZoom ()
    {
      _viewport = computeDefaultViewport ();
      update ();
    }

    /* Compute pixmap point matching the given widget point */
    QPointF ImageWidget::toPixmapPoint (const QPointF& p) const
    {
      double scaling = _viewport.width () / width ();
      QPointF point (_viewport.x () + p.x () * scaling, _viewport.y () + p.y () * scaling);

      if (_viewport.height () > _pixmap.rect ().height ())
        point.setY (point.y () - (_viewport.height () - _pixmap.rect ().height ()) / 2);
      else
        point.setY (point.y () + _viewport.y ());

      if (_viewport.width () > _pixmap.rect ().width ())
        point.setX (point.x () - (_viewport.width () - _pixmap.rect ().width ()) / 2);
      else
        point.setX (point.x () + _viewport.x ());

      return point;
    }

    /* Paint widget */
    void ImageWidget::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      QPainter painter (this);
      painter.drawPixmap (rect (), _pixmap, _viewport);
    }

    void ImageWidget::resizeEvent (QResizeEvent* event)
    {
      QWidget::resizeEvent (event);
      resetZoom ();
    }

    void ImageWidget::mousePressEvent (QMouseEvent* event)
    {
      _clicked_point = event->pos ();

      if (event->buttons ().testFlag (Qt::MidButton))
        {
          setCursor (Qt::SizeAllCursor);
          _dragged = _viewport;
        }
    }

    void ImageWidget::mouseMoveEvent (QMouseEvent* event)
    {
      if (event->buttons ().testFlag (Qt::MidButton))
        {
          QPointF delta = (_clicked_point - event->pos ()) * _viewport.width () / width ();
          _viewport = _dragged;
          _viewport.moveCenter (_viewport.center () + delta);

          ensureBounds ();
          update ();
        }
    }

    void ImageWidget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);
      unsetCursor ();
    }

    /*! Handle mouse wheel events */
    void ImageWidget::wheelEvent (QWheelEvent* event)
    {
      double scaling = 0.1 * (event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);
      QPointF delta (_viewport.width () * scaling, _viewport.height () * scaling);

      QRectF v = _viewport;
      v.setTopLeft (_viewport.topLeft () + delta);
      v.setBottomRight (_viewport.bottomRight () - delta);

      if (v.width () > 64 && v.height () > 64)
        {
          _viewport = v;
          ensureBounds ();
          update ();
        }
    }

    /*
     * Ensure that the viewportvrect is within pixmap bounds
     */
    void ImageWidget::ensureBounds ()
    {
      if ( _viewport.width () > _pixmap.width () &&
           _viewport.height () > _pixmap.height () )
        resetZoom ();

      QRectF v = computeDefaultViewport ();

      if (_viewport.left () < v.left ())
        _viewport.moveLeft (v.left ());
      else if (_viewport.right () > v.right ())
        _viewport.moveRight (v.right ());

      if (_viewport.top () < v.top ())
        _viewport.moveTop (v.top ());
      else if (_viewport.bottom () > v.bottom ())
        _viewport.moveBottom (v.bottom ());
    }


    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Constructor */
    ImageView::ImageView (const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui     (new Ui::HIP_Image_ImageView),
      _widget (0)
    {
      _ui->setupUi (this);
      _widget = Tools::addToParent (new ImageWidget (image, _ui->_view_w));

      connect (_ui->_reset_zoom_w, SIGNAL (clicked ()), SLOT (onResetZoom ()));
    }

    /*! Destructor */
    ImageView::~ImageView ()
    {
      delete _ui;
    }

    void ImageView::onResetZoom ()
    {
      _widget->resetZoom ();
    }

  }
}

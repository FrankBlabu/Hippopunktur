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

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      QPointF toPixmapPoint (const QPointF& widget_point) const;
      QRectF ensureBounds (const QRectF& r) const;

    private:
      QPixmap _pixmap;
      QRectF _source_rect;
      QRectF _target_rect;

      QPointF _clicked_point;
    };

    /*! Constructor */
    ImageWidget::ImageWidget (const Database::Image& image, QWidget* parent)
      : QWidget (parent),
        _pixmap        (image.getPath ()),
        _source_rect   (_pixmap.rect ()),
        _target_rect   (),
        _clicked_point ()
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

    /* Reset zoom to original pixmap scale */
    void ImageWidget::resetZoom ()
    {
      _source_rect = _pixmap.rect ();
      update ();
    }

    /* Compute pixmap point matching the given widget point */
    QPointF ImageWidget::toPixmapPoint (const QPointF& p) const
    {
       QPointF source_point (_source_rect.width () *
                             (p.x () - _target_rect.left ()) / _target_rect.width (),
                             _source_rect.height () *
                             (p.y () - _target_rect.top ()) / _target_rect.height ());

       return _source_rect.topLeft () + source_point;
    }

    /* Paint widget */
    void ImageWidget::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      _target_rect = rect ();

      //
      // Aspect ratio > 1 --> Fit width
      //
      if ( _source_rect.width () / _source_rect.height () >
           _target_rect.width () / _target_rect.height () )
        {
          double height = _target_rect.width () * (_source_rect.height () / _source_rect.width ());
          _target_rect = QRectF (_target_rect.left (), _target_rect.center ().y () - height / 2.0,
                                _target_rect.width (), height);
        }

      //
      // Aspect ratio < 1 --> Fit height
      //
      else
        {
          double width = _target_rect.height () * (_source_rect.width () / _source_rect.height ());
          _target_rect = QRectF (_target_rect.center ().x () - width / 2.0, _target_rect.top (),
                                width, _target_rect.height ());
        }

      QPainter painter (this);

#if 0
      painter.setPen (QPen (Qt::red, 3.0));
      painter.drawRect (_target_rect);
#endif

      painter.drawPixmap (_target_rect, _pixmap, _source_rect);
    }

    void ImageWidget::mousePressEvent (QMouseEvent* event)
    {
      //qDebug () << "Click: " << toPixmapPoint (event->pos ());

      _clicked_point = toPixmapPoint (event->pos ());

      if (event->buttons ().testFlag (Qt::MidButton))
        setCursor (Qt::SizeAllCursor);
    }

    void ImageWidget::mouseMoveEvent (QMouseEvent* event)
    {
      if (event->buttons ().testFlag (Qt::MidButton))
        {
          _source_rect.moveCenter (_source_rect.center () - (toPixmapPoint (event->pos ()) - _clicked_point));
          _source_rect = ensureBounds (_source_rect);
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
      double delta = -(event->angleDelta ().x () + event->angleDelta ().y ()) / (15 * 8);

      QRectF r (_source_rect.x (), _source_rect.y (),
                _source_rect.width () + _source_rect.width () * 0.1 * delta,
                _source_rect.height () + _source_rect.height () * 0.1 * delta);
      r.moveCenter (toPixmapPoint (event->posF ()));

      if (r.width () >= _pixmap.width () || r.height () >= _pixmap.height ())
        _source_rect = _pixmap.rect ();
      else if (r.width () > 64 && r.height () > 64)
        _source_rect = ensureBounds (r);

      update ();
    }

    /*
     * Ensure that the given source rect is within pixmap bounds
     */
    QRectF ImageWidget::ensureBounds (const QRectF& rect) const
    {
      QRectF r = rect;

      if (r.left () < _pixmap.rect ().left ())
        r.moveLeft (_pixmap.rect ().left ());
      if (r.right () > _pixmap.rect ().right ())
        r.moveRight (_pixmap.rect ().right ());
      if (r.top () < _pixmap.rect ().top ())
        r.moveTop (_pixmap.rect ().top ());
      if (r.bottom () > _pixmap.rect ().bottom ())
        r.moveBottom (_pixmap.rect ().bottom ());

      return r;
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

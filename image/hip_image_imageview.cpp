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
      ImageWidget (Database::Database* database, const Database::Image& image, QWidget* parent);
      virtual ~ImageWidget ();

      void resetZoom ();

      void updateAll ();
      void updatePoint (const QString& id);

    protected:
      QPointF toPixmapPoint (const QPointF& widget_point) const;
      QPointF toWidgetPoint (const QPointF& pixmap_point) const;

      virtual void paintEvent (QPaintEvent* event);
      virtual void resizeEvent (QResizeEvent* event);

      virtual void mousePressEvent (QMouseEvent* event);
      virtual void mouseMoveEvent (QMouseEvent* event);
      virtual void mouseReleaseEvent (QMouseEvent* event);
      virtual void wheelEvent (QWheelEvent* event);

    private:
      QRectF computeDefaultViewport () const;
      void ensureBounds ();

    private:
      Database::Database* _database;
      Database::Image _image;

      QPixmap _pixmap;  // Source pixmap
      QRectF _viewport; // Source viewport in pixmap coordinates

      QPointF _clicked_point; // Origin for dragging movements in widget coordinates
      QRectF _dragged;

      static const double POINT_RADIUS;
    };

    /*! Point radius [STATIC] */
    const double ImageWidget::POINT_RADIUS = 5.0;

    /*! Constructor */
    ImageWidget::ImageWidget (Database::Database* database, const Database::Image& image, QWidget* parent)
      : QWidget (parent),
        _database      (database),
        _image         (image),
        _pixmap        (image.getPath ()),
        _viewport      (),
        _clicked_point (),
        _dragged       ()
    {
      if (_pixmap.isNull ())
        throw Exception (tr ("Unable to load image '%1'").arg (image.getPath ()));

      qDebug () << "Pixmap size: " << _pixmap.size ();

      setMouseTracking (true);
      updateAll ();
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

    /* Update all point related data */
    void ImageWidget::updateAll ()
    {
    }

    /* Called when a single point has been changed and needs an update */
    void ImageWidget::updatePoint (const QString& id)
    {
      Q_UNUSED (id);
      updateAll ();
    }

    /* Compute pixmap point matching the given widget point */
    QPointF ImageWidget::toPixmapPoint (const QPointF& p) const
    {
      return QPointF (_viewport.x () + p.x () * ( _viewport.width () / width ()),
                      _viewport.y () + p.y () * ( _viewport.height () / height ()));
    }

    /* Compute widget point matching the given pixmap point */
    QPointF ImageWidget::toWidgetPoint (const QPointF& p) const
    {
      return QPointF ((p.x () - _viewport.x ()) * width () / _viewport.width (),
                      (p.y () - _viewport.y ()) * height () / _viewport.height ());
    }

    /* Paint widget */
    void ImageWidget::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      QPainter painter (this);
      painter.drawPixmap (rect (), _pixmap, _viewport);

      painter.setPen (QPen ());
      painter.setFont (QFont ());

      foreach (const Database::Point& point, _database->getPoints ())
        {
          foreach (const Database::Position& position, point.getPositions ())
            {
              if (position.getImage () == _image.getId ())
                {
                  QPointF p = toWidgetPoint (position.getCoordinate ().toPointF ());

                  painter.setBrush (point.getColor ());
                  painter.drawEllipse (toWidgetPoint (position.getCoordinate ().toPointF ()),
                                       POINT_RADIUS, POINT_RADIUS);
                  painter.drawText (p + QPointF (2.0 * POINT_RADIUS, POINT_RADIUS), point.getId ());
                }
            }
        }
    }

    void ImageWidget::resizeEvent (QResizeEvent* event)
    {
      QWidget::resizeEvent (event);
      resetZoom ();
    }

    void ImageWidget::mousePressEvent (QMouseEvent* event)
    {
      if (event->buttons ().testFlag (Qt::MidButton))
        {
          setCursor (Qt::SizeAllCursor);
          _clicked_point = event->pos ();
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
    ImageView::ImageView (Database::Database* database, const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui       (new Ui::HIP_Image_ImageView),
      _database (database),
      _widget   (0)
    {
      _ui->setupUi (this);
      _widget = Tools::addToParent (new ImageWidget (database, image, _ui->_view_w));

      connect (_database, &Database::Database::pointChanged, this, &ImageView::onPointChanged);
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

    void ImageView::onPointChanged (const QString &id)
    {
      _widget->updatePoint (id);
    }

  }
}

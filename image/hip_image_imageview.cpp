/* * hip_image_image_view.cpp - View displaying an acupunctur image
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPImageImageView.h"
#include "ui_hip_image_imageview.h"

#include "core/HIPException.h"
#include "core/HIPImageLoader.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"

#include <QImage>
#include <QPainter>
#include <QScrollArea>
#include <QToolTip>
#include <QWheelEvent>
#include <QDebug>

namespace HIP {
  namespace Image {


    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Point radius [STATIC] */
    const double ImageWidget::POINT_RADIUS = 5.0;

    /*! Constructor */
    ImageWidget::ImageWidget (Database::Database* database, const Database::Image& image, QWidget* parent)
      : QWidget (parent),
        _database      (database),
        _image         (image),
        _tag           (),
        _loader        (0),
        _pixmap        (),
        _viewport      (),
        _clicked_point (),
        _dragged       ()
    {
      setMouseTracking (true);

      _loader = new Tools::ImageLoader (image.getPath (), this);

      connect (_loader, SIGNAL (finished ()), SLOT (onImageLoaded ()), Qt::QueuedConnection);
    }

    /*! Destructor */
    ImageWidget::~ImageWidget ()
    {
    }

    /* Set filter tag */
    void ImageWidget::setTag (const QString& tag)
    {
      if (!_pixmap.isNull ())
        {
          _tag = tag;
          update ();
        }
    }

    /* Reset zoom to original pixmap scale */
    void ImageWidget::resetZoom ()
    {
      if (!_pixmap.isNull ())
        {
          _viewport = computeDefaultViewport ();
          update ();
        }
    }

    /* Called when a single point has been changed and needs an update */
    void ImageWidget::updatePoint (const QString& id)
    {
      Q_UNUSED (id);
      update ();
    }

    /* Compute pixmap point matching the given widget point */
    QPointF ImageWidget::toPixmapPoint (const QPointF& p) const
    {
      if (_pixmap.isNull ())
        return QPointF ();

      return QPointF (_viewport.x () + p.x () * ( _viewport.width () / width ()),
                      _viewport.y () + p.y () * ( _viewport.height () / height ()));
    }

    /* Compute widget point matching the given pixmap point */
    QPointF ImageWidget::toWidgetPoint (const QPointF& p) const
    {
      if (_pixmap.isNull ())
        return QPointF ();

      return QPointF ((p.x () - _viewport.x ()) * width () / _viewport.width (),
                      (p.y () - _viewport.y ()) * height () / _viewport.height ());
    }

    /* General widget event handling */
    bool ImageWidget::event (QEvent* e)
    {
      bool processed = false;

      if (!_pixmap.isNull () && e->type () == QEvent::ToolTip)
        {
          QHelpEvent* help_event = dynamic_cast<QHelpEvent*> (e);
          Q_ASSERT (help_event != 0);

          QString id = getPointAt (help_event->pos ());
          if (!id.isEmpty ())
            QToolTip::showText (help_event->globalPos (), computeToolTipText (id));
          else
            QToolTip::hideText ();
        }

      if (!processed)
        processed = QWidget::event (e);

      return processed;
    }


    /* Paint widget */
    void ImageWidget::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      if (!_pixmap.isNull ())
        {
          QColor unselected_color (0x00, 0x00, 0x00, 0x33);

          QPainter painter (this);
          painter.drawPixmap (rect (), _pixmap, _viewport);

          painter.setFont (QFont ());

          foreach (const Database::Point& point, _database->getPoints ())
            {
              if (point.matches (_tag))
                {
                  foreach (const Database::Position& position, point.getPositions ())
                    {
                      if (position.getImage () == _image.getId ())
                        {
                          QPointF p = toWidgetPoint (position.getCoordinate ().toPointF ());

                          painter.setPen (point.getSelected () ? point.getColor () : unselected_color);
                          painter.setBrush (point.getSelected () ? point.getColor () : unselected_color);
                          painter.drawEllipse (toWidgetPoint (position.getCoordinate ().toPointF ()),
                                               POINT_RADIUS, POINT_RADIUS);

                          painter.setPen (point.getSelected () ? QColor ("black") : unselected_color);
                          painter.drawText (p + QPointF (2.0 * POINT_RADIUS, POINT_RADIUS), point.getId ());
                        }
                    }
                }
            }
        }
    }

    void ImageWidget::resizeEvent (QResizeEvent* event)
    {
      QWidget::resizeEvent (event);
      resetZoom ();
    }

    /*
     * Returns the ID of the point close to the given widget position
     */
    QString ImageWidget::getPointAt (const QPointF& pos) const
    {
      QString id;

      if (!_pixmap.isNull ())
        {
          QPointF p = toPixmapPoint (pos);
          double min_distance = std::numeric_limits<double>::max ();

          foreach (const Database::Point& point, _database->getPoints ())
            {
              if (point.matches (_tag))
                {
                  foreach (const Database::Position& position, point.getPositions ())
                    {
                      if (position.getImage () == _image.getId ())
                        {
                          double distance = qAbs ((position.getCoordinate ().toPointF () - p).manhattanLength ());
                          if ( distance < min_distance &&
                               distance < 30 * POINT_RADIUS )
                            {
                              min_distance = distance;
                              id = point.getId ();
                            }
                        }
                    }
                }
            }
        }

      return id;
    }


    void ImageWidget::mousePressEvent (QMouseEvent* event)
    {
      if (!_pixmap.isNull ())
        {
          if (event->buttons ().testFlag (Qt::LeftButton))
            {
              QString id = getPointAt (event->pos ());
              if (!id.isEmpty ())
                {
                  Database::Database::SelectionMode mode = Database::Database::SELECT;

                  if (event->modifiers ().testFlag (Qt::ControlModifier))
                    {
                      if (_database->getPoint (id).getSelected ())
                        mode = Database::Database::DESELECT;
                      else
                        mode = Database::Database::SELECT;
                    }
                  else
                    mode = Database::Database::EXCLUSIV;

                  _database->setSelected (id, mode);
                }
            }
          else if (event->buttons ().testFlag (Qt::MidButton))
            {
              setCursor (Qt::SizeAllCursor);
              _clicked_point = event->pos ();
              _dragged = _viewport;
            }
        }
    }

    void ImageWidget::mouseMoveEvent (QMouseEvent* event)
    {
      if (!_pixmap.isNull ())
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
    }

    void ImageWidget::mouseReleaseEvent (QMouseEvent* event)
    {
      Q_UNUSED (event);

      if (!_pixmap.isNull ())
        unsetCursor ();
    }

    /*! Handle mouse wheel events */
    void ImageWidget::wheelEvent (QWheelEvent* event)
    {
      if (!_pixmap.isNull ())
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
    }

    /*
     * Called when the image has been loaded
     */
    void ImageWidget::onImageLoaded ()
    {
      Q_ASSERT (_loader != 0);
      _pixmap = QPixmap::fromImage (_loader->getImage ());

      delete _loader;
      _loader = 0;

      resetZoom ();
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

    /*! Compute tool tip text for the given id */
    QString ImageWidget::computeToolTipText (const QString& id) const
    {
      const Database::Point& point = _database->getPoint (id);

      QString text ("<b>%1</b><p>%2");

      return text.arg (Tools::quoteHTML (point.getId ()), Tools::quoteHTML (point.getDescription ()));
    }


    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Constructor */
    ImageView::ImageView (Database::Database* database, const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui       (new Ui::HIP_Image_ImageView),
      _database (database),
      _image    (image),
      _widget   (0)
    {
      _ui->setupUi (this);
      _widget = Tools::addToParent (new ImageWidget (database, image, _ui->_view_w));

      connect (_database, &Database::Database::pointChanged, this, &ImageView::onPointChanged);
      connect (_database, &Database::Database::selectionChanged, this, &ImageView::onPointChanged);
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

    void ImageView::onTagChanged (const QString &id)
    {
      _widget->setTag (id);
    }

  }
}

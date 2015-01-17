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

    protected:
      virtual void paintEvent (QPaintEvent* event);

    private:
      QPixmap _pixmap;
    };

    /*! Constructor */
    ImageWidget::ImageWidget (const Database::Image& image, QWidget* parent)
      : QWidget (parent),
        _pixmap (image.getPath ())
    {
      if (_pixmap.isNull ())
        throw Exception (tr ("Unable to load image '%1'").arg (image.getPath ()));

      qDebug () << _pixmap.size ();
    }

    /*! Destructor */
    ImageWidget::~ImageWidget ()
    {
    }

    /* Paint widget */
    void ImageWidget::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      QRectF source_rect = _pixmap.rect ();
      QRectF target_rect = rect ();

      //
      // Aspect ratio > 1 --> Fit width
      //
      if ( source_rect.width () / source_rect.height () >
           target_rect.width () / target_rect.height () )
        {
          double height = target_rect.width () * (source_rect.height () / source_rect.width ());
          target_rect = QRectF (target_rect.left (), target_rect.center ().y () - height / 2.0,
                                target_rect.width (), height);
        }

      //
      // Aspect ratio < 1 --> Fit height
      //
      else
        {
          double width = target_rect.height () * (source_rect.width () / source_rect.height ());
          target_rect = QRectF (target_rect.center ().x () - width / 2.0, target_rect.top (),
                                width, target_rect.height ());
        }

      QPainter painter (this);
      painter.drawPixmap (target_rect, _pixmap, source_rect);
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
    }

    /*! Destructor */
    ImageView::~ImageView ()
    {
      delete _ui;
    }

#if 0
    /*! Event filter */
    bool ImageView::eventFilter (QObject* obj, QEvent* event)
    {
      bool processed = false;

      //
      // Mouse wheel events
      //
      if (obj == _image && event->type () == QEvent::Wheel)
        {
          QWheelEvent* wheel_event = dynamic_cast<QWheelEvent*> (event);
          Q_ASSERT (wheel_event != 0);

          double delta = wheel_event->angleDelta ().x () + wheel_event->angleDelta ().y ();
          setScaling (_scaling + _scaling * (0.005 * delta / 15));

          processed = true;
        }

      return processed;
    }
#endif

  }
}

/*
 * hip_image_image_view.cpp - View displaying an acupunctur image
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPImageImageView.h"
#include "ui_hip_image_imageview.h"

#include "core/HIPException.h"
#include "database/HIPDatabase.h"

#include <QImage>
#include <QScrollArea>
#include <QWheelEvent>
#include <QDebug>

namespace HIP {
  namespace Image {

    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Constructor */
    ImageView::ImageView (const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui      (new Ui::HIP_Image_ImageView),
      _image   (new QLabel (this)),
      _pixmap  (),
      _scaling (1.0)
    {
      _ui->setupUi (this);

      QImage content (image.getPath ());
      if (content.isNull ())
        throw Exception (tr ("Unable to load image from path '%1'").arg (image.getPath ()));

      qDebug () << image.getPath () << ", " << content.size ();

      _pixmap = QPixmap::fromImage (content);

      _image->setBackgroundRole (QPalette::Base);
      _image->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
      _image->setScaledContents (true);
      _image->setPixmap (_pixmap);
      _image->adjustSize ();

      _ui->_view_w->setBackgroundRole (QPalette::Dark);
      _ui->_view_w->setWidgetResizable (false);
      _ui->_view_w->setWidget (_image);

      installEventFilter (this);
      _ui->_view_w->installEventFilter (this);
      _ui->_view_w->viewport ()->installEventFilter (this);
      _image->installEventFilter (this);
    }

    /*! Destructor */
    ImageView::~ImageView ()
    {
      delete _ui;
    }

    /*! Set image scaling factor */
    void ImageView::setScaling (double factor)
    {
      _scaling = qBound (0.01, factor, 20.0);
      _image->resize (_pixmap.size () * _scaling);
      qDebug () << "Scaling: " << _scaling;
    }

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


  }
}

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
#include <QDebug>

namespace HIP {
  namespace Image {

    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Constructor */
    ImageView::ImageView (const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui    (new Ui::HIP_Image_ImageView),
      _image (new QLabel (this))
    {
      Q_UNUSED (image);

      _ui->setupUi (this);

      QImage content (image.getPath ());
      if (content.isNull ())
        throw Exception (tr ("Unable to load image from path '%1'").arg (image.getPath ()));

      qDebug () << image.getPath () << ", " << content.size ();

      _image->setBackgroundRole (QPalette::Base);
      _image->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
      _image->setScaledContents (true);
      _image->setPixmap (QPixmap::fromImage (content));
      _image->adjustSize ();

      _ui->_view_w->setBackgroundRole (QPalette::Dark);
      _ui->_view_w->setWidgetResizable (false);
      _ui->_view_w->setWidget (_image);
    }

    /*! Destructor */
    ImageView::~ImageView ()
    {
      delete _ui;
    }

  }
}

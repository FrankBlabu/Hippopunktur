/*
 * hip_image_image_view.cpp - View displaying an acupunctur image
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPImageImageView.h"
#include "ui_hip_image_imageview.h"

#include "database/HIPDatabase.h"

namespace HIP {
  namespace Image {

    //#**********************************************************************
    // CLASS HIP::Image::ImageView
    //#**********************************************************************

    /*! Constructor */
    ImageView::ImageView (const Database::Image& image, QWidget* parent)
      : QWidget(parent),
      _ui (new Ui::HIP_Image_ImageView)
    {
      Q_UNUSED (image);

      _ui->setupUi (this);
    }

    /*! Destructor */
    ImageView::~ImageView ()
    {
      delete _ui;
    }

  }
}

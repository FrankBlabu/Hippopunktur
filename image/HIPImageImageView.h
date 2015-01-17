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

namespace Ui {
  class HIP_Image_ImageView;
}

namespace HIP {

  namespace Database {
    class Image;
  }

  namespace Image {

    /*
     * View displaying an acupunctur image
     */
    class ImageView : public QWidget
    {
      Q_OBJECT

    public:
      ImageView (const Database::Image& image, QWidget* parent);
      virtual ~ImageView ();

      void setScaling (double factor);

    protected:
      virtual bool eventFilter (QObject* obj, QEvent* event);

    private:
      Ui::HIP_Image_ImageView* _ui;
      QLabel* _image;

      QPixmap _pixmap;
      double _scaling;
    };

  }
}

#endif

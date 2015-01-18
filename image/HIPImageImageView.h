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
    class Database;
    class Image;
  }

  namespace Image {

    class ImageWidget;

    /*
     * View displaying an acupunctur image
     */
    class ImageView : public QWidget
    {
      Q_OBJECT

    public:
      ImageView (Database::Database* database, const Database::Image& image, QWidget* parent);
      virtual ~ImageView ();

    private slots:
      void onPointChanged (const QString& id);
      void onResetZoom ();

    private:
      Ui::HIP_Image_ImageView* _ui;
      Database::Database* _database;
      ImageWidget* _widget;
    };

  }
}

#endif

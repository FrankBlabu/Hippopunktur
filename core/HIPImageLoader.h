/*
 * HIPImageLoader.h - Threaded image loader
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPImageLoader_h__
#define __HIPImageLoader_h__

#include <QFuture>
#include <QImage>

namespace HIP {
  namespace Tools {

    /*!
     * Non memory static Image
     */
    class ImageLoader : public QObject
    {
      Q_OBJECT;

    public:
      ImageLoader (const QString& path, QObject* parent);
      virtual ~ImageLoader ();

      bool isLoaded () const;

      QImage getImage () const;

    signals:
      void finished ();
      void progressValueChanged (int progress);

    private:
      static QImage loadImage (const QString& path);

    private:
      QFuture<QImage> _image;
    };

  }
}

#endif

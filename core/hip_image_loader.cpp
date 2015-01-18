/*
 * hip_image_loader.cpp - Threaded image loader
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPImageLoader.h"

#include <QFutureWatcher>
#include <QtConcurrent>

namespace HIP {
  namespace Tools {

    /*! Constructor */
    ImageLoader::ImageLoader (const QString& path, QObject* parent)
      : QObject (parent)
    {
      QFutureWatcher<QImage>* watcher = new QFutureWatcher<QImage> (this);

      connect (watcher, SIGNAL (finished ()), SIGNAL (finished ()));
      connect (watcher, SIGNAL (progressValueChanged (int)), SIGNAL (progressValueChanged (int)));

      _image = QtConcurrent::run (loadImage, path);

      watcher->setFuture (_image);
    }

    /*! Destructor */
    ImageLoader::~ImageLoader ()
    {
    }

    /*! Return if the Image has been loaded */
    bool ImageLoader::isLoaded () const
    {
      return _image.isFinished ();
    }

    /*! Access to the loaded Image */
    QImage ImageLoader::getImage () const
    {
      Q_ASSERT (_image.isFinished ());
      return _image.result ();
    }

    /*! Load single Image [STATIC] */
    QImage ImageLoader::loadImage (const QString& path)
    {
      return QImage (path);
    }

  }
}

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

    //#**********************************************************************
    // CLASS HIP::Tools::ImageLoader
    //#**********************************************************************

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

    /*!
     * Access to the loaded Image
     *
     * This is legal only if the image already has been loaded completely by the thread. The function
     * will *not* stall and wait for it, but throw an assert otherwise.
     */
    QImage ImageLoader::getImage () const
    {
      Q_ASSERT (_image.isFinished ());
      return _image.result ();
    }

    /*!
     * Load single Image [STATIC]
     *
     * This function is called from within the QFuture thread to perform the image loading
     */
    QImage ImageLoader::loadImage (const QString& path)
    {
      return QImage (path);
    }

  }
}

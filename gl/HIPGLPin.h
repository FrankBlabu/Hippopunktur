/*
 * HIPGLSphere.h - GL based pin
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPGLPin_h__
#define __HIPGLPin_h__

#include <QColor>
#include <QMatrix4x4>
#include <QSharedPointer>
#include <QVector3D>

namespace HIP {
  namespace GL {

    class PinImpl;

    /*!
     * GL based pin
     */
    class Pin
    {
    public:
      Pin ();
      ~Pin ();

      void draw (const QMatrix4x4& mvp, const QVector3D& position, const QColor& color, double radius);

    private:
      QSharedPointer<PinImpl> _data;
    };

  }
}

#endif

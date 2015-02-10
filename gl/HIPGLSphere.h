/*
 * HIPGLSphere.h - GL based sphere
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPGLSphere_h__
#define __HIPGLSphere_h__

#include <QColor>
#include <QMatrix4x4>
#include <QSharedPointer>
#include <QVector3D>

class QOpenGLFunctions;

namespace HIP {
  namespace GL {

    class SphereImpl;

    /*!
     * GL based sphere
     */
    class Sphere
    {
    public:
      Sphere (double radius);
      ~Sphere ();

      void draw (const QMatrix4x4& mvp, const QVector3D& position, const QColor& color);

    private:
      QSharedPointer<SphereImpl> _data;
    };

  }
}

#endif

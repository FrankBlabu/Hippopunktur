/*
 * HIPGLOverlay.h - Overlay widgets for the 3D view
 *
 * Frank Blankenburg, Feb. 2013
 */

#ifndef __HIPGLOverlay_h__
#define __HIPGLOverlay_h__

#include <QSharedPointer>
#include <QWidget>

namespace HIP {
  namespace GL {

    /*!
     * Overlay widgets for the 3D view
     */
    class Overlay : public QWidget
    {
    public:
      Overlay (QWidget* parent);
      virtual ~Overlay ();

    protected:
      virtual void paintEvent (QPaintEvent* event);
    };

    typedef QSharedPointer<Overlay> OverlayPtr;

  }
}

#endif

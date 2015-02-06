/*
 * hip_gl_overlay.cpp - Overlay widget for the 3D view
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLOverlay.h"

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::Overlay
    //#**********************************************************************

    /*! Constructor */
    Overlay::Overlay (QWidget* parent)
      : QWidget (parent)
    {
      setAttribute (Qt::WA_NoBackground);
      setAttribute (Qt::WA_TransparentForMouseEvents);
    }

    /*! Destructor */
    Overlay::~Overlay ()
    {
    }

  }
}

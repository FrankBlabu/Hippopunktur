/*
 * hip_tools.cpp - Tool functions
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPTools.h"

namespace HIP {
  namespace Tools {

    //#************************************************************************
    // CLASS HIP::Tools
    //#************************************************************************

    QString toString (const QQmlError& error)
    {
      return error.toString ();
    }

  } // namespace Tools
} // namespace HIP


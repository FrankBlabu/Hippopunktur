/*
 * hip_exception.cpp - General exception class
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPException.h"

namespace HIP {

  //#************************************************************************
  // CLASS HIP::Exception
  //#************************************************************************

  /*! Constructor */
  Exception::Exception (const QString& text)
    : _text (text)
  {
  }

  /*! Destructor */
  Exception::~Exception ()
  {
  }

  /*! Return exception text */
  const QString& Exception::getText () const
  {
    return _text;
  }

}

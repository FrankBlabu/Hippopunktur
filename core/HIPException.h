/*
 * HIPException.h - General exception class
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPException_h__
#define __HIPException_h__

#include <QString>

namespace HIP {

  /*
   * General exception class
   */
  class Exception
  {
  public:
    Exception (const QString& text);
    virtual ~Exception ();

    const QString& getText () const;

  private:
    QString _text;
  };

}

#endif

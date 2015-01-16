/*
 * hip_tools.cpp - Tool functions
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPTools.h"
#include "core/HIPException.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace HIP {
  namespace Tools {

    //#************************************************************************
    // CLASS HIP::Tools
    //#************************************************************************

    /* Convert QQmlError to string */
    QString toString (const QQmlError& error)
    {
      return error.toString ();
    }

    /* Load string resource */
    template <>
    QString loadResource<QString> (const QString& name)
    {
      QFile file (name);

      if (!file.open (QFile::ReadOnly | QFile::Text))
        throw Exception (QObject::tr ("Unable to open resource file %1").arg (name));

      QTextStream in (&file);
      QString text = in.readAll ();

      file.close ();
      return text;
    }


  } // namespace Tools
} // namespace HIP

/*
 * HIPTools.h - Tool functions
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPTools_h__
#define __HIPTools_h__

#include <QHBoxLayout>
#include <QQmlError>

namespace HIP {
  namespace Tools {

    //#**********************************************************************
    // Layout functions
    //#**********************************************************************

    /*!
     * Add widget to its parent in a simple borderless layout
     */
    template<class T>
    T* addToParent (T* widget)
    {
      QBoxLayout* layout = new QHBoxLayout (qobject_cast<QWidget*> (widget->parent ()));
      layout->setMargin (0);
      layout->setSpacing (0);
      layout->addWidget (widget);

      return widget;
    }

    //#**********************************************************************
    // Quoting
    //#**********************************************************************

    QString quoteHTML (const QString& text);

    //#**********************************************************************
    // String conversion
    //#**********************************************************************

    QString toString (const QQmlError& error);

    template<class T>
    QString toString (const QList<T>& l)
    {
      QString text;
      QString separator;

      for (int i=0; i < l.size (); ++i)
      {
        text += toString (l[i]) + separator;
        separator = ", ";
      }

      return text;
    }

    //#**********************************************************************
    // Resources
    //#**********************************************************************

    template<class T>
    T loadResource (const QString& name)
    {
      Q_ASSERT (false && "Not implemented.");
    }

    template<>
    QString loadResource (const QString& name);

    template<>
    QImage loadResource (const QString& name);
  }
}

#endif

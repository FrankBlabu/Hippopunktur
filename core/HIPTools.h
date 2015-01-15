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

    template<class T>
    T* addToParent (T* widget)
    {
      QBoxLayout* layout = new QHBoxLayout (qobject_cast<QWidget*> (widget->parent ()));
      layout->setMargin (0);
      layout->setSpacing (0);
      layout->addWidget (widget);

      return widget;
    }

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

  }
}

#endif

/*
 * HIPStatusBar.h - Status bar handler
 *
 * Frank Blankenburg, Jabn. 2015
 */

#ifndef __HIPStatusBar_h__
#define __HIPStatusBar_h__

#include <QObject>

class QStatusBar;

namespace HIP {
  namespace Tools {

    /*
     * Status bar handler
     */
    class StatusBar : public QObject
    {
    public:
      StatusBar (QStatusBar* bar);
      virtual ~StatusBar ();

      static void showMessage (const QString& text);
      static void clearMessage ();

    private:
      QStatusBar* _bar;

      static StatusBar* _instance;
    };

  }
}

#endif

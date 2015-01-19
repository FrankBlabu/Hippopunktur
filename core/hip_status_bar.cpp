/*
 * HIPStatusBar.h - Status bar handler
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPStatusBar.h"

#include <QStatusBar>

namespace HIP {
  namespace Tools {

    //#**********************************************************************
    // CLASS HIP::Tools::StatusBar
    //#**********************************************************************

    /*! Singleton instance [STATIC] */
    StatusBar* StatusBar::_instance = 0;

    /*! Constructor */
    StatusBar::StatusBar (QStatusBar* bar)
      : QObject (bar),
        _bar (bar)
    {
      Q_ASSERT (_instance == 0 && "Singleton class");
      _instance = this;
    }

    /*! Destructor */
    StatusBar::~StatusBar ()
    {
      _instance = 0;
    }

    /*! Show status message [STATIC] */
    void StatusBar::showMessage (const QString& text)
    {
      Q_ASSERT (_instance != 0 && "Status bar not initialized");
      _instance->_bar->showMessage (text);
    }

    /*! Clear status message [STATIC] */
    void StatusBar::clearMessage ()
    {
      Q_ASSERT (_instance != 0 && "Status bar not initialized");
      _instance->_bar->clearMessage ();
    }

  }
}

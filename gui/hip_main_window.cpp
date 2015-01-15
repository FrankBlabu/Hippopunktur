/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPMainWindow.h"
#include "ui_hip_main_window.h"

namespace HIP {
  namespace Gui {

    //#**********************************************************************
    // CLASS HIP::GUI::MainWindow
    //#**********************************************************************

    /*! Constructor */
    MainWindow::MainWindow (QWidget *parent)
      : QMainWindow (parent),
      _ui (new Ui::HIP_Gui_MainWindow)
    {
      _ui->setupUi (this);
    }

    /*! Destructor */
    MainWindow::~MainWindow ()
    {
      delete _ui;
    }

  } // namespace Gui
} // namespace HIP

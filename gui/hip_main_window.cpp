/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPMainWindow.h"
#include "explorer/HIPExplorer.h"
#include "ui_hip_main_window.h"

#include <QHBoxLayout>

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

      Explorer::Explorer* explorer = new Explorer::Explorer (_ui->_main_window_w);
      QBoxLayout* main_window_layout = new QHBoxLayout (_ui->_main_window_w);
      main_window_layout->setMargin (0);
      main_window_layout->setSpacing (0);
      main_window_layout->addWidget (explorer);
    }

    /*! Destructor */
    MainWindow::~MainWindow ()
    {
      delete _ui;
    }

  } // namespace Gui
} // namespace HIP

/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPMainWindow.h"
#include "core/HIPTools.h"
#include "explorer/HIPExplorer.h"
#include "ui_hip_main_window.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

namespace HIP {
  namespace Gui {

    //#**********************************************************************
    // CLASS HIP::GUI::MainWindow
    //#**********************************************************************

    /*! Constructor */
    MainWindow::MainWindow (Database::Database* database, QWidget *parent)
      : QMainWindow (parent),
      _ui (new Ui::HIP_Gui_MainWindow)
    {
      _ui->setupUi (this);

      Explorer::Explorer* explorer = new Explorer::Explorer (database, _ui->_main_window_w);
      QBoxLayout* main_window_layout = new QHBoxLayout (_ui->_main_window_w);
      main_window_layout->setMargin (0);
      main_window_layout->setSpacing (0);
      main_window_layout->addWidget (explorer);

      connect (_ui->_action_about, SIGNAL (triggered (bool)), SLOT (onAbout ()));
      connect (_ui->_action_exit, SIGNAL (triggered (bool)), qApp, SLOT (quit ()));
    }

    /*! Destructor */
    MainWindow::~MainWindow ()
    {
      delete _ui;
    }

    /*! Show about dialog */
    void MainWindow::onAbout ()
    {
      QMessageBox::information (this, tr ("Hippopunktur V0.1"), Tools::loadResource<QString> (":/assets/about.html"));
    }

  } // namespace Gui
} // namespace HIP

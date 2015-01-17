/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPMainWindow.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"
#include "explorer/HIPExplorer.h"
#include "explorer/HIPExplorerTagSelector.h"
#include "image/HIPImageImageView.h"
#include "ui_hip_main_window.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
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

      Explorer::TagSelector* selector = Tools::addToParent (new Explorer::TagSelector (database, _ui->_selector_w));
      Explorer::Explorer* explorer = Tools::addToParent (new Explorer::Explorer (database, _ui->_explorer_w));
      explorer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

      _ui->_tab_w->clear ();

      foreach (const Database::Image& image, database->getImages ())
        _ui->_tab_w->addTab (new Image::ImageView (image, _ui->_tab_w), image.getTitle ());

      connect (selector, SIGNAL (tagChanged (const QString&)), explorer, SLOT (onTagChanged (const QString&)));
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

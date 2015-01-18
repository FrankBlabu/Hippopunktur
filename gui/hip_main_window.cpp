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
      _ui       (new Ui::HIP_Gui_MainWindow),
      _database (database)
    {
      _ui->setupUi (this);

      Explorer::TagSelector* tag_selector = Tools::addToParent (new Explorer::TagSelector (database, _ui->_selector_w));
      Explorer::Explorer* explorer = Tools::addToParent (new Explorer::Explorer (database, _ui->_explorer_w));
      explorer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

      _ui->_tab_w->clear ();

      foreach (const Database::Image& image, database->getImages ())
        {
          Image::ImageView* view = new Image::ImageView (database, image, _ui->_tab_w);
          _ui->_tab_w->addTab (view, image.getTitle ());
        }

      for (int i=0; i < _ui->_tab_w->count (); ++i)
        {
          Image::ImageView* view = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (i));
          Q_ASSERT (view != 0);

          connect (tag_selector, &Explorer::TagSelector::tagChanged, view, &Image::ImageView::onTagChanged);
          connect (view, &Image::ImageView::pointClicked, explorer, &Explorer::Explorer::onPointClicked);
        }

      connect (tag_selector, SIGNAL (tagChanged (const QString&)), explorer, SLOT (onTagChanged (const QString&)));
      connect (tag_selector, SIGNAL (tagChanged (const QString&)), this, SLOT (onTagChanged (const QString&)));
      connect (_ui->_action_about, SIGNAL (triggered (bool)), SLOT (onAbout ()));
      connect (_ui->_action_exit, SIGNAL (triggered (bool)), qApp, SLOT (quit ()));

      onTagChanged ("");
    }

    /*! Destructor */
    MainWindow::~MainWindow ()
    {
      delete _ui;
    }

    /*! Adapt tab sensitivity to selector tag */
    void MainWindow::onTagChanged (const QString& tag)
    {
      QSet<QString> used_images;

      foreach (const Database::Point& point, _database->getPoints ())
        {
          if (point.matches (tag))
            {
              foreach (const Database::Position& position, point.getPositions ())
                used_images.insert (position.getImage ());
            }
        }

      for (int i=0; i < _ui->_tab_w->count (); ++i)
        {
          Image::ImageView* view = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (i));
          Q_ASSERT (view != 0);

          _ui->_tab_w->setTabEnabled (i, used_images.contains (view->getImage ().getId ()));
        }
    }

    /*! Show about dialog */
    void MainWindow::onAbout ()
    {
      QMessageBox::information (this, tr ("Hippopunktur V0.1"), Tools::loadResource<QString> (":/assets/about.html"));
    }

  } // namespace Gui
} // namespace HIP

/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiMainWindow.h"

#include "core/HIPException.h"
#include "core/HIPStatusBar.h"
#include "core/HIPTools.h"
#include "core/HIPVersion.h"
#include "database/HIPDatabase.h"
#include "explorer/HIPPointExplorerView.h"
#include "explorer/HIPExplorerTagSelector.h"
#include "gl/HIPGLView.h"
#include "gl/HIPGLDebugOverlay.h"

#include "ui_hip_gui_main_window.h"

#include <QApplication>
#include <QDebug>
#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextStream>


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

      setWindowTitle (QString ("%1 V%2 - Rev. %3")
                      .arg (qApp->applicationName ())
                      .arg (qApp->applicationVersion ())
                      .arg (REVISION));
      setAcceptDrops (true);

      new Tools::StatusBar (_ui->_status_bar_w);

      Tools::addToParent (new Explorer::TagSelector (database, _ui->_selector_w));

      QWidget* explorer = Tools::addToParent (new Explorer::PointExplorerView (database, _ui->_explorer_w));
      explorer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

      GL::View* view = Tools::addToParent (new GL::View (database, _ui->_gl_frame_w));
      view->addOverlay (GL::OverlayPtr (new GL::DebugOverlay (database, 0)));

      connect (_ui->_action_export_database, SIGNAL (triggered (bool)), SLOT (onExportDatabase ()));
      connect (_ui->_action_exit, SIGNAL (triggered (bool)), qApp, SLOT (quit ()));
      connect (_ui->_action_about, SIGNAL (triggered (bool)), SLOT (onAbout ()));
    }

    /*! Destructor */
    MainWindow::~MainWindow ()
    {
      delete _ui;
    }

    /*! Set main window visible */
    void MainWindow::setVisible (bool visible)
    {
      QMainWindow::setVisible (visible);

      QList<int> main_window_sizes;
      main_window_sizes.append (_ui->_splitter_w->width () * 30 / 100);
      main_window_sizes.append (_ui->_splitter_w->width () * 70 / 100);

      _ui->_splitter_w->setSizes (main_window_sizes);

      QList<int> explorer_sizes;
      explorer_sizes.append (_ui->_explorer_splitter_w->width () * 50 / 100);
      explorer_sizes.append (_ui->_explorer_splitter_w->width () * 50 / 100);

      _ui->_explorer_splitter_w->setSizes (explorer_sizes);
    }


    /*! Export current database into file */
    void MainWindow::onExportDatabase ()
    {
      QFileDialog dialog (this);
      dialog.setFileMode (QFileDialog::AnyFile);
      dialog.setAcceptMode (QFileDialog::AcceptSave);
      dialog.setNameFilter (tr ("Database file (*.xml)"));
      dialog.setDefaultSuffix (".xml");
      dialog.setDirectory (QStandardPaths::writableLocation (QStandardPaths::DocumentsLocation));

      if (dialog.exec ())
        {
          Q_ASSERT (dialog.selectedFiles ().size () == 1);

          QFile file (dialog.selectedFiles ().front ());
          if (file.open (QIODevice::WriteOnly | QIODevice::Text))
            {
              QTextStream out (&file);
              out << _database->toXML ();

              file.close ();
            }
          else
            QMessageBox::critical (this, tr ("Cannot save database file"), file.errorString ());
        }
    }

    /*! Show about dialog */
    void MainWindow::onAbout ()
    {
      QMessageBox::information (this, tr ("Hippopunktur V0.1"), Tools::loadResource<QString> (":/assets/about.html"));
    }

    /*! Handle drag events */
    void MainWindow::dragEnterEvent (QDragEnterEvent* event)
    {
      bool accepted = false;

      if (event->mimeData ()->hasText ())
        {
          QFileInfo info (event->mimeData ()->text ());
          if (info.isReadable ())
            {
              if ( info.suffix ().toLower () == "xml" ||
                   info.suffix ().toLower () == "css" )
                accepted = true;
            }
        }

      event->acceptProposedAction ();
    }

    void MainWindow::dragMoveEvent (QDragMoveEvent* event)
    {
        event->acceptProposedAction ();
    }

    void MainWindow::dropEvent (QDropEvent* event)
    {
      Q_ASSERT (event->mimeData ()->hasText ());
      event->acceptProposedAction ();

      QString path = QUrl (event->mimeData ()->text ()).path ();
      if (path.startsWith ('/') || path.startsWith ('\\'))
        path.remove (0, 1);

      try
      {
        QFileInfo info (event->mimeData ()->text ());
        if (info.suffix ().toLower () == "xml")
          _database->load (Tools::loadResource<QString> (path));
        else if (info.suffix ().toLower () == "css")
          {
            qDebug () << "Reapply style sheet";
            qApp->setStyleSheet (Tools::loadResource<QString> (path));
            update ();
          }
        else
          Q_ASSERT (false && "Illegal file suffix, should be filtered.");
      }
      catch (const Exception& exception)
      {
        QMessageBox::critical (this, tr ("Load error"), tr ("Unable to load '%1':\n\n %2")
                               .arg (path)
                               .arg (exception.getText ()));
      }

    }

  } // namespace Gui
} // namespace HIP

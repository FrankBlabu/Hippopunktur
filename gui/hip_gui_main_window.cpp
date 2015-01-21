/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiMainWindow.h"

#include "core/HIPException.h"
#include "core/HIPStatusBar.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"
#include "explorer/HIPExplorerView.h"
#include "explorer/HIPExplorerTagSelector.h"
#include "gui/HIPGuiPointEditor.h"
#include "image/HIPImageImageView.h"

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
    // CLASS HIP::GUI::ImageViewInterfaceImpl
    //#**********************************************************************

    /*
     * Interface used to access the image views
     */
    class ImageViewInterfaceImpl : public PointEditor::ImageViewInterface
    {
    public:
      ImageViewInterfaceImpl (QTabWidget* tabs, QObject* parent);
      virtual ~ImageViewInterfaceImpl ();

      virtual QString getActiveImage () const;
      virtual bool selectCoordinate (const QString& id, QPointF* coordinate) const;

    private:
      QTabWidget* _tabs;
    };

    /*! Constructor */
    ImageViewInterfaceImpl::ImageViewInterfaceImpl (QTabWidget* tabs, QObject* parent)
      : PointEditor::ImageViewInterface (parent),
        _tabs (tabs)
    {
    }

    /*! Destructor */
    ImageViewInterfaceImpl::~ImageViewInterfaceImpl ()
    {
    }

    /*! Return the id of the currently active image */
    QString ImageViewInterfaceImpl::getActiveImage () const
    {
      Image::ImageView* view = qobject_cast<Image::ImageView*> (_tabs->currentWidget ());
      Q_ASSERT (view != 0);
      return view->getImage ().getId ();
    }

    /*! Select coordinate in image */
    bool ImageViewInterfaceImpl::selectCoordinate (const QString& id, QPointF* coordinate) const
    {
      Image::ImageView* view = 0;
      for (int i=0; i < _tabs->count () && view == 0; ++i)
        {
          Image::ImageView* candidate = qobject_cast<Image::ImageView*> (_tabs->widget (i));
          Q_ASSERT (candidate != 0);

          if (candidate->getImage ().getId () == id)
            view = candidate;
        }

      Q_ASSERT (view != 0);

      return view->selectCoordinate (coordinate);
    }


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

      setAcceptDrops (true);

      new Tools::StatusBar (_ui->_status_bar_w);

      Tools::addToParent (new Explorer::TagSelector (database, _ui->_selector_w));

      Explorer::ExplorerView* explorer = Tools::addToParent (new Explorer::ExplorerView (database, _ui->_explorer_w));
      explorer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

      Gui::PointEditor* point_editor = Tools::addToParent (new Gui::PointEditor (database, _ui->_point_editor_w));
      point_editor->setImageViewInterface (new ImageViewInterfaceImpl (_ui->_tab_w, this));

      _ui->_tab_w->clear ();

      foreach (const Database::Image& image, database->getImages ())
        {
          Image::ImageView* view = new Image::ImageView (database, image, _ui->_tab_w);
          _ui->_tab_w->addTab (view, image.getTitle ());
        }

      connect (_database, &Database::Database::databaseChanged, this, &MainWindow::onDatabaseChanged);
      connect (_ui->_tab_w, &QTabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);
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


    /*! React on database changes  */
    void MainWindow::onDatabaseChanged (Database::Database::Reason_t reason, const QString& id)
    {
      switch (reason)
        {
        case Database::Database::Reason::DATA:
        case Database::Database::Reason::POINT:
        case Database::Database::Reason::FILTER:
          {
            QSet<QString> used_images;

            foreach (const Database::Point& point, _database->getPoints ())
              {
                if (point.matches (_database->getFilter ()))
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
          break;

        case Database::Database::Reason::SELECTION:
          break;

        case Database::Database::Reason::VISIBLE_IMAGE:
          {
            Image::ImageView* view = 0;

            for (int i=0; i < _ui->_tab_w->count () && view == 0; ++i)
              {
                Image::ImageView* candidate = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (i));
                if (candidate->getImage ().getId () == id)
                  view = candidate;
              }

            Q_ASSERT (view != 0);

            if (view != _ui->_tab_w->currentWidget ())
              _ui->_tab_w->setCurrentWidget (view);
          }
          break;
        }
    }

    /*! Called when the currently visible image view tab changes */
    void MainWindow::onCurrentTabChanged (int index)
    {
      Image::ImageView* view = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (index));
      Q_ASSERT (view != 0);

      _database->setVisibleImage (view->getImage ().getId ());
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

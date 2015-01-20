/*
 * hip_main_window.cpp - Main window for the hippopunktur application
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiMainWindow.h"

#include "core/HIPStatusBar.h"
#include "core/HIPTools.h"
#include "database/HIPDatabase.h"
#include "explorer/HIPExplorer.h"
#include "explorer/HIPExplorerTagSelector.h"
#include "gui/HIPGuiPointEditor.h"
#include "image/HIPImageImageView.h"

#include "ui_hip_gui_main_window.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
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
      _ui           (new Ui::HIP_Gui_MainWindow),
      _database     (database),
      _point_editor (0)
    {
      _ui->setupUi (this);
      new Tools::StatusBar (_ui->_status_bar_w);

      Explorer::TagSelector* tag_selector = Tools::addToParent (new Explorer::TagSelector (database, _ui->_selector_w));

      Explorer::Explorer* explorer = Tools::addToParent (new Explorer::Explorer (database, _ui->_explorer_w));
      explorer->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);

      _point_editor = Tools::addToParent (new Gui::PointEditor (database, _ui->_point_editor_w));
      _point_editor->setImageViewInterface (new ImageViewInterfaceImpl (_ui->_tab_w, this));

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
        }

      connect (tag_selector, SIGNAL (tagChanged (const QString&)), explorer, SLOT (onTagChanged (const QString&)));
      connect (tag_selector, SIGNAL (tagChanged (const QString&)), this, SLOT (onTagChanged (const QString&)));

      connect (_database, SIGNAL (pointChanged (const QString&)), explorer, SLOT (onPointChanged (const QString&)));
      connect (_database, SIGNAL (selectionChanged (const QString&)), explorer, SLOT (onPointChanged (const QString&)));
      connect (_database, SIGNAL (dataChanged ()), explorer, SLOT (onDataChanged ()));
      connect (_database, SIGNAL (selectionChanged (const QString&)), _point_editor, SLOT (onPointSelectionChanged (const QString&)));

      connect (_point_editor, SIGNAL (imageSelected (const QString&)), SLOT (onImageSelected (const QString&)));

      connect (_ui->_tab_w, SIGNAL (currentChanged (int)), SLOT (onCurrentTabChanged (int)));

      connect (_ui->_action_export_database, SIGNAL (triggered (bool)), SLOT (onExportDatabase ()));
      connect (_ui->_action_exit, SIGNAL (triggered (bool)), qApp, SLOT (quit ()));

      connect (_ui->_action_about, SIGNAL (triggered (bool)), SLOT (onAbout ()));

      onTagChanged ("");
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

    /*! React on image entry selection in the point editor */
    void MainWindow::onImageSelected (const QString& id)
    {
      Image::ImageView* view = 0;

      for (int i=0; i < _ui->_tab_w->count () && view == 0; ++i)
        {
          Image::ImageView* candidate = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (i));
          if (candidate->getImage ().getId () == id)
            view = candidate;
        }

      Q_ASSERT (view != 0);

      _ui->_tab_w->setCurrentWidget (view);
    }

    /*! Called when the currently visible image view tab changes */
    void MainWindow::onCurrentTabChanged (int index)
    {
      Image::ImageView* view = qobject_cast<Image::ImageView*> (_ui->_tab_w->widget (index));
      Q_ASSERT (view != 0);

      _point_editor->onCurrentImageChanged (view->getImage ().getId ());
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

      qDebug () << QStandardPaths::writableLocation (QStandardPaths::DocumentsLocation);

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

  } // namespace Gui
} // namespace HIP

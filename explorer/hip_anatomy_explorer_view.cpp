/*
 * hip_anatomy_explorer_view.cpp - Explorer for the model anatomy
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPAnatomyExplorerView.h"
#include "ui_hip_anatomy_explorer_view.h"

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::AnatomyExplorerView
    //#**********************************************************************

    /*! Constructor */
    AnatomyExplorerView::AnatomyExplorerView (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _ui       (new Ui::HIP_Explorer_AnatomyExplorerView),
        _database (database)
    {
      _ui->setupUi (this);
    }

    /*! Destructor */
    AnatomyExplorerView::~AnatomyExplorerView ()
    {
      delete _ui;
    }

    /*! Called when the central database changes */
    void AnatomyExplorerView::onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data)
    {
      Q_UNUSED (reason);
      Q_UNUSED (data);
    }

  }
}



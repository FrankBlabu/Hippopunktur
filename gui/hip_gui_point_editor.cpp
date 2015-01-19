/*
 * hip_gui_point_editor.cpp - Editor for the point database
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGuiPointEditor.h"
#include "ui_hip_gui_pointeditor.h"

namespace HIP {
  namespace Gui {

    //#**********************************************************************
    // CLASS HIP::Gui::PointEditor
    //#**********************************************************************

    /*! Constructor */
    PointEditor::PointEditor (Database::Database* database, QWidget* parent)
      : QWidget(parent),
        _ui       (new Ui::HIP_Gui_PointEditor),
        _database (database)
    {
      _ui->setupUi (this);
    }

    /*! Destructor */
    PointEditor::~PointEditor ()
    {
      delete _ui;
    }

  }
}

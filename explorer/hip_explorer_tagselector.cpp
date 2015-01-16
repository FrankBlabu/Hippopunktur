/*
 * hip_explorer_tag_selector.cpp - Widget for selecting filter tags
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorerTagSelector.h"
#include "ui_hip_explorer_tagselector.h"

#include "database/HIPDatabase.h"

namespace HIP {
  namespace Explorer {

    /* Constructor */
    TagSelector::TagSelector (Database::Database* database, QWidget* parent)
      : QWidget(parent),
      _ui (new Ui::HIP_Explorer_TagSelector)
    {
      Q_UNUSED (database);
      _ui->setupUi (this);
    }

    /*! Destructor */
    TagSelector::~TagSelector ()
    {
      delete _ui;
    }

  }
}

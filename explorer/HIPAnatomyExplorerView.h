/*
 * HIPAnatomyExplorerView.h - Explorer for the model anatomy
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPAnatomyExplorerView_h__
#define __HIPAnatomyExplorerView_h__

#include <QWidget>
#include "database/HIPDatabase.h"

namespace Ui {
  class HIP_Explorer_AnatomyExplorerView;
}

namespace HIP {
  namespace Explorer {

    /*!
     * Explorer for the model anatomy
     *
     * This explorer lists the various parts of the model which can be selected or
     * switched visible independently from each other.
     */
    class AnatomyExplorerView : public QWidget
    {
      Q_OBJECT

      public:
        AnatomyExplorerView (Database::Database* database, QWidget* parent);
        virtual ~AnatomyExplorerView ();

      private slots:
        void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);

      private:
        Ui::HIP_Explorer_AnatomyExplorerView* _ui;

        Database::Database* _database;
    };

  }
}


#endif

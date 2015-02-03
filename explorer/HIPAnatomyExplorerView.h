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

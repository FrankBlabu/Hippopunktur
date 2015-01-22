/*
 * HIPGLView.h - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLView_h__
#define __HIPGLView_h__

#include "database/HIPDatabase.h"

#include <QWidget>

namespace Ui {
  class HIP_GL_View;
}

namespace HIP {
  namespace GL {

    class Widget;

    /*
     * View displaying a GL scene
     */
    class View : public QWidget
    {
      Q_OBJECT

    public:
      explicit View (const Database::Database* database, const QString& model_path, QWidget* parent);
      virtual ~View ();

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);

    private:
      Ui::HIP_GL_View* _ui;
      Widget* _widget;
    };

  }
}

#endif

/*
 * HIPGLView.h - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLView_h__
#define __HIPGLView_h__

#include "database/HIPDatabase.h"

#include <QMap>
#include <QMatrix4x4>
#include <QWidget>

class QAction;
class QActionGroup;
class QToolBar;

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
      explicit View (Database::Database* database, QWidget* parent);
      virtual ~View ();

    signals:
      void modelViewMatrixChanged (const QMatrix4x4& matrix);

    private slots:
      void onDatabaseChanged (Database::Database::Reason_t reason, const QVariant& data);
      void onResetView ();
      void onSelectView ();

    private:
      void updateToolBar ();

    private:
      Ui::HIP_GL_View* _ui;
      Database::Database* _database;

      QToolBar* _toolbar;
      QActionGroup* _action_group;
      Widget* _widget;

      typedef QMap<QAction*, QString> ActionViewMap;
      ActionViewMap _action_view_map;
    };

  }
}

#endif

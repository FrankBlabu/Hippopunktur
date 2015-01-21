/*
 * HIPGLView.h - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPGLView_h__
#define __HIPGLView_h__

#include <QWidget>

namespace Ui {
  class HIP_GL_View;
}

namespace HIP {
  namespace GL {

    /*
     * View displaying a GL scene
     */
    class View : public QWidget
    {
      Q_OBJECT

    public:
      explicit View (QWidget* parent);
      virtual ~View ();

    private:
      Ui::HIP_GL_View* _ui;
    };

  }
}

#endif

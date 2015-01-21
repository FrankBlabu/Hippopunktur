/*
 * hip_gl_view.cpp - View displaying a GL scene
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPGLView.h"
#include "ui_hip_gl_view.h"

#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QQuickWidget>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::View
    //#**********************************************************************

    /*! Constructor */
    View::View (QWidget* parent)
      : QWidget(parent),
        _ui (new Ui::HIP_GL_View)
    {
      _ui->setupUi (this);

      QQuickWidget* content = Tools::addToParent (new QQuickWidget (_ui->_view_w));

      content->setSource (QUrl ("qrc:/gl/View.qml"));

      if (content->status () != QQuickWidget::Ready)
        throw Exception (Tools::toString (content->errors ()));
    }

    /*! Destructor */
    View::~View ()
    {
      delete _ui;
    }

  }
}

/*
 * hip_explorer.cpp - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorer.h"
#include "core/HIPTools.h"
#include "core/HIPException.h"

#include <QQuickWidget>
#include <QUrl>
#include <QDebug>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::Explorer
    //#**********************************************************************

    /*! Constructor */
    Explorer::Explorer (QWidget* parent)
      : QWidget (parent)
    {
      QQuickWidget* view = Tools::addToParent (new QQuickWidget (this));
      view->setResizeMode (QQuickWidget::SizeRootObjectToView);
      view->setSource (QUrl ("qrc:/explorer/Explorer.qml"));

      if (view->status () != QQuickWidget::Ready)
        throw Exception (Tools::toString (view->errors ()));
    }

    /*! Destructor */
    Explorer::~Explorer ()
    {
    }

  } // namespace Explorer
} // namespace HIP



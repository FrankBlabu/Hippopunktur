/*
 * hip_explorer.cpp - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorer.h"
#include "core/HIPTools.h"
#include "core/HIPException.h"
#include "database/HIPDatabaseModel.h"

#include <QQuickWidget>
#include <QUrl>
#include <QQmlContext>
#include <QDebug>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::Explorer
    //#**********************************************************************

    /*! Exposed model name in QML [STATIC] */
    const char* const Explorer::QML_MODEL_NAME = "explorer_model";


    /*! Constructor */
    Explorer::Explorer (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _model (new Database::DatabaseModel (database))
    {
      QQuickWidget* view = Tools::addToParent (new QQuickWidget (this));
      view->setResizeMode (QQuickWidget::SizeRootObjectToView);
      view->rootContext ()->setContextProperty (QString (QML_MODEL_NAME), _model);
      view->setSource (QUrl ("qrc:/explorer/Explorer.qml"));

      if (view->status () != QQuickWidget::Ready)
        throw Exception (Tools::toString (view->errors ()));
    }

    /*! Destructor */
    Explorer::~Explorer ()
    {
      delete _model;
    }

    /*! Called when the filter tag changed */
    void Explorer::onTagChanged (const QString& tag)
    {
      qDebug () << "Tag: " << tag;
    }


  } // namespace Explorer
} // namespace HIP



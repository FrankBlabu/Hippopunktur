/*
 * hip_explorer.cpp - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPQMLExplorer.h"
#include "core/HIPTools.h"
#include "core/HIPException.h"
#include "database/HIPDatabase.h"
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
    const char* const Explorer::QML_DATABASE_NAME = "database";
    const char* const Explorer::QML_MODEL_NAME    = "explorer_model";

    /*! Constructor */
    Explorer::Explorer (Database::Database* database, QWidget* parent)
      : QWidget (parent),
        _database (database),
        _model    (new Database::DatabaseModel (database, this)),
        _filter   (new Database::DatabaseFilterProxyModel (this))
    {
      _filter->setSourceModel (_model);

      QQuickWidget* view = Tools::addToParent (new QQuickWidget (this));
      view->setResizeMode (QQuickWidget::SizeRootObjectToView);

      view->rootContext ()->setContextProperty (QString (QML_MODEL_NAME), _filter);
      view->rootContext ()->setContextProperty (QString (QML_DATABASE_NAME), _database);

      view->setSource (QUrl ("qrc:/explorer/Explorer.qml"));

      if (view->status () != QQuickWidget::Ready)
        throw Exception (Tools::toString (view->errors ()));
    }

    /*! Destructor */
    Explorer::~Explorer ()
    {
    }

    /*! Called when the filter tag changed */
    void Explorer::onTagChanged (const QString& tag)
    {
      _database->clearSelection ();
      _filter->setTag (tag);
    }

    /*! Called when a single point in the database has been changed */
    void Explorer::onPointChanged (const QString& id)
    {
      Database::DatabaseModel* model = dynamic_cast<Database::DatabaseModel*> (_model);
      Q_ASSERT (model != 0);

      model->onChanged (id);
    }

    /*! Called when big changes occured in the database and the model has to be reset */
    void Explorer::onDataChanged ()
    {
      Database::DatabaseModel* model = dynamic_cast<Database::DatabaseModel*> (_model);
      Q_ASSERT (model != 0);

      model->reset ();
    }

  } // namespace Explorer
} // namespace HIP



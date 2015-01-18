/*
 * hip_explorer.cpp - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorer.h"
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
    const char* const Explorer::QML_MODEL_NAME = "explorer_model";


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
      foreach (const Database::Point& point, _database->getPoints ())
        _database->setSelected (point.getId (), false);

      _filter->setTag (tag);
    }

    /*! Caller when an image point has been clicked */
    void Explorer::onPointClicked (const QString &id)
    {
      Q_ASSERT (!id.isEmpty ());

      Database::DatabaseModel* model = dynamic_cast<Database::DatabaseModel*> (_model);
      Q_ASSERT (model != 0);

      QModelIndex index = model->getIndex (id);
      Q_ASSERT (index.isValid ());

      model->setData (index, qVariantFromValue (true), Database::DatabaseModel::Role::SELECTED);
    }

  } // namespace Explorer
} // namespace HIP



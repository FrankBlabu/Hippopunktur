/*
 * HIPExplorer.h - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorer_h__
#define __HIPExplorer_h__

#include <QWidget>

namespace HIP {

  namespace Database {
    class Database;
    class DatabaseModel;
  }

  namespace Explorer {

    /*
     * Point explorer
     */
    class Explorer : public QWidget
    {
      Q_OBJECT

    public:
      Explorer (Database::Database* database, QWidget* parent);
      virtual ~Explorer ();

    public slots:
      void onTagChanged (const QString& tag);

    private:
      Database::DatabaseModel* _model;

      static const char* const QML_MODEL_NAME;
    };

  }
}

#endif

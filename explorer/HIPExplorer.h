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
    public:
      Explorer (Database::Database* database, QWidget* parent);
      virtual ~Explorer ();

    private:
      Database::DatabaseModel* _model;

      static const char* const QML_MODEL_NAME;
    };

  }
}

#endif

/*
 * HIPGLDebugOverlay.h - Overlay for displaying debug information
 *
 * Frank Blankenburg, Feb. 2015
 */

#ifndef __HIPGLDebugOverlay_h__
#define __HIPGLDebugOverlay_h__

#include "gl/HIPGLOverlay.h"

#include <QString>

namespace HIP {

  namespace Database {
    class Database;
  }

  namespace GL {

    /*!
     * Overlay displaying debugging information
     */
    class DebugOverlay : public Overlay
    {
      Q_OBJECT

    public:
      DebugOverlay (Database::Database* database, QWidget* parent);
      virtual ~DebugOverlay ();

    protected:
      virtual void paintEvent (QPaintEvent* event);

    private slots:
      void onViewChanged (const QVariant& data);

    private:
      Database::Database* _database;

      QRect _rect;
      QString _text;
    };

  }
}

#endif

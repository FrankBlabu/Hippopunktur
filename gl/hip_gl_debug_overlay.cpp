/*
 * hip_gp_debug_overlay.cpp - Overlay for displaying debugging information
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "HIPGLDebugOverlay.h"
#include "database/HIPDatabase.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QMatrix4x4>

namespace HIP {
  namespace GL {

    //#**********************************************************************
    // CLASS HIP::GL::DebugOverlay
    //#**********************************************************************

    /*! Constructor */
    DebugOverlay::DebugOverlay (Database::Database* database, QWidget* parent)
      : Overlay (parent),
        _database (database),
        _rect     (),
        _text     ()
    {
      connect (database, &Database::Database::viewChanged, this, &DebugOverlay::onViewChanged);
    }

    /*! Destructor */
    DebugOverlay::~DebugOverlay ()
    {
    }

    /*! Paint widget */
    void DebugOverlay::paintEvent (QPaintEvent* event)
    {
      Q_UNUSED (event);

      QPainter painter (this);
      painter.fillRect (rect (), Qt::transparent);

      if (!_rect.isNull ())
        {
          QRect r = _rect;
          r.moveTopLeft (rect ().topLeft () + QPoint (width () - r.width () - 10,
                                                      height () - r.height () - 10));

          painter.setPen (Qt::lightGray);
          painter.drawText (r, Qt::AlignTop | Qt::AlignLeft, _text);
        }
    }

    /*! Slot called when the view transformation changed */
    void DebugOverlay::onViewChanged (const QVariant& data)
    {
      if (data.userType () == qMetaTypeId<QMatrix4x4> ())
        {
          QMatrix4x4 m = data.value<QMatrix4x4> ();

          QString text = "Zeile 1\nZeile 2\nZeile 3";

          if (text != _text)
            {
              QFontMetrics metrics (font ());
              _rect = metrics.boundingRect (rect (), Qt::AlignTop | Qt::AlignLeft, text);
              _text = text;
            }
        }
    }


  }
}

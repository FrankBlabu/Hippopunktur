/*
 * HIPExplorer.h - Point explorer
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPExplorer_h__
#define __HIPExplorer_h__

#include <QWidget>

namespace HIP {
  namespace Explorer {

    /*
     * Point explorer
     */
    class Explorer : public QWidget
    {
    public:
      Explorer (QWidget* parent);
      virtual ~Explorer ();
    };

  }
}

#endif

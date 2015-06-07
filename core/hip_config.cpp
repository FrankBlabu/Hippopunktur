/*
 * hip_config.cpp - Application configuration
 *
 * Frank Blankenburg, Feb. 2015
 */

#include "core/HIPConfig.h"

namespace HIP {
  namespace Config {

    //
    // CSS file
    //
    const char* const CSS_FILE = ":/assets/style/hippopunktur.css";

    //
    // Database to display
    //
    //const char* const DATABASE_FILE = ":/assets/models/horse/horse.xml";
    const char* const DATABASE_FILE = "assets/models/horse/horse.xml";
    //const char* const DATABASE_FILE = "assets/models/elephant/elephant.xml";
    //const char* const DATABASE_FILE = "assets/models/mandible/mandible.xml";
    //const char* const DATABASE_FILE = "assets/models/max/max.xml";
    //const char* const DATABASE_FILE = "assets/models/skeleton/skeleton.xml";
    //const char* const DATABASE_FILE = "assets/models/goose/goose.xml";
    //const char* const DATABASE_FILE = "assets/models/tie/tie.xml";

    //
    // Model modeling the pin
    //
    const char* const PIN_MODEL_FILE = "assets/models/pin/pin.obj";

    //
    // Cursors
    //
    const char* const CURSOR_ROTATE   = ":assets/cursors/cursor_rotate.png";
    const char* const CURSOR_ROTATE_Y = ":assets/cursors/cursor_rotate_y.png";

  }
}



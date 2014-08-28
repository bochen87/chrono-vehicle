// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban, Justin Madsen
// =============================================================================
//
// =============================================================================

#ifndef HMMWV_H
#define HMMWV_H

#include "ChronoT_config.h"

namespace hmmwv {

enum VisualizationType {
  NONE,
  PRIMITIVES,
  MESH
};

enum TireModelType {
  RIGID,
  PACEJKA,
  LUGRE
};


} // end namespace hmmwv


#endif

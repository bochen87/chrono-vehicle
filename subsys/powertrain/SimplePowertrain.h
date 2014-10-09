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
// Authors: Radu Serban
// =============================================================================
//
// Simplified powertrain model constructed with data from file (JSON format).
//
// =============================================================================

#ifndef SIMPLE_POWERTRAIN_H
#define SIMPLE_POWERTRAIN_H

#include "subsys/ChApiSubsys.h"
#include "subsys/powertrain/ChSimplePowertrain.h"

#include "rapidjson/document.h"

namespace chrono {


class CH_SUBSYS_API SimplePowertrain : public ChSimplePowertrain
{
public:

  SimplePowertrain(const std::string& filename);
  SimplePowertrain(const rapidjson::Document& d);
  ~SimplePowertrain() {}

  virtual double GetForwardGearRatio() const { return m_fwd_gear_ratio; }
  virtual double GetReverseGearRatio() const { return m_rev_gear_ratio; }
  virtual double GetMaxTorque() const        { return m_max_torque; }
  virtual double GetMaxSpeed() const         { return m_max_speed; }

private:

  void Create(const rapidjson::Document& d);

  double m_fwd_gear_ratio;  // forward gear ratio (single gear transmission)
  double m_rev_gear_ratio;  // reverse gear ratio
  double m_max_torque;      // maximum motor torque
  double m_max_speed;       // maximum motor speed
};


} // end namespace chrono


#endif

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
// Authors: Radu Serban, Justin Madsen, Daniel Melanz
// =============================================================================
//
// Base class for a solid axle suspension modeled with bodies and constraints.
// Derived from ChSuspension, but still an abstract base class.
//
// The suspension subsystem is modeled with respect to a right-handed frame,
// with X pointing towards the rear, Y to the right, and Z up. All point
// locations are assumed to be given for the right half of the supspension and
// will be mirrored (reflecting the y coordinates) to construct the left side.
//
// If marked as 'driven', the suspension subsystem also creates the ChShaft axle
// element and its connection to the spindle body (which provides the interface
// to the driveline subsystem).
//
// =============================================================================

#ifndef CH_SOLIDAXLE_H
#define CH_SOLIDAXLE_H

#include <vector>

#include "subsys/ChApiSubsys.h"
#include "subsys/ChSuspension.h"

namespace chrono {


class CH_SUBSYS_API ChSolidAxle : public ChSuspension
{
public:

  ChSolidAxle(const std::string& name,
                   bool               steerable = false,
                   bool               driven = false);
  virtual ~ChSolidAxle() {}

  virtual void Initialize(ChSharedBodyPtr   chassis,
                          const ChVector<>& location);

  virtual void ApplySteering(double displ);

  double GetSpringForce(ChSuspension::Side side);
  double GetSpringLen(ChSuspension::Side side);

  void LogHardpointLocations(const ChVector<>& ref,
                             bool              inches = false);
  void LogConstraintViolations(ChSuspension::Side side);

protected:

  enum PointId {
    AXLE_OUTER, // outer axle point
    SHOCK_A,    // shock, axle
    SHOCK_C,    // shock, chassis
    KNUCKLE_L,  // lower knuckle point
    KNUCKLE_U,  // upper knuckle point
    LL_A,       // lower link, axle
    LL_A_X,     // lower link, axle, x-direction     
    LL_A_Z,     // lower link, axle, z-direction
    LL_C,       // lower link, chassis
    LL_C_X,     // lower link, chassis, x-direction
    LL_C_Z,     // lower link, chassis, z-direction
    UL_A,       // upper link, axle
    UL_A_X,     // upper link, axle, x-direction     
    UL_A_Z,     // upper link, axle, z-direction
    UL_C,       // upper link, chassis
    UL_C_X,     // upper link, chassis, x-direction
    UL_C_Z,     // upper link, chassis, z-direction
    SPRING_A,   // spring, axle
    SPRING_C,   // spring, chassis
    TIEROD_C,   // tierod, chassis
    TIEROD_K,   // tierod, knuckle
    SPINDLE,    // spindle location
    KNUCKLE_CM, // knuckle, center of mass
    LL_CM,      // lower link, center of mass
    UL_CM,      // upper link, center of mass
    AXLE_CM,    // axle, center of mass
    NUM_POINTS
  };

  virtual const ChVector<> getLocation(PointId which) = 0;

  virtual double getAxleTubeMass() const = 0;
  virtual double getSpindleMass() const = 0;
  virtual double getULMass() const = 0;
  virtual double getLLMass() const = 0;
  virtual double getKnuckleMass() const = 0;

  virtual double getAxleTubeRadius() const = 0;
  virtual double getSpindleRadius() const = 0;
  virtual double getSpindleWidth() const = 0;
  virtual double getULRadius() const = 0;
  virtual double getLLRadius() const = 0;
  virtual double getKnuckleRadius() const = 0;

  virtual const ChVector<>& getAxleTubeInertia() const = 0;
  virtual const ChVector<>& getSpindleInertia() const = 0;
  virtual const ChVector<>& getULInertia() const = 0;
  virtual const ChVector<>& getLLInertia() const = 0;
  virtual const ChVector<>& getKnuckleInertia() const = 0;

  virtual double getAxleInertia() const = 0;

  virtual double getSpringCoefficient() const = 0;
  virtual double getDampingCoefficient() const = 0;
  virtual double getSpringRestLength() const = 0;


  ChSharedBodyPtr                   m_axleTube;
  ChSharedBodyPtr                   m_knuckle[2];
  ChSharedBodyPtr                   m_upperLink[2];
  ChSharedBodyPtr                   m_lowerLink[2];

  ChSharedPtr<ChLinkLockRevolute>   m_revoluteKingpin[2];
  ChSharedPtr<ChLinkLockSpherical>  m_sphericalUpperLink[2];
  ChSharedPtr<ChLinkLockSpherical>  m_sphericalLowerLink[2];
  ChSharedPtr<ChLinkLockSpherical>  m_universalUpperLink[2];
  ChSharedPtr<ChLinkLockSpherical>  m_universalLowerLink[2];
  ChSharedPtr<ChLinkDistance>       m_distTierod[2];

  ChSharedPtr<ChLinkSpring>         m_shock[2];
  ChSharedPtr<ChLinkSpring>         m_spring[2];

  ChVector<>                        m_tierod_marker[2];

private:

  void CreateSide(ChSuspension::Side side,
                  const std::string& suffix);
  void InitializeSide(ChSuspension::Side              side,
                      ChSharedBodyPtr                 chassis,
                      const std::vector<ChVector<> >& points);

  static void AddVisualizationAxleTube(ChSharedBodyPtr    axle,
                                      const ChVector<>&  pt_axleOuter,
                                      const ChVector<>&  pt_lowerLinkAxle,
                                      const ChVector<>&  pt_upperLinkAxle,
                                      double             radius_axle,
                                      double             radius_link);
  static void AddVisualizationLink(ChSharedBodyPtr    link,
                                      const ChVector<>&  pt_linkAxle,
                                      const ChVector<>&  pt_linkChassis,
                                      double             radius);
  static void AddVisualizationKnuckle(ChSharedBodyPtr knuckle,
                                      const ChVector<>&  pt_upperKnuckle,
                                      const ChVector<>&  pt_lowerKnuckle,
                                      const ChVector<>&  pt_spindle,
                                      double             radius);
  static void AddVisualizationSpindle(ChSharedBodyPtr spindle,
                                      double          radius,
                                      double          width);

  static const std::string  m_pointNames[NUM_POINTS];
};


} // end namespace chrono


#endif
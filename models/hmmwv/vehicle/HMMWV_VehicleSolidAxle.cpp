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
// HMMWV full vehicle model with solid axle suspension...
//
// =============================================================================

#include "assets/ChBoxShape.h"
#include "assets/ChTriangleMeshShape.h"

#include "utils/ChUtilsData.h"
#include "utils/ChUtilsInputOutput.h"

#include "models/hmmwv/vehicle/HMMWV_VehicleSolidAxle.h"

using namespace chrono;

namespace hmmwv {


// -----------------------------------------------------------------------------
// Static variables
// -----------------------------------------------------------------------------

static const double in2m = 0.0254;

const double     HMMWV_VehicleSolidAxle::m_chassisMass = 7747.0 / 2.2;	// chassis sprung mass
const ChVector<> HMMWV_VehicleSolidAxle::m_chassisCOM = in2m * ChVector<>(3.8, 0.585, -18.329);  // COM location
const ChVector<> HMMWV_VehicleSolidAxle::m_chassisInertia(125.8, 497.4, 531.4); // chassis inertia (roll,pitch,yaw)

const std::string HMMWV_VehicleSolidAxle::m_chassisMeshName = "hmmwv_chassis";
const std::string HMMWV_VehicleSolidAxle::m_chassisMeshFile = utils::GetModelDataFile("hmmwv/hmmwv_chassis.obj");


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
HMMWV_VehicleSolidAxle::HMMWV_VehicleSolidAxle(const bool           fixed,
                             VisualizationType    chassisVis,
                             VisualizationType    wheelVis)
{
  // -------------------------------------------
  // Create the chassis body
  // -------------------------------------------

  m_chassis = ChSharedPtr<ChBodyAuxRef>(new ChBodyAuxRef);

  m_chassis->SetIdentifier(0);
  m_chassis->SetName("chassis");
  m_chassis->SetMass(m_chassisMass);
  m_chassis->SetFrame_COG_to_REF(ChFrame<>(m_chassisCOM, ChQuaternion<>(1, 0, 0, 0)));
  m_chassis->SetInertiaXX(m_chassisInertia);
  m_chassis->SetBodyFixed(fixed);

  switch (chassisVis) {
  case PRIMITIVES:
  {
    ChSharedPtr<ChBoxShape> box1(new ChBoxShape);
    box1->GetBoxGeometry().SetLengths(ChVector<>(5, 1.7, 0.4));
    box1->Pos = ChVector<>(0, 0, -0.4);
    m_chassis->AddAsset(box1);

    ChSharedPtr<ChBoxShape> box2(new ChBoxShape);
    box2->GetBoxGeometry().SetLengths(ChVector<>(4, 1.7, 0.4));
    box2->Pos = ChVector<>(0.5, 0, 0);
    m_chassis->AddAsset(box2);

    break;
  }
  case MESH:
  {
    geometry::ChTriangleMeshConnected trimesh;
    trimesh.LoadWavefrontMesh(m_chassisMeshFile, false, false);

    ChSharedPtr<ChTriangleMeshShape> trimesh_shape(new ChTriangleMeshShape);
    trimesh_shape->SetMesh(trimesh);
    trimesh_shape->SetName(m_chassisMeshName);
    m_chassis->AddAsset(trimesh_shape);

    break;
  }
  }

  Add(m_chassis);

  // -------------------------------------------
  // Create the suspension subsystems
  // -------------------------------------------

  m_front_susp = ChSharedPtr<HMMWV_SolidAxleFront>(new HMMWV_SolidAxleFront("FrontSusp", false));
  m_rear_susp = ChSharedPtr<HMMWV_SolidAxleRear>(new HMMWV_SolidAxleRear("RearSusp", true));

  // -----------------
  // Create the wheels
  // -----------------

  m_front_right_wheel = ChSharedPtr<HMMWV_Wheel>(new HMMWV_WheelRight(wheelVis));
  m_front_left_wheel = ChSharedPtr<HMMWV_Wheel>(new HMMWV_WheelLeft(wheelVis));
  m_rear_right_wheel = ChSharedPtr<HMMWV_Wheel>(new HMMWV_WheelRight(wheelVis));
  m_rear_left_wheel = ChSharedPtr<HMMWV_Wheel>(new HMMWV_WheelLeft(wheelVis));

  // ----------------------------------------------
  // Create the driveline and powertrain subsystems
  // ----------------------------------------------

  m_driveline = ChSharedPtr<HMMWV_Driveline2WD>(new HMMWV_Driveline2WD(this));
  m_powertrain = ChSharedPtr<HMMWV_Powertrain>(new HMMWV_Powertrain(this));

  // -----------------
  // Create the brakes
  // -----------------
  m_front_right_brake = ChSharedPtr<HMMWV_BrakeSimple>(new HMMWV_BrakeSimple);
  m_front_left_brake = ChSharedPtr<HMMWV_BrakeSimple>(new HMMWV_BrakeSimple);
  m_rear_right_brake = ChSharedPtr<HMMWV_BrakeSimple>(new HMMWV_BrakeSimple);
  m_rear_left_brake = ChSharedPtr<HMMWV_BrakeSimple>(new HMMWV_BrakeSimple);
}


HMMWV_VehicleSolidAxle::~HMMWV_VehicleSolidAxle()
{
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void HMMWV_VehicleSolidAxle::Initialize(const ChCoordsys<>& chassisPos)
{
  m_chassis->SetPos(chassisPos.pos);
  m_chassis->SetRot(chassisPos.rot);

  // Initialize the suspension subsystems
  m_front_susp->Initialize(m_chassis, in2m * ChVector<>(-66.59, 0, 1.039));
  m_rear_susp->Initialize(m_chassis, in2m * ChVector<>(66.4, 0, 1.039));

  // Initialize wheels
  m_front_right_wheel->Initialize(m_front_susp->GetSpindle(ChSuspension::RIGHT));
  m_front_left_wheel->Initialize(m_front_susp->GetSpindle(ChSuspension::LEFT));
  m_rear_right_wheel->Initialize(m_rear_susp->GetSpindle(ChSuspension::RIGHT));
  m_rear_left_wheel->Initialize(m_rear_susp->GetSpindle(ChSuspension::LEFT));

  // Initialize the driveline subsystem (RWD)
  m_driveline->Initialize(m_chassis, m_rear_susp->GetAxle(ChSuspension::LEFT), m_rear_susp->GetAxle(ChSuspension::RIGHT));

  // Initialize the powertrain subsystem
  m_powertrain->Initialize(m_chassis, m_driveline->GetDriveshaft());

  // Initialize the four brakes
  m_front_right_brake->Initialize(m_front_susp->GetRevolute(ChSuspension::RIGHT));
  m_front_left_brake->Initialize(m_front_susp->GetRevolute(ChSuspension::LEFT));
  m_rear_right_brake->Initialize(m_rear_susp->GetRevolute(ChSuspension::RIGHT));
  m_rear_left_brake->Initialize(m_rear_susp->GetRevolute(ChSuspension::LEFT));
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ChSharedBodyPtr HMMWV_VehicleSolidAxle::GetWheelBody(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpindle(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpindle(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpindle(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpindle(ChSuspension::RIGHT);
  default:
    return m_front_susp->GetSpindle(ChSuspension::RIGHT);  // should not happen
  }
}

const ChVector<>& HMMWV_VehicleSolidAxle::GetWheelPos(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpindlePos(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpindlePos(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpindlePos(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpindlePos(ChSuspension::RIGHT);
  default:
    return m_front_susp->GetSpindlePos(ChSuspension::RIGHT);  // should not happen
  }
}

const ChQuaternion<>& HMMWV_VehicleSolidAxle::GetWheelRot(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpindleRot(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpindleRot(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpindleRot(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpindleRot(ChSuspension::RIGHT);
  default:
    return m_front_susp->GetSpindleRot(ChSuspension::RIGHT);  // should not happen
  }
}

const ChVector<>& HMMWV_VehicleSolidAxle::GetWheelLinVel(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpindleLinVel(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpindleLinVel(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpindleLinVel(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpindleLinVel(ChSuspension::RIGHT);
  default:
    return m_front_susp->GetSpindleLinVel(ChSuspension::RIGHT);  // should not happen
  }
}

ChVector<> HMMWV_VehicleSolidAxle::GetWheelAngVel(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpindleAngVel(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpindleAngVel(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpindleAngVel(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpindleAngVel(ChSuspension::RIGHT);
  default:
    return m_front_susp->GetSpindleAngVel(ChSuspension::RIGHT);  // should not happen
  }
}

double HMMWV_VehicleSolidAxle::GetWheelOmega(ChWheelId which) const
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetAxleSpeed(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetAxleSpeed(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetAxleSpeed(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetAxleSpeed(ChSuspension::RIGHT);
  default:
    return -1;  // should not happen
  }
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
double HMMWV_VehicleSolidAxle::GetSpringForce(chrono::ChWheelId which)
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpringForce(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpringForce(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpringForce(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpringForce(ChSuspension::RIGHT);
  default:
    return -1;  // should not happen
  }
}

double HMMWV_VehicleSolidAxle::GetSpringLength(chrono::ChWheelId which)
{
  switch (which) {
  case FRONT_LEFT:
    return m_front_susp->GetSpringLen(ChSuspension::LEFT);
  case FRONT_RIGHT:
    return m_front_susp->GetSpringLen(ChSuspension::RIGHT);
  case REAR_LEFT:
    return m_rear_susp->GetSpringLen(ChSuspension::LEFT);
  case REAR_RIGHT:
    return m_rear_susp->GetSpringLen(ChSuspension::RIGHT);
  default:
    return -1;  // should not happen
  }
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void HMMWV_VehicleSolidAxle::Update(double              time,
                           double              throttle,
                           double              steering,
                           double              braking,
                           const ChTireForces& tire_forces)
{
  // Apply steering input.
  double displ = 0.08 * steering;

  m_front_susp->ApplySteering(displ);

  // Let the powertrain subsystem process the throttle input.
  m_powertrain->Update(time, throttle);

  // Apply tire forces to spindle bodies.
  m_front_susp->ApplyTireForce(ChSuspension::RIGHT, tire_forces[FRONT_RIGHT]);
  m_front_susp->ApplyTireForce(ChSuspension::LEFT, tire_forces[FRONT_LEFT]);
  m_rear_susp->ApplyTireForce(ChSuspension::RIGHT, tire_forces[REAR_RIGHT]);
  m_rear_susp->ApplyTireForce(ChSuspension::LEFT, tire_forces[REAR_LEFT]);

  // Apply braking
  m_front_right_brake->ApplyBrakeModulation(braking);
  m_front_left_brake->ApplyBrakeModulation(braking);
  m_rear_right_brake->ApplyBrakeModulation(braking);
  m_rear_left_brake->ApplyBrakeModulation(braking);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void HMMWV_VehicleSolidAxle::ExportMeshPovray(const std::string& out_dir)
{
  utils::WriteMeshPovray(m_chassisMeshFile, m_chassisMeshName, out_dir, ChColor(0.82f, 0.7f, 0.5f));
}


// -----------------------------------------------------------------------------
// Log the hardpoint locations for the front-right and rear-right suspension
// subsystems (display in inches)
// -----------------------------------------------------------------------------
void HMMWV_VehicleSolidAxle::LogHardpointLocations()
{
  GetLog().SetNumFormat("%7.3f");

  GetLog() << "\n---- FRONT suspension hardpoint locations (RIGHT side)\n";
  m_front_susp->LogHardpointLocations(ChVector<>(37.78, 0, 30.77), true);

  GetLog() << "\n---- REAR suspension hardpoint locations (RIGHT side)\n";
  m_rear_susp->LogHardpointLocations(ChVector<>(170.77, 0, 30.77), true);

  GetLog() << "\n\n";

  GetLog().SetNumFormat("%g");
}


// -----------------------------------------------------------------------------
// Log the spring error, force, and length at design.
// Log constraint violations of suspension joints.
// -----------------------------------------------------------------------------
void HMMWV_VehicleSolidAxle::DebugLog(int what)
{
  if (what & DBG_SHOCKS)
  {
    // spring forces, front left & right (lbs)
    double springF_FL = GetSpringForce(FRONT_LEFT) / 4.45;       // design = 3491 lb.
    double springF_FR = GetSpringForce(FRONT_RIGHT) / 4.45;      // design = 3491 lb.
    // spring forces, back left & right (lbs)
    double springF_RL = GetSpringForce(REAR_LEFT) / 4.45;        // design = 6388 lb.
    double springF_RR = GetSpringForce(REAR_RIGHT) / 4.45;       // design = 6388 lb.

    // spring lengths, front left & right (inches)
    double springLen_FL = GetSpringLength(FRONT_LEFT) * 39.37;   // design = 9.7" + 4.65"
    double springLen_FR = GetSpringLength(FRONT_RIGHT) * 39.37;  // design = 9.7" + 4.65
    // spring lengths, rear left & right (inches)
    double springLen_RL = GetSpringLength(REAR_LEFT) * 39.37;    // design = 12.0" + 2.35"
    double springLen_RR = GetSpringLength(REAR_RIGHT) * 39.37;   // design = 12.0" + 2.35"

    // springs are mounted at shock points, add the distance between top shock
    // and spring hardpoints
    double dl_F = 9.7 + 4.65;
    double dl_R = 12.0 + 2.35;

    GetLog() << "---- Spring, Shock info\n";
    GetLog() << "Forces[lbs.]:"
      << "\n  FL = " << springF_FL << "\n  FR= " << springF_FR
      << "\n  RL= " << springF_RL << "\n  RR= " << springF_RR << "\n\n";
    GetLog() << "Lengths[inches]:"
      << "\n  FL = " << springLen_FL << "\n  FR = " << springLen_FR
      << "\n  RL= " << springLen_RL << "\n  RR= " << springLen_RR << "\n\n";

    GetLog() << "---- Spring Force, length error relative to design \n";
    GetLog() << "Force ERROR[lbs.]:"
      << "\n  FL = " << springF_FL - 3491.0 << "\n  FR= " << springF_FR - 3491.0
      << "\n  RL= " << springF_RL - 6388.0 << "\n  RR= " << springF_RR - 6388 << "\n\n";
    GetLog() << "Length ERROR [inches]:"
      << "\n  FL= " << springLen_FL - dl_F << "\n  FR= " << springLen_FR - dl_F
      << "\n  RL= " << springLen_RL - dl_R << "\n  RR= " << springLen_RR - dl_R << "\n\n";
  }

  if (what & DBG_CONSTRAINTS)
  {
    // Report constraint violations for the suspension joints
    GetLog() << "\n---- FRONT-RIGHT suspension constraint violation\n\n";
    m_front_susp->LogConstraintViolations(ChSuspension::RIGHT);
    GetLog() << "\n---- FRONT-LEFT suspension constraint violation\n\n";
    m_front_susp->LogConstraintViolations(ChSuspension::LEFT);
    GetLog() << "\n---- REAR-RIGHT suspension constraint violation\n\n";
    m_rear_susp->LogConstraintViolations(ChSuspension::RIGHT);
    GetLog() << "\n---- REAR-LEFT suspension constraint violation\n\n";
    m_rear_susp->LogConstraintViolations(ChSuspension::LEFT);
  }
}


} // end namespace hmmwv
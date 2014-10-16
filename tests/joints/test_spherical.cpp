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
// Authors: Mike Taylor, Radu Serban
// =============================================================================
//
// Test for spherical joint
//
// Recall that Irrlicht uses a left-hand frame, so everything is rendered with
// left and right flipped.
//
// =============================================================================
// TO DO:
//    Report test run time & test pass/fail (determine what the criteria is)
// =============================================================================

#include <ostream>
#include <fstream>

#include "core/ChFileutils.h"

#include "physics/ChSystem.h"
#include "physics/ChBody.h"

#include "unit_IRRLICHT/ChIrrApp.h"

#include "ChronoT_config.h"
#include "utils/ChUtilsData.h"
#include "utils/ChUtilsInputOutput.h"
#include "utils/ChUtilsValidation.h"

using namespace chrono;
using namespace irr;


// =============================================================================

void TestSpherical(const ChVector<>&     loc,
                   const ChQuaternion<>& revAxisRot,
                   double                simTimeStep,
                   const std::string&    outputFilename,
                   bool                  animate)
{

  //Settings
  //----------------------------------------------------------------------------
  // There are no units in Chrono, so values must be consistant (MKS is used in this example)

  double mass = 1.0;                // mass of pendulum
  double length = 4.0;              // length of pendulum
  ChVector<> inertiaXX(1, 1, 1);    // mass moments of inertia of pendulum
  double g = 9.80665;

  double timeRecord = 5;            // Stop recording to the file after this much simulated time
  double printTimeStep = 0.001;     // Write the output file at this simulation time step

  SetChronoDataPath(CHRONO_DATA_DIR);


  // Create the mechanical system
  // ----------------------------

  // 1- Create a ChronoENGINE physical system: all bodies and constraints
  //    will be handled by this ChSystem object.

  ChSystem my_system;
  my_system.Set_G_acc(ChVector<>(0.0, 0.0, -g));

  my_system.SetIntegrationType(ChSystem::INT_ANITESCU);
  my_system.SetIterLCPmaxItersSpeed(100);
  my_system.SetIterLCPmaxItersStab(100); //Tasora stepper uses this, Anitescu does not
  my_system.SetLcpSolverType(ChSystem::LCP_ITERATIVE_SOR);

  // 2- Create the rigid bodies of the system

  // ..the ground
  ChSharedBodyPtr  ground(new ChBody);
  my_system.AddBody(ground);
  ground->SetBodyFixed(true);
  // Add some geometry to the ground body for visualizing the spherical joint
  ChSharedPtr<ChSphereShape> sph_g(new ChSphereShape);
  sph_g->GetSphereGeometry().center = loc;
  sph_g->GetSphereGeometry().rad = 0.2;
  ground->AddAsset(sph_g);

  // ..the pendulum (Assumes the pendulum's CG is at half its length)
  ChSharedBodyPtr  pendulum(new ChBody);
  my_system.AddBody(pendulum);
  pendulum->SetPos(loc + ChVector<>(length / 2, 0, 0));   // position of COG of pendulum in the Global Reference Frame
  pendulum->SetMass(mass);
  pendulum->SetInertiaXX(inertiaXX);   // Set the body's inertia about the CG in the Global Reference Frame 
  // Add some geometry to the pendulum for visualization
  ChSharedPtr<ChCylinderShape> cyl_p(new ChCylinderShape);
  cyl_p->GetCylinderGeometry().p1 = ChVector<>(-length / 2, 0, 0);
  cyl_p->GetCylinderGeometry().p2 = ChVector<>(length / 2, 0, 0);
  cyl_p->GetCylinderGeometry().rad = 0.1;
  pendulum->AddAsset(cyl_p);

  // 3- Create constraints: the mechanical joints between the rigid bodies.

  // .. a spherical joint between pendulum and ground at "loc" in the global reference frame with the applied rotation
  ChSharedPtr<ChLinkLockSpherical>  sphericalJoint(new ChLinkLockSpherical);
  sphericalJoint->Initialize(pendulum, ground, ChCoordsys<>(loc, revAxisRot));
  my_system.AddLink(sphericalJoint);


  // Create the Irrlicht application for visualization
  // -------------------------------------------------
  ChIrrApp * application;
  if(animate){
    application = new ChIrrApp(&my_system, L"ChLinkLockSpherical demo", core::dimension2d<u32>(800, 600), false, true);
    application->AddTypicalLogo();
    application->AddTypicalSky();
    application->AddTypicalLights();
    core::vector3df lookat((f32)loc.x, (f32)loc.y, (f32)loc.z);
    application->AddTypicalCamera(lookat + core::vector3df(0, 3, -6), lookat);

    application->AssetBindAll();     //Now have the visulization tool (Irrlicht) create its geometry from the assets defined above
    application->AssetUpdateAll();

    application->SetTimestep(simTimeStep);
  }

  // Create output file for results & add in column headers (tab deliminated)
  // ------------------------------------------------------------------------
  std::ofstream outf(outputFilename.c_str());
  if (outf) {
    outf << "timeElapsed(s)\t";
    outf << "X_Pos(m)\tY_Pos(m)\tZ_Pos\tLength_Pos(m)\t";
    outf << "X_Vel(m/s)\tY_Vel(m/s)\tZ_Vel(m/s)\tLength_Vel(m/s)\t";
    outf << "X_Accel(m/s^2)\tY_Accel(m/s^2)\tZ_Accell(m/s^2)\tLength_Accel(m/s^2)\t";
    outf << "e0_quaternion\te1_quaternion\te2_quaternion\te3_quaternion\t";
    outf << "X_AngVel(rad/s)\tY_AngVel(rad/s)\tZ_AngVel(rad/s)\tLength_AngVel(rad/s)\t";
    outf << "X_AngAccel(rad/s^2)\tY_AngAccel(rad/s^2)\tZ_AngAccell(rad/s^2)\tLength_AngAccel(rad/s^2)\t";
    outf << "X_Glb_ReactionFrc(N)\tY_Glb_ReactionFrc(N)\tZ_Glb_ReactionFrc(N)\tLength_Glb_ReactionFrc(N)\t";
    outf << "X_Glb_ReactionTrq(Nm)\tY_Glb_ReactionTrq(Nm)\tZ_Glb_ReactionTrq(Nm)\tLength_Glb_ReactionTrq(Nm)\t";
    outf << "Total_Kinetic_Energy(J)\tTranslational_Kinetic_Energy(J)\tAngular_Kinetic_Energy(J)\tDelta_Potential_Energy(J)\t";
    outf << std::endl;
  }
  else {
    std::cout << "Output file is invalid" << std::endl;
  }


  // Simulation loop
  // ---------------

  double timeElapsed = 0;
  double lastPrint = -printTimeStep;
  bool continueSimulation = true;

  while (continueSimulation)
  {
    // Write current translational and rotational position, velocity, acceleration, 
    // reaction force, and reaction torque of pendulum to output file

    //Add a little error tolerance on the end time to ensure that the final data point is recorded
    if (outf && (timeElapsed <= timeRecord+simTimeStep/2) && (timeElapsed+simTimeStep/2>=lastPrint+printTimeStep)) {
      lastPrint = lastPrint + printTimeStep;

      // Time elapsed
      outf << timeElapsed << "\t";

      // Position of the Pendulum's CG in the Global Reference Frame
      ChVector<double> position = pendulum->GetPos();
      outf << position.x << "\t" << position.y << "\t" << position.z << "\t" << (position - loc).Length() << "\t";

      // Velocity of the Pendulum's CG in the Global Reference Frame
      ChVector<double> velocity = pendulum->GetPos_dt();
      outf << velocity.x << "\t" << velocity.y << "\t" << velocity.z << "\t" << velocity.Length() << "\t";

      // Acceleration of the Pendulum's CG in the Global Reference Frame
      ChVector<double> acceleration = pendulum->GetPos_dtdt();
      outf << acceleration.x << "\t" << acceleration.y << "\t" << acceleration.z << "\t" << acceleration.Length() << "\t";

      // Angular Position quaternion of the Pendulum with respect to the Global Reference Frame
      ChQuaternion<double> rot = pendulum->GetRot();
      outf << rot.e0 << "\t" << rot.e1 << "\t" << rot.e2 << "\t" << rot.e3 << "\t";

      // Angular Velocity of the Pendulum with respect to the Global Reference Frame
      ChVector<double> angVel = pendulum->GetWvel_par();
      outf << angVel.x << "\t" << angVel.y << "\t" << angVel.z << "\t" << angVel.Length() << "\t";

      // Angular Acceleration of the Pendulum with respect to the Global Reference Frame
      ChVector<double> angAccel = pendulum->GetWacc_par();
      outf << angAccel.x << "\t" << angAccel.y << "\t" << angAccel.z << "\t" << angAccel.Length() << "\t";

      // Reaction Force and Torque
      // These are expressed in the link coordinate system. We convert them to
      // the coordinate system of Body2 (in our case this is the ground).
      ChCoordsys<> linkCoordsys = sphericalJoint->GetLinkRelativeCoords();
      ChVector<double> reactForce = sphericalJoint->Get_react_force();
      ChVector<double> reactForceGlobal = linkCoordsys.TransformDirectionLocalToParent(reactForce);
      outf << reactForceGlobal.x << "\t" << reactForceGlobal.y << "\t" << reactForceGlobal.z << "\t" << reactForceGlobal.Length() << "\t";

      ChVector<double> reactTorque = sphericalJoint->Get_react_torque();
      ChVector<double> reactTorqueGlobal = linkCoordsys.TransformDirectionLocalToParent(reactTorque);
      outf << reactTorqueGlobal.x << "\t" << reactTorqueGlobal.y << "\t" << reactTorqueGlobal.z << "\t" << reactTorqueGlobal.Length() << "\t";

      // Conservation of Energy
      //Translational Kinetic Energy (1/2*m*||v||^2)
      //Rotational Kinetic Energy (1/2 w'*I*w)  ChMatrix33*vector is valid since [3x3]*[3x1] = [3x1]
      //Delta Potential Energy (m*g*dz)
      ChMatrix33<> inertia = pendulum->GetInertia(); //3x3 Inertia Tensor in the local coordinate frame
      ChVector<> angVelLoc = pendulum->GetWvel_loc();
      double transKE = 0.5*mass*velocity.Length2();
      double rotKE = 0.5*Vdot(angVelLoc, inertia * angVelLoc);
      double deltaPE = mass*g*(position.z-loc.z);
      double totalKE = transKE + rotKE;
      outf << totalKE  << "\t" << transKE  << "\t" << rotKE  << "\t" << deltaPE  << "\t"  << std::endl;;
    }


    // Output a message to the command window once timeRecord has been reached
    //   Add a little error tolerance to make sure this event is captured
    if ((timeElapsed >= timeRecord-simTimeStep/2) && (timeElapsed <= timeRecord+simTimeStep/2)) {
        std::cout << "All Simulation Results have been recorded to file." << std::endl;
    }

    // Advance simulation by one step
    timeElapsed += simTimeStep;
    if(animate) {
      application->BeginScene();
      application->DrawAll();

      // Draw an XZ grid at the global origin to add in visualization
      ChIrrTools::drawGrid(
        application->GetVideoDriver(), 1, 1, 20, 20,
        ChCoordsys<>(ChVector<>(0, 0, 0), Q_from_AngX(CH_C_PI_2)),
        video::SColor(255, 80, 100, 100), true);

      application->DoStep();  //Take one step in time
      application->EndScene();

      continueSimulation = application->GetDevice()->run();
    }
    else{
      my_system.DoStepDynamics(simTimeStep);  //Take one step in time
      continueSimulation = (timeElapsed <= timeRecord+simTimeStep/2);
    }
  }

  // Close output file
  outf.close();


}

// =============================================================================

int main(int argc, char* argv[])
{
  bool animate = (argc > 1);

  // Set the path to the Chrono data folder
  // --------------------------------------

  SetChronoDataPath(CHRONO_DATA_DIR);

  // Create output directory (if it does not already exist)
  if (ChFileutils::MakeDirectory("../VALIDATION") < 0) {
    std::cout << "Error creating directory '../VALIDATION'" << std::endl;
    return 1;
  }
  if (ChFileutils::MakeDirectory("../VALIDATION/SPHERICAL_JOINT") < 0) {
    std::cout << "Error creating directory '../VALIDATION/SPHERICAL_JOINT'" << std::endl;
    return 1;
  }

  std::string out_dir = "../VALIDATION/SPHERICAL_JOINT/";
  std::string ref_dir = "validation/spherical_joint/";

  bool check;
  bool test_passed = true;


  std::cout << "\nStarting Spherical Test Case 01\n\n";
  //Case 1 - Spherical Joint at the origin, and aligned with the global coordinate system
  //  Note the spherical joint only allows 3 DOF(all 3 rotations)
  TestSpherical(ChVector<> (0, 0, 0), ChQuaternion<> (Q_from_AngX(0)), .001, "SphericalJointData_Case01.txt",animate);

  std::cout << "\nStarting Spherical Test Case 02\n\n";
  //Case 2 - Spherical Joint at (1,2,3), and rotated to align the z axis with line Y=Z
  //  Note the spherical joint only allows 3 DOF(all 3 rotations)
  //    A joint rotation here does not change the kinematics, its just for test purposes
  TestSpherical(ChVector<> (1, 2, 3), ChQuaternion<> (Q_from_AngX(-CH_C_PI_4)), .001, "SphericalJointData_Case02.txt",animate);


  // Return 0 if all test passed and 1 otherwise
  // -------------------------------------------

  return !test_passed;
}

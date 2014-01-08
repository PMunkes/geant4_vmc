// $Id$

//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007, 2008 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: vmc@pcroot.cern.ch
//-------------------------------------------------

/// \file TG4SpecialPhysicsList.cxx
/// \brief Implementation of the TG4SpecialPhysicsList class 
///
/// \author I. Hrivnacova; IPN, Orsay

#include "TG4SpecialPhysicsList.h"
#include "TG4SpecialCutsPhysics.h"
#include "TG4StepLimiterPhysics.h"
#include "TG4StackPopperPhysics.h"
#include "TG4UserParticlesPhysics.h"
#include "TG4ExtDecayerPhysics.h"
#include "TG4ProcessControlMapPhysics.h"
#include "TG4ProcessMCMapPhysics.h"
#include "TG4EmModelPhysics.h"
#include "TG4GeometryServices.h"
#include "TG4G3PhysicsManager.h"
#include "TG4G3ControlVector.h"
#include "TG4ProcessControlMap.h"

#include <G4ParticleDefinition.hh>
#include <G4ProcessManager.hh>
#include <G4ProcessTable.hh>

// According to G4VModularPhysicsList.cc
#include <G4StateManager.hh>
// This macros change the references to fields that are now encapsulated
// in the class G4VMPLData.
#define G4MT_physicsVector ((G4VMPLsubInstanceManager.offset[g4vmplInstanceID]).physicsVector)

G4ThreadLocal TG4SpecialPhysicsList* TG4SpecialPhysicsList::fgInstance = 0;

//
// static methods
//

//_____________________________________________________________________________
G4String TG4SpecialPhysicsList::AvailableSelections()
{
/// Return list of all available selections

  G4String selections;
  selections += "stepLimiter ";
  selections += "specialCuts ";
  selections += "stackPopper ";
  
  return selections;
}  

//_____________________________________________________________________________
G4bool TG4SpecialPhysicsList::IsAvailableSelection(const G4String& selection)
{
/// Return list of all available selections

  G4int itoken = 0;
  TString token = TG4Globals::GetToken(itoken, selection);
     
  while ( token != "" ) {
    if ( ! AvailableSelections().contains(token.Data()) ) return false; 
    token = TG4Globals::GetToken(++itoken, selection); 
  }        
  
  return true;
}  

//
// ctors, dtor
//

//_____________________________________________________________________________
TG4SpecialPhysicsList::TG4SpecialPhysicsList(const G4String& selection)
  : G4VModularPhysicsList(),
    TG4Verbose("specialPhysicsList"),
    fStackPopperPhysics(0),
    fEmModelPhysics(0),
    fIsSpecialCuts(false)
{
/// Standard constructor

  if (VerboseLevel() > 1)
    G4cout << "TG4SpecialPhysicsList::TG4SpecialPhysicsList" << G4endl;

  if (fgInstance) {
    TG4Globals::Exception(
      "TG4SpecialPhysicsList", "TG4SpecialPhysicsList",
      "Cannot create two instances of singleton.");
  }
  fgInstance = this;  

  Configure(selection);

  SetVerboseLevel(TG4VVerbose::VerboseLevel());
}

//_____________________________________________________________________________
TG4SpecialPhysicsList::TG4SpecialPhysicsList()
  : G4VModularPhysicsList(),
    TG4Verbose("physicsList"),
    fStackPopperPhysics(0),
    fEmModelPhysics(0),
    fIsSpecialCuts(false)
{
/// Default constructor

  G4cout << "TG4SpecialPhysicsList::TG4SpecialPhysicsList" << G4endl;

  Configure("");

  SetVerboseLevel(TG4VVerbose::VerboseLevel());
}

//_____________________________________________________________________________
TG4SpecialPhysicsList::~TG4SpecialPhysicsList() 
{
/// Destructor

  //delete fExtDecayer;
       // fExtDecayer is deleted in G4Decay destructor

  fgInstance = 0; 
}

//
// private methods
//

//_____________________________________________________________________________
void TG4SpecialPhysicsList::Configure(const G4String& selection)
{
/// Create the selected physics constructors
/// and registeres them in the modular physics list.

  Int_t tg4VerboseLevel = TG4VVerbose::VerboseLevel();

  RegisterPhysics(new TG4ProcessControlMapPhysics(tg4VerboseLevel));

  G4int itoken = 0;
  TString token = TG4Globals::GetToken(itoken, selection);
  while ( token != "" ) {

    if ( token == "specialCuts" ) { 
      // G4cout << "Registering special cuts physics" << G4endl;
      RegisterPhysics(new TG4SpecialCutsPhysics(tg4VerboseLevel));
      fIsSpecialCuts = true;
    }  
    else if ( token == "stepLimiter" ) { 
      // G4cout << "Registering step limiter physics" << G4endl;
      RegisterPhysics(new TG4StepLimiterPhysics(tg4VerboseLevel));
    }  
    else if ( token == "stackPopper" ) {    
      // G4cout << "Registering stack popper physics" << G4endl;
      fStackPopperPhysics
        = new TG4StackPopperPhysics(tg4VerboseLevel); 
      RegisterPhysics(fStackPopperPhysics);
    }
    else {
      TG4Globals::Warning("TG4SpecialPhysicsList", "Configure",
        "Unrecognized option " + token);
    }
    token = TG4Globals::GetToken(++itoken, selection); 
  }  
  RegisterPhysics(new TG4UserParticlesPhysics(tg4VerboseLevel));
  RegisterPhysics(new TG4ExtDecayerPhysics(tg4VerboseLevel));
  RegisterPhysics(new TG4ProcessMCMapPhysics(tg4VerboseLevel));
  
  fEmModelPhysics = new TG4EmModelPhysics(tg4VerboseLevel);
  RegisterPhysics(fEmModelPhysics);
}    

//
// public methods
//

//_____________________________________________________________________________
void TG4SpecialPhysicsList::ConstructProcess()
{
/// Construct all processes.

  // lock physics manager
  TG4G3PhysicsManager* g3PhysicsManager = TG4G3PhysicsManager::Instance();
  g3PhysicsManager->Lock();  

  // create processes for registered physics
  // To avoid call AddTransportation twice we do not call directly
  // G4VModularPhysicsList::ConstructProcess();
  // but call registered processes ourselves:
  G4PhysConstVector::iterator itr;
  for (itr = G4MT_physicsVector->begin(); itr!= G4MT_physicsVector->end(); ++itr) {
    (*itr)->ConstructProcess();
  }
}

//_____________________________________________________________________________
G4int TG4SpecialPhysicsList::VerboseLevel() const 
{
/// Return verbose level (via TG4VVerbose)

  return TG4VVerbose::VerboseLevel();
}

//_____________________________________________________________________________
void TG4SpecialPhysicsList::VerboseLevel(G4int level) 
{
/// Set the specified level to both TG4Verbose and 
/// G4VModularPhysicsList.
/// The verbose level is also propagated to registered physics contructors.

  TG4VVerbose::VerboseLevel(level);
  SetVerboseLevel(level);
  
  G4PhysConstVector::iterator it;
  for ( it = G4MT_physicsVector->begin(); it != G4MT_physicsVector->end(); ++it ) {
    TG4Verbose* verbose = dynamic_cast<TG4Verbose*>(*it);
    if ( verbose )
      verbose->VerboseLevel(level);
    else
      (*it)->SetVerboseLevel(level);  
  }
}

//_____________________________________________________________________________
void TG4SpecialPhysicsList::SetStackPopperSelection(const G4String& selection)
{
/// Select particles with stack popper process

  if ( !fStackPopperPhysics ) {
    TG4Globals::Exception(
      "TG4SpecialPhysicsList", "SetStackPopperSelection",
      "SetStackPopper physics is not activated.");
  }  
  
  fStackPopperPhysics->SetSelection(selection); 

  G4cout << "TG4SpecialPhysicsList::SetStackPopperSelection: " << selection << G4endl;
}   
 
//_____________________________________________________________________________
void TG4SpecialPhysicsList::SetEmModel(G4int mediumId, 
                                       const G4String& elossModelName,
                                       const G4String& fluctModelName,
                                       const G4String& particles)
{ 
/// Set extra EM models to a region corresponding to given tracking medium.                                       

  fEmModelPhysics->SetEmModel(mediumId, elossModelName, fluctModelName, particles);
}    

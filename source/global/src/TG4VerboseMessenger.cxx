// $Id: TG4VerboseMessenger.cxx,v 1.3 2004/11/10 11:39:28 brun Exp $
// Category: global
//
// Class TG4VerboseMessenger
// ------------------
// See the class description in the header file.
//
// Author: I. Hrivnacova

#include "TG4VerboseMessenger.h"
#include "TG4VVerbose.h"
#include "TG4Globals.h"

#include <G4UImanager.hh>
#include <G4UIcmdWithAnInteger.hh>
#include <G4UIdirectory.hh>
#include <G4UImanager.hh>
#include <G4UIcommandTree.hh>

//_____________________________________________________________________________
TG4VerboseMessenger::TG4VerboseMessenger(const G4String& directoryName)
  : fkDirectoryName(directoryName)
{ 
//

  fDirectory = new G4UIdirectory(directoryName);
  fDirectory->SetGuidance("TGeant4 verbose control commands.");

  // sets the given level to all verbose instances
  fGlobalVerboseCmd 
    = new G4UIcmdWithAnInteger(G4String(directoryName + "all"), this);
  G4String guidance("Set a given verbose level to all verbose instances.");
  fGlobalVerboseCmd->SetGuidance(guidance);
  fGlobalVerboseCmd->SetParameterName("GlobalVerbose", false);
  fGlobalVerboseCmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);
}

//_____________________________________________________________________________
TG4VerboseMessenger::TG4VerboseMessenger(){
//
} 

//_____________________________________________________________________________
TG4VerboseMessenger::TG4VerboseMessenger(const TG4VerboseMessenger& right) {
// 
  TG4Globals::Exception("TG4VerboseMessenger is protected from copying.");
}

//_____________________________________________________________________________
TG4VerboseMessenger::~TG4VerboseMessenger() {
//

  delete fDirectory;
  delete fGlobalVerboseCmd;

  for (G4int i=0; i<G4int(fCommandVector.size()); i++)
    delete fCommandVector[i];
}

//
// operators
//

//_____________________________________________________________________________
TG4VerboseMessenger& TG4VerboseMessenger::operator=(const TG4VerboseMessenger& right)
{
  // check assignement to self
  if (this == &right) return *this;

  TG4Globals::Exception("TG4VerboseMessenger is protected from assigning.");
    
  return *this;  
}    
          
//
// private methods
//

//_____________________________________________________________________________
void TG4VerboseMessenger::SetNewValueToAll(const G4String value) const
{
/// Set the value to all registered verbose instances.
   
   G4UIcommandTree* cmdTree
     = G4UImanager::GetUIpointer()->GetTree()->GetTree(fkDirectoryName);

   for (G4int i=0; i<cmdTree->GetCommandEntry(); i++) {
     if (cmdTree->GetCommand(i+1)->GetCommandName() != "all") {    
        // skip the first command in the tree ("all")
        cmdTree->GetCommand(i+1)->DoIt(value);     
     }	
   }  
}

//
// public methods
//

//_____________________________________________________________________________
G4UIcommand* TG4VerboseMessenger::AddCommand(TG4VVerbose* verbose, 
                                             const G4String& cmdName)
{
/// Add the command specified by cmdName and associate verbose object.
//--

  G4UIcmdWithAnInteger* cmd 
    = new G4UIcmdWithAnInteger(G4String(fkDirectoryName + cmdName), this);

  fVerboseVector.push_back(verbose);
  fCommandVector.push_back(cmd);

  G4String guidance("Set  verbose level.");
  guidance.insert(4,cmdName);
  cmd->SetGuidance(guidance);

  G4String parameterName("Verbose");
  parameterName.insert(0,cmdName);
  cmd->SetParameterName(parameterName, false);
  
  cmd->AvailableForStates(G4State_PreInit, G4State_Init, G4State_Idle);
  
  return cmd;
}

//_____________________________________________________________________________
void TG4VerboseMessenger::RemoveCommand(TG4VVerbose* verbose, 
                                        G4UIcommand* command)
{
/// Remove the specified verbose and associate command
//--

  // Remove verbose from the array
  VerboseVector::iterator pos1
    = find(fVerboseVector.begin(), fVerboseVector.end(), verbose);
  if (pos1 != fVerboseVector.end()) fVerboseVector.erase(pos1);

  // Remove command from the array
  CommandVector::iterator pos2
    = find(fCommandVector.begin(), fCommandVector.end(), command);
  if (pos2 != fCommandVector.end())  fCommandVector.erase(pos2);

  // Delete command
  // Command is automatically removed from UI manager
  delete command;
}

//_____________________________________________________________________________
void TG4VerboseMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{ 
/// Apply command to the associated object.

  if (command == fGlobalVerboseCmd) {
    SetNewValueToAll(newValue); 
  }    
  for (G4int i=0; i<G4int(fCommandVector.size()); i++)  
    if (command == fCommandVector[i]) {
      fVerboseVector[i]
        ->VerboseLevel(fCommandVector[i]->GetNewIntValue(newValue)); 
    }    
}

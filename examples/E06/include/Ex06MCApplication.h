// $Id: $
//
// Geant4 ExampleN06 adapted to Virtual Monte Carlo 
//
// Class Ex06MCApplication
// ----------------------- 
// Implementation of the TVirtualMCApplication
//
// by Ivana Hrivnacova, 16.5.2005

#ifndef EX06_MC_APPLICATION_H
#define EX06_MC_APPLICATION_H

#include <TVirtualMCApplication.h>
#include "TMCVerbose.h"

class TCanvas;

class Ex03MCStack;
class Ex06DetectorConstruction;
class Ex06PrimaryGenerator;

class Ex06MCApplication : public TVirtualMCApplication
{
  public:
    Ex06MCApplication(const char* name,  const char *title);
    Ex06MCApplication();
    virtual ~Ex06MCApplication();
  
    // static access method
    static Ex06MCApplication* Instance(); 

    // methods
    void InitMC(const char *setup);
    void RunMC(Int_t nofEvents);
 
    virtual void ConstructGeometry();
    virtual void ConstructOpGeometry();
    virtual void InitGeometry();
    virtual void GeneratePrimaries();
    virtual void BeginEvent();
    virtual void BeginPrimary();
    virtual void PreTrack();
    virtual void Stepping();
    virtual void PostTrack();
    virtual void FinishPrimary();
    virtual void FinishEvent();
    virtual void Field(const Double_t* x, Double_t* b) const;
    
    // set methods
    void  SetVerboseLevel(Int_t verboseLevel);
 
    // get methods
    Ex06PrimaryGenerator*  GetPrimaryGenerator() const;
 
  private:
    // methods
    void RegisterStack();
  
    // data members
    Int_t                     fEventNo;
    Int_t                     fGammaCounter;
    TMCVerbose                fVerbose;
    Ex03MCStack*              fStack;
    Ex06DetectorConstruction* fDetConstruction;
    Ex06PrimaryGenerator*     fPrimaryGenerator;
    TCanvas*                  fCanvas;

  ClassDef(Ex06MCApplication,1)  //Interface to MonteCarlo application
};

// inline functions

inline Ex06MCApplication* Ex06MCApplication::Instance()
{ return (Ex06MCApplication*)(TVirtualMCApplication::Instance()); }

inline void  Ex06MCApplication::SetVerboseLevel(Int_t verboseLevel)
{ fVerbose.SetLevel(verboseLevel); }

inline Ex06PrimaryGenerator*  Ex06MCApplication::GetPrimaryGenerator() const
{ return fPrimaryGenerator; }

#endif //EX06_MC_APPLICATION_H

// Created by Miljenko Suljic, m.suljic@cern.ch

#ifndef ALIGNMENT_HPP
#define ALIGNMENT_HPP

#include <TObject.h>
#include <TVector3.h>
//#include <TMath.h>
#include <TRotation.h>

/*
*___Chip coordinates__________
*         ____________
*  ^ Y    |0         |
*  | Row  |__________|
*  ------> X Column
* 
*___Local coordinates_________
*    _____          
*    |  0|      ^ Y
*    |   |      |   
*    |   |  X   |   
*    |___|  <---- (Z entering the screen)
*_____________________________
*/

class Alignment: public TObject {

public:

    Alignment();
    Alignment(Float_t px, Float_t py, Float_t pz, TRotation rot=TRotation());
    virtual ~Alignment() {}

    void     RecalcVariables(); // for faster computation, so the same operations are not done over and over again
    
    Float_t  PixColToLocalY(Float_t col)           { return fPixDX-(col+0.5)*fPixX; }
    Float_t  PixRowToLocalX(Float_t row)           { return (row+0.5)*fPixY-fPixDY; }
    TVector3 PixToLocal(Float_t col, Float_t row)  { return TVector3(PixRowToLocalX(row), PixColToLocalY(col), 0.); }
    TVector3 LocalToGlobal(TVector3 local)         { return fPos + fRot*local; }
    TVector3 PixToGlobal(Float_t col, Float_t row) { return LocalToGlobal(PixToLocal(col, row)); }

    TVector3 GlobalToLocal(TVector3 global);
    Float_t  LocalYToPixCol(Float_t ly)            { return (fPixDX-ly)/fPixX; } // TODO check 0.5
    Float_t  LocalXToPixRow(Float_t lx)            { return (lx+fPixDY)/fPixY; } // TODO check 0.5
    void     LocalToPix(TVector3 local, Float_t &col, Float_t &row);
    void     GlobalToPix(TVector3 global, Float_t &col, Float_t &row);

    TVector3 IntersectionPointWithLine(TVector3 o, TVector3 d);
    Bool_t   CheckPointIsInPlane(TVector3 p);
    Bool_t   CheckPointIsInMatrix(TVector3 p);

    TVector3 DistPixLine(Float_t col, Float_t row, TVector3 o, TVector3 d, Bool_t local=kTRUE);
    
    void SetPos      (TVector3 pos)  { fPos = pos;    RecalcVariables(); }
    void SetPosX     (Float_t px)    { fPos.SetX(px); RecalcVariables(); }
    void SetPosY     (Float_t py)    { fPos.SetY(py); RecalcVariables(); }
    void SetPosZ     (Float_t pz)    { fPos.SetZ(pz); RecalcVariables(); }
    void SetRotation (TRotation rot) { fRot = rot;    RecalcVariables(); }

    TVector3  GetPos      () const { return fPos;     }
    Float_t   GetPosX     () const { return fPos.X(); }
    Float_t   GetPosY     () const { return fPos.Y(); }
    Float_t   GetPosZ     () const { return fPos.Z(); }
    TRotation GetRotation () const { return fRot;     }

private:
    // alignment variables
    TVector3  fPos; // position of the centre of the plane wrt ref system
    TRotation fRot; // rotation around local axes
    // constants
    static const Short_t fNPixX; // number of columns
    static const Short_t fNPixY; // number of columns
    static const Float_t fPixX;  // pixel dimension x
    static const Float_t fPixY;  // pixel dimension y
    static const Float_t fPixDX; // distance x of pixel (0, 0) farmost corner and chip centre i.e. half sensor size
    static const Float_t fPixDY; // distance x of pixel (0, 0) farmost corner and chip centre i.e. half sensor size
    // calculation variables
    TVector3 fN;  // normal vector (in global coo)
    TVector3 fP;  // point in plane (in global coo)
    Float_t  fD;  // plane parameter d
    
    ClassDef(Alignment,1);
};

#endif









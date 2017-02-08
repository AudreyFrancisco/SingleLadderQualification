#include "Riostream.h"
#include <TString.h>
#include "Alignment.hpp"

using namespace std;

ClassImp(Alignment)

// in chip coordinates!!!
const Short_t Alignment::fNPixX = 1024;
const Short_t Alignment::fNPixY = 512;
const Float_t Alignment::fPixX  = 0.02924;
const Float_t Alignment::fPixY  = 0.02688;
const Float_t Alignment::fPixDX = 14.97088;
const Float_t Alignment::fPixDY = 6.88128;

//______________________________________________________________________
Alignment::Alignment():
    TObject(),
    fPos(0,0,0),
    fRot()
{
    RecalcVariables();
} 

//______________________________________________________________________
Alignment::Alignment(Float_t px, Float_t py, Float_t pz, TRotation rot):
    TObject(),
    fPos(px,py,pz),
    fRot(rot)
{
    RecalcVariables();
}

//______________________________________________________________________
void Alignment::RecalcVariables() {
    fN = LocalToGlobal(TVector3(0.,0.,1.))-fPos;
    fP = LocalToGlobal(TVector3(1.,1.,0.));
    fD = fN * fP;
}

//______________________________________________________________________
TVector3 Alignment::GlobalToLocal(TVector3 global) {
    TRotation invrot = fRot.Inverse();
    return invrot*(global-fPos);
}

//______________________________________________________________________
void Alignment::LocalToPix(TVector3 local, Float_t &col, Float_t &row) {
    col = LocalYToPixCol(local.Y());
    row = LocalXToPixRow(local.X());
}

//______________________________________________________________________
void Alignment::GlobalToPix(TVector3 global, Float_t &col, Float_t &row) {
    LocalToPix(GlobalToLocal(global), col, row);
}

//______________________________________________________________________
TVector3 Alignment::IntersectionPointWithLine(TVector3 o, TVector3 d) {
    // get intersection point with a line
    // o - line point, d - line direction (global coordinates)
    Float_t t = (fD - fN*o)/(fN*d);
    return o + t*d;
}


//______________________________________________________________________
Bool_t Alignment::CheckPointIsInPlane(TVector3 p) {
    // point in global coordinates
    Float_t val = fN*p - fD;
    if( -1.e-5 < val && val < 1.e-5 )
        return kTRUE;
    else
        return kFALSE;
}

//______________________________________________________________________
Bool_t Alignment::CheckPointIsInMatrix(TVector3 p) {
    // point in global coordinates
    if(!CheckPointIsInPlane(p)) return kFALSE;
    Float_t col, row;
    GlobalToPix(p, col, row);
    if(col < 0. || col > fNPixX || row < 0. || row > fNPixY)
        return kFALSE;
    else
        return kTRUE;
}

//______________________________________________________________________
TVector3 Alignment::DistPixLine(Float_t col, Float_t row, TVector3 o, TVector3 d, Bool_t local) {
    // return distance between pixel and line
    // local toggles global or local coordinates
    TVector3 p = IntersectionPointWithLine(o, d);
    TVector3 c;
    if(local) {
        p = GlobalToLocal(p);
        c = PixToLocal(col, row);
    }
    else
        c = PixToGlobal(col, row);
    return (c - p);
}

//______________________________________________________________________
Bool_t Alignment::WriteFile(TString fname) {
    ofstream out;
    out.open(fname.Data());
    if(!out.is_open()) return kFALSE;
    out << "Alignment_parameters" << endl
        << "X\t" << fPos.X() << endl
        << "Y\t" << fPos.Y() << endl
        << "Z\t" << fPos.Z() << endl
        << "XPhi\t" << fRot.GetXPhi() << endl
        << "XTheta\t" << fRot.GetXTheta() << endl
        << "XPsi\t" << fRot.GetXPsi() << endl;
        out.close();
    return kTRUE;
}

//______________________________________________________________________
Bool_t Alignment::ReadFile(TString fname) {
    TString s;
    Float_t x, y, z, xphi, xpsi, xtheta;
    ifstream in;
    in.open(fname.Data());
    if(!in.is_open()) return kFALSE;
    in >> s; if(!s.Contains("Alignment_parameters")) {in.close(); return kFALSE;}
    in >> s >> x; if(!s.EqualTo("X")) {in.close(); return kFALSE;}
    in >> s >> y; if(!s.EqualTo("Y")) {in.close(); return kFALSE;}
    in >> s >> z; if(!s.EqualTo("Z")) {in.close(); return kFALSE;}
    in >> s >> xphi; if(!s.EqualTo("XPhi")) {in.close(); return kFALSE;}
    in >> s >> xtheta; if(!s.EqualTo("XTheta")) {in.close(); return kFALSE;}
    in >> s >> xpsi; if(!s.EqualTo("XPsi")) {in.close(); return kFALSE;}
    if(!in.good()) {in.close(); return kFALSE;}
    in.close();
    fPos.SetXYZ(x, y, z);
    fRot.SetToIdentity();
    fRot.SetXEulerAngles(xphi, xtheta, xpsi);
    return kTRUE;
}

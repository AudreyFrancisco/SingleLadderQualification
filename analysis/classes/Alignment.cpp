#include "Riostream.h"
#include "Alignment.hpp"

using namespace std;

ClassImp(Alignment)

// in chip coordinates!!!
const Float_t Alignment::fPixX = 0.02924;
const Float_t Alignment::fPixY = 0.02688;
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
TVector3 Alignment::IntersectionPointWithLine(TVector3 p, TVector3 d) {
    // get intersection point with a line
    // p - line point, d - line direction (global coordinates)
    Float_t t = (fD - fN*p)/(fN*d);
    return p + t*d;
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

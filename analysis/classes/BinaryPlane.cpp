#include "BinaryPlane.hpp"
#include <iostream>

using namespace std;

ClassImp(BinaryPlane)

// default constructor
//______________________________________________________________________
BinaryPlane::BinaryPlane()
:TObject(),
    fNClusters(0),
    fNPixelsX(-1),
    fNPixelsY(-1),
    fPlaneID(-1),
    fPosX(-1.),
    fPosY(-1.),
    fPosZ(-1.),
    fNHitPix(-1),
    fClusters(0)
{
    fClusters=new TClonesArray(BinaryCluster::Class());
    fChipName[0]=0;
}

// copy constructor
//______________________________________________________________________
BinaryPlane::BinaryPlane(const BinaryPlane& orig)
    :TObject(orig),
     fNClusters(0),
     fNPixelsX(-1),
     fNPixelsY(-1),
     fPlaneID(-1),
     fPosX(-1.),
     fPosY(-1.),
     fPosZ(-1.),
     fNHitPix(-1),
     fClusters(0)
{
    fClusters=new TClonesArray(BinaryCluster::Class());
    fChipName[0]=0;
    cout << "copy constructor should not be used" << endl;
}

// assignment operator
//______________________________________________________________________
BinaryPlane& BinaryPlane::operator=(const BinaryPlane& orig) {
    if (this!=&orig) {
        strcpy(fChipName,orig.fChipName);
        fNClusters=orig.fNClusters;
        fNPixelsX=orig.fNPixelsX;
        fNPixelsY=orig.fNPixelsY;
        fPlaneID=orig.fPlaneID;
        fPosX=orig.fPosX;
        fPosY=orig.fPosY;
        fPosZ=orig.fPosZ;    
        fNHitPix=orig.fNHitPix;
        *fClusters=*orig.fClusters;
    }
    return *this;
}

// add cluster to plane
//______________________________________________________________________
void BinaryPlane::AddCluster(BinaryCluster *cluster) {
    BinaryCluster *add_clust = (BinaryCluster*)fClusters->ConstructedAt(fNClusters++);
    *add_clust = *cluster;
}
   
// get specific cluster with index idx from plane
//______________________________________________________________________
BinaryCluster* BinaryPlane::GetCluster(Int_t idx) {
    return static_cast<BinaryCluster*>(fClusters->UncheckedAt(idx));
}

// get number of hit pixels
// if recalc = false, use the saved info
// else recalculate humber of hit pixels
//______________________________________________________________________
Int_t BinaryPlane::GetNHitPix(Bool_t recalc) {
    if(recalc) {
        Int_t npix = 0;
        for(Int_t i=0; i < GetNClustersSaved(); ++i)
            npix += GetCluster(i)->GetNPixels();
        fNHitPix = npix;
    }
    return fNHitPix;
}


// reset plane
//______________________________________________________________________
void BinaryPlane::Reset() {
    fChipName[0]=0;
    fNClusters=0;
    fNPixelsX=-1;
    fNPixelsY=-1;
    fPlaneID=-1;
    fPosX=-1.;
    fPosY=-1.;
    fPosZ=-1.;
    fNHitPix=-1;
    fClusters->Clear();
}





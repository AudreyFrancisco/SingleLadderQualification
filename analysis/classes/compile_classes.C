#include <iostream>
#include "TROOT.h"

void compile_classes() {
    gROOT->LoadMacro("AliPALPIDEFSRawStreamMS.cpp+g");
    gROOT->LoadMacro("BinaryPixel.cpp+g");
    gROOT->LoadMacro("BinaryCluster.cpp+g");
    gROOT->LoadMacro("BinaryPlane.cpp+g");
    gROOT->LoadMacro("BinaryEvent.cpp+g");
    gROOT->LoadMacro("helpers.cpp+g");

    gROOT->LoadMacro("AliALPIDEModuleStreamMS.cpp+g");
    
    cout << "compile_classes() : Classes compiled." << endl;
}

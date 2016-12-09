void x_csa() {

    gROOT->LoadMacro("../classes/AliPALPIDEFSRawStreamMS.cpp+");
    gROOT->LoadMacro("../classes/BinaryPixel.cpp+");
    gROOT->LoadMacro("../classes/BinaryCluster.cpp+");
    gROOT->LoadMacro("../classes/BinaryPlane.cpp+");
    gROOT->LoadMacro("../classes/BinaryEvent.cpp+");
    gROOT->LoadMacro("../classes/helpers.cpp+");
    gROOT->LoadMacro("csa.C+");
    gROOT->LoadMacro("basic_analysis.C+");

    csa("/home/palpidefs/MOSAIC/new-alpide-software/Data/na61_161208_night//NoiseOccupancy_161208_231852_Chip2.dat",
        "/home/palpidefs/MOSAIC/new-alpide-software/Data/na61_161208_night//NoiseOccupancy_161208_231852_Chip2_tree.root");
//        "");
//
////    basic_analysis();
////    interesting_events();
////    TBrowser* lal = new TBrowser();
}
    

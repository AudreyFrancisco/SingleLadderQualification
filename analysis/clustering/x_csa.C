void x_csa() {

    gROOT->LoadMacro("../classes/AliPALPIDEFSRawStreamMS.cpp+");
    gROOT->LoadMacro("../classes/BinaryPixel.cpp+");
    gROOT->LoadMacro("../classes/BinaryCluster.cpp+");
    gROOT->LoadMacro("../classes/BinaryPlane.cpp+");
    gROOT->LoadMacro("../classes/BinaryEvent.cpp+");
    gROOT->LoadMacro("../classes/helpers.cpp+");
    gROOT->LoadMacro("csa.C+");

    //csa("/home/msuljic/work/na61/data/NoiseOccupancy_161211_001735/NoiseOccupancy_161211_001735_Chip4.dat",
    //  "/home/msuljic/work/na61/data/NoiseOccupancy_161211_001735/NoiseOccupancy_161211_001735_Chip4_tree.root");

    csa("/home/msuljic/work/na61/data/NoiseOccupancy_161211_001735/NoiseOccupancy_161211_001735_Chip5.dat",
        "/home/msuljic/work/na61/data/NoiseOccupancy_161211_001735/NoiseOccupancy_161211_001735_Chip5.dat");


//        "");
//
////    basic_analysis();
////    interesting_events();
////    TBrowser* lal = new TBrowser();
}
    

void monitoring_main(char *filename="") {

    gSystem->Load("AliPALPIDEFSRawStreamMS_cpp.so");
    //gROOT->LoadMacro("monitoring_palpidefs.C+");
    //gROOT->LoadMacro("monitoring_IBHIC.C+");
    gROOT->LoadMacro("monitoring_IBHICw.C+");
    Monitoring r;
    r.SetFile(filename);
    r.Run();

}



#include <iostream>
#include <fstream>

#include <TStyle.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>

#include "helpers.h"


//_______________________________________________________________________________________________
void reset_meas_config(MeasConfig_t *conf) {
    conf->VCASN       = -1;
    conf->VCASP       = -1;
    conf->ITHR        = -1;
    conf->IDB         = -1;
    conf->IBIAS       = -1;
    conf->IRESET      = -1;
    conf->VRESETP     = -1;
    conf->VRESETD     = -1;
    conf->VCASN2      = -1;
    conf->VCLIP       = -1;
    // temp
    conf->Temp        = -1.;
    // chip config
    conf->STROBELENGTH= -1; 
    // general DAQBOARD config
    conf->TRIGGERDELAY= -1;
    conf->PULSEDELAY  = -1;
    // stuff for pulselength measurements
    conf->NTRIGGERS   = -1;
    conf->ROW         = -1;
    conf->COL         = -1;
    conf->CHARGESTART = -1;
    conf->CHARGESTOP  = -1;
    conf->CHARGESTEP  = -1;
    conf->PULSEDELAYSTART = -1;
    conf->PULSEDELAYSTOP  = -1;
    conf->PULSEDELAYSTEP  = -1;
    // chiller temperature
    conf->TEMP_SET  = -1;
    // VBB
    conf->VBB       = -1;
}


//_______________________________________________________________________________________________
void read_int_parameter (const char *line, int *address, bool expect_rest) {
    char param[128];
    char rest[896];
    if (!expect_rest) {
        sscanf (line,"%s\t%d", param, address);
        //std::cout << "found parameter " << param << ", value " << *address << std::endl;
    }
    else {
        sscanf (line,"%s\t%d\t%s", param, address, rest);
        //std::cout << "found parameter " << param << ", value " << *address << std::endl;
    }
}


//_______________________________________________________________________________________________
void read_float_parameter (const char *line, float *address, bool expect_rest) {
    char param[128];
    char rest[896];
    if (!expect_rest) {
        sscanf (line,"%s\t%f", param, address);
        //std::cout << "found parameter " << param << ", value " << *address << std::endl;
    }
    else {
        sscanf (line,"%s\t%f%s", param, address, rest);
        //std::cout << "found parameter " << param << ", value " << *address << std::endl;
    }
}


//_______________________________________________________________________________________________
void decode_line(const char *line, MeasConfig_t *conf) {
    char param[128], rest[896];
    if ((line[0] == '\n') || (line[0] == '#')) {   // empty Line or comment
        return;
    }
    sscanf (line,"%s\t%s", param, rest);

    if (!strcmp(param,"VCASN")) {
        read_int_parameter(line, &(conf->VCASN));
    }
    if (!strcmp(param,"VCASP")) {
        read_int_parameter(line, &(conf->VCASP));
    }
    if (!strcmp(param,"IRESET")) {
        read_int_parameter(line, &(conf->IRESET));
    }
    if (!strcmp(param,"IBIAS")) {
        read_int_parameter(line, &(conf->IBIAS));
    }
    if (!strcmp(param,"IDB")) {
        read_int_parameter(line, &(conf->IDB));
    }
    if (!strcmp(param,"ITHR")) {
        read_int_parameter(line, &(conf->ITHR));
    }
    if (!strcmp(param,"VRESETP")) {
        read_int_parameter(line, &(conf->VRESETP));
    }
    if (!strcmp(param,"VRESETD")) {
        read_int_parameter(line, &(conf->VRESETD));
    }
    if (!strcmp(param,"VCASN2")) {
        read_int_parameter(line, &(conf->VCASN2));
    }
    if (!strcmp(param,"VCLIP")) {
        read_int_parameter(line, &(conf->VCLIP));
    }


    if (!strcmp(param,"FROMU_CONFIG2")) {
        read_int_parameter(line, &(conf->STROBELENGTH), true);
    }
    if (!strcmp(param,"TRIGGERDELAY")) {
        read_int_parameter(line, &(conf->TRIGGERDELAY), true);
    }
    if (!strcmp(param,"PULSEDELAY")) {
        read_int_parameter(line, &(conf->PULSEDELAY), true);
    }

    if (!strcmp(param,"NTRIGGERS")) {
        read_int_parameter(line, &(conf->NTRIGGERS), true);
    }
    if (!strcmp(param,"ROW")) {
        read_int_parameter(line, &(conf->ROW), true);
    }
    if (!strcmp(param,"COL")) {
        read_int_parameter(line, &(conf->COL), true);
    }
    if (!strcmp(param,"CHARGESTART")) {
        read_int_parameter(line, &(conf->CHARGESTART), true);
    }
    if (!strcmp(param,"CHARGESTOP")) {
        read_int_parameter(line, &(conf->CHARGESTOP), true);
    }
    if (!strcmp(param,"CHARGESTEP")) {
        read_int_parameter(line, &(conf->CHARGESTEP), true);
    }
    if (!strcmp(param,"PULSEDELAYSTART")) {
        read_int_parameter(line, &(conf->PULSEDELAYSTART), true);
    }
    if (!strcmp(param,"PULSEDELAYSTOP")) {
        read_int_parameter(line, &(conf->PULSEDELAYSTOP), true);
    }
    if (!strcmp(param,"PULSEDELAYSTEP")) {
        read_int_parameter(line, &(conf->PULSEDELAYSTEP), true);
    }


    if (!strcmp(param,"TEMP")) {
        read_int_parameter(line, &(conf->TEMP_SET));
    }
    if (!strcmp(param,"VBB")) {
        read_float_parameter(line, &(conf->VBB));
    }

    if (!strcmp(param,"TempI")) {
        read_float_parameter(line, &(conf->Temp));
    }
}


//_______________________________________________________________________________________________
void print_meas_config(MeasConfig_t conf) {
    std::cout << "VCASN:            " << conf.VCASN     << std::endl; 
    std::cout << "VCASP:            " << conf.VCASP     << std::endl; 
    std::cout << "ITHR:             " << conf.ITHR      << std::endl; 
    std::cout << "IDB:              " << conf.IDB       << std::endl; 
    std::cout << "IBIAS:            " << conf.IBIAS     << std::endl; 
    std::cout << "IRESET:           " << conf.IRESET    << std::endl; 
    std::cout << "VCASN2:           " << conf.VCASN2    << std::endl; 
    std::cout << "VCLIP:            " << conf.VCLIP     << std::endl; 
    std::cout << "VRESETP:          " << conf.VRESETP   << std::endl; 
    std::cout << "VRESETD:          " << conf.VRESETD   << std::endl; 
    std::cout << std::endl;
    std::cout << "Temp:             " << conf.Temp      << std::endl; 
    std::cout << std::endl;
    std::cout << "STROBELENGTH:     " << conf.STROBELENGTH << std::endl; 
    std::cout << std::endl;
    std::cout << "TRIGGERDELAY:     " << conf.TRIGGERDELAY << std::endl; 
    std::cout << "PULSEDELAY:       " << conf.PULSEDELAY << std::endl; 
    std::cout << std::endl;
    std::cout << "NTRIGGERS:        " << conf.NTRIGGERS << std::endl; 
    std::cout << "ROW:              " << conf.ROW << std::endl; 
    std::cout << "COL:              " << conf.COL << std::endl; 
    std::cout << "CHARGESTART:      " << conf.CHARGESTART << std::endl; 
    std::cout << "CHARGESTOP:       " << conf.CHARGESTOP << std::endl; 
    std::cout << "CHARGESTEP:       " << conf.CHARGESTEP << std::endl; 
    std::cout << "PULSEDELAYSTART:  " << conf.PULSEDELAYSTART << std::endl; 
    std::cout << "PULSEDELAYSTOP:   " << conf.PULSEDELAYSTOP << std::endl; 
    std::cout << "PULSEDELAYSTEP:   " << conf.PULSEDELAYSTEP << std::endl; 
    std::cout << std::endl;
    std::cout << "TEMP SET: " << conf.TEMP_SET  << std::endl; 
    std::cout << "VBB:      " << conf.VBB       << std::endl; 
    std::cout << std::endl;
}


//_______________________________________________________________________________________________
MeasConfig_t read_config_file (const char *fName) {
    char line[1024];
    FILE *fp = fopen (fName, "r");
    MeasConfig_t conf;
    reset_meas_config(&conf);

    if (!fp) {
        std::cout << "WARNING: Config file " << fName << " not found" << std::endl;
        return conf;
    }

    // process the file
    while (fgets(line, 1023, fp) != NULL) {
        decode_line(line, &conf);
    }
    
    //print_meas_config(conf);
    return conf;
}



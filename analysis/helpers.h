#ifndef HELPERS_H
#define HELPERS_H

struct MeasConfig_t {
    // bias config
    Int_t VCASN;
    Int_t VCASP;
    Int_t ITHR;
    Int_t IDB;
    Int_t IBIAS;
    Int_t IRESET;
    Int_t VRESETP;
    Int_t VRESETD;
    Int_t VCASN2;
    Int_t VCLIP;
    // temp
    Float_t Temp;
    // chip config
    Int_t STROBELENGTH;
    // general DAQBOARD config
    Int_t TRIGGERDELAY;
    Int_t PULSEDELAY;
    // stuff for pulselength measurements
    Int_t NTRIGGERS;
    Int_t ROW;
    Int_t COL;
    Int_t CHARGESTART;
    Int_t CHARGESTOP;
    Int_t CHARGESTEP;
    Int_t PULSEDELAYSTART;
    Int_t PULSEDELAYSTOP;
    Int_t PULSEDELAYSTEP;

    Int_t MASKSTAGES;
    // chiller temperature
    Int_t TEMP_SET;
    // vbb
    Float_t VBB;
};


void read_float_parameter (const char *line, float *address, bool expect_rest=false);
void read_int_parameter (const char *line, int *address, bool expect_rest=false);
void decode_line(const char *line, MeasConfig_t *conf); 
void print_meas_config(MeasConfig_t conf);
void reset_meas_config(MeasConfig_t *conf);
MeasConfig_t read_config_file (const char *fName);


#endif

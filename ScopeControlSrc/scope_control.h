#pragma once
#include <string>
#include <serial/serial.h>

class scope_control
{
  public:
    scope_control();
    ~scope_control();
    bool open(std::string port, uint32_t baud = 9600, uint32_t timeoutms = 1500);
    bool open_auto(uint32_t timeoutms = 1500);
    void close();
    std::string enumerate_ports();
    void write(std::string data);
    void write_cmd(std::string data);
    std::string write_query(std::string data);
    std::string read();
    std::string get_model();
    std::string model;
    void reset();
    void cls();
    void enable_ch(uint8_t ch);
    void disable_ch(uint8_t ch);
    void set_vscale_ch(uint8_t ch, double scale);
    void set_timescale(double time);
    void start_quick_meas();
    void stop_quick_meas();
    void get_quick_meas();
    void set_trigger_ext();
    void single_capture();
    void wait_for_trigger();
    void get_errors();
    bool debug_en = false;
    // Returned by get quick measurments
    double peak; // Peak to peak
    double upe;  // Vp+
    double lpe;  // Vp-
    double cycr; // RMS cycl
    double cycm; // Mean cycl
    double per;  // Period
    double freq; // Frequency
    double rtim; // Risetime
    double ftim; // Falltime
  private:
    void msleep(unsigned long  milliseconds);
    serial::Serial *link;
    bool link_init = 0;
    void throw_ex(const char*);
    void debug_print(const char*);
    bool check_model();
    bool eval_ch(uint8_t ch);
};


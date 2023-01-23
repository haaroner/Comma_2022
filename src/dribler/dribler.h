#include "project_config.h"
#include "pin_setup.h"
#include "time_service.h"

class dribler
{
  public:
    dribler(pin &control_pin, uint8_t tim_num, bool initialization);
    void set_speed(int16_t speed, uint16_t d_speed);
    
  private:
    uint16_t convert_to_pulse(int16_t data);
    pin _control_pin;
    int16_t _init_pulse;
    int16_t _cur_speed;
    uint16_t _period;
    int16_t _max_speed;
    int16_t _min_speed;
    uint32_t _cur_time;
};
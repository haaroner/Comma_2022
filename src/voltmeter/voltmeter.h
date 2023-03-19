#include "project_config.h"
#include "pin_setup.h"
#include "adc.h"
#include "dma.h"

class voltmeter
{
  public:
    voltmeter(Dma &dma_meter); 

    float get_voltage(uint16_t _data);
  private:
    double _data;
    double _kv;
    //pin _adc_pin;
    //Adc _adc_meter;
    Dma _dma_meter;
};
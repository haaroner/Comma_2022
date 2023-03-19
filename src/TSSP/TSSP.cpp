#include "TSSP.h"

TSSP::TSSP(uint16_t reading_mode, pin &write_1, pin &write_2, pin &write_3, pin &write_4,pin &read_1,
            pin &read_2, Dma &left_dma, Dma &right_dma):
_write_1(write_1), _write_2(write_2), _write_3(write_3), _write_4(write_4), 
_read_1(read_1), _read_2(read_2), _left_dma(left_dma), _right_dma(right_dma)
{ 
  for(int i = 0; i < 32; i++)
  {
    _data[i] = 0;
    _result[i] = 0;
  }
  _is_see = true;
  _mode = reading_mode;
  _min_analog_level = 3000;//the highest level that considered to be "is see signal"
  //(the signal from tssp inverted 0 - see 1 - dont see)
}

void TSSP::get_data()
{
  if(_mode == digital) get_digital_data();
  else if(_mode == analog) get_analog_data();
}

//function that calculates angle and distance to the ball using digital data from sensors
void TSSP::get_digital_data()
{
  _x = 0;
  _y = 0;
  //reading data from tssps: high - doesnt see low - do see //esli kto to prideretsa k do
  //to mogu poyasnit za ispolzovanie etogo slova zdes
  for(int i = 0; i < 16; i++)
  {
    set_addres(i);
    _data[31 - i] = !(_read_1.getGPIOx()->IDR & _read_1.getPinNumber());//_read_1.read();
    _data[15 - i] = !(_read_2.getGPIOx()->IDR & _read_2.getPinNumber());
  }
  
  for(int i = 0; i < 32; i++)
  {
    _result[i] = floor(double(_data[i])); // just the ability to add some filters: _data[i] / number of reading iterations
    _x += _result[i] * sin(double(get_angle_from_index(i) * DEG2RAD));
    _y += _result[i] * cos(double(get_angle_from_index(i) * DEG2RAD));
    _test = get_angle_from_index(i);//just testing variable
  }
  _result_angle = int(atan2(double(_x), double(_y)) * 57.3) + 180;
  
  if(_result_angle < -180)
      _result_angle += 360;
  else if(_result_angle > 180)
      _result_angle -= 360; //aligning to -180d to 180d borders(standart format in this program)
  
  _result_distance = int(sqrt(pow(double(_x), 2) + pow(double(_y), 2)));
}

//the same function but instead of digital reading, adc is used 
void TSSP::get_analog_data()
{
  _x = 0;
  _y = 0;
  for(int i = 0; i < 16; i++)
  {
    set_addres(i);
    _adc_data[31 - i] = 4096 - _left_dma.dataReturn(0);//_read_1.read();
    _adc_data[15 - i] = 4096 - _right_dma.dataReturn(0);
  }
  
  for(int i = 0; i < 32; i++)
  {
    _result[i] = floor(double(_data[i])); 
    _x += _result[i] * sin(double(get_angle_from_index(i) * DEG2RAD));
    _y += _result[i] * cos(double(get_angle_from_index(i) * DEG2RAD));
    _test = get_angle_from_index(i);
  }
  _result_angle = int(atan2(double(_x), double(_y)) * 57.3) + 180;
  
  if(_result_angle < -180)
      _result_angle += 360;
  else if(_result_angle > 180)
      _result_angle -= 360;
  
  _result_distance = int(sqrt(pow(double(_x), 2) + pow(double(_y), 2)));
}

int16_t TSSP::get_angle()
{   
  return _result_angle;
}

bool TSSP::is_see()
{
  _is_see = false;
  for(int i = 0; i < 32; i++)
  {
    if((_data[i] == 1 && _mode == digital) || 
      (_adc_data[i] < _min_analog_level && _mode == analog))
    {
      _is_see = true;
      break;
    }
  }
  return _is_see;
}

uint16_t TSSP::get_distance()
{
  return _result_distance;
}

void TSSP::set_addres(uint8_t _data)
{
  uint16_t channels[16][4] = {
                                {0,0,0,0},
                                {0,0,0,1},
                                {0,0,1,0},
                                {0,0,1,1},
                                {0,1,0,0},
                                {0,1,0,1},
                                {0,1,1,0},
                                {0,1,1,1},
                                {1,0,0,0},
                                {1,0,0,1},
                                {1,0,1,0},
                                {1,0,1,1},
                                {1,1,0,0},
                                {1,1,0,1},
                                {1,1,1,0},
                                {1,1,1,1}
                              };
  
  _write_1.write(channels[_data][0]);
  _write_2.write(channels[_data][1]);
  _write_3.write(channels[_data][2]);
  _write_4.write(channels[_data][3]);
}

uint16_t TSSP::get_angle_from_index(uint16_t _num)
{
  _num *= 11.25;
  _num += 180;
  
  if(_num > 360)
    _num -= 360;
  
  return _num;
}
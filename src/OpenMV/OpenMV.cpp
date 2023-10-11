#include "OpenMV.h"

uint8_t camera::crc8(uint8_t* data, int len)
{
    uint8_t crc = 0xFF, i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (char)((crc << 1) ^ 0x31);
            else crc <<= 1;
        }
    }
    return crc;
}

uint16_t camera::lead_to_degree_borders(int num)
{
    if (num > 360)
        num -= 360;
    else if (num < 0)
        num += 360;
    return num;
    }

camera::camera(pin &tx, pin &rx):m_tx(tx), m_rx(rx)
{
  m_tx.pinInit();
  m_rx.pinInit();
  usart6::usart6Init(460800, 8, 1);
  sign = 1;
  num = 0;
  _x = 0;
  _yellow_angle = 0;
  _blue_angle = 180;
  _yellow_distance = 100;
  _blue_distance = 80;
  near_gate_color = 2;
  color = true;
  old_data = 0;
  read = true;
  _received = false;
  _center_y = 115;
  _yellow_first_receive = false;
  _blue_first_receive = false;
  _first_receive = false;
  _length_between_gates = 235; //(234,8)
}
void camera::getData()
{
  if(usart6::available() > 5)
  {
    camera_data = usart6::read();
    if(camera_data == 255)
    {
      for(int i = 0; i < 5; i++)
        data[i] = usart6::read();

      if(data[4] == crc8(data, 4))
      {
        _state = 1;
        if(data[1] != 0)
        {
          _yellow_angle = data[0]  * 4;
          _yellow_distance = data[1] * 4;
          _yellow_first_receive = true;
        }
        else
        {
          _yellow_first_receive = false;
          _state = 0;
        }

        if(data[3] != 0)
        {
          _blue_angle = data[2] * 4;
          _blue_distance = data[3] * 4;
          _blue_first_receive = true;
        }
        else
        {
          _blue_first_receive = false;
          _state = 2;
        }

        if(data[1] == 0 && data[3] == 0)
          _state = 8; 
        _received = true;
        _first_receive = true;
     }
    }
  }
}

void camera::calculate_pos(int16_t angle, bool side)
{
  if(_received)
  {
    //angle = lead_to_degree_borders(angle);
    _yellow_angle = _yellow_angle + angle;
    _blue_angle = _blue_angle + angle;
    _yellow_angle = lead_to_degree_borders(_yellow_angle);
    _blue_angle = lead_to_degree_borders(_blue_angle);
//    if(_yellow_first_receive && !_blue_first_receive)
//    {
//      if (_yellow_angle > 270 or _yellow_angle < 90)
//        _blue_angle = 180;
//      else
//        _blue_angle = 0;
//    }
//    else if(!_yellow_first_receive && _blue_first_receive)
//    {
//      if (_blue_angle > 270 or _blue_angle < 90)
//        _yellow_angle = 180;
//      else 
//        _yellow_angle = 0;
//    }
    if(side) //enemy gate: yellow - 1; blue - 0
    {
       _front_angle = _yellow_angle;
       _front_distance = _yellow_distance;
       _backward_angle = _blue_angle;
       _backward_distance = _blue_distance;
    }
    else
    {
       _front_angle = _blue_angle;
       _front_distance = _blue_distance;
       _backward_angle = _yellow_angle;
       _backward_distance = _yellow_distance;
        if(_state == 0)
          _state = 2;
        else if(_state == 2)
          _state = 0;
     }
    
     switch(_state)
     {
       case 0: _front_angle = 0; break;
       case 2: _backward_angle = 180; break;
       case 8: _front_angle = 0; _backward_angle = 180; break;
     }
     
//    _front_angle = _blue_angle;
//    _front_distance = _blue_distance;
//    _backward_angle = _yellow_angle;
//    _backward_distance = _yellow_distance;


    if(_front_distance + _backward_distance == 0)
      _front_distance += 1;
    _forward_sin = sin(double(_front_angle) / 57.3);
    _backward_sin = sin(double(_backward_angle) / 57.3);

    if(_state == 0)
      _x = _backward_distance * _backward_sin;
    else if(_state == 2)
      _x = _front_distance * _forward_sin;
    else
      _x = (_front_distance * _forward_sin + _backward_distance * _backward_sin) / 2;

    
    if(_state == 1)
    {
      if(_front_distance < 100)
        _y = _length_between_gates - _front_distance * abs(cos(_front_angle * DEG2RAD));
      else if(_backward_distance < 100)
        _y = _backward_distance * abs(cos(_backward_angle * DEG2RAD));
      else
        _y = (_length_between_gates - _front_distance * abs(cos(_front_angle * DEG2RAD)) + 
      _backward_distance * abs(cos(_backward_angle * DEG2RAD))) / 2;
    }
    else
    {
      switch(_state)
      {
        case 0: _y = abs(_backward_distance * cos(_backward_angle / 57.3) * -1); break;
        case 2: _y = _length_between_gates - _front_distance * abs(cos(_front_angle * DEG2RAD)); break;
      }
    }
//    if(_state == 1)
//    {
//      if (_front_distance < _backward_distance)
//          _y = abs(_center_y * 2 - _front_distance * cos(_front_angle / 57.3));
//      else
//          _y = abs(_backward_distance * cos(_backward_angle / 57.3) * -1);
//    }
//    else
//    {
//      if(_state == 0)
//        _y = abs(_backward_distance * cos(_backward_angle / 57.3) * -1);
//      else
//        _y = abs(_center_y * 2 - _front_distance * cos(_front_angle / 57.3));
//    }

    _x = int(ceil(double(_x))) * -1;
    _y = int(ceil(double(_y)));
    _received = false;
  }
}

int16_t camera::get_x()
{
  return _x;
}

int16_t camera::get_y()
{
  return _y;
}

int16_t camera::get_forward_angle()
{
  if (_front_angle > 180)
        _front_angle -= 360;
    else if (_front_angle < -180)
        _front_angle += 360;
  return _front_angle;
}

int16_t camera::get_backward_angle()
{
  if (_backward_angle > 180)
    _backward_angle -= 360;
  else if (_backward_angle < -180)
    _backward_angle += 360;
  
  return _backward_angle;
}

int16_t camera::get_forward_distance()
{
  return _front_distance;
}

int16_t camera::get_backward_distance()
{
  return _backward_distance;
}

uint8_t camera::get_data_state()
{
  return _state;
}

bool camera::is_first_data_received()
{
  return _first_receive;
}

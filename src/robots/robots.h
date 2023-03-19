#include "project_config.h"

#include "usart3.h"
#include "time_service.h"

class robots
{
  public:
    robots()
    {
      _angle = 0;
      _dist = 0;
      _robot_dist = 0;
      _start_byte = 0;
    }
    void send_data(uint16_t _my_angle, uint16_t _my_dist)
    {
      _robot_dist = _my_dist;
      if(_my_angle + 180 >= 360) _my_angle = 359;
      else _my_angle += 180;
      if(_my_dist >= 1000) _my_dist = 1000;
      usart3::write(255);
      usart3::write(floor(double(_my_angle / 4)));
      usart3::write(floor(double(_my_dist / 4)));
    }
    void get_data()
    {
      if(usart3::available() >= 3)
      {
        _start_byte = usart3::read();
        if(_start_byte == 255)
        {
          _angle = usart3::read() * 4 - 180;
          _dist = usart3::read() * 4;
          _received = true;
        }
        else if(_start_byte == 254)
          _received = false;
        _time = time_service::getCurTime();
      }
      if(time_service::getCurTime() - _time > 100)
        _received = false;
    }
    bool get_role()
    {
      if(_received)
      {
        if(_robot_dist > _dist)
          return 0;
        else
          return 1;
      }
      return 1;
    }
  private:
    bool _received;
    uint32_t _time;
    int _angle, _dist, _robot_dist, _start_byte;
};
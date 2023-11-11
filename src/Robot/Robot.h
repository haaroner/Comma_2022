#pragma once

#include "libs.h"
#include "tools.h"


namespace Robot
{
  pin led1('A', 0, write_UP); 
  pin led2('A', 1, write_UP); 
  pin led3('A', 2, write_UP); 
  
  Motor m1('D', 12, tim4, CHANNEL1, 'D', 13, tim4, CHANNEL2);					
  Motor m3('B', 3, tim2, CHANNEL2, 'B', 4, tim3, CHANNEL1);
  Motor m2('E', 13, tim1, CHANNEL3, 'E', 14, tim1, CHANNEL4);
  Motor m4('D', 14, tim4, CHANNEL3, 'D', 15, tim4, CHANNEL4);
  motors motors(m1, m2, m3, m4, 100, 7);//1.5
  
  pin spi2_sck('B', 13, spi2);
  pin spi2_mosi('B', 15, spi2);
  pin spi2_miso('B', 14, spi2);
  pin spi2_ck('B', 12, write_DOWN);
    
  pin spi3_mosi('C', 12, spi3);
  pin spi3_sck('C', 10, spi3);
  pin OLED_DC('A', 15, write_UP);
  pin spi3_nss('E', 0, write_UP);
  pin OLED_res('C', 11, write_UP);
  
  pin usart6_tx('C', 6,  uart6);	
  pin usart6_rx('C', 7,  uart6);
    
    
  pin motors_move('B', 1, read_UP);
  pin gyro_reset('B', 0, read_UP);	
  pin cap_charge('B', 9, write_);
  pin cap_discharge('E', 0, write_);
  pin dribler_control('A', 3, dribler_);
  pin adc_voltage_pin('C', 5, adc);
  pin ball_sen('A', 7, adc);
    
  pin up('B', 5, read_UP);
  pin reset_gyro('B', 6, read_UP);
  pin enter('B', 7, read_UP);
    
  camera camera(usart6_tx, usart6_rx);
    
  mpu9250_spi mpu_sensor(spi2_ck);
  IMU mpu(mpu_sensor);
  
  void control_led(uint8_t _led_num, bool _data);
  void moveRobot(int16_t _angle, uint8_t _speed);
  void rotateRobot(int16_t _angular_speed);
  void update();
  
  uint32_t time;
  int16_t move_angle = 0, angular_speed = 0, max_angular_speed, gyro = 0,
  robot_x, robot_y;
  uint8_t move_speed = 0;
  
  void init_robot(uint8_t role)
  { 
    time_service::init();
    time_service::startTime();
    
    control_led(0, ON);
    
    usart2::usart2Init(115200, 8, 1);//gyro
    usart6::usart6Init(460800, 8, 1);//camera
       
    mpu.init();
    mpu.update();
  }
  
  void control_led(uint8_t _led_num, bool _data)
  {
    switch(_led_num)
    {
      case 0: led1.write(_data); led2.write(_data); led3.write(_data); break;
      case 1: led1.write(_data); break;
      case 2: led2.write(_data); break;
      case 3: led3.write(_data); break;
    }
  }
  
  void moveRobot(int16_t _angle, uint8_t _speed)
  {
    move_angle = lead_to_degree_borders(_angle);
    if(_speed < 0) _speed *= -1;
    if(_speed > 100) _speed = 100;
    move_speed = _speed;
  }
  void rotateRobot(int16_t _angular_speed, int16_t _max_angular_speed)
  {
    if(_angular_speed < 0) _angular_speed *= -1;
    if(_angular_speed > 100) _angular_speed = 100;
    angular_speed = _angular_speed;
    
    if(_max_angular_speed < 0) _max_angular_speed *= -1;
    if(_max_angular_speed > 100) _max_angular_speed = 100;
    max_angular_speed = _max_angular_speed;
  }
  
  void update()
  {
    time = time_service::getCurTime();
    gyro = lead_to_degree_borders(mpu.update());
    motors.moveRobot(move_speed, max_angular_speed, move_angle, angular_speed, time, 0);
  }
}
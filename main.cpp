#include "project_config.h"
#include "pin_setup.h"
#include "motor.h"
#include "time_service.h"
#include "usart6.h"
#include "usart3.h"
#include "soft_i2c.h"
#include "IRLocator.h"
#include "motors.h"
#include "OpenMv.h"
#include "kicker.h"
#include "SPI_1.h"
#include "SPI_2.h"
#include "SPI_3.h"
#include "MPU9250.h"

using namespace time_service;

#define CHANNEL1 1
#define CHANNEL2 2
#define CHANNEL3 3
#define CHANNEL4 4

#define Usart1 01
#define Usart2 02
#define Usart3 03
#define Usart6 04
#define i2c 05
#define read_UP 06
#define write 07
#define spi 9
#define spi1 91
#define spi2 92
#define spi3 93


#define tim1 81
#define tim2 82
#define tim3 83
#define tim4 84
#define tim5 85
#define tim8 88
#define tim9 89
#define tim10 810
#define tim11 811
#define tim12 812
#define tim13 813
#define tim14 814

int main(){
  volatile int dop = 0;
  
	volatile bool change_role = false, dist_state = false, line_data[4] = {0, 0, 0, 0}, out_side_en = false, out_side = false, go_zone = 0, stop = 0, gate_color = 0,
	line_zone[4] = {0, 0, 0, 0}, side_kick = false, gyro_reset_en = false,
  side_kick_en = false, kick_en = false, kick_over_time_en = false,
  block = false, detour_side_ch_en = false, sub_dist = true, motor_move,
  kick_state = 0, dopusk = false, gyro_reset_on, motor_stop_on = false;
	
	volatile int state = 0, dist, long_dist, seeker_state = 0, role = 0, old_role = 0, 
		new_role = 0, u = 0, priority_state = 0;
	
	volatile int gyro = 0, seeker = 0, x0_gyro = 0,
		 locator_offset = 0, sign = 1, last_robot_x = 0, 
		 robot_x = 0, last_robot_y = 0, robot_y = 0, forward_angle = 0,
		 backward_angle = 0, goal_atan = 0, state_ob = 0, gate_dist = 0, ball_angle,
     round_dist = 11, detour_side = 0, detour_side_ch = 0;
	
	volatile uint32_t sub = 0, kick_start_tim = 0, kick_over_tim = 0,
	 kick_tim = 0, gyro_reset_tim = 0, change_role_tim = 0, time = 0,
	 dist_state_tim = 0, d_timer = 1000, angle_control_tim = 0, detour_side_ch_tim;
	
	volatile int e_gyro = 0, move_seeker = 0, u_angle = 0, e_gate = 0, k_gate = 0, p_gate = 0, x0_back_gate = 180, x0_forward_gate = 0, 
		move_gate = 0, speed_seeker = 3095, start_gyro, speed_angle = 1000, 
    ring = 0, move_x = 0, move_y = 0, e_old, center_error, center_er_x,
    center_error_y, forward_distance, backward_distance, x0_backward_distance = 20, backward_distance_e;
	
	volatile float e_seeker = 0, p_seeker = 0, i_seeker = 0, pi_seeker = 0, k_seeker = 1,
		ki_seeker, x0_seeker = 0,p_angle, d_angle, i_angle = 0, kp_angle = -28, kd_angle = -0, 
    ki_angle = -0.007, actual_dist = 0, last_dist = 0, 
		robot_pos_change = 1,
    angle_change_const = (180 / 3), move_angle = 0;
	volatile char back_gate;
  volatile uint8_t gates_state = 1;
	volatile double test;
  
  
	Motor m1('E', 5, tim9, CHANNEL1, 'E', 6, tim9, CHANNEL2);				
	Motor m2('B', 5, tim3, CHANNEL2, 'B', 3, tim2, CHANNEL2);
	Motor m3('A', 0, tim2, CHANNEL1, 'A', 1, tim5, CHANNEL2);
	Motor m4('E', 11, tim1, CHANNEL2, 'E', 13, tim1, CHANNEL3);
    
	pin usart6_tx('C', 6, Usart6);		
	pin usart6_rx('C', 7,  Usart6);		
	pin usart3_tx('B', 10, Usart3);		
	pin usart3_rx('B', 11, Usart3);		
	pin i2c3_scl('A', 8, i2c);		
	pin i2c3_sda('C', 9, i2c);		
	pin motors_move('B', 1, read_UP);
	pin gyro_reset('B', 0, read_UP);	
  pin cap_charge('B', 9, write);
  pin cap_discharge('E', 0, write);
  
  soft_i2c ir(i2c3_scl, i2c3_sda);
	IRlocator locator360(ir, 0x0E);
	usart6::usart6Init();
	usart3::usart3Init();
	camera camera(usart3_tx, usart3_rx);
  MPU9250_ mpu(ir);
	
	motors motors(m1, m2, m3, m4);
  kicker kicker(cap_charge, cap_discharge);
	time_service::init();
  
  role = 0;
  speed_seeker = 0;
  motor_move = true;
//  while(!mpu.setup(0x68));
//  mpu.calibrateGyro(500, 2500);
//  mpu.update();
//  start_gyro = mpu.getRealYaw();
  
  time_service::delay_ms(30000);
  while(usart6::available() < 1);
  start_gyro = usart6::read();
  //dma for dist
  //	Adc adcL(ADC1, 1, 0, ADC_Channel_8, RCC_APB2Periph_ADC1, distL);
  //	adcL.sendMeChannel(ADC_Channel_8);
  //	Dma dmaL(RCC_AHB1Periph_DMA2, adcL);

  //	Adc adcR(ADC1, 1, 0, ADC_Channel_8, RCC_APB2Periph_ADC1, distR);
  //	adcL.sendMeChannel(ADC_Channel_8);
  //	Dma dmaR(RCC_AHB1Periph_DMA2, adcL);//!!!change settings!!! #Frog 
	
	//dist_sen dist_sen(dmaL,dmaR,e_gyro);
  
	while(true)
  {
    time = time_service::getCurTime();
    gyro = (usart6::read() * 2) - start_gyro;
		if(gyro < -180)
      gyro += 360;
    else if(gyro > 180)
      gyro -= 360;
		camera.getData();
    camera.calculate_pos(gyro);
		robot_x = camera.get_x();
		robot_y = camera.get_y();
    forward_angle = camera.get_forward_angle();
    forward_distance = camera.get_forward_distance();
    backward_angle = camera.get_backward_angle();
    backward_distance = camera.get_backward_distance();
    gates_state = camera.get_data_state();

    dist = locator360.getData(0x07);

    long_dist = locator360.getData(0x05);
		if(dist < 11)
		{
			seeker = locator360.getData(0x04) * 5;
			seeker_state = 1;
			if(time - dist_state_tim > 500)
				sub_dist = false;
		}
		else
		{
			sub_dist = true;
			dist_state_tim = time;
			seeker = locator360.getData(0x06) * 5;
			seeker_state = 0;
		}
    if(long_dist < 27 && (seeker > 345 || seeker < 15) && forward_distance < 50)
      dist_state = true;
    else
      dist_state = false;

    last_dist = long_dist;
    
    block = false;
    
    ball_angle = seeker + gyro;
		if(ball_angle < -180)
      ball_angle += 360;
    if(seeker > 180)
      ball_angle -= 360;
//    if(seeker < -180)
//      ball_angle = seeker + 360;
//    if(seeker > 180)
//      ball_angle = seeker - 360;
    
    motor_move = !(motors_move.getGPIOx()->IDR & motors_move.getPinNumber());
    gyro_reset_en = !(gyro_reset.getGPIOx()->IDR & gyro_reset.getPinNumber());
    if(gyro_reset_en)
    {
      motor_move = 1;
      start_gyro = usart6::read() * 2;
    }
    
    kicker.check(time);
    
		if(role == 0)
		{
      if((old_role == 1 && time - kick_over_tim > 1500)/* || seeker > 270 || seeker < 90 */)
      {
        role = 1;
        old_role = 0;
      }
      speed_seeker = 3100;
      e_seeker = seeker;
      if(e_seeker < -180)
				 e_seeker += 360;
			else if(e_seeker > 180)
				 e_seeker -= 360;	
      
      if(seeker > 335 || seeker < 25)
        state = 0;
      else
        state = 1;
      
      if(priority_state != 0)
        state = priority_state - 1;
      
      if(state == 0)
      {
        if(dist_state == 0 || (e_gyro < 5 && (seeker > 350 || seeker < 10)))
          p_seeker = seeker;
        else
          p_seeker = forward_angle;
        
        if(long_dist > 100)
        {
          if(kick_en == false)
            kick_tim = time;
          kick_en = true;
          if(time - kick_tim > 100 && motor_move == 0)
            kicker.keck();	
        }
        else
          kick_en = false;
          
      }
      else
      {
        if(long_dist > 75 || sub_dist == true)
        {
          detour_side = 0;
          if(detour_side == 0)
          {
            if(e_seeker > 0)
            {
              if(robot_x < -40 && ball_angle < 180 && ball_angle > 90)
                p_seeker = seeker - 90;
              else if(robot_x < -30 && (ball_angle > 115 || ball_angle < -115))
                p_seeker = seeker - 90;
              else
                p_seeker = seeker + 90;
            }
            else
            {
              if(robot_x > 40 && ball_angle > -180 && ball_angle < -90)
                p_seeker = seeker + 90;
              else if(robot_x > 30 && (ball_angle > 115 || ball_angle < -115))
                p_seeker = seeker + 90;
              else
                p_seeker = seeker - 90;
            }
          }
          else
          {
            if(detour_side - 1 == 0)
              p_seeker = seeker - 90;
            else
              p_seeker = seeker + 90;
          }
        }
        else
        {
          if(seeker != 0 && seeker != 180)
            p_seeker = seeker + 30 * (abs(double(180 - seeker)) / (180 - seeker));
          else
            p_seeker = seeker;
        }
				
				if(p_seeker < -180)
				 p_seeker += 360;
				else if(p_seeker > 180)
				 p_seeker -= 360;	
      }
      if(gates_state == 8)
      {
        if(robot_x > 0)
          x0_gyro = 340;
        else
          x0_gyro = 20;
      }
      else
      {        
        if(gates_state != 0)
          x0_gyro = forward_angle;
        else
          x0_gyro = 0;
      }
      if(robot_y < 90 || backward_distance < 90)
        x0_gyro = 0;
			if(d_timer != time)
      { 
         if(seeker == 1275)
         {
           ki_angle = -0.001;
           kd_angle = -0;
           kp_angle = -14;
         }
         e_gyro = x0_gyro - gyro;
         if(e_gyro < -180)
            e_gyro += 360;
         else if(e_gyro > 180)
            e_gyro -= 360;	
         p_angle = e_gyro * kp_angle;
          
         if(i_angle <= 300 && i_angle >= -300)
         {
           i_angle += e_gyro * ki_angle;
         }
         else 
         {
           if(i_angle > 300)
            i_angle = 300;
           else
             i_angle = -300;
         }
         
         d_angle = (e_old - e_gyro) * kd_angle;
         
         u_angle = p_angle + i_angle + d_angle;
         
         e_old = e_gyro;
         
         d_timer = time;
      }
      
      if((backward_distance < 51 && gates_state != 2) || robot_y < 51)
      {
        p_seeker = 0 - gyro;
        speed_seeker = 2500;
        //if(robot_y > 23)
        //{
//          if(seeker > 120 || seeker < 240) 
//            p_seeker = 0 - gyro;
//          else
//            p_seeker = seeker;
        if(seeker < 90 || seeker > 270)
        {
          p_seeker = seeker;
          speed_seeker = 2700;
        }
        else if(seeker < 110 || seeker > 250)
        {
          if(robot_y < 41)
          {
            p_seeker = seeker;
            speed_seeker = 2750;
          }
          else
          {
            if(seeker < 180)
              p_seeker = seeker + 20;
            else
              p_seeker = seeker - 20;
            speed_seeker = 2200;
          }
        }
        if(robot_y < 30 || backward_distance < 30)
          p_seeker = 0;
          speed_seeker = 2100;
        //}
      }
      else if(robot_y < 50 && (seeker < 90 || seeker > 270))
        speed_seeker = 2250;

      round_dist = 11;
      if(robot_x < -47/* || (gates_state == 2 && robot_x < -55 && robot_y < 110)*/)
      { 
        if(robot_x > -52 &&  ball_angle < 90 && ball_angle > 0)
          p_seeker = seeker;
        else
        {
          priority_state = 0;
          p_seeker = 90 - gyro;
        }
      }
      
      if(robot_x > 40/* || (gates_state == 2 && robot_x > 55 && robot_y < 110)*/)
      {
        if(robot_x < 40 && ball_angle > -90 && ball_angle < 0)
          p_seeker = seeker;
        else
        {
          priority_state = 0;
          p_seeker = 270 - gyro;
        }
      }
      
      if(abs(double(robot_x)) > 30)
      {
        if(robot_x > 0 && ball_angle > 0 && ball_angle < 180)
          speed_seeker = 2100;
        else if(robot_x < 0 && seeker < 350 && seeker > 180)
          speed_seeker = 2100;
        else
          speed_seeker = 3700;
      }
      else
      {
        if(backward_distance > 46)
        {
          if(abs(double(p_seeker)) < 11 && robot_y < 170 && abs(double(e_gyro)) < 20)
            speed_seeker = 3900;
          else
          {
            if(abs(double(p_seeker)) < 11 && robot_y < 170)
              speed_seeker = 3850;
            if(seeker < 90 && seeker > 270)
              speed_seeker = 3850;
            else if(long_dist > 70)
              speed_seeker = 3600;
            else
              speed_seeker = 3650;
          }
        }
      }
      if(robot_y < 100 && seeker < 260 && seeker > 100)
        speed_seeker = 2000;
      if((abs(double(robot_x)) < 20 && robot_y > 169) || (abs(double(robot_x)) > 20 && forward_distance < 25) || robot_y > 185 || (forward_angle > 65 && forward_angle < 295))
        p_seeker = 180 - gyro;
      
      if(gates_state == 8 && robot_x > 0)
        p_seeker = 270 - gyro;
      if(gates_state == 8 && robot_x < 0)
        p_seeker = 90 - gyro;
      
      if(p_seeker < -180)
				 p_seeker += 360;
			else if(p_seeker > 180)
				 p_seeker -= 360;	
      
      if((forward_distance > 40 && robot_y > 100) && abs(double(gyro)) < 10)
        speed_seeker = 3800;
      else if(forward_distance > 25 && robot_y > 100 && abs(double(p_seeker)) < 10)
        speed_seeker = 3200;
      
      if(robot_y < 30)
        p_seeker = 0;
      
      if(angle_control_tim != time)
      {
        if(abs(double(move_angle - p_seeker)) > angle_change_const)
          move_angle += angle_change_const * ((move_angle - p_seeker) / abs(double(move_angle - p_seeker))) * -1;        
        else
          move_angle = p_seeker;
        angle_control_tim = time;
      }
      
      if(block)
        motors.stopRobot();
      else if(motor_move)
        motors.moveRobot(0, 0, 0, 0);
      else if(seeker == 1275)
        motors.moveRobot(0, 1500, 0, u_angle);
      else
        motors.moveRobot(speed_seeker, 1500, int(move_angle), u_angle);
		}
    else if(role == 1)
    {
      speed_seeker = 2750;
      x0_backward_distance = 50;
      
      if(long_dist < 10 || seeker == 1275)
        e_seeker = 0;
      
      if(abs(double(robot_x)) < 30)
      {
        x0_backward_distance = 32;
        backward_distance = robot_y;
      }
      else
      {
        if(robot_x < 0)
          x0_backward_distance = 41;
        else 
          x0_backward_distance = 34;
      }
      backward_distance_e = x0_backward_distance - backward_distance;

      if(seeker < -180)
				 seeker += 360;
			else if(seeker > 180)
				 seeker -= 360;      
      
      x0_seeker = 0;
      
      if(x0_seeker < -180)
				 x0_seeker += 360;
			else if(x0_seeker > 180)
				 x0_seeker -= 360;	
      
      e_seeker = seeker - x0_seeker;
      
      if(long_dist < 15 || seeker == 1275)
        e_seeker = 0;
      
      if((e_seeker > 2 && long_dist > 70) || (e_seeker > 5 && long_dist < 70) || ((seeker == 1275 || long_dist < 15) && backward_angle < 170))
      {
        if(abs(double(robot_x)) < 30)
          p_seeker = 90 - int(backward_distance_e * 1.7);   
        else
          p_seeker = backward_angle - 90 - int(backward_distance_e * 1.7);   
        if(long_dist > 75)
        {
          if(e_seeker < 7)
            speed_seeker = 1700;
          else if(e_seeker < 12)
            speed_seeker = 2000;
          else if(e_seeker < 20)
            speed_seeker = 2400;
          else
            speed_seeker = 3900;
        }
        else
        {
          if(e_seeker < 7)
            speed_seeker = 1900;
          else if(e_seeker < 10)
            speed_seeker = 2000;
          else if(e_seeker < 15)
            speed_seeker = 2150;
          else if(e_seeker < 20)
            speed_seeker = 2450;
          else
            speed_seeker = 3000;
        }
        
        if(backward_angle > 218)
        {
          p_seeker = backward_angle + 90 + int(backward_distance_e * 1.4);
          //if(backward_angle > 240)
            //speed_seeker = 1800;
          //else
            speed_seeker = 0;
        }
        
      }
      else if((e_seeker < -2 && long_dist > 70) || (e_seeker < -5 && long_dist < 70) || ((seeker == 1275 || long_dist < 15) && backward_angle > 190))
      {
        if(abs(double(robot_x)) < 40)
          p_seeker = 90 + int(backward_distance_e * 1.7);
        else;
          p_seeker = backward_angle + 90 + int(backward_distance_e * 1.7);
        if(long_dist > 70)
        {
          if(e_seeker > -7)
            speed_seeker = 1700;
          else if(e_seeker > -12)
            speed_seeker = 2000;
          else if(e_seeker > -20)
            speed_seeker = 2400;
          else
            speed_seeker = 3900;
        }
        else
        {
          if(e_seeker > -7)
            speed_seeker = 1900;
          else if(e_seeker > -10)
            speed_seeker = 2000;
          else if(e_seeker > -15)
            speed_seeker = 2150;
          else if(e_seeker > -20)
            speed_seeker = 2450;
          else
            speed_seeker = 3100;
        }
        
        if(backward_angle < 137)
        {
          p_seeker = backward_angle - 90 - int(backward_distance_e * 1.4);
          //if(backward_angle < 120)
            //speed_seeker = 1800;///////
          //else
            speed_seeker = 0;
        }
      }
      else
      {
        speed_seeker = 0;
      }
      
      if(p_seeker < -180)
        p_seeker += 360;
      else if(p_seeker > 180)
        p_seeker -= 360; 
      
      if(backward_distance < x0_backward_distance - 18 && kick_state == 0)
      {
        if(backward_angle < 160 && e_seeker > 5)
          p_seeker = 20;
        else
          p_seeker = 0;
        speed_seeker = 2600;
      }
      
      if((backward_distance > x0_backward_distance + 20 && kick_state == 0) || gates_state == 8)
      {
        if(robot_y > 80)
        {
          p_seeker = backward_angle;
          x0_gyro = 0;
        }
        else
          p_seeker = 180;
        speed_seeker = 2600;
      }
      else
        if(((seeker > 167 && seeker < 183)) && kick_state == 0)
          speed_seeker = 0;
      
      if(seeker > 155 && seeker < 205)
      {
         if(backward_distance > x0_backward_distance + 15)
         {
           if(seeker > 180)
             p_seeker = seeker - 27;
           else
             p_seeker = seeker + 27;
          }
          else
          {
            if(seeker < 190 && seeker > 170)  
              speed_seeker = 0;
          }
       }  
        
      if(p_seeker < -180)
				 p_seeker += 360;
			else if(p_seeker > 180)
				 p_seeker -= 360;	
      if(robot_y > 80)
        x0_gyro = 0;
      else
        x0_gyro = backward_angle + 180;
      
      if(x0_gyro < -180)
				 x0_gyro += 360;
			else if(x0_gyro > 180)
				 x0_gyro -= 360;	
      
      if(d_timer != time)
      {  
         if(seeker == 1275)
         {
           ki_angle = -0.01;
           kd_angle = -0;
           kp_angle = -14;
         }
         else
           kp_angle = -30;
         e_gyro = x0_gyro - gyro;
         if(e_gyro < -180)
            e_gyro += 360;
         else if(e_gyro > 180)
            e_gyro -= 360;	
         p_angle = e_gyro * kp_angle;
          
         
         
         if(i_angle <= 300 && i_angle >= -300)
         {
           i_angle += e_gyro * ki_angle;
         }
         else 
         {
           if(i_angle > 300)
            i_angle = 300;
           else
             i_angle = -300;
         }
         
         d_angle = (e_old - e_gyro) * kd_angle;
         
         u_angle = p_angle + i_angle + d_angle;
         
         e_old = e_gyro;
         
         d_timer = time;
      }
      
//      if(backward_distance > 70 || robot_y > 80)
//      {
//        if(seeker > 160 && seeker < 200)
//        {
//          if(seeker > 180)
//            p_seeker = seeker - 30;
//          else
//            p_seeker = seeker + 30;
//        }
//        else
//          p_seeker = backward_angle;
//      }
//      else
//        old_role = 1;
      if(robot_y < 12 || gates_state == 8)
      {
        p_seeker = 0 - gyro;
        speed_seeker = 2100;
      }
      
      if(long_dist > 50 && abs(double(gyro)) < 35 && (backward_angle > 150 && backward_angle < 200) 
          && (seeker > 340 || seeker < 20))
        {
          if(!kick_en)
            kick_start_tim = time;
          kick_en = true;
          if(time - kick_start_tim > 1200)
          {
            kick_state = 1;
            kick_start_tim = time;
          }
        }
        
      if(kick_state == 1)
      {
        p_seeker = seeker;
        speed_seeker = 3900;
      }
      
      if(time - kick_start_tim > 1700 || abs(double(robot_x)) > 23 || robot_y > 150 || (seeker < 325 && seeker > 15))
      {
        kick_state = 0;
        kick_over_tim = time;
      }
      
      if(angle_control_tim != time)
      {
        if(abs(double(move_angle - p_seeker)) > angle_change_const)
          move_angle += angle_change_const * ((move_angle - p_seeker) / abs(double(move_angle - p_seeker))) * -1;        
        else
          move_angle = p_seeker;
        angle_control_tim = time;
      }
      if(block)
        motors.stopRobot();
      else if(motor_move)
        motors.moveRobot(0, 0, 0, 0);
      else if(seeker == 1276)
        motors.moveRobot(0, 1500, 0, u_angle);
      else
        motors.moveRobot(speed_seeker, 1500, int(move_angle), u_angle);

    }
  }
}

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
#include "MPU9250.h"
#include "SPI_1.h"
#include "SPI_2.h"
#include "SPI_3.h"
#include "ball_detour.h"
#include "usart2.h"
#include "hc-05.h"
//#include "SSD1306.h"
#include "robot_math.h"
#include "dribler.h"
#include "adc.h"
#include "dma.h"
#include "voltmeter.h"
#include "IMU_SPI.h"
#include "TSSP.h"

#define CHANNEL1 1
#define CHANNEL2 2
#define CHANNEL3 3
#define CHANNEL4 4


int main(){
  time_service::init();
  time_service::startTime();
  
  bool role, old_role, motor_move, seeker_state, dist_state, sub_dist, 
    gyro_reset_en, see_ball, block_motor;
  
  volatile float kp_angle = -28, ki_angle = -0.007, kd_angle = 0,
    angle_change_const = 180/3, seeker, voltage = 12.6;
  
  uint8_t gates_state, dist, long_dist;
  
  volatile int16_t robot_x, robot_y,gyro, x0_gyro, e_gyro, e_old, start_gyro;
  
  volatile int16_t ball_angle, robot_angle, forward_angle, forward_distance, backward_angle, 
    backward_distance, p_angle, i_angle, d_angle, u_angle;
  
  volatile uint32_t time, dist_state_tim,kick_over_tim, d_timer, 
  angle_control_tim, out_kick_timer, end_out_kick_timer;
  //attacker
  volatile uint16_t speed_seeker, speed_angle;
  
  //defender
  bool out_kick_en = false;
  volatile uint8_t defender_state = 0;
  volatile int32_t x_result, y_result, x_ball, y_ball, x_error, y_error, 
    kp_error, error_angle, defence_angle, error_speed, result_angle, 
    move_speed, move_angle, ball_dist_formulka, gates_x0;
  
  volatile double seeker_error;
  
  float k_base_speed, k_angle_speed, k_dist_speed;
  
  pin power('E', 12, write_);
  power.resetBit();
	
	Motor m1('D', 12, tim4, CHANNEL1, 'D', 13, tim4, CHANNEL2);				
	Motor m2('E', 13, tim1, CHANNEL3, 'E', 14, tim1, CHANNEL4);
	Motor m3('B', 3, tim2, CHANNEL2, 'B', 4, tim3, CHANNEL1);
	Motor m4('D', 14, tim4, CHANNEL3, 'D', 15, tim4, CHANNEL4);
    
	//m1.motorMove(2000);
  
  pin spi2_sck('B', 13, spi2);
  pin spi2_mosi('B', 15, spi2);
  pin spi2_miso('B', 14, spi2);
  pin spi2_ck('B', 12, write_DOWN);
  
  pin spi3_mosi('C', 12, spi3);
  pin spi3_sck('C', 10, spi3);
  pin OLED_DC('A', 15, write_UP);
  pin spi3_nss('E', 0, write_UP);
  pin OLED_res('C', 11, write_UP);
  OLED_res.resetBit();
  
	pin usart6_tx('C', 6,  uart6);	
  pin usart6_rx('C', 7,  uart6);
  
	
	pin motors_move('B', 1, read_UP);
	pin gyro_reset('B', 0, read_UP);	
  pin cap_charge('B', 9, write_);
  pin cap_discharge('E', 0, write_);
  pin dribler_control('C', 8, tim3);//tim8!!! prsc = 419
  pin adc_voltage_pin('C', 5, 11);
  
  pin up('B', 5, read_UP);
  pin down('B', 6, read_UP);
  pin enter('B', 7, read_UP);
  
  pin tssp_write_4('D', 8, write_);
  pin tssp_write_3('D', 9, write_);
  pin tssp_write_2('D', 10, write_);
  pin tssp_write_1('D', 11, write_);
  
  pin tssp_left_read('C', 3, adc);
  pin tssp_right_read('C', 2, read_);
  
  
  Adc adc_voltage(ADC2, 1, 15, RCC_APB2Periph_ADC2, adc_voltage_pin);
  adc_voltage.sendMeChannel(15);
  Dma dma_voltage(RCC_AHB1Periph_DMA2, adc_voltage);
  dma_voltage.dmaInit(DMA2_Stream2, DMA_Channel_1, 1);
  dma_voltage.adcInitInDma();
  
  Adc tssp_left_read_adc(ADC1, 1, 14, RCC_APB2Periph_ADC1, tssp_left_read);
  tssp_left_read_adc.sendMeChannel(15);
  Dma tssp_left_read_dma(RCC_AHB1Periph_DMA1, tssp_left_read_adc);
  tssp_left_read_dma.dmaInit(DMA1_Stream0, DMA_Channel_0, 1);
  tssp_left_read_dma.adcInitInDma();
  
  Adc tssp_right_read_adc(ADC2, 1, 14, RCC_APB2Periph_ADC2, tssp_right_read);
  tssp_right_read_adc.sendMeChannel(15);
  Dma tssp_right_read_dma(RCC_AHB1Periph_DMA2, tssp_right_read_adc);
  tssp_right_read_dma.dmaInit(DMA2_Stream3, DMA_Channel_1, 1);
  tssp_right_read_dma.adcInitInDma();
  
  //voltmeter voltmeter(dma_voltage);
  
  pin led1('A', 0, write_);
  pin led2('A', 1, write_);
  pin led3('A', 2, write_);
  
  motors motors(m1, m2, m3, m4);
  
  //SSD1306 display(3, OLED_DC, OLED_res, spi3_nss, spi3_sck, spi3_mosi);
  //display.begin();
  //\display.clear();
  
  //spi2_miso.resetBit();
  //spi2_ck.setBit();
  
  //soft_i2c ir(spi2_sck, spi2_mosi);
  
  //ir.softI2cInit();
  //mpu9250_spi mpu_sensor(spi2_ck);
  //IMU mpu(spi2_ck, mpu_sensor);
  
  //mpu.init();
  //mpu.calibrate(2000);
  //mpu.update();
  //start_gyro = mpu.getRealYaw();
  TSSP ball(digital ,tssp_write_4, tssp_write_3, tssp_write_2, 
  tssp_write_1, tssp_left_read, tssp_right_read, tssp_left_read_dma, 
  tssp_right_read_dma);
  
  while(true)
  {
    ball.get_data();
    move_angle = ball.get_angle();
//    mpu.update();
//    gyro = mpu.getAngle();
//    //display.drawPixel(10, 10, 1);
//    //display.display();
//    time_service::delay_ms(1);
  }
  
  //308
  //dribler_control.setBit();
  //dribler dribler(dribler_control, 8);
  //dribler_control.pwmInit(RCC_APB1ENR_TIM3EN, 159, 2000, 0, CHANNEL3, TIM3, 1);	
  //dribler_control.pwm(150);
  //time_service::delay_ms(5000);
//  while(true)
//  {
//    dribler_control.pwm(165);
//    time_service::delay_ms(2000);
//    dribler_control.pwm(150);
//    time_service::delay_ms(1000);
//  }
  
  kicker kicker(cap_charge, cap_discharge);
  
  role = 1;
  speed_seeker = 0;
  motor_move = true;
  
  time_service::delay_ms(30000);
  while(usart6::available() < 1);
  start_gyro = usart6::read();
  
//  while(true)
//  {
//    time = time_service::getCurTime();
//    gyro = (usart6::read() * 2) - start_gyro;
//		if(gyro < -180)
//      gyro += 360;
//    else if(gyro > 180)
//      gyro -= 360;
//    
//    gyro_reset_en = !(gyro_reset.getGPIOx()->IDR & gyro_reset.getPinNumber());
//    
//		camera.getData();
//    camera.calculate_pos(gyro, gyro_reset_en);
//		robot_x = camera.get_x();
//		robot_y = camera.get_y();
//    forward_angle = camera.get_forward_angle();
//    forward_distance = camera.get_forward_distance();
//    backward_angle = camera.get_backward_angle();
//    backward_distance = camera.get_backward_distance();
//    gates_state = camera.get_data_state();

//    dist = locator360.getData(0x07);

//    long_dist = locator360.getData(0x05);
//    if(long_dist == 0) long_dist = 1;
//		if(dist < 11)
//		{
//			seeker = locator360.getData(0x04) * 5;
//			seeker_state = 1;
//			if(time - dist_state_tim > 500)
//				sub_dist = false;
//		}
//		else
//		{
//			sub_dist = true;
//			dist_state_tim = time;
//			seeker = locator360.getData(0x06) * 5;
//			seeker_state = 0;
//		}
//    
//    if(seeker == 1275) see_ball = false;
//    else see_ball = true;
//    
//    if(seeker < -180)
//      seeker += 360;
//    if(seeker > 180)
//      seeker -= 360;
//    
//    ball_angle = seeker + gyro;
//		if(ball_angle < -180)
//      ball_angle += 360;
//    if(seeker > 180)
//      ball_angle -= 360;
//    
//    block_motor = false;

//    
//    motor_move = !(motors_move.getGPIOx()->IDR & motors_move.getPinNumber());
////    if(gyro_reset_en)
////    {
////      motor_move = 1;
////      start_gyro = usart6::read() * 2;
////    }
//    
//    block_motor = false;
//      
//    if(motor_move)
//    {
//      kicker.discharge();
//    }
//      kicker.check(time);
//    
//    if(role == 0)
//		{
//      if((old_role == 1 && time - kick_over_tim > 1500)/* || seeker > 270 || seeker < 90 */)
//      {
//        role = 1;
//        old_role = 0;
//      }
//      
//      if(gates_state == 8)
//      {
//        if(robot_x > 0)
//          x0_gyro = -20;
//        else
//          x0_gyro = 20;
//      }
//      else
//      {        
//        if(gates_state != 0)
//          x0_gyro = forward_angle;
//        else
//          x0_gyro = 0;
//      }
//       move_angle = seeker + exponential_detour(seeker, long_dist, 0.055, 0.67, 0.017, 6.4);
//			if(d_timer != time)
//      { 
//         if(seeker == 1275)
//         {
//           ki_angle = -0.001;
//           kd_angle = -0;
//           kp_angle = -14;
//         }
//         e_gyro = x0_gyro - gyro;
//         if(e_gyro < -180)
//            e_gyro += 360;
//         else if(e_gyro > 180)
//            e_gyro -= 360;	
//         p_angle = e_gyro * kp_angle;
//          
//         if(i_angle <= 300 && i_angle >= -300)
//         {
//           i_angle += e_gyro * ki_angle;
//         }
//         else 
//         {
//           if(i_angle > 300)
//            i_angle = 300;
//           else
//             i_angle = -300;
//         }
//         
//         d_angle = (e_old - e_gyro) * kd_angle;
//         
//         u_angle = p_angle + i_angle + d_angle;
//         
//         e_old = e_gyro;
//         
//         d_timer = time;
//      }
//      
//      if((backward_distance < 51 && gates_state != 2) || robot_y < 51)
//      {
//        move_angle = 0 - gyro;
//        speed_seeker = 2500;

//        if(abs(double(seeker)) < 90)
//        {
//          move_angle = seeker;
//          speed_seeker = 2700;
//        }
//        else if(abs(double(seeker)) < 110)
//        {
//          if(robot_y < 41)
//          {
//            move_angle = seeker;
//            speed_seeker = 2750;
//          }
//          else
//          {
//            if(seeker > 0)
//              move_angle = seeker + 20;
//            else
//              move_angle = seeker - 20;
//            speed_seeker = 2400;
//          }
//        }
//        if(robot_y < 30 || backward_distance < 30)
//          move_angle = 0;
//          speed_seeker = 2100;
//      }

//      if(robot_x < -47)
//      { 
//        if(robot_x > -52 &&  ball_angle < 90 && ball_angle > 0)
//          move_angle = seeker;
//        else
//        {
//          move_angle = 90 - gyro;
//        }
//      }
//      
//      if(robot_x > 40)
//      {
//        if(robot_x < 40 && ball_angle > -90 && ball_angle < 0)
//          move_angle = seeker;
//        else
//        {
//          move_angle = 270 - gyro;
//        }
//      }
//      
//      if(abs(double(move_angle)) < 11 && abs(double(e_gyro)) < 25)
//            speed_seeker = 3900;
//      
//      if(dist < 9)
//        if(abs(double(move_angle)) < 135 || sub_dist == false)
//          speed_seeker = 3900;
//      
//      if(abs(double(robot_x)) > 30)
//      {
//        if(robot_x > 0 && ball_angle > 0 && ball_angle < 180)
//          speed_seeker = 2100;
//        else if(robot_x < 0 && ball_angle < 0 && ball_angle > -180)
//          speed_seeker = 2100;
//        else
//          speed_seeker = 3700;
//      }
//      if(robot_y < 100 && seeker < 260 && seeker > 100)
//        speed_seeker = 2000;
//      if((abs(double(robot_x)) < 20 && robot_y > 169) || (abs(double(robot_x)) > 20 && forward_distance < 25) || robot_y > 185 || (forward_angle > 65 && forward_angle < 295))
//        move_angle = 180 - gyro;
//      
//      if(gates_state == 8 && robot_x > 0)
//        move_angle = 270 - gyro;
//      if(gates_state == 8 && robot_x < 0)
//        move_angle = 90 - gyro;
//      
//      if(robot_y < 30)
//        move_angle = 0;
//      
//      if(motor_move)
//        motors.moveRobot(0, 0, 0, 0);
//      else if(seeker == 1275)
//        motors.moveRobot(0, 1500, 0, u_angle);
//      else
//        motors.moveRobot(speed_seeker, 1500, int(move_angle), u_angle);
//		}
//    else if(role == 1)
//    {
//      static const uint8_t R0 = 35;
//      static const int linear_error_x[2] = {-17, 17};
//      static const int max_degree[2][2] = {{-130, -110},
//                                           {130, 110}};
//      static int ball_norm;
//      static const double k1_defender = 1350, k2_defender = 0.64, k3_defender = 0.21;
//      
//      kp_error = 300;
//      
//                                           
//      if(backward_distance > 90) defender_state = 1;
//      else 
//      {
//        if(long_dist > 50 && abs(double(x0_gyro)) < 45)
//        {
//          if(out_kick_en == false)
//            out_kick_timer = time;
//          out_kick_en = true;
//          if(time - out_kick_timer > 1700)
//          {
//            if(defender_state != 2)
//              end_out_kick_timer = time;
//            defender_state = 2;
//          }
//        }
//        else
//        {
//          defender_state = 0;
//          out_kick_en = false;
//        }
//      }
//      
//      if(defender_state == 0)
//      {
//        x0_gyro = backward_angle + 180;

//        ball_norm = (gyro - x0_gyro) + seeker;
//        
//        if(ball_norm < -180)
//          ball_norm += 360;
//        if(ball_norm > 180)
//          ball_norm -= 360;
//        
//        if(x0_gyro < -180)
//          x0_gyro += 360;
//        if(x0_gyro > 180)
//          x0_gyro -= 360;
//        //motor_move = false;
//        if(see_ball == false)
//        {
//          seeker = backward_angle + 180;
//          
//          seeker *= -1;
//          long_dist = 50;//!!!
//        }
//        
//        seeker_error = seeker / 90;
//        if(seeker_error > 1) seeker_error = 1;
//        else if(seeker_error < -1) seeker_error = -1;
//        
//        ball_dist_formulka = long_dist;
//        if(ball_dist_formulka > 90) ball_dist_formulka = 90;
//        else if(ball_dist_formulka < 10) ball_dist_formulka = 10;
//        
//        speed_seeker = k1_defender * pow(abs(double(seeker_error)), k2_defender) * pow(ball_dist_formulka, k3_defender);
//        
//        if(speed_seeker > 3000) speed_seeker = 3000;
//        
//        if((backward_angle > max_degree[0][0] && backward_angle < 0) || (backward_angle < max_degree[1][0] && backward_angle > 0))
//        {
//         
//         if(robot_x > 0) move_angle = -45;
//         else move_angle = 45;
//         speed_seeker = 2000;
//         
//        }
//        else if((backward_angle > max_degree[0][1] && backward_angle < 0) || (backward_angle < max_degree[1][1] && backward_angle > 0))
//        {
//          if(x0_gyro > 0 && defence_angle > 0)
//            speed_seeker = 0;
//          else if(x0_gyro < 0 && defence_angle < 0)
//            speed_seeker = 0;
//        }
//        else
//        {
//          if(seeker >= x0_gyro) defence_angle = backward_angle - 90;
//          else defence_angle = backward_angle + 90;
//          
//          if(defence_angle < -180)
//            defence_angle += 360;
//          if(defence_angle > 180)
//            defence_angle -= 360;     
//        }
//                
//        if(robot_x > linear_error_x[1] || robot_x < linear_error_x[0])
//        {
//          if(R0 > backward_distance) error_angle = backward_angle + 180;
//          else error_angle = backward_angle;

//          if(error_angle < -180)
//            error_angle += 360;
//          if(error_angle > 180)
//            error_angle -= 360;
//          
//          y_error = R0 - backward_distance;
//          
//        }
//        else 
//        {  
//          y_error = R0 - robot_y;
//          if(y_error > 0) error_angle = 0;
//          else error_angle = 180;
//        }
//        
//        error_speed = abs(double(y_error)) * kp_error; 
//        
//        x_ball = int(speed_seeker * sin(double(defence_angle / 57.3)));
//        y_ball = int(speed_seeker * cos(double(defence_angle / 57.3)));
//        
//        x_error = int(error_speed * sin(double(error_angle / 57.3)));
//        y_error = int(error_speed * cos(double(error_angle / 57.3)));
//        
//        x_result = x_ball + x_error;
//        y_result = y_ball + y_error;
//        
//        if(y_result == 0) move_angle = defence_angle;
//        else move_angle = atan2(double(x_result), double(y_result)) * 57.3;
//        
//        move_speed = sqrt(pow(double(x_result), 2) + pow(double(y_result), 2));
//          
//        if(robot_y < 15)
//          move_angle = 0;
//        
//        move_angle -= gyro;
//        
//      }
//      
//      if(defender_state == 1)
//      {
//        if(gates_state != 2)
//        {
//          x0_gyro = backward_angle + 180;
//          
//          if(x0_gyro < -180)
//            x0_gyro += 360;
//          else if(x0_gyro > 180)
//            x0_gyro -= 360;	
//        }
//        else
//          x0_gyro = 0;
//        
//        if(abs(double(seeker)) < 35)
//        {
//          if(robot_x > 20)
//            move_angle = -145;
//          else if(robot_x < -20)
//            move_angle = 145;
//          else
//            move_angle = 180;
//        }
//        else
//        {
//          move_angle = seeker + exponential_detour(seeker, long_dist, 0.055, 0.67, 0.017, 6.4);
//        }
//      }
//      
//      if(defender_state == 2)
//      {
//        if(abs(double(seeker)) < 30 && time - end_out_kick_timer < 4000)
//        {
//          if(long_dist > 80 && abs(double(forward_angle)) < 20)
//          {
//            move_angle = seeker + exponential_detour(seeker, long_dist, 0.055, 0.67, 0.017, 6.4);
//            x0_gyro = forward_angle;
//          }
//          else
//          {
//            move_angle = seeker;
//          }
//          move_speed = 3900;
//        }
//        else 
//        {
//          out_kick_en = false;
//        }
//      }
//      
//      if(d_timer != time)
//      {  
//         if(seeker == 1275)
//         {
//           ki_angle = -0.01;
//           kd_angle = -0;
//           kp_angle = -14;
//         }
//         else
//           kp_angle = -35;
//         e_gyro = x0_gyro - gyro;
//         if(e_gyro < -180)
//            e_gyro += 360;
//         else if(e_gyro > 180)
//            e_gyro -= 360;	
//         p_angle = e_gyro * kp_angle;
//   
//         
//         if(i_angle <= 300 && i_angle >= -300)
//         {
//           i_angle += e_gyro * ki_angle;
//         }
//         else 
//         {
//           if(i_angle > 300)
//            i_angle = 300;
//           else
//             i_angle = -300;
//         }
//         
//         d_angle = (e_old - e_gyro) * kd_angle;
//         
//         u_angle = p_angle + i_angle + d_angle;
//         
//         e_old = e_gyro;
//         
//         d_timer = time;
//      }
//      
//      if(motor_move)
//        motors.moveRobot(0, 0, 0, 0);
//      else if(block_motor)
//        motors.stopRobot(1500);
//      else
//        motors.moveRobot(move_speed, 1500, int(move_angle), u_angle);

//    }
//  }
}
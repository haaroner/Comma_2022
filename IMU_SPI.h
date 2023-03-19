#ifndef IMU_LIB
#define IMU_LIB

#include "mpu9250_spi.h"

class IMU
{
	public:
    IMU(pin &ss_pin, mpu9250_spi &mpuSensor): _ss_pin(ss_pin), _mpuSensor(mpuSensor)
    {
    }
		void init();
		void turnOn();
		void turnOff();
		void setZeroAngle();
		void calibrate(uint32_t t);
		unsigned int update();
		void updateAnglesFromFIFO();
		double getAngle();
		double imuFloatValue, angleChange;
		long long int imuFloatTime;
		double getXa();
		double getYa();
		double getZa();
		double getXg();
		double getYg();
		double getZg();
    int gyro_sel();
    int acc_sel();
	private:
    pin _ss_pin;
    mpu9250_spi _mpuSensor;
		volatile double angle, zeroAngle;
		bool working;
};


void IMU::init()
{
	
	time_service::delay_ms(100);
	_mpuSensor.initIMU(_ss_pin);
	time_service::delay_ms(100);
	
	setZeroAngle(); // IMU calibrated angle estimating
	
	imuFloatTime = time_service::getCurTime();
	imuFloatValue = 0;
  working = true;
}


void IMU::setZeroAngle()
{
	update();
	zeroAngle = angle;
}


unsigned int IMU::update()
{
	updateAnglesFromFIFO();
	double neangle = _mpuSensor.yaw;
	angle = neangle;
	adduction(angle);
	return 0;
}


void IMU::updateAnglesFromFIFO()
{
	_mpuSensor.updateAnglesFromFIFO();
}


void IMU::calibrate(uint32_t t)
{
	_mpuSensor.calibrate(t);
}


double IMU::getAngle()
{
	double a = angle-zeroAngle;
	adduction(a)
	return -a;
}

int IMU::gyro_sel()
{
  return _mpuSensor.read_gyro_fs_sel();
}

int IMU::acc_sel()
{
  return _mpuSensor.read_acc_fs_sel();
}

double IMU::getXa()
{
	return _mpuSensor.getXa();
}

double IMU::getYa()
{
	return _mpuSensor.getYa();
}

double IMU::getZa()
{
	return _mpuSensor.getZa();
}

double IMU::getXg()
{
	return _mpuSensor.getXg();
}

double IMU::getYg()
{
	return _mpuSensor.getYg();
}

double IMU::getZg()
{
	return _mpuSensor.getZg();
}


#endif

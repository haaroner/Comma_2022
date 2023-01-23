#include "project_config.h"
#include "stdint.h"



namespace time_service 
{
	extern volatile uint32_t systemTime;
	uint32_t getCurTime(void);
  uint32_t getCurTime_micros(void);
	void init();
	void stopTime(void);
	void startTime(void);
  void startTime_DOT(void);
	void delay_ms(uint32_t delta);
  void delay_micros(uint32_t delta);
}
//#endif TIME_SERVICE_H

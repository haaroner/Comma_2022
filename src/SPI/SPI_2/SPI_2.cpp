#include "SPI_2.h"
namespace Spi_2
{
   SPI_TypeDef *_SPIx;
   int _spi_mode;
   int _spi_work_mode;
   int _spi_num;
   int _spi_direction;
   int _data_size;
   int _baud_rate;
   int _first_bit;
   int _data;
   int _rx_buffer[8];
   uint16_t _cur_rx_read;
   uint16_t _cur_rx_write;
   uint16_t _buf_size;
  void spi_init(int mode, int work_mode, int direction, int dataSize, int baudRate, int firstBit)
  {  
    _spi_num = spi_2;
    _SPIx = SPI2;
    _spi_mode = mode;
    _spi_work_mode = work_mode;
    _spi_direction = direction;
    _data_size = dataSize;
    _baud_rate = baudRate;
    _first_bit = firstBit;
    _cur_rx_read = 0;
    _cur_rx_write = 0;
    _buf_size = 0;
    for(int i = 0; i < 8; i++)
    {
      _rx_buffer[i] = 0;
    }
    
    //sets clock for spi
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); 
    
    SPI_InitTypeDef _spi;
    
    //sets direction of transmition of data
    if(_spi_direction == fullDuplex) _spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    else if(_spi_direction == two_lines_RxOnly) _spi.SPI_Direction = SPI_Direction_2Lines_RxOnly;
    else if(_spi_direction == RxOnly) _spi.SPI_Direction = SPI_Direction_1Line_Rx;
    else if(_spi_direction == TxOnly) _spi.SPI_Direction = SPI_Direction_1Line_Tx;
    
    if(_spi_mode == master_mode) _spi.SPI_Mode = SPI_Mode_Master; // sets to master mode
    else if(_spi_mode == slave_mode) _spi.SPI_Mode = SPI_Mode_Slave; // sets to slave mode
    
    if(_data_size == d8b) _spi.SPI_DataSize = SPI_DataSize_8b; //sets word length(8b or 16b)
    else if(_data_size == d16b) _spi.SPI_DataSize = SPI_DataSize_16b;
    
    _spi.SPI_CPOL = SPI_CPOL_Low; //sets polarity(0 - low, 1 - high)
    _spi.SPI_CPHA = SPI_CPHA_1Edge; //reading during the first front line
    _spi.SPI_NSS = SPI_NSS_Soft; //sets to soft slave select
    
    //sets the baudrate(speed of transmition)
      if(_baud_rate == b2r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; 
      else if(_baud_rate == b4r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; 
      else if(_baud_rate == b8r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 
      else if(_baud_rate == b16r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; 
      else if(_baud_rate == b32r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
      else if(_baud_rate == b64r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
      else if(_baud_rate == b128r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; 
      else if(_baud_rate == b256r) _spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; 
    
    //sets the start bit(msb or lsb)
    if(_first_bit == msb) _spi.SPI_FirstBit = SPI_FirstBit_MSB;
    else if(_first_bit == lsb) _spi.SPI_FirstBit = SPI_FirstBit_LSB;
    
//    if(_spi_mode == master_mode)
//    {
//      SPI2->CR1 |= SPI_CR1_SSM;
//      _SPIx->CR1 |= SPI_CR1_SSI;
//    }
    
    SPI_Init(SPI2, &_spi); //init spi
    if(_spi_work_mode == interruption)
    {
      SPI_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);//configure interrupt to the empty rx buffer
      NVIC_EnableIRQ(SPI2_IRQn);
    }
    
    SPI_Cmd(SPI2, ENABLE); 
  }

  void spi_deinit()
  {
    SPI_I2S_DeInit(SPI2); //deinit spi
  }

  void send(uint16_t data)
  {
    //waiting when the tx buffer will be empty
    //TXE - buffer is empty BSY - all data have been sent succesfully
    ENTER_CRITICAL_SECTION();
    while(!(SPI2->SR & SPI_SR_TXE));

    SPI2->DR = data;
    EXIT_CRITICAL_SECTION();
  } 

  uint16_t read()
  {
    if(_spi_work_mode == simple)
    {
      ENTER_CRITICAL_SECTION();
      //SPI2->DR = 0; 
      
      //waiting new data in rx buffer
      while(!(SPI2->SR & SPI_SR_RXNE));
      
      EXIT_CRITICAL_SECTION();
      
      return SPI2->DR;
    }
    else if(_spi_work_mode == interruption)
    {
      if(_buf_size > 0)
      {
        _data = _rx_buffer[_cur_rx_read];
        if(_cur_rx_read < 7) _cur_rx_read++;
        else _cur_rx_read = 0;
        
        _buf_size--;
      }
      else
      {
        _data = 0;
      }
      return _data;
    }
  }
  uint8_t available()
  {
    return _buf_size;
  }
}

extern "C"
{
  void SPI2_IRQHandler(void)
  {
    uint16_t it_data;
    if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE))
    {
      //SPI2->DR = 0; 
    
      //waiting new data in rx buffer
      while(!(SPI2->SR & SPI_SR_RXNE));
      
      it_data = SPI2->DR;
      
      Spi_2::_rx_buffer[Spi_2::_cur_rx_write] = it_data;
      
      if(Spi_2::_cur_rx_write == 7) Spi_2::_cur_rx_write = 0;
      else Spi_2::_cur_rx_write++;
      
      if(Spi_2::_buf_size < 7) Spi_2::_buf_size++;
    }
  } 
}


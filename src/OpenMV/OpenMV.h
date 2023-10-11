#pragma once
#include "project_config.h"
#include "usart6.h"
#include "pin_setup.h"

#define RAD2DEG	57.2957795130823208767
#define DEG2RAD	0.01745329251994329576

class camera
{
	public:
		camera(pin &tx, pin &rx);
		void getData();
    uint16_t lead_to_degree_borders(int num);
    void calculate_pos(int16_t angle, bool side);
		int16_t get_x();
		int16_t get_y();
    int16_t get_forward_angle();
    int16_t get_backward_angle();
    int16_t get_forward_distance();
    int16_t get_backward_distance();
    uint8_t get_data_state();
    bool is_first_data_received();

	private:
		int8_t sign;
		pin m_rx, m_tx;
		uint8_t camera_data;
		int16_t _data;
		int _yellow_distance;
    int _yellow_angle;
    int _blue_distance;
		int _blue_angle;
    int _front_angle;
    int _front_distance;
    int _backward_distance;
    int _backward_angle;
    int _center_distance;
    int _center_angle;
    int32_t _x, _y;
    int32_t _x_centered, _y_centered;
    int32_t _center_y;
    double _forward_sin, _backward_sin;
    double _forward_x, _backward_x;
    int _dist1, _dist2;
		int16_t old_data;
		int8_t near_gate_color;
		bool color; //1=y 0=b
		bool num;
		bool finaly;
		bool read;
    bool _received;
    bool _first_receive;
    bool _yellow_first_receive, _blue_first_receive;
		uint8_t data[5];
		uint8_t crc8(uint8_t* data, int len);
    uint8_t _state;
    
    uint8_t _length_between_gates;
};
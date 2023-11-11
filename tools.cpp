#include "tools.h"

int lead_to_degree_borders(int _num)
{
  while(_num < -180 || _num > 180)
  {
    if(_num < -180)
        _num += 360;
    else if(_num > 180)
        _num -= 360;
  }
  return _num;
}
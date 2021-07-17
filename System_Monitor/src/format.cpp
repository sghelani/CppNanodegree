#include "format.h"

#include <string>

using std::string;

string Format::ElapsedTime(long seconds) {
  char time[9];
  int hour = seconds / 3600;
  int minute = (seconds % 3600) / 60;
  int second = (seconds % 3600) % 60;
  sprintf(time, "%02d:%02d:%02d", hour, minute, second);
  return string(time);
}
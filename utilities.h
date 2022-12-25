#pragma once
#include <stdint.h>

template<typename T>
inline int ArrayLength(const T & t)
{
  return sizeof(T) / sizeof(t[0]);
}

class Ticker {
  public:
    unsigned long last;

    // Ticker will still sum up small delays,
    // a guaranteed (but with jitter) ticker could use a "next" time, 
    // that is incemented each time it has happened
    bool NewTick(int timeout, unsigned long current) {     
      // alway use differnce to handle overflows:
      long diff = current - last; 
      if ((diff < 0) ||  // should never happen: not called for a long time!
          (diff > timeout)) {
        last = current;
        return true;
      } else {
        return false;
      }
    }

};

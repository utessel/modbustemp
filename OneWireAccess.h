#pragma once

#include "utilities.h"

class OneWireAccess
{
  private:
    Ticker ticker;
    unsigned char pos;
    unsigned char state;

    void ReadAddress(struct OneWireData &);
    void ReadData(struct OneWireData &);
    void Clear(struct OneWireData &);
    void Next();
    void Convert( struct OneWireData & item, const unsigned char * data );

  public:
    void setup();
    void tick();
};

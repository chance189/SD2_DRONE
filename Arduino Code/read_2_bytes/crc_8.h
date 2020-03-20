/*
// The CCITT CRC 8 polynomial is X^8 + X^7 + X^6 + X^4 + X^2 + 1.
// In convert it to binary, with MSB not accounted for, in hex is: 0x07
// This segment of code reverses the polynomial to 0xE0
*/

#define POLY 0xE0
 
  class crc_8 {
  private:
  unsigned int data;
  unsigned int crc;
 
  public:

  crc_8() : crc ( 0XFF)
  {
   
  }
 
  void reset()
  {
        crc = 0XFF;
  }

  void next (char dval)
  {
      int i;
      for (i = 0, data = (unsigned int)0xff & dval; i < 8; i++, data >>= 1) {
        if ((crc & 0x01) ^ (data & 0x01))
             {  crc = (crc >> 1) ^ POLY;}
        else
             {  crc >>= 1;}
      }   
  }
 
  // taking copies of variables so this can be called validly more than once
  short get_crc() {
       return crc;
   }
  }
;
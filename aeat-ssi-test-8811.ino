#include "AEAT8811.h"

#define BAUDRATE        115200

/*
#ifdef ESP32
#define MSEL    26
#define M0       5 
#define M1      23
#define M2      13
#define M3      19
#else
#define MSEL     0 // NO MSEL
#define M0      10
#define M1      11
#define M2      18
#define M3      12
#endif
*/

unsigned int reg;
AEAT8811 aeat;

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(uint8_t byte)
{
//    char buf[17];
    Serial.printf("%s-%s ", bit_rep[(byte >> 4)&0x0F], bit_rep[byte & 0x0F]);
//    return buf;
}



void setup() {
//  Serial.printf("SCLK=%d\n",SCLK);
  Serial.begin(BAUDRATE);
//  pinMode(4,  OUTPUT);
//  digitalWrite(4,  LOW);

//  aeat.read_reg4();
//  aeat.read_reg5();
//  aeat.read_reg6();
//  aeat.spi_write(6,0);
  aeat.spi_write(2, 0);
  aeat.spi_write(3, 0);
  aeat.write_res(2); //16-bits
  aeat.print_registers();
//  unsigned int data;
//  for (int i=0; i<=10; i++){
//    delay(100);
//    data = aeat.ssi_read();
//    Serial.printf("SSI HW   0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
//                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 18-бітного числа
//                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
//  }
//  
//  while (Serial.available() == 0) {}     //wait for data available
//
//  Serial.printf("DATA   0x%02x 0x%02x  \n",
//                 data&0xff, (data>>8)&0xff);
//  
//  aeat.spi_write(2, data&0xff);
//  aeat.spi_write(3, (data>>8)&0xff);
//  aeat.print_register(3);
////  unsigned long long int data;
//  for (int i=0; i<=10; i++){
//    delay(100);
//    data = aeat.ssi_read();
//    Serial.printf("SSI HW   0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
//                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 18-бітного числа
//                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
//  }
  Serial.printf("sizeof(unsigned int)=%d\n",sizeof(unsigned int));
}

void loop() {

  
  aeat.print_register(3);
  unsigned int data;
  for (int i=0; i<=10; i++){
    delay(300);
    data = aeat.ssi_read();
    Serial.printf("SSI HW   0x%06x=%6d=%7.3f | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 18-бітного числа
                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
  }
  
  Serial.print("Ввод: пустой - пропуск обнуления, '0' - записать ноль в регистрі, '1' - сохранить текущую позицию\n");
  while (Serial.available() == 0) {}    //wait for data available

//  int r = Serial.read()-'0';
  int r = Serial.read();
  while (Serial.available() != 0) {Serial.read();}     //wait for data available
  if (r==10) { // ''
    Serial.printf("Skip zeroizing\n",r);
  }
  else if (r=='0') {
    Serial.printf("Zeroizing registers\n",data);
    aeat.spi_write(2, 0);
    aeat.spi_write(3, 0);
  }
  else {
    aeat.set_zero(data);
  }


  
//  Serial.printf("DATA   0x%02x 0x%02x  \n", data&0xff, (data>>8)&0xff);
  
//  aeat.spi_write(2, data&0xff);
//  aeat.spi_write(3, (data>>8)&0xff);
//  aeat.print_register(3);
////  unsigned long long int data;
//  for (int i=0; i<=10; i++){
//    delay(100);
//    data = aeat.ssi_read();
//    Serial.printf("SSI HW   0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
//                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 16-бітного числа
//                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
//  }
/*  Serial.print("Enter resolution (0..3):");
  while (Serial.available() == 0) {}     //wait for data available
  int r = Serial.read()-'0';
  if (r>3 or r<0) { return; }
  Serial.printf("Setting resolution to %d\n",r);
  
  aeat.write_res(r);
  aeat.print_registers();
  for (int i=0; i<=50; i++){
//    aeat.print_register(6);

    delay(100);

    unsigned long long int data=0;
    data = aeat.ssi_read();
    Serial.printf("SSI HW   0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 18-бітного числа
                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
  }
*/
  
/*  return;
  unsigned long long int res=0;
  unsigned long long int data=0;
//  data = aeat.read_enc(12);
//  Serial.printf("SSI 0x%04lx=%6lld=%7.3Lf \n",
//                 data, data, double(data)*360.0/65536.0);
  
//  aeat.setup_ssi3();
  data = aeat.ssi_read(12);
  Serial.printf("SSI HW   0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 18-бітного числа
                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);

  aeat.print_registers();
//                 
//  aeat.init_pin_ssi();
//  data = aeat.ssi_read_pins(12);
//  Serial.printf("SSI PINS 0x%04lx=%6lld=%7.3Lf | rdy=%d mhi=%d mlo=%d par=%d err=%d \n",
//                 data, data, double(data)*360.0/65536.0, // результат вже приведено до 16-бітного числа
//                 aeat.rdy,aeat.mhi,aeat.mlo,aeat.par,aeat.error_parity);
  delay(100);
  */
}

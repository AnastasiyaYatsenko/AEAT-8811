#include "AEAT8811.h"

AEAT8811::AEAT8811(){
  read_reg4();
  read_reg5();
  read_reg6();
}

//unsigned long int AEAT8811::read_enc(unsigned int bits){
//  return ssi_read_pins(bits);
//}

void AEAT8811::setup_ssi3(uint8_t M0_T, uint8_t NSL_T, uint8_t SCLK_T, uint8_t DO_T, uint8_t MSEL_T) {
  M0   = M0_T;
  NSL  = NSL_T;
  DO   = DO_T;
  SCLK = SCLK_T;
  MSEL = MSEL_T;
  
  SPI.end(); // Так треба після перемикання режимів! Інакше дані спотворюються
//  if (MSEL != 0){
    pinMode(MSEL,  OUTPUT);
    digitalWrite(MSEL,  HIGH); // -> not SPI4 mode
//  }

//  if (mode!=_AEAT_SPI4 && mode!=_AEAT_SPI3 && mode!=_AEAT_SSI3 && mode!=_AEAT_SSI2) {
//  pinMode(NSL,  OUTPUT);
//  pinMode(SCLK, OUTPUT);
  pinMode(DO,   INPUT);

  // https://doc.arduino.ua/ru/prog/SPI
    SPI.setClockDivider(SPI_CLOCK_DIV64);
    SPI.setBitOrder(MSBFIRST);
//    SPI.setDataMode(SPI_MODE1); // CPOL=0, CPHA=1
    SPI.setDataMode(SPI_MODE2); // ??? не допомогло
    SPI.begin(); // внутрішня комутація зовнішніх пінів на mosi/miso/sck
    SPI.setDataMode(SPI_MODE2);
//  }

//  if (mode!=_AEAT_SSI3 && mode!=_AEAT_SSI2) {
//    spiSSDisable(SPI.bus());
//    spiDetachSS(SPI.bus(), M0); // комутуємо зовнішній пін на gpio
//    pinMode(M0,   OUTPUT);
//    digitalWrite(M0, HIGH); // -> spi3/ssi mode

    spiDetachMOSI(SPI.bus(), NSL); // комутуємо зовнішній пін на gpio
    pinMode(NSL, OUTPUT);
    digitalWrite(NSL, HIGH);


//  }
  mode = _AEAT_SSI3;
}

void AEAT8811::setup_spi3(uint8_t M0_T, uint8_t M1_T, uint8_t M2_T, uint8_t M3_T, uint8_t MSEL_T) {
  CS   = M0_T; // не використовується, можна видаляти як параметр
  MOSI = M1_T;
  SCLK = M2_T;
  MISO = M3_T;
  MSEL = MSEL_T;
  
  SPI.end(); // Так треба після перемикання режимів! Інакше дані спотворюються
  pinMode(MSEL,  OUTPUT);
  digitalWrite(MSEL,  HIGH); // -> SPI4 mode

//  if (mode !=_AEAT_SPI4 && mode!=_AEAT_SPI3 && mode!=_AEAT_SSI3 && mode!=_AEAT_SSI2) {
  // https://doc.arduino.ua/ru/prog/SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3); // CPOL=1, CPHA=1
//  }
/*
  if (mode==_AEAT_SSI3) {
    spiAttachSS(SPI.bus(), 0, CS); // комутуємо зовнішний пін на SPI.CS
    spiSSEnable(SPI.bus());

    spiAttachMOSI(SPI.bus(), MOSI); // комутуємо зовнішній пін на SPI.MOSI
  }
*/
  mode = _AEAT_SPI3;
}

unsigned int AEAT8811::parity(unsigned int n) {
   unsigned int b;
   b = n ^ (n >> 1);
   b = b ^ (b >> 2);
   b = b ^ (b >> 4);
   b = b ^ (b >> 8);
   b = b ^ (b >> 16);
   return b & 1;
}

unsigned long int AEAT8811::spi_transfer(unsigned int reg, unsigned int RW) {
    digitalWrite(MSEL, LOW);
    delayMicroseconds(1);
    SPI.transfer((reg & 0x3f) | RW); // передаємо: старші 2 біти - флаг R/W, молодші 6 бітів - адреса регістру. Енкодер тут нічого не повертає, тому ігноруємо.
    unsigned int data  = SPI.transfer(0) & 0xff; // після цього тільки вичитуємо дані з енкодера
    delayMicroseconds(1);
    digitalWrite(MSEL, HIGH);
    delayMicroseconds(1);
    return (data);
}

// void SPIClass::transferBits(uint32_t data, uint32_t * out, uint8_t bits);

unsigned long int AEAT8811::ssi_read() {
// у цьому режимі службові прапорці йдуть ЗА числом, тому для отримання коректних прапорців треба задавати реальну бітову точність 
    if (mode != _AEAT_SSI3) {
      setup_ssi3();
    }
    
    unsigned long long int res=0;
    uint32_t buffer=0;
    uint8_t bits_returned= ceil(float(read_bits+4)/8.0)*8; // у скільки байт буде запакований результат, 2 або 3
//    SPI.setDataMode(SPI_MODE2); // ??? не допомогло
    digitalWrite(NSL, LOW);
    delayMicroseconds(1);
//    digitalWrite(4,  HIGH); 
//  виявилось, що esp32 може робити транзакції (кількість sclk) з довільним числом бітів <=32
    SPI.transferBits(0x0, &buffer, read_bits+4); // 16 бітів позиції+4 службових
//    unsigned int high = SPI.transfer(0xff);
//    unsigned int mid  = SPI.transfer(0xff);
//    unsigned int low  = SPI.transfer(0xff);
    digitalWrite(NSL, HIGH);
    delayMicroseconds(1);
//    SPI.end(); // 50 us
//    digitalWrite(4,  LOW);
//    Serial.print("SPI mode: ");
//    Serial.println(spiGetDataMode(SPI.bus()));
//    buffer = (msb<<24)|buffer;
//    raw_data = buffer >> (24-4-bits); // це така фіча - замовляємо 17..22 біти, а повертається число у 24х бітах, "знизу" доклеєне нулями. Їх потрібно відсікти
//    unsigned int high = SPI.transfer(0xff); // стара версія з побайтовим вичитуванням
    
    raw_data = buffer >> (bits_returned-4-read_bits); // це така фіча - замовляємо 17..22 біти, а повертається число у 24х бітах, "знизу" доклеєне нулями. Їх потрібно відсікти

//    delayMicroseconds(1);
//    digitalWrite(NSL, HIGH);
//    delayMicroseconds(1);
//    digitalWrite(4,  LOW);
    
    error_parity = parity(raw_data); // рахуємо парність разом з переданим бітом парності, результат повинен тотожно дорівнювати нулю. Якщо одиниця - значить був збій парності.
    par = raw_data&1;
    mlo = (raw_data&2)>>1;
    mhi = (raw_data&4)>>2;
    rdy = (raw_data&8)>>3;
    res = raw_data>>4;
// для зручності подальших розрахунків приводимо результат до 16-бітового числа, незалежно від реальної точності датчика
      if (read_bits<16) 
      res = res<<(16-read_bits);
    return res;
}

unsigned long int AEAT8811::spi_read(unsigned int reg) {
  if (mode != _AEAT_SPI3) {
     setup_spi3();
  }
    digitalWrite(MSEL, LOW);
    delayMicroseconds(1);
    SPI.transfer((reg & 0x3f) | READ); // передаємо: старші 2 біти - флаг R/W, молодші 6 бітів - адреса регістру. Енкодер тут нічого не повертає, тому ігноруємо.
    unsigned int data  = SPI.transfer(0) & 0xff; // після цього тільки вичитуємо дані з енкодера
    delayMicroseconds(1);
    digitalWrite(MSEL, HIGH);
    delayMicroseconds(1);
    return (data);
}

// елементарна функція запису, БЕЗ LOCK/UNLOCK
unsigned long int AEAT8811::spi_write_(unsigned int reg, unsigned int data) {
  if (mode != _AEAT_SPI3) {
     setup_spi3();
  }
    digitalWrite(MSEL, LOW);
    delayMicroseconds(1);
    SPI.transfer((reg & 0x3f) | WRITE); // передаємо: старші 2 біти - флаг R/W, молодші 6 бітів - адреса регістру. Енкодер тут нічого не повертає, тому ігноруємо.
    SPI.transfer(data & 0xff); // після цього передаємо дані на енкодер
    delayMicroseconds(1);
    digitalWrite(MSEL, HIGH);
    delayMicroseconds(1);
    return (data);
}

// 
unsigned long int AEAT8811::spi_write(unsigned int reg, unsigned int data) {
  if (mode != _AEAT_SPI3) {
     setup_spi3();
  }
  unsigned int check;
  spi_write_(0x10,0xAB); // UNLOCK
  spi_write_(reg,data);  // write data
  spi_write_(0x10,0x0);  // LOCK
  check = spi_read(reg);
  if (check == data) error_flag = 0;
  else error_flag = 1;
  return !error_flag; // це для виклику функції: if (!spi_write(reg,data)) Serial.println("ALARM!!!");
}

void AEAT8811::set_zero(unsigned int data) {
  Serial.printf("Current position is %06x | Reg[3]=%02x | Reg[2]=%02x\n",data, spi_read(3), spi_read(2));
//  Serial.printf("Regs are %06x | unmasked sum =%06x | masked =%06x\n", regs, data_unmasked, data_masked);
  data = (data + (spi_read(3)<<8) | spi_read(2)) & 0xffff;
  Serial.printf("Setting current position %06x as ZERO\n",data);
  spi_write(2, data&0xff);
  spi_write(3, (data>>8)&0xff);
}

void AEAT8811::print_register(unsigned int reg) {
  unsigned long int data;
  switch (reg){
    case 0:
      data = spi_read(0);
      Serial.printf("0:   0x%02x Customer Reserve 0\n", data);
      break;
    case 1:
      data = spi_read(1);
      Serial.printf("1:   0x%02x Customer Reserve 1\n", data);
      break;
    case 2:
    case 3:
      data = (spi_read(3)<<8) | spi_read(2) ;
      Serial.printf("2-3: 0x%04x Zero Reset\n", data);
      break;
    case 4:
      data = spi_read(4);
      Serial.printf("0x04:0x%02x\n    [7]    UVW Select=%d\n    [4:3]    I-width setting=%d\n    [2:0]    UVW Setting/PWM Setting=%d\n",
                     data,
                     data&0x80?1:0,            (data&0x18)>>3,                  data&0x07);
      break;
    case 5:
      data = spi_read(5);
      Serial.printf("0x05:0x%02x\n    [6:3] CPR Setting 1=0x%x\n    [2:0] Hysteresis Setting=%d\n",
                    data,
                    (data&0x78)>>5,               data&0x07);
      break;
    case 6:
      data = spi_read(6);
      unsigned int res = (data&0x30)>>4;
      read_bits = 0;
      switch (res){ 
        case 0: read_bits=12; break;
        case 1: read_bits=10; break;
        case 2: read_bits=16; break;
        case 3: read_bits=14; break;
      }
      Serial.printf("0x06:0x%02x\n    [7]   Dir=%d\n    [6]   Zero Latency Mode=%d\n    [5:4] Absolute Resolution=%d (%d bits)\n    [3]   SSI_Select=%d\n    [2:0] CPR Setting 2=%d\n",
                            data,
                            data&0x80?1:0,  data&0x40?1:0,                          res, read_bits,                           data&0x08?1:0,         data&0x07);
      break;
  }
}

void AEAT8811::print_registers() {
  unsigned long int data;
  Serial.print("\nRegisters:\n");
  data = spi_read(0);
  Serial.printf("0:   0x%02x Customer Reserve 0\n", data);
  data = spi_read(1);
  Serial.printf("1:   0x%02x Customer Reserve 1\n", data);
  data = (spi_read(3)<<8) | spi_read(2);
  Serial.printf("2-3: 0x%04x Zero Reset\n", data);
  data = spi_read(4);

  Serial.printf("0x04:0x%02x\n    [7]   UVW Select=%d\n    [4:3] I-width setting=%d\n    [2:0] UVW Setting/PWM Setting=%d\n",
                            data,
                            data&0x80?1:0,            (data&0x18)>>3,                  data&0x07);  

  data = spi_read(5);
  Serial.printf("0x05:0x%02x\n    [6:3] CPR Setting 1=0x%x\n    [2:0] Hysteresis Setting=%d\n",
                            data,
                            (data&0x78)>>5,               data&0x07);

  data = spi_read(6);
  unsigned int res = (data&0x30)>>4;
  read_bits = 0;
  switch (res){ 
    case 0: read_bits=12; break;
    case 1: read_bits=10; break;
    case 2: read_bits=16; break;
    case 3: read_bits=14; break;
  }
  Serial.printf("0x06:0x%02x\n    [7]   Dir=%d\n    [6]   Zero Latency Mode=%d\n    [5:4] Absolute Resolution=%d (%d bits)\n    [3]   SSI_Select=%d\n    [2:0] CPR Setting 2=%d\n",
                            data,
                            data&0x80?1:0,  data&0x40?1:0,                          res, read_bits,                           data&0x08?1:0,         data&0x07);
}


//REWRITTEN FROM AS6600
void AEAT8811::init_pin_ssi(uint8_t M0_T, uint8_t NSL_T, uint8_t SCLK_T, uint8_t DO_T, uint8_t MSEL_T) {
  M0   = M0_T;
  NSL  = NSL_T;
  DO   = DO_T;
  SCLK = SCLK_T;
  MSEL = MSEL_T;
  
  SPI.end(); // Так треба після перемикання режимів! Інакше дані спотворюються
  if (MSEL != 0){
    pinMode(MSEL,  OUTPUT);
    digitalWrite(MSEL,  HIGH); // -> not SPI4 mode
  }
  
//  pinMode(PWRDOWN, OUTPUT); //?
  
  pinMode(DO, INPUT);
  pinMode(SCLK, OUTPUT);
  pinMode(NSL, OUTPUT);
  
//  pinMode(MAG_HI, INPUT);
//  pinMode(MAG_LO, INPUT);

  digitalWrite(NSL, HIGH);
  digitalWrite(SCLK, HIGH);
  delayMicroseconds(1);

  mode = _AEAT_SSI3P;
}

unsigned long int AEAT8811::ssi_read_pins() {
// у цьому режимі службові прапорці йдуть ЗА числом, тому для отримання коректних прапорців треба задавати реальну бітову точність 
    if (mode != _AEAT_SSI3P) {
      init_pin_ssi();
    }
    
    unsigned long long int res=0;
    uint32_t buffer=0;
    digitalWrite(NSL, LOW);
    delayMicroseconds(1);
    
//    buffer = digitalRead(DO) & 0x01; // не треба!!
//    delayMicroseconds(1);

//  виявилось, що esp32 може робити транзакції (кількість sclk) з довільним числом бітів <=32
//    digitalWrite(17,  LOW);

    for (int i = 0; i < read_bits+4; i++){
      buffer <<= 1;
      digitalWrite(SCLK, LOW);
      buffer |= (digitalRead(DO) & 0x01);
      delayMicroseconds(1);
      digitalWrite(SCLK, HIGH);
      delayMicroseconds(1);
    }
    
//    buffer = (msb<<24)|buffer;
//    raw_data = buffer >> (24-4-bits); // це така фіча - замовляємо 17..22 біти, а повертається число у 24х бітах, "знизу" доклеєне нулями. Їх потрібно відсікти //???
    raw_data = buffer;
    delayMicroseconds(1);
    digitalWrite(NSL, HIGH);
    delayMicroseconds(1);
    
    error_parity = parity(raw_data); // рахуємо парність разом з переданим бітом парності, результат повинен тотожно дорівнювати нулю. Якщо одиниця - значить був збій парності.
    par = raw_data&1;
    mlo = (raw_data&2)>>1;
    mhi = (raw_data&4)>>2;
    rdy = (raw_data&8)>>3;
    res = raw_data>>4;
//    res >>= 4;
// для зручності подальших розрахунків приводимо результат до 16-бітового числа, незалежно від реальної точності датчика
    if (read_bits<16) 
      res = res<<(16-read_bits);
    return res;
}

//запис полів регістру 4
void AEAT8811::write_uvw_sel(uint8_t data) {
  reg4.bits = spi_read(4);
  reg4.uvw_sel = data;
  spi_write(4, reg4.bits);
}

void AEAT8811::write_iwidth(uint8_t data) {
  reg4.bits = spi_read(4);
  reg4.iwidth = data;
  spi_write(4, reg4.bits);
}

void AEAT8811::write_uvw_pwm(uint8_t data) {
  reg4.bits = spi_read(4);
  reg4.uvw_pwm = data;
  spi_write(4, reg4.bits);
}

//запис полів регістру 5
void AEAT8811::write_cpr1(uint8_t data) {
  reg5.bits = spi_read(5);
  reg5.cpr1 = data;
  spi_write(5, reg5.bits);
}

void AEAT8811::write_hyst(uint8_t data) {
  reg5.bits = spi_read(5);
  reg5.hyst = data;
  spi_write(5, reg5.bits);
}

//запис полів регістру 6
void AEAT8811::write_dir(uint8_t data) {
  reg6.bits = spi_read(6);
  reg6.dir = data;
  spi_write(6, reg6.bits);
}

void AEAT8811::write_zero_lat_mode(uint8_t data) {
  reg6.bits = spi_read(6);
  reg6.zero_lat_mode = data;
  spi_write(6, reg6.bits);
}

/*
 * 11 = 3: 14-b absolute resolution (SSI)
 * 10 = 2: 16-b absolute resolution (SSI)
 * 01 = 1: 10-b absolute resolution (SSI)
 * 00 = 0: 12-b absolute resolution (SSI)
 */
void AEAT8811::write_res(uint8_t data) {
  reg6.bits = spi_read(6);
  reg6.res = data;
  spi_write(6, reg6.bits);
  read_bits = 0;
  switch (reg6.res){ 
    case 0: read_bits=12; break;
    case 1: read_bits=10; break;
    case 2: read_bits=16; break;
    case 3: read_bits=14; break;
  }
}

void AEAT8811::write_ssi_sel(uint8_t data) {
  reg6.bits = spi_read(6);
  reg6.ssi_sel = data;
  spi_write(6, reg6.bits);
}

void AEAT8811::write_cpr2(uint8_t data) {
  reg6.bits = spi_read(6);
  reg6.cpr2 = data;
  spi_write(6, reg6.bits);
}

void AEAT8811::read_reg4(){
  reg4.bits = spi_read(4);
}
void AEAT8811::read_reg5(){
  reg5.bits = spi_read(5);
}
void AEAT8811::read_reg6(){
  reg6.bits = spi_read(6);
  read_bits = 0;
  switch (reg6.res){ 
    case 0: read_bits=12; break;
    case 1: read_bits=10; break;
    case 2: read_bits=16; break;
    case 3: read_bits=14; break;
  }
}

//читання полів
//регістру 4
uint8_t AEAT8811::read_uvw_sel(){
  reg4.bits = spi_read(4);
  return reg4.uvw_sel;
}
uint8_t AEAT8811::read_iwidth(){
  reg4.bits = spi_read(4);
  return reg4.iwidth;
}
uint8_t AEAT8811::read_uvw_pwm(){
  reg4.bits = spi_read(4);
  return reg4.uvw_pwm;
}
  
//регістру 5
uint8_t AEAT8811::read_cpr1(){
  reg5.bits = spi_read(5);
  return reg5.cpr1;
}

uint8_t AEAT8811::read_hyst(){
  reg5.bits = spi_read(5);
  return reg5.hyst;
}
  
//регістру 6
uint8_t AEAT8811::read_dir(){
  reg6.bits = spi_read(6);
  return reg6.dir;
}
uint8_t AEAT8811::read_zero_lat_mode(){
  reg6.bits = spi_read(6);
  return reg6.zero_lat_mode;
}
uint8_t AEAT8811::read_res(){
  reg6.bits = spi_read(6);
  read_bits = 0;
  switch (reg6.res){ 
    case 0: read_bits=12; break;
    case 1: read_bits=10; break;
    case 2: read_bits=16; break;
    case 3: read_bits=14; break;
  }
  return reg6.res;
}
uint8_t AEAT8811::read_ssi_sel(){
  reg6.bits = spi_read(6);
  return reg6.ssi_sel;
}
uint8_t AEAT8811::read_cpr2(){
  reg6.bits = spi_read(6);
  return reg6.cpr2;
}

//читання з ЮНІОНУ полів
//регістру 4
uint8_t AEAT8811::get_uvw_sel(){
  return reg4.uvw_sel;
}
uint8_t AEAT8811::get_iwidth(){
  return reg4.iwidth;
}
uint8_t AEAT8811::get_uvw_pwm(){
  return reg4.uvw_pwm;
}
  
//регістру 5
uint8_t AEAT8811::get_cpr1(){
  return reg5.cpr1;
}

uint8_t AEAT8811::get_hyst(){
  return reg5.hyst;
}
  
//регістру 6
uint8_t AEAT8811::get_dir(){
  return reg6.dir;
}
uint8_t AEAT8811::get_zero_lat_mode(){
  return reg6.zero_lat_mode;
}
uint8_t AEAT8811::get_res(){
  return reg6.res;
}
uint8_t AEAT8811::get_ssi_sel(){
  return reg6.ssi_sel;
}
uint8_t AEAT8811::get_cpr2(){
  return reg6.cpr2;
}

#include <RTClib.h>

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

void setup () {
  Serial.begin(9600);

  // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }

  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // manually sets the RTC with an explicit date & time, for example to set
  // January 21, 2021 at 3am you would call:
  // rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));
}

void loop () {
  DateTime now = rtc.now();
  Serial.print("Date & Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  delay(1000); // delay 1 seconds
}




//---- clock timer code ^^^

#define RDA 0x80
#define TBE 0x20

#include <dht.h> //install the DHTLib library
#include <LiquidCrystal.h>
#include <Stepper.h> // Include the header file

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int *myUBRR0 = (unsigned int *)0x00C4;
volatile unsigned char *myUDR0 = (unsigned char *)0x00C6;

volatile unsigned char *my_ADMUX = (unsigned char *)0x7C;
volatile unsigned char *my_ADCSRB = (unsigned char *)0x7B;
volatile unsigned char *my_ADCSRA = (unsigned char *)0x7A;
volatile unsigned int *my_ADC_DATA = (unsigned int *)0x78;

volatile unsigned char *port_b = (unsigned char *)0x25;
volatile unsigned char *ddr_b = (unsigned char *)0x24;
volatile unsigned char *pin_b = (unsigned char *)0x23;
volatile unsigned char *port_c = (unsigned char *)0x28;
volatile unsigned char *ddr_c = (unsigned char *)0x27;
volatile unsigned char *pin_c = (unsigned char *)0x26;

volatile unsigned char *port_d = (unsigned char *)0x2B;
volatile unsigned char *ddr_d = (unsigned char *)0x2A;
volatile unsigned char *pin_d = (unsigned char *)0x29;

volatile unsigned char *port_j = (unsigned char *)0x105;
volatile unsigned char *ddr_j = (unsigned char *)0x104;
volatile unsigned char *pin_j = (unsigned char *)0x103;

volatile unsigned char *port_h = (unsigned char *)0x102;
volatile unsigned char *ddr_h = (unsigned char *)0x101;
volatile unsigned char *pin_h = (unsigned char *)0x100;

volatile unsigned char *myTCCR1A = (unsigned char *)0x80;
volatile unsigned char *myTCCR1B = (unsigned char *)0x81;
volatile unsigned char *myTCCR1C = (unsigned char *)0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *)0x6F;
volatile unsigned int *myTCNT1 = (unsigned int *)0x84;
volatile unsigned char *myTIFR1 = (unsigned char *)0x36;

int value = 0;
unsigned char date;
// b0 is water sensor enable pin 53
// b1 is humidity/temp enable pin 52
// b2 is button for enable/disable
//  liquidcrystal is using pb6, pb5, pe3, pg5, pe5, pe4, aka 12, 11, 5, 4, 3, 2
// dht is using pin 10, aka pb4
// fan motor is using pin 14, 15, 16, aka pj1, pj0, ph1
// water sensor is analog pin 0
// potentiometer for stepper motor is analog pin 1
// stepper motor is 22,23,24,25 aka pa0, pa1, pa2, pa3
// leds are pd0-pd3 red, yellow, green, blue
// reset button is pc1 36
#define DHT11_PIN 10
#define STEPS 32
const int rs = 12,
          en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2, waterPin = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Stepper stepper(STEPS, 22, 23, 24, 25);
dht DHT;
int Pval = 0;
int potVal = 0;

void setup()
{
  U0init(9600);          // initialize
  adc_init();            // initialize ADC
  *ddr_b |= 0b00000011;  // set b0/b1 to be configurable on/off for water sensor
  *ddr_b &= 0b11111011;  // b2 is  button input for enable/disable should be set to read
  *ddr_c &= 0b11111101;  // pc1 is reset button
  *port_b &= 0b11111100; // set b0/b1 to be off by default
  *port_b |= 0b00000100; // set b2 to have pull up resistor
  *port_c |= 0b00000010; // pc1 gets pull up resistor
  *ddr_j |= 0b00000010;  // set enable to be output
  *ddr_j |= 0b00000001;  // set dira to be output
  *ddr_h |= 0b00000010;  // set dirb to be output
  *port_j |= 0b00000001; // set direction fan 1
  *port_h &= 0b11111101; // turn direction fan 2
  *ddr_d |= 0b00001111;  // set pd0-pd3
  stepper.setSpeed(200); // set up stepper
  lcd.begin(16, 2);
}

bool enabled = false;
bool error = true;
bool fanon = false;
void loop()
{

  if ((*pin_b & 0b00000010 > 0) && !enabled)
  {
    enabled = true;
  }
  else if (*pin_b & 0b00000010 > 0 && enabled)
  {
    enabled = false;
  }

  // put fan on when it needs to
  // put fan off when it needs to
  if (fanon)
  {
    *port_j |= 0b00000010; // set pj1
  }
  else
  {
    *port_j &= 0b11111101; // set pj1
  }
  if (!error)
  { // adjust vent position, when no error
    potVal = map(adc_read(1), 0, 1024, 0, 500);
    if (potVal > Pval)
      stepper.step(5);
    if (potVal < Pval)
      stepper.step(-5);
    Pval = potVal;
  }

  if (enabled)
  {
    *port_b |= 0b00000001;                        // turn  water sensor on
    *port_b |= 0b00000010;                        // turn temp/humidity snsor on
    unsigned int watervalue = adc_read(waterPin); // read the analog value from sensor
    if (watervalue < 50)
    {
      error = true;
    }
    else
    {
      error = false;
    }

    int chk = DHT.read11(DHT11_PIN);

    if (!error)
    {
      lcd.setCursor(0, 0);
      lcd.print("Temp = ");
      lcd.print(DHT.temperature);
      lcd.setCursor(0, 1);
      lcd.print("Hmty = ");
      lcd.print(DHT.humidity);
      if (DHT.temperature > 10)
      {
        fanon = true;
        //blue led on
        *port_d &= 0b11111000;
        *port_d |= 0b00001000;
      }
      else
      {
        fanon = false;
        // green led on
        *port_d &= 0b11110100;
        *port_d |= 0b00000100;
      }
    }
    else
    { // if error
      fanon = false;
      lcd.setCursor(0, 0);
      lcd.print("Water level is too low!");
      // turn red on turn all others off
      *port_d &= 0b11110001;
      *port_d |= 0b00000001;
    }
  }
  else
  { // DISABLED
    // turn yellow on turn all others off
    *port_d &= 0b11110010;
    *port_d |= 0b00000010;
    lcd.setCursor(0, 0);
    lcd.print("disabled :/");
  }
  my_delay(500);
}

void U0init(int U0baud)
{
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud - 1);
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0 = tbaud;
}

void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if (adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while ((*my_ADCSRA & 0x40) != 0)
    ;
  // return the result in the ADC data register
  return *my_ADC_DATA;
}

unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while ((*myUCSR0A & TBE) == 0)
    ;
  *myUDR0 = U0pdata;
}

// void U0putstr(un)

void my_delay(unsigned int freq)
{
  // calc period
  double period = 1.0 / double(freq);
  // 50% duty cycle
  double half_period = period / 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int)(65536 - ticks);
  // start the timer
  *myTCCR1B |= 0b00000001;
  // wait for overflow
  while ((*myTIFR1 & 0x01) == 0)
    ; // 0b 0000 0000
  // stop the timer
  *myTCCR1B &= 0xF8; // 0b 0000 0000
  // reset TOV
  *myTIFR1 |= 0x01;
}
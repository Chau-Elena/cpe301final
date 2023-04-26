#include <dht.h> //install the DHTLib library
#define DHT11_PIN 10


dht DHT; 
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/*void loop(){
int chk = DHT.read11(DHT11_PIN);
Serial.print("Temperature = ");
Serial.println(DHT.temperature);
Serial.print("Humidity = ");
Serial.println(DHT.humidity);
delay(1000);
}*/



//-----

#define RDA 0x80
#define TBE 0x20


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

volatile unsigned char *myTCCR1A = (unsigned char *)0x80;
volatile unsigned char *myTCCR1B = (unsigned char *)0x81;
volatile unsigned char *myTCCR1C = (unsigned char *)0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *)0x6F;
volatile unsigned int *myTCNT1 = (unsigned int *)0x84;
volatile unsigned char *myTIFR1 = (unsigned char *)0x36;

int value = 0; // variable to store the sensor value
/*unsigned char wString[14] = "Water level: ";
unsigned char hString[17] = "Humidity Level: ";
unsigned char tString[19] = "Temperature Level: ";
unsigned char number[7] = "0000.00";*/
// solution to multiple adc problem probably has to do with admux
// i think you just call the same adc read function with different channel name

void setup()
{
  U0init(9600);          // initialize
  adc_init();            // initialize ADC
  *ddr_b |= 0b00000001;  // set b0 to be configurable on/off for water sensor
  *port_b &= 0b11111110; // set b0 to be off by default
  lcd.begin(16, 2);
}
int waterPin = 0;

void loop()
{
  *port_b |= 0b00000001;                        // turn sensor on
  my_delay(100);                                // delay. i need to configure this to not take frequencies but seconds lol
  unsigned int watervalue = adc_read(waterPin); // read the analog value from sensor
  int chk = DHT.read11(DHT11_PIN);
  lcd.setCursor(0, 0);
  lcd.print("Temp = ");
  lcd.print(DHT.temperature);
  lcd.setCursor(0, 1);
  lcd.print("Hmty = ");
  lcd.print(DHT.humidity);
  delay(1000);
  /*unsigned char wthou = (watervalue / 1000) + '0';
  unsigned char whund = ((watervalue % 1000) / 100) + '0';
  unsigned char wtens = ((watervalue % 100) / 10) + '0';
  unsigned char wones = ((watervalue % 10)) + '0';
  *port_b &= 0b11111110;                       // turn sensor off
  for (int i = 0; i < 14; i++)
  { // i think this is how you would do this
    U0putchar(wString[i]);
  }

  if (wthou != '0') // print number vvvvvvv
  {
    U0putchar(wthou);
  }
  if (whund != '0')
  {
    U0putchar(whund);
  }
  if (wtens != '0')
  {
    U0putchar(wtens);
  }
  U0putchar(wones);
  U0putchar('\n');*/
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
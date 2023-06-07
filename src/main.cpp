#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <U8g2lib.h>
//#include <OneButton.h>
#include <Adafruit_VEML7700.h>
#include <XMCEEPROMLib.h>

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
Adafruit_VEML7700 veml = Adafruit_VEML7700();
//iso buttons
//OneButton button1(2, true); //iso- 2
//OneButton button2(0, true); //iso+ 0
//shutter/diaphaghm buttons
//OneButton button3(3, true); //l- 3
//OneButton button4(1, true); //l+ 

//OneButton button5(4, true); // ok 4

const uint16_t isos[] = {6,12,25,50,64,100,160,200,250,320,400,500,800,1600,3200};
const float shutters[] = {0.001, 0.002, 0.004, 0.008, 0.0167, 0.0333, 0.0667, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16};
const String shutters_str[] = {"1/1000", "1/500", "1/250", "1/125", "1/60", "1/30", "1/15", "1/8", "1/4", "1/2", "1\"", "2\"", "4\"", "8\"", "16\""};

void click_iso_m(); 
void click_iso_p(); 
void click_s_m(); 
void click_s_p(); 
void clicks();
void u8g2update();
void lightupdate();

float lux = 0.0f;
volatile int iso_no = 5;
volatile int iso = isos[iso_no];
volatile int shutter_no = 3;
volatile float t = shutters[shutter_no];
volatile float N = 2.8;
float LV = 0;
float EV = 0;

void setup() {
	EEPROM.begin(4096);

	u8g2.begin();

	if (veml.begin()) {
		Serial.println("Found a VEML7700 sensor");
	} else {
		Serial.println("No sensor found ... check your wiring?");
		while (1);
	} 
	Serial.print("Status: "); Serial.println(EEPROM.status());

	int iso_cell = EEPROM.read(0);
	iso_no = iso_cell <= 14 ? iso_cell : 5;
	iso = isos[iso_no];

	int shutter_cell = EEPROM.read(10);
	shutter_no = shutter_cell <= 14 ? shutter_cell : 5;
	t = shutters[shutter_no];

	pinMode(9, INPUT_PULLUP);
	pinMode(0, INPUT_PULLUP);
	pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	pinMode(4, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(9), clicks, FALLING);

	interrupts();
}

void u8g2update() {
		u8g2.firstPage();
		do {
			u8g2.setFont(u8g2_font_6x12_tr  );
			u8g2.drawStr(0,8,"t: ");
			u8g2.setCursor(18,8);
			u8g2.print(shutters_str[shutter_no]);

			u8g2.drawStr(0, 16,"Lux: ");
			u8g2.setCursor(26,16);
			u8g2.print(lux);

			u8g2.drawStr(0,24,"ISO: ");
			u8g2.setCursor(26,24);
			u8g2.print(iso);

			u8g2.drawStr(0,32,"LV: ");
			u8g2.setCursor(18,32);
			u8g2.print(LV);

			u8g2.drawStr(53,32,"EV: ");
			u8g2.setCursor(71,32);
			u8g2.print(EV);

			u8g2.drawStr(110,32,"A");
			
			u8g2.setFont(u8g2_font_logisoso24_tn );
			u8g2.setCursor(80,24);
			u8g2.print(N);
			//u8g2.drawStr(80,24,"2.8");

		} while ( u8g2.nextPage() );
} //u8g2update

void lightupdate() {
		lux = veml.readLux(VEML_LUX_AUTO);
		LV = logf(lux/2.5f)/_M_LN2;
		EV = LV + logf(iso/100.0f)/_M_LN2;
		N = sqrt(t*powf(2,EV));
} //lightupdate


void loop() {
	lightupdate();
	u8g2update();
} //loop

void clicks() {
	if (digitalRead(0) == LOW) 
		click_iso_p(); 
	else
	if (digitalRead(2) == LOW) 
		click_iso_m();
	else
	if (digitalRead(3) == LOW) 
		click_s_m();
	else
	if (digitalRead(1) == LOW) 
		click_s_p();
} //ticks

void click_iso_m() {
  	if (iso_no > 0) iso_no--;
	iso = isos[iso_no];
	EEPROM.write(0, iso_no);
	EEPROM.commit();
} // click1

void click_iso_p() {
  	if (iso_no <= 13) iso_no++;
	iso = isos[iso_no];
	EEPROM.write(0, iso_no);
	EEPROM.commit();
} // click2

void click_s_m() {
  	if (shutter_no > 0) shutter_no--;
	t = shutters[shutter_no];
	EEPROM.write(10, shutter_no);
	EEPROM.commit();
} // click1

void click_s_p() {
  	if (shutter_no <= 13) shutter_no++;
	t = shutters[shutter_no];
	EEPROM.write(10, shutter_no);
	EEPROM.commit();
} // click2

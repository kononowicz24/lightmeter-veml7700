#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include <Adafruit_VEML7700.h>
#include <XMCEEPROMLib.h>
#include <CoopTask.h>
#include <CoopSemaphore.h>

#if defined(ARDUINO_AVR_MICRO)
#define STACKSIZE_8BIT 92
#else
#define STACKSIZE_8BIT 40
#endif

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
Adafruit_VEML7700 veml = Adafruit_VEML7700();
//iso buttons
OneButton button1(2, true); //iso-
OneButton button2(0, true); //iso+
//shutter/diaphaghm buttons
OneButton button3(3, true); //l-
OneButton button4(1, true); //l+

OneButton button5(4, true);

CoopSemaphore taskSema(1, 1);
int taskToken = 1;

const uint8_t isos[] = {6,12,25,50,64,100,160,200,250,320,400,500,800,1600,3200};
void click1(); 
void click2(); 
void ticks();
void u8g2update();
void lightupdate();

float lux = 0.0f;
int iso_no = 5;
int iso = isos[iso_no];
float t = 125.0f;
float LV = 0;
float EV = 0;

BasicCoopTask<CoopTaskStackAllocatorAsMember<sizeof(unsigned) >= 4 ? 800 : STACKSIZE_8BIT>> task1("l1", u8g2update);
BasicCoopTask<CoopTaskStackAllocatorAsMember<sizeof(unsigned) >= 4 ? 800 : STACKSIZE_8BIT>> task2("l2", lightupdate);
BasicCoopTask<CoopTaskStackAllocatorFromLoop<sizeof(unsigned) >= 4 ? 800 : STACKSIZE_8BIT>> task3("l3", ticks, sizeof(unsigned) >= 4 ? 800 : STACKSIZE_8BIT);

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
  	delay(1000);
	int iso_cell = EEPROM.read(0);
	iso_no = iso_cell <= 14 ? iso_cell : 5;
	iso = isos[iso_no];

	button1.attachClick(click1);
	button2.attachClick(click2);
	//button3.attachClick(click3);
	//button4.attachClick(click4);
	//button5.attachClick(click5);
}

void u8g2update() {
	u8g2.firstPage();
	do {
		u8g2.setFont(u8g2_font_6x12_tr  );
		u8g2.drawStr(0,8,"t: 1/125");

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
		u8g2.drawStr(80,24,"2.8");

	} while ( u8g2.nextPage() );
}

void lightupdate() {
	lux = veml.readLux(VEML_LUX_AUTO);
	LV = logf(lux/2.5f)/_M_LN2;
	EV = LV + logf(iso/100.0f)/_M_LN2;
}


void loop() {

	ticks();
	lightupdate();
	u8g2update();

}

void ticks() {
	button1.tick();
	button2.tick();
	button3.tick();
	button4.tick();
	button5.tick();
}

void click1() {
  	if (iso_no > 0) iso_no--;
	iso = isos[iso_no];
	EEPROM.write(0, iso_no);
	EEPROM.commit();
} // click1

void click2() {
  	if (iso_no < 13) iso_no++;
	iso = isos[iso_no];
	EEPROM.write(0, iso_no);
	EEPROM.commit();
} // click2


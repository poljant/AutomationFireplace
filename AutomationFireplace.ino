#include "Arduino.h"
/*  AutomationFireplace.ino
 *
 *	Copyright (c) 2016. All rights reserved.
 *
 *      Author: Jan Trzciński  <poljant@post.pl>
 *
 * Układ załącza wentylatory obiegu ciepła z kominka
 * Załączenia są zależne od teperatury powietrza w komorze kominka
 * Ustawiane są 3 progi temperatury temp_on1, temp_on2 i temp_on3.
 * Załączenie poszczególnych przekaźników następuje po przekroczeniu
 * poszczególnych progów, a wyłączenie przy temperaturach niższych
 * od progów o wartość ustawioną w zmiennej hyster.
 * Wszystkie te zmienne ustawiamy w pliku FireplaceControlle.h.
 * Progi:
 * 1. załącza przekazniki 1 i 3 oraz przekaźniki RF1 i RF2
 * 2. załącza przekaźnik 2
 * 3. załącza przekaźnik 4
 * Przekaźniki RF to wyłączniki sterowane pilotem na 433MHz. (np. CLARUS z Biedronki)
 * Wejście w trybu MANUAL po zalogowaniu się http://IP/login lub wciśnięciu tryb AUTO.
 * Tryb "MANUAL" umożliwia ręczne przełączanie przekaźników,
 * Po określomym czasie (timeM), gdy nic nie jest zmieniane -
 * automatycznie przechodzi w stan AUTO.
 * Zmienna ManualTime podaje ile minut może trwać czas Manual.
 * MANUAL sygnalizowany jest migotaniem LED-a wmontowanego w układ ESP8266 ESP-12E.
 * LED wbudowany świeci się ciągle, gdy brak połączenia z WiFi.
 * Gdy brak połączenia z WiFi układ przechodzi w stan AP_STA.
 * Można sie z nim połączyć poprzez uruchomiony AP.
 * Po połączeniu z AP dostęp przez http://192.168.4.1 login: admin pass: esp8266
 * tryb "AUTO" - automatyczne sterowanie przekaźnikami
 *                zależnie od temperatury
 * Aktualizacja programu poprzez WiFi
 * http://IP/update   login i hasło jak dla logowania
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
#include <Relay.h>

#include "secrets.h"
#include "../AutomationFireplace/RF2260.h"
#include "../AutomationFireplace/FireplaceController.h"
#include "../AutomationFireplace/WebPages.h"

String version = "1.2.9";
FireplaceController fc;
int ManualTime = 15; // how many minutes the manual state lasts
double long timeM = 0; //time to start MANUAL mode
double long M15; // time start mode WIFI_AP_STA + 15minutes
extern double long fminutes(int);
double long timed = 500; //time to delay display
double long timec = 0; //time current to display
bool _display = true; //display On or Off
extern ESP8266WebServer server;
// set the details of your WiFi network
//const char* ssid = "SSID"; //SSID your WiFi
//const char* pass = "password"; // password your WiFi

//#define DEBUG
#define IP_STATIC
#ifdef IP_STATIC
IPAddress IPadr(10, 110, 3, 10); //static IP
IPAddress netmasfc(255, 255, 0, 0);
IPAddress gateway(10, 110, 0, 1);
#endif
#define SDA D2
#define SCL D1
bool disp = true;
LiquidCrystal_PCF8574 lcd(0x27);
String strTemp(float t) {
	String s = "";
	if (t < 10) {
		s = "  ";
		s += int(t);
		return s;
	}
	if (t < 100) {
		s = " ";
		s += int(t);
		return s;
	}
	s = int(t);
	return s;
}
;
void Alarm(void) {

}
//The setup function is called once at startup of the sketch
void setup() {
// Add your initialization code here
#ifdef DEBUG
Serial.begin(115200);
#endif

	WiFi.mode(WIFI_AP_STA);
#ifdef IP_STATIC
	WiFi.config(IPadr, gateway, netmasfc);
#endif

	lcd.begin(16, 2);
	lcd.home();
	lcd.clear();
	lcd.setBacklight(255);
	lcd.setCursor(0, 0);
	lcd.print(F("  Fireplace!  "));
	lcd.setCursor(0, 1);
	lcd.print(F("Uruchamianie..."));

	WiFi.begin(ssid, pass);
	int i = 0;
	while ((WiFi.status() != WL_CONNECTED) && (i <= 15)) { //  wait for connection with WiFi
		delay(500);
		i += 1;
#ifdef DEBUG
    Serial.print(".");
#endif
	}

	if (WiFi.status() != WL_CONNECTED) {
		if (WiFi.getMode() != WIFI_AP_STA) {
			WiFi.mode(WIFI_AP_STA);
		}  //set mode AP+STATION
	} else {

		M15 = fminutes(ManualTime);   // current time + 15 minutes
	}

#ifdef DEBUG
  if (WiFi.status() == WL_CONNECTED){
  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.println(WiFi.localIP());         // print IP
  Serial.println(WiFi.macAddress());      // print MAC address
  }else Serial.println(F("Disconnect!!!"));
 // printconfig();
#endif

	setservers(); //start servers www
	fc.begin(); //initiation fireplace controller
	fc.readTemp(false); // start read temperature at fireplace
	fc.breadTemp = true;

}
;

// The loop function is called in an endless loop
void loop() {

	if (timec <= millis()) {
		timec = millis() + timed;
		String s1 = "T=";
		s1 += (strTemp(fc.temp_current));
//		s1+=( strTemp(fc.temp_in_box));
		s1 += "C";
		s1 += ((fc.relay1.read()) ? " R1+" : " R1-");
		s1 += ((fc.relay2.read()) ? "R2+" : "R2-");
		s1 += ((fc.relay3.read()) ? "R3+" : "R3-");
		String s2 = "F1=";
		s2 += fc.fan1;
		s2 += "%  ";
		String s3 = "F2=";
		s3 += fc.fan2;
		s3 += "%  ";
		lcd.home();
		lcd.clear();
		lcd.setBacklight(255);
		lcd.setCursor(0, 0);
		lcd.print(s1);
		lcd.setCursor(0, 1);
		if (fc.temp_in_box >= fc.temp_alarm) {
			// lcd.setBacklight(120);
			lcd.setCursor(0, 1);
			lcd.print(F("**** ALARM! ****"));
			_display ? lcd.noDisplay() : lcd.display();
			_display = !_display;
/*			lcd.noDisplay();
			delay(300);
			lcd.display();*/
		} else {
			_display=true;
			lcd.display();
			lcd.setCursor(0, 1);
			lcd.print(s2);
			lcd.setCursor(7, 1);
			lcd.print(s3);
		}
		lcd.setCursor(14, 1);
		if (fc.program == 1) {
			lcd.print("P1");
		} else {
			lcd.print("P0");
		}
	}
	server.handleClient(); //wait for the connection with the client

	if (!fc.bmode) {	// if mode MANUAL
		if (timeM <= millis()) {
			fc.setAuto();
		} // check the duration of MANUAL when it switches to AUTO
	}
	fc.working(); // start automation fireplace

	if (WiFi.status() != WL_CONNECTED) {

		if (WiFi.getMode() != WIFI_AP_STA) {
			WiFi.mode(WIFI_AP_STA);  //set mode AP+STATION
			M15 = fminutes(ManualTime); // current time + 15 minutes
		}
	} else {
		if (M15 <= millis())
			WiFi.mode(WIFI_STA); // set mode WIFI_STA
	}

}


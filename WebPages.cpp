/*
 * WebPages.cpp
 *
 *  Created on: 3 wrz 2018
 *      Author: jant
 */
/*
 * FireplaceControllerWeb.cpp
 *
 *   Copyright (c) 2016. All rights reserved.
 *
 *      Author: Jan Trzciński  <poljant@post.pl>
 */
//#include <ESP8266HTTPClient.h>
#include "WebPages.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include "FireplaceController.h"
#include "RF2260.h"

//#define POLISH
extern String version;
extern double long timeM;
// login i hasło do sytemu
const char* wwwlogin = "admin";
const char* wwwpass = "esp8266";
//extern const char* ssid;
//extern const char* pass;
//extern uint8_t etemp;
//extern uint8_t Eetemp;
//const char* epass[32];
//const char* essid[64];

///const char* modes[] = {"NULL","STA","AP","STA+AP"};
//const char* phymodes[] = { "","B", "G", "N"};
//const char* encrypType[] = {"OPEN", "WEP", "WPA", "WPA2", "WPA_WPA2"};

const int port = 80;                 // port serwera www
ESP8266WebServer server(port);
ESP8266HTTPUpdateServer httpUpdate;

extern FireplaceController fc;
//extern RF2260 rf2;
//extern String setfile;

//funkcja oblicza ile minut ma trwać dana procedura
extern int ManualTime; // ile minut trwa czas manual
unsigned long fminutes( int ile) {
	return (millis()+(1000*60*ile));
}

char* IPAdrToStr(int ip)
{
  char* ip_str = new char[16];
  sprintf(ip_str, "%d.%d.%d.%d",
    (ip 	) & 0xFF,
    (ip >> 8) & 0xFF,
    (ip >> 16) & 0xFF,
    (ip >> 24) & 0xFF);
  return ip_str;
}


String HTMLHeader() {           //  Header page
String  h = F("<!DOCTYPE html>\n"
  "<html>"
  "<head>"
  "<title> FireplaceController</title>"
  "<meta charset=\"utf-8\">"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\" >"
  "</head>"
  "<body style=\"text-align: center;color: white; background: black;font-size: 1.5em;\">\n");

  return h;
}

String HTMLFooter() {             //  stopka strony www
String  f ="";
#ifdef POLISH
  f += F("<p><a href = \"/\"><button class=\"btn btn-info\">Odświerz stronę</button></a></p>");
#else
  f += F("<p><a href = \"/\"><button class=\"btn btn-info\">Reload</button></a></p>");
#endif
  f += F("<p>Jan Trzciński &copy; 2016-2018</p></td></tr>"
   "</body>\n"
   "</html>\n");
  return f;
}

String HTMLPage1() {      // pierwsza część strony www
 String t;
 unsigned long sec = millis() / 1000;
 unsigned long min = (sec / 60);
 unsigned long hour = (min / 60);
 unsigned long days = hour / 24;
  sec = sec % 60;
  min = min % 60;
  hour = hour % 24;
#ifdef POLISH
 t  = F("<h1><p>Automatyka kominka</p></h1>"
  "<p> Wersja ");
#else
 t  = F("<h1><p>Automation Fireplace</p></h1>"
  "<p> version ");
#endif
 t += (version);
 t += "</p>";
// t += "<p>Napięcie zasilania:";
// t += (powersuply/1000);
// t += "V</p>";
#ifdef POLISH
 t += F("<p>Czas od uruchomienia dni: ");
#else
 t += F("<p>UpTime days: ");
#endif
 t += (days);
#ifdef POLISH
 t += F(" godz:" );
#else
 t += F(" hours:" );
#endif
 t += ((hour<10) ? "0" : "");
 t += (hour);
 t += ":";
 t += ((min<10) ? "0" : "");
 t += (min);
 t += ":";
 t += ((sec < 10) ? "0" : "");
 t += (sec);
 t += "</p>";
 return t;
}
String HTMLPage1a(){
	 String t = "";
	  fc.readTemp();
	  float temperature = fc.readTempIn();// (sensors.getTempCByIndex(0));
	 // float powersuply = ESP.getVcc()*0.913; //popraw błąd przeliczania napięcia
#ifdef POLISH
 t += F("<p> Temperatura powietrza w komorze: ");
#else
 t += F("<p> Air temperature in the fireplace chamber: ");
#endif
 t += (temperature);
 t += " ºC</p>";
#ifdef POLISH
 t += F("<p> Temperatura maksymalna: ");
#else
 t += F("<p> Last maximum temperature:: ");
#endif
 t += (fc.temp_max);
 t += F(" ºC</p>"
  "<p> Fan1: ");
 t += (fc.fan1);
 t += F("% Fan2: ");
 t += (fc.fan2);
 t += F(" %</p>");

 return t;
}

String HTMLInfo(){
	String p="";
#ifdef POLISH
	 p += F( "<p>Połączenia: 1-Wire na D3, Nad. lub odb. RF na D4 </p>\n"
	  "<p>Odbiornik RF CLARUS gniazdo 2 z serii 0047</p>\n"
	  "<p> relay1 na D8, relay2 na D0</p>"
	  "<p> PWM1 na D5, PWM2 na D6, relay3 na D7 </p>\n");
#else
	 p += F( "<p>connections: 1-Wire to D3, TX RF or RX RF to D4 </p>\n"
	  "<p>Receiver RF CLARUS socket 2 series 0047</p>\n"
	  "<p> relay1 to D8, relay2 to D0</p>"
	  "<p> PWM1 to D5, PWM2 to D6, relay3 to D7 </p>\n");
#endif
	  return p;
}
String HTMLPage2() {            // główna strona www
  String p = "";
  //  display links depending on current state of relay 1,2,3 i 4
  if (!fc.bmode) { //gdy ustawiony tryb ręczny
#ifdef POLISH
  p += (fc.relay3.read()) ? F("<p><a href = \"/relay1/0\"><button class=\"btn btn-danger\">Przekaźnik 1 ON</button></a></p>\n") \
  : F("<p><a href = \"/relay1/1\"><button class=\"btn btn-success\">Przekaźnik 1 OFF</button></a></p>\n");
  p += (fc.relay1.read()) ? F("<p><a href = \"/relay2/0\"><button class=\"btn btn-danger\">Przekaźnik 2 ON</button></a></p>\n") \
  : F("<p><a href = \"/relay2/1\"><button class=\"btn btn-success\">Przekaźnik 2 OFF</button></a></p>\n");
  p += (fc.rf3.readRF()) ? F("<p><a href = \"/relay3/0\"><button class=\"btn btn-danger\">Przekaźnik 3 ON</button></a></p>\n") \
    : F("<p><a href = \"/relay3/1\"><button class=\"btn btn-success\">Przekaźnik 3 OFF</button></a></p>\n");
  p +=  (fc.relay2.read()) ? F("<p><a href = \"/alarm/0\"><button class=\"btn btn-danger\">ALARM ON</button></a></p>\n") \
      : F("<p><a href = \"/alarm/1\"><button class=\"btn btn-success\">ALARM OFF</button></a></p>\n");
#else
  p += (fc.relay3.read()) ? F("<p><a href = \"/relay1/0\"><button class=\"btn btn-danger\">Relay 1 is ON</button></a></p>\n") \
  : F("<p><a href = \"/relay1/1\"><button class=\"btn btn-success\">Relay 1 is OFF</button></a></p>\n");
  p += (fc.relay1.read()) ? F("<p><a href = \"/relay2/0\"><button class=\"btn btn-danger\">Relay 2 is ON</button></a></p>\n") \
  : F("<p><a href = \"/relay2/1\"><button class=\"btn btn-success\">Relay 2 is OFF</button></a></p>\n");
  p += (fc.rf3.readRF()) ? F("<p><a href = \"/relay3/0\"><button class=\"btn btn-danger\">Relay 3 is ON</button></a></p>\n") \
    : F("<p><a href = \"/relay3/1\"><button class=\"btn btn-success\">Relay is 3 OFF</button></a></p>\n");
  p +=  (fc.relay2.read()) ? F("<p><a href = \"/alarm/0\"><button class=\"btn btn-danger\">ALARM is ON</button></a></p>\n") \
  : F("<p><a href = \"/alarm/1\"><button class=\"btn btn-success\">ALARM is OFF</button></a></p>\n");
#endif
  }
  else { // if type AUTO
#ifdef POLISH
	  // gdy tryb AUTO
  p += (fc.relay3.read()) ? F("<p><button class=\"btn btn-danger\">Przekaźnik 1 ON</button></p>\n") \
  : F("<p><button class=\"btn btn-success\">Przekaźnik 1 OFF</button></p>\n");
  p += (fc.relay1.read()) ? F(<p><button class=\"btn btn-danger\">Przekaźnik 2 ON</button></p>\n") \
  : F("<p><button class=\"btn btn-success\">Przekaźnik 2 OFF</button></p>\n");
  p += (fc.rf3.readRF()) ? F("<p><button class=\"btn btn-danger\">Przekaźnik 3 ON</button></p>\n") \
    : F("<p><button class=\"btn btn-success\">Przekaźnik 3 OFF</button></p>\n");
  p += (fc.relay2.read()) ? F("<p><button class=\"btn btn-danger\">ALARM ON</button></p>\n") \
   : F("<p><button class=\"btn btn-success\">ALARM OFF</button></p>\n");
#else
  p += (fc.relay3.read()) ? F("<p><button class=\"btn btn-danger\">Relay 1 is ON</button></p>\n") \
  : F("<p><button class=\"btn btn-success\">Relay 1 is OFF</button></p>\n");
  p += (fc.relay1.read()) ? F("<p><button class=\"btn btn-danger\">Relay 2 is ON</button></p>\n") \
  : F("<p><button class=\"btn btn-success\">Relay 2 OFF</button></p>\n");
  p += (fc.rf3.readRF()) ? F("<p><button class=\"btn btn-danger\">Relay 3 is ON</button></p>\n") \
    : F("<p><button class=\"btn btn-success\">Relay 3 is OFF</button></p>\n");
  p += (fc.relay2.read()) ? F("<p><button class=\"btn btn-danger\">ALARM is ON</button></p>\n") \
  : F("<p><button class=\"btn btn-success\">ALARM is OFF</button></p>\n");
#endif
  }
#ifdef POLISH
  // wyświetl jaki tryb wybrany
  p += (fc.bmode) ? F("<p><a href = \"/login\"><button class=\"btn btn-success\">Tryb AUTO</button></a></p>\n") \
  : F("<p><a href = \"/auto\"> <button class=\"btn btn-danger\"> Tryb MANUAL</button></a></p>\n");
#else
  // display type mode
  p += (fc.bmode) ? F("<p><a href = \"/login\"><button class=\"btn btn-success\">Type AUTO</button></a></p>\n") \
  : F("<p><a href = \"/auto\"> <button class=\"btn btn-danger\"> Type MANUAL</button></a></p>\n");
#endif
  return p;
}


 String WebPage() {       // połącz wszystkie części strony www
 return (HTMLHeader() + HTMLPage1()+ HTMLPage1a() + HTMLPage2() + HTMLFooter());
 }

// funkcja ustawia wszystkie strony www
void setservers(void){
// FSservers();

 httpUpdate.setup(&server,"/update", wwwlogin, wwwpass); // umożliwia aktualizację poprzez WiFi

 server.on("/", [](){      // reload - odświerz stronę www
    server.send(200, "text/html", WebPage());
  });

 server.on("/login", [](){	//zaloguj się do strony
     if(!server.authenticate(wwwlogin, wwwpass))
       return server.requestAuthentication();
     fc.setManual(); // = false;
 //    k.led0.setOff(); ////sygnalizacja załączenia trybu MANUAL (LED ESP8266)
     timeM = fminutes(ManualTime);  // odnów czas trybu MANUAL
     server.send(200, "text/html", WebPage());
   });

 server.on("/auto", [](){      // ustaw tryb AUTO
     fc.bmode = true;
 //    k.led0.setOn();
     server.send(200, "text/html", WebPage());
  });

 server.on("/relay1/0", [] ()     //  wyłącz przekaźnik 1
  { if (!fc.bmode) {
	  fc.relay3.setOff();
	  fc.rf1.writeRF(1); //wymuś możliwość wyłączenia wyłącznika RF
	  fc.rf1.sendOff();
	  fc.fan1 = fc.Fan1Speed[0];
	  fc.pwm.write(0, fc.percent2duration(fc.fan1));
	  fc.fan2 = fc.Fan2Speed[0];
	  fc.pwm.write(1, fc.percent2duration(fc.fan2));
	  timeM = fminutes(ManualTime);	// odnów czas trybu MANUAL
  }
    server.send(200, "text/html", WebPage());
  });

 server.on("/relay1/1", []()      // on relay 1
  { if (!fc.bmode) {
    fc.relay3.setOn();
    fc.rf1.writeRF(0); //wymuś możliwość załączenia wyłącznika RF
    fc.rf1.sendOn();
    fc.fan1 = fc.Fan1Speed[1];
    fc.pwm.write(0, fc.percent2duration(fc.fan1));
    fc.fan2 = fc.Fan2Speed[1];
    fc.pwm.write(1, fc.percent2duration(fc.fan2));
    timeM = fminutes(ManualTime); // odnów czas trybu MANUAL
  }
      server.send(200, "text/html", WebPage());
  });

 server.on("/relay2/0", [] ()     // off relay 2
  { if (!fc.bmode) {
	  fc.relay1.setOff();
	  fc.rf2.writeRF(1); //wymuś możliwość wyłączenia wyłącznika RF
	  fc.rf2.sendOff();
	  fc.fan1 = fc.Fan1Speed[1];
	  fc.pwm.write(0, fc.percent2duration(fc.fan1));
	  fc.fan2 = fc.Fan2Speed[1];
	  fc.pwm.write(1, fc.percent2duration(fc.fan2));
	  timeM = fminutes(ManualTime);   // odnów czas trybu MANUAL
  }
   server.send(200, "text/html", WebPage());
  });

 server.on("/relay2/1", []()      // on relay 2
  { if (!fc.bmode) {
	  fc.relay1.setOn();
	  fc.rf2.writeRF(0); //wymuś możliwość załączenia wyłącznika RF
	  fc.rf2.sendOn();
	  fc.fan1 = fc.Fan1Speed[2];
	  fc.pwm.write(0, fc.percent2duration(fc.fan1));
	  fc.fan2 = fc.Fan2Speed[2];
	  fc.pwm.write(1, fc.percent2duration(fc.fan2));
	  timeM = fminutes(ManualTime); // odnów czas trybu MANUAL
  }
    server.send(200, "text/html", WebPage());
  });

 server.on("/relay3/0", [] ()     // off relay 3
  { if (!fc.bmode) {
	 // fc.relay2.setOff();
	  fc.rf3.writeRF(1); //wymuś możliwość wyłączenia wyłącznika RF
	  fc.rf3.sendOff();
	  fc.fan1 = fc.Fan1Speed[2];
	  fc.pwm.write(0, fc.percent2duration(fc.fan1));
	  fc.fan2 = fc.Fan2Speed[2];
	  fc.pwm.write(1, fc.percent2duration(fc.fan2));
	  timeM = fminutes(ManualTime);   // odnów czas trybu MANUAL
  }
    server.send(200, "text/html", WebPage());
  });

 server.on("/relay3/1", []()      // załącz przekaźnik 3
  { if (!fc.bmode) {
	 // fc.relay2.setOn();
	  fc.rf3.writeRF(0); //wymuś możliwość załączenia wyłącznika RF
	  fc.rf3.sendOn();
	  fc.fan1 = fc.Fan1Speed[3];
	  fc.pwm.write(0, fc.percent2duration(fc.fan1));
	  fc.fan2 = fc.Fan2Speed[3];
	  fc.pwm.write(1, fc.percent2duration(fc.fan2));
	  timeM = fminutes(ManualTime); // reset time trybu MANUAL
  }
    server.send(200, "text/html", WebPage());
  });

 server.on("/alarm/0", [] ()     // set alarm off
  {
		 //if (!fc.bmode) {
	  fc.relay2.setOff();
	  fc.alarm=false;

    server.send(200, "text/html", WebPage());
  });

 server.on("/alarm/1", []()      // set alarm on
  {
		 //if (!fc.bmode) {
	  fc.relay2.setOn();
	  fc.alarm= true;

    server.send(200, "text/html", WebPage());
  });

 server.on("/tempmax", []()      // reset temp_max
  { if (!fc.bmode) {
	  fc.temp_max = 20;
	  timeM = fminutes(ManualTime); // odnów czas trybu MANUAL
  }
    server.send(200, "text/html", WebPage());
  });

 server.on("/program/0", []()      // set progran 0
   { if (!fc.bmode) {
 	 fc.program=0;
   }
   server.send(200, "text/html", "program = 0");
  });

 server.on("/program/1", []()      // set program 1
    { if (!fc.bmode) {
  	 fc.program=1;
    }
 server.send(200, "text/html", "program = 1");
   });

 server.on("/alarm", []()      // alarm off / on
    {
	 fc.alarm=!fc.alarm;
  server.send(200, "text/html", ((fc.alarm)?"Alarm On":"Alarm Off"));
  });

server.on("/reboot", []()      // reset system
  {
	if (fc.bmode) return ;
	ESP.restart();
	delay(0);
	server.send(200, "text/html", (F("Reboot system!")));
  });

 server.begin();                // Start server www
#ifdef DEBUG
  Serial.println(F("Server started"));
#endif
 }


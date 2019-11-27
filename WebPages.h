/*
 * WebPages.h
 *
 *  Created on: 3 wrz 2018
 *      Author: jant
 */

#ifndef WEBPAGES_H_
#define WEBPAGES_H_

#include "WebPages.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "FireplaceController.h"
//#include "FSBrowser.h"
String HTMLHeader();
String HTMLFooter();
String HTMLPage();
String HTMLPage1();
String HTMLPage2();
String HTMLInfo();
void setservers(void);
bool is_authentified(void);
void handleLogin(void);
void handleRoot(void);


#endif /* WEBPAGES_H_ */

/*
 * FireplaceController.h
 *
 *  Created on: 3 wrz 2018
 *      Author: jant
 */

#ifndef FIREPLACECONTROLLER_H_
#define FIREPLACECONTROLLER_H_
#include <Relay.h>
#include "../AutomationFireplace/PWM.h"
#include "../AutomationFireplace/RF2260.h"
/* Układ załącza wentylatory obiegu ciepła z kominka
 * Załączenia są zależne od teperatury powietrza w komorze kominka
 * Ustawiane są 3 progi temperatury temp_on1, temp_on2 i temp_on3
 * Załączenie poszczególnych przekaźników następuje po przekroczeniu
 * poszczególnych progów, a wyłączenie przy temperaturach niższych
 * od progów o wartość ustawioną w zmiennej hyster.
 * Progi:
 * 1. załącza przekaźniki DC 3 oraz przekaźniki RF1
 * 2. załącza przekaźnik AC 1 i RF2
 * 3. załącza przekaźnik RF3
 * 4. alarm załącza przekaźmik AC 2
 * Przekaźniki RF to wyłączniki sterowane pilotem na 433MHz. (np. CLARUS)
 * Tryb "MANUAL" umożliwia ręczne sterowanie przekaźników,
 * Tryb "AUTO" - automatyczne sterowanie przekaźnikami
 *                zależnie od temperatury
 */

class FireplaceController {
public:
	float temp_current = 20.0; //reading current temperature
	float temp_in_box = temp_current; //temperature of the air in the fireplace
	float temp_on1 = 35.0; //level 1 of the fan activation
	float temp_on2 = 60.0; //level 2 of the fan activation
	float temp_on3 = 80.0; // level 3 of the fan activation
	float temp_alarm = 100; // start alarm
	float hyster = 5; // hysteresis
	float temp_off = temp_on1;
	float temp_off1 = temp_on1 - hyster; //level 1 switch off the fan
	float temp_off2 = temp_on2 - hyster; //level 2 switch off the fan
	float temp_off3 = temp_on3 - hyster; //level 3 switch off the fan
	float temp_max = temp_current; //remember the last maximum temperature
	int PWMfreq = 5000; //frequency PWM
	long int Fan1Speed[4] = { 0, 55, 80, 100 }; //percent %
	long int Fan2Speed[4] = { 0, 55, 80, 100 };
	int fan1 = Fan1Speed[0];
	int fan2 = Fan2Speed[0];
	int fanx = 0;
	int storey = 0; // temperature dependent level ( 1, 2, 3, 4 = alarm)
	bool bmode = true; // true mode AUTO, false mode MANUAL
	bool alarm = false; // alarm
	bool breadTemp = false;
	bool start_automation = false; // true if star automation (temp_in_box > temp_on1)
	int program = 1;
	unsigned long  timecurrent = 0;
	int timedelay = 30000; //read temperature 30 sek

	RF2260 rf1;   // RF1 switch
	RF2260 rf2;   // RF2 switch
	RF2260 rf3;   // RF2 switch
	Relay relay1; // relay 1
	Relay relay2; // relay 2
	Relay relay3; // alarm ON
	PWM pwm;		//  PWM

	FireplaceController();
	virtual ~FireplaceController();

	void setAuto(void);
	void setManual(void);
	void begin(void);
	void setTemp(float);
	void readTemp(bool);
	float readTempIn(void);
	void working(void);
	bool readMode(void);
	int percent2duration(long int);
	int duration2percent(long int);
	int temp2duration(long int);
	void setFans(int);

};

#endif /* FIREPLACECONTROLLER_H_ */

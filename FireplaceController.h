/*
 * FireplaceController.h
 *
 *  Created on: 3 wrz 2018
 *      Author: jant
 */

#ifndef FIREPLACECONTROLLER_H_
#define FIREPLACECONTROLLER_H_
#include "PWM.h"
#include <Relay.h>
#include "RF2260.h"
 /* Układ załącza wentylatory obiegu ciepła z kominka
 * Załączenia są zależne od teperatury powietrza w komorze kominka
 * Ustawiane są 3 progi temperatury temp_on1, temp_on2 i temp_on3
 * Załączenie poszczególnych przekaźników następuje po przekroczeniu
 * poszczególnych progów, a wyłączenie przy temperaturach niższych
 * od progów o wartość ustawioną w zmiennej hyster.
 * Progi:
 * 1. załącza przekaźniki 1 i 3 oraz przekaźniki RF1 i RF2
 * 2. załącza przekaźnik 2
 * 3. załącza przekaźnik 4
 * Przekaźniki RF to wyłączniki sterowane pilotem na 433MHz. (np. CLARUS)
 * Tryb "MANUAL" umożliwia ręczne sterowanie przekaźników,
 * Tryb "AUTO" - automatyczne sterowanie przekaźnikami
 *                zależnie od temperatury
 */

class FireplaceController {
public:
	float temp_in_box = 20.0; //temperature of the air in the fireplace
	float temp_on1 = 33.0; //level 1 of the fan activation
	float temp_on2 = 53.0; //level 2 of the fan activation
	float temp_on3 = 90.0; // level 3 of the fan activation
	float temp_alarm = 105; // start alarm
	float hyster = 4; // hysteresis
	float temp_off1 = temp_on1 - hyster; //level 1 switch off the fan
	float temp_off2 = temp_on2 - hyster; //level 2 switch off the fan
	float temp_off3 = temp_on3 - hyster; //level 3 switch off the fan
	float temp_max = 20.0; //remember the last maximum temperature
	int PWMfreq = 5000; //frequency PWM
	long int Fan1Speed[4]={0,40,70,100}; //percent %
	long int Fan2Speed[4]={0,40,70,100};
	int fan1 = Fan1Speed[0];
	int fan2 = Fan2Speed[0];
	int fanx = 0;
	bool bmode = true; // true mode AUTO, false mode MANUAL
	bool alarm = true; // alarm
	int program = 1;

	RF2260 rf1;   // RF1 switch
	RF2260 rf2;   // RF2 switch
	RF2260 rf3;   // RF2 switch
	Relay relay1; // relay 1
	Relay relay2; // relay 2
	Relay relay3; // alarm ON
	PWM   pwm;		//  PWM

	FireplaceController();
	virtual ~FireplaceController();

	void setAuto(void);
	void setManual(void);
	void begin(void);
	void setTemp(float);
	void readTemp(void);
	float readTempIn(void);
	void working(void);
	bool readMode(void);
	int percent2duration(long int);
	int duration2percent(long int);
	int temp2duration(long int);
	void setFans(int);

};





#endif /* FIREPLACECONTROLLER_H_ */

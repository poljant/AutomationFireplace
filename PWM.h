/*
 * PWM.h
 *
 *  Created on: 3 wrz 2018
 *      Author: jant
 */

#ifndef PWM_H_
#define PWM_H_

class PWM {
	int channels;
	int channel[3][2];
	int freq;
public:
	PWM();
	virtual ~PWM();
	//void setPin(int);
	void init(int, int, int);
	void init(int, int);
	void init(int);
	void set(void);
	void write(int, int);
	void setFreq(int);
	int read(int);
	void start(void);
	int readFreq(void);
};



#endif /* PWM_H_ */

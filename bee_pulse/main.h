/*
 * main.h
 *
 *  Created on: Dec 16, 2017
 *      Author: ujagaga
 */

#ifndef MAIN_H_
#define MAIN_H_

#define SCR_CLR		BLACK	/* Main background color */
#define TXT_CLR		GREEN	/* Default text color */
#define FRAME_CLR	RED

extern void MAIN_processButtons(TSPoint* touchPoint);
extern void MAIN_timerControl(bool start);

#endif /* MAIN_H_ */

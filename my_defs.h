/*   
 *      my_defs.h
 *   ================
 *   Uwe Berger; 2013
 * 
 * einige Definitionen fuer die Scopeclock
 * 
 * 
 * ---------
 * Have fun!
 *  
 */
#ifndef __MY_TYPES_H__
#define __MY_TYPES_H__ 
 

// Edit-Modi
#define	EDIT_NONE			0
#define	EDIT_HOUR			1
#define	EDIT_MINUTE			2
#define	EDIT_SECOUND		3
#define	EDIT_DAY			4
#define	EDIT_MONTH			5
#define	EDIT_YEAR			6


// Temperatur-Einheit
#define TEMP_CELSIUS		0
#define TEMP_FAHRENHEIT		1


// Datum/Uhrzeit
typedef struct datetime {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t wt;
	uint8_t hour;
	uint8_t minute;
	uint8_t secound;	
} datetime_t;


// Tasten
typedef struct keymap {
	uint8_t			state;
	unsigned long	time;
} keymap_t;


// Temperatur
typedef struct temperature {
	uint32_t celsius;
	uint32_t fahrenheit;
} temperature_t;

// Screensaver
typedef struct screensaver {
	uint8_t x;
	uint8_t y;
	int8_t speed_x;
	int8_t speed_y;
	uint8_t mx;
	uint8_t my;
} screensaver_t;


#endif

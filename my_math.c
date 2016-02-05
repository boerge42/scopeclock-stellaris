/*  
 *    my_math.c  
 * ================
 * Uwe Berger; 2013
 * 
 * 
 * ---------
 * Have fun!
 *  
 */
 
 
#include <stdint.h>
#include "my_math.h"
 
// *********************************  	   
int my_abs(int value)
{
	return value<0 ? -value : value;
}

// *********************************  	   
int my_sign(int value)
{
	return value<0 ? -1 : 1;
}

// *********************************  	   
int my_sin(int degree)
{
	if (degree > 360) degree = degree - 360*(degree/360);
	return sin_tab[degree];
/*
	if  (degree<=90) 					return sin_tab[degree];
	if ((degree>90)&&(degree<=180))		return sin_tab[180 - degree];
	if ((degree>180)&&(degree<=270))	return (-1) * sin_tab[degree - 180];
	if ((degree>270)&&(degree<=360))	return (-1) * sin_tab[360 - degree];
	return 0;
*/
}

// *********************************  	   
int my_cos(int degree)
{
	if (degree > 360) 					degree = degree - 360*(degree/360);
	if  (degree<=90) 					return sin_tab[90-degree];
	if ((degree>90)&&(degree<=180))		return (-1) * sin_tab[degree - 90];
	if ((degree>180)&&(degree<=270))	return (-1) * sin_tab[270-degree];
	if ((degree>270)&&(degree<=360))	return sin_tab[degree - 270];
	return 0;
}

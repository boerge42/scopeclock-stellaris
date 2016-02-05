/*  
 *    scope_graph.c
 *   ================
 *   Uwe Berger; 2013
 * 
 * Grafikroutinen fuer Scopeclock...
 * 
 * 
 * ---------
 * Have fun!
 *  
 */
 
#include <stdint.h>
#include "scope_graph.h"
#include "tux1.xbm"
//#include "tux2.xbm"
//#include "tux3.xbm"

#include "uartstdio.h"



// *********************************  
// http://de.wikipedia.org/wiki/Bresenham-Algorithmus
//	   
void plot_line(int x0, int y0, int x1, int y1)
{
	int dx =  my_abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = -my_abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx+dy, e2; /* error value e_xy */
	for(;;){  /* loop */
		plot_point(x0,y0);
		if (x0==x1 && y0==y1) break;
		e2 = 2*err;
		if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
		if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	}
}

// *********************************  	   
void plot_rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	plot_line (x0, y0, x1, y0);
	plot_line (x1, y0, x1, y1);
	plot_line (x1, y1, x0, y1);
	plot_line (x0, y1, x0, y0);
}
 
// *********************************  	   
void plot_char (uint8_t px, uint8_t py, uint8_t c, uint8_t zoom) 
{
	uint8_t i, x, y, x0, y0;
	uint8_t first = 1;
	x0=y0=0;
	for (i=0; d[c][i][0]<E; i++) {
		if (d[c][i][0] == F) {
			first = 1;
			i++;
		}
		x=d[c][i][0] * zoom;
		y=(6-d[c][i][1]) * zoom;
		if (first==0) {
			plot_line(x0+px, y0+py, x+px, y+py);
		} else {
			first=0;	
		}
		x0=x;
		y0=y;
	}
} 

// *********************************  	   
void digital_clock (uint8_t x, uint8_t y, datetime_t now, uint8_t mark)
{	
	uint8_t tx, rx;

	// Uhrzeit
	tx=x;
	rx=tx;
	plot_char_new(tx, y, now.hour/10, ZOOM_T);
	tx=tx+get_char_width(now.hour/10)*ZOOM_T+DPX;
	plot_char_new(tx, y, now.hour%10, ZOOM_T);
	tx=tx+get_char_width(now.hour%10)*ZOOM_T+DPX;
	if (mark==EDIT_HOUR) {	
		plot_rectangle(	rx, y, tx, y+(get_char_height(0)*ZOOM_T));
	}
	plot_char_new(tx, y, 11, ZOOM_T);
	tx=tx+get_char_width(11)*ZOOM_T+DPX;
	rx=tx;
	plot_char_new(tx, y, now.minute/10, ZOOM_T);
	tx=tx+get_char_width(now.minute/10)*ZOOM_T+DPX;
	plot_char_new(tx , y, now.minute%10, ZOOM_T);
	tx=tx+get_char_width(now.minute%10)*ZOOM_T+DPX;
	if (mark==EDIT_MINUTE) {
		plot_rectangle(	rx, y, tx, y+(get_char_height(0)*ZOOM_T));
	}
	plot_char_new(tx, y, 11, ZOOM_T);
	tx=tx+get_char_width(11)*ZOOM_T+DPX;
	rx=tx;
	plot_char_new(tx, y, now.secound/10, ZOOM_T);
	tx=tx+get_char_width(now.secound/10)*ZOOM_T+DPX;
	plot_char_new(tx, y, now.secound%10, ZOOM_T);
	tx=tx+get_char_width(now.secound%10)*ZOOM_T+DPX;
	if (mark==EDIT_SECOUND) {
		plot_rectangle(	rx, y, tx-1, y+(get_char_height(0)*ZOOM_T));
	}
	// Datum
	tx=x+67;
	rx=tx;
	y=y-(get_char_height(0)*ZOOM_D)-DPY;
	plot_char_new(tx, y, now.day/10, ZOOM_D);
	tx=tx+get_char_width(now.day/10)*ZOOM_D+DPX;
	plot_char_new(tx, y, now.day%10, ZOOM_D);
	tx=tx+get_char_width(now.day%10)*ZOOM_D+DPX;
	if (mark==EDIT_DAY) {
		plot_rectangle(	rx, y, tx-1, y+(get_char_height(0)*ZOOM_D));
	}
	plot_char_new(tx, y, 10, ZOOM_D);
	tx=tx+get_char_width(10)*ZOOM_D+DPX;
	rx=tx;
	plot_char_new(tx, y, now.month/10, ZOOM_D);
	tx=tx+get_char_width(now.month/10)*ZOOM_D+DPX;
	plot_char_new(tx, y, now.month%10, ZOOM_D);
	tx=tx+get_char_width(now.month%10)*ZOOM_D+DPX;
	if (mark==EDIT_MONTH) {
		plot_rectangle(	rx, y, tx-1, y+(get_char_height(0)*ZOOM_D));
	}
	plot_char_new(tx, y, 10, ZOOM_D);
	tx=tx+get_char_width(10)*ZOOM_D+DPX;
	plot_char_new(tx, y, 2, ZOOM_D); // 20xx
	tx=tx+get_char_width(2)*ZOOM_D+DPX;
	plot_char_new(tx, y, 0, ZOOM_D);
	tx=tx+get_char_width(0)*ZOOM_D+DPX;
	rx=tx;
	plot_char_new(tx, y, now.year/10, ZOOM_D);
	tx=tx+get_char_width(now.year/10)*ZOOM_D+DPX;
	plot_char_new(tx, y, now.year%10, ZOOM_D);
	tx=tx+get_char_width(now.year%10)*ZOOM_D+DPX;
	if (mark==EDIT_YEAR) {
		plot_rectangle(	rx, y, tx, y+(get_char_height(0)*ZOOM_D));
	}
}	

// *********************************  	   
void draw_temperature(uint8_t x, uint8_t y, temperature_t t, uint8_t mode)
{
	uint32_t temp;
	uint8_t tx=x;
	
	if (mode==0)
		temp = t.celsius;
	else 
		temp = t.fahrenheit;
	plot_char_new(tx, y, temp/100, ZOOM_TEMP);
	tx=tx+get_char_width(temp/100)*ZOOM_TEMP+DPX;
	temp=temp%100;
	plot_char_new(tx, y, temp/10, ZOOM_TEMP);
	tx=tx+get_char_width(temp/10)*ZOOM_TEMP+DPX;
	plot_char_new(tx, y, 10, ZOOM_TEMP);
	tx=tx+get_char_width(10)*ZOOM_TEMP+DPX;
	plot_char_new(tx, y, temp%10, ZOOM_TEMP);
	tx=tx+get_char_width(temp%10)*ZOOM_TEMP+DPX;
	plot_char_new(tx, y, 12, ZOOM_TEMP);
	tx=tx+get_char_width(12)*ZOOM_TEMP+DPX;
	if (mode==TEMP_CELSIUS) {
		plot_char_new(tx, y, 15, ZOOM_TEMP);
	} else {
		plot_char_new(tx, y, 16, ZOOM_TEMP);
	}
}

// *********************************  	   
void draw_all_pixel(void)
{
	uint16_t i, j;
	for (i=0; i<=255; i++)
		for (j=0; j<=255; j++)
			plot_point(j, i);
}

// *********************************  	   
void analog_clock  (datetime_t now)
{	
	uint8_t x, y;
	int x0,x1,y0,y1,h,d;
	uint8_t i;
	// Minutenpunkte
	for (i=0; i<60; i++) {
		y0 = 127 + (127-5) * my_cos(i*6)/SIN_FACTOR;
		x0 = 127 + (127-5) * my_sin(i*6)/SIN_FACTOR;
		if ((i%5)==0) plot_circle_segm(x0, y0, 3, 0 , 359); else plot_circle_segm(x0, y0, 1, 0 , 359);
	}

	// Sekunden-Zeiger (einfacher Strich vom Mittelpunkt...)
	y0 = 127 + (127-5) * my_cos(now.secound*6)/SIN_FACTOR;
	x0 = 127 + (127-5) * my_sin(now.secound*6)/SIN_FACTOR;
	plot_line(127, 127, x0, y0);
/*
	// Minuten-Zeiger 
	y0 = 127 + (127-15) * my_cos(now.minute*6)/SIN_FACTOR;
	x0 = 127 + (127-15) * my_sin(now.minute*6)/SIN_FACTOR;
	plot_line(127, 127, x0, y0);
	plot_line(127+1, 127+1, x0+1, y0+1);
	plot_line(127-1, 127-1, x0-1, y0-1);
 
	// Stunden-Zeiger 
	if (now.hour > 11) {h = now.hour - 12;} else {h = now.hour;} 
	h = h * 5 + now.minute/12;
	y0 = 127 + (127-40) * my_cos(h)/SIN_FACTOR;
	x0 = 127 + (127-40) * my_sin(h)/SIN_FACTOR;
	plot_line(127, 127, x0, y0);
	plot_line(127+1, 127+1, x0+1, y0+1);
	plot_line(127-1, 127-1, x0-1, y0-1);
*/

	// Minuten-Zeiger
	fat_hand(now.minute, 10);
	// Stunden-Zeiger
	if (now.hour > 11) {h = now.hour - 12;} else {h = now.hour;} 
	h = h * 5 + now.minute/12;
	fat_hand(h, 25);

}


// *********************************  	   
void fat_hand (int val, int dist)
{
	int x0, x1, x2, x3, y0, y1, y2, y3, ir;
	ir = 127-5;
		
	// 0 Grad
	y0 = 127 + (127 - dist) * my_cos(val*6)/SIN_FACTOR;
	x0 = 127 + (127 - dist) * my_sin(val*6)/SIN_FACTOR;
	
	// 180 Grad
	val = val + 30;
	if (val > 59) val = val - 60;
	y2 = (127 - (127-ir) * my_cos(val*6)/SIN_FACTOR);
	x2 = 127 + (127-ir) * my_sin(val*6)/SIN_FACTOR;
	
	// 90 Grad
	val = val + 15;
	if (val > 59) val = val - 60;
	y3 = (127 - (127-ir) * my_cos(val*6)/SIN_FACTOR);
	x3 = 127 + (127-ir) * my_sin(val*6)/SIN_FACTOR;
	
	// 180 Grad
	val = val + 30;
	if (val > 59) val = val - 60;
	y1 = (127 - (127-ir) * my_cos(val*6)/SIN_FACTOR);
	x1 = 127 + (127-ir) * my_sin(val*6)/SIN_FACTOR;
	
	// ...und Zeiger zeichnen	
	plot_line(x0, y0, x1, y1);
	plot_line(x1, y1, x2, y2);
	plot_line(x2, y2, x3, y3);
	plot_line(x3, y3, x0, y0);
}




// *********************************  	 
void draw_xbm(uint8_t x, uint8_t y, uint8_t width, uint8_t height, unsigned char *xbm)
{
	uint8_t i, j, cols;
	uint8_t col_x=x;
	// Anzahl Bytes pro Zeile berechnen
	cols = width/8;
	if (width%8) cols++;
	// ueber jede Zeile...
	for (j=0; j<height; j++) { 
		// ueber jedes Byte einer Zeile...
		for (i=0; i<cols; i++) {
			// ist ueberhaupt ein Bit gesetzt?
			if (*xbm) {
				// ueber jedes Bit des Bytes... (for-Schleife ist langsamer!)
				if (*xbm & 1)   plot_point(col_x, y);	
				col_x++;
				if (*xbm & 2)   plot_point(col_x, y);	
				col_x++;
				if (*xbm & 4)   plot_point(col_x, y);	
				col_x++;
				if (*xbm & 8)   plot_point(col_x, y);	
				col_x++;
				if (*xbm & 16)  plot_point(col_x, y);	
				col_x++;
				if (*xbm & 32)  plot_point(col_x, y);	
				col_x++;
				if (*xbm & 64)  plot_point(col_x, y);	
				col_x++;
				if (*xbm & 128) plot_point(col_x, y);	
				col_x++;
			} else {
				col_x=col_x+8;
			}
			// naechstes Byte aus Bild-Array
			xbm++;
		}
		y--;		// naechste Zeile...
		col_x=x;	// ...beginnt wieder bei x
	}
}



// *********************************  	 
void draw_tux1(uint8_t x, uint8_t y) 
{
	draw_xbm(x, y, tux1_width, tux1_height, tux1_bits);	
//	draw_xbm(x, y, tux2_width, tux2_height, tux2_bits);	
//	draw_xbm(x, y, tux3_width, tux3_height, tux3_bits);	
}


// *********************************  	 
void plot_circle_segm(int x, int y, int radius, int angle_begin, int angle_end)
{
	int i;

	// ein Kreis ist Lissajous-Figur mit:
	// * einer Sinus-Schwingung gleicher Frequenz an x und y
	// * Sinus-Schwingung an y um 90 Grad phasenverschoben 
	for (i=angle_begin; i<=angle_end; i++) {
		plot_point(	x+(long int)(radius*my_sin(i)/SIN_FACTOR), 
					y+(long int)(radius*my_sin(i+90)/SIN_FACTOR));
	}		
}

// *********************************
// Zeichnet eine Lissjous-Figure
//   fx    --> Frequenz x-Kanal
//   fy    --> Frequenz y-Kanal
//   phase --> Phasenverschiebung zw. x- und y-Kanal
void lissajous_figure(int fx, int fy, int phase)
{
	int i;
	for (i=0; i<=360; i++) {
		plot_point(	127+(long int)(127*my_sin(i*fx)/SIN_FACTOR), 
					127+(long int)(127*my_sin(i*fy+phase)/SIN_FACTOR));
	}
}


// *********************************  	 
// Zeichnet eine Lissjous-Figure (x-/y-Kanal gleiche Frequenz)
void plot_lissajous_element(digit_element_t e)
{
	int i;
	if (e.sp & 0b00000001){ 
		for (i=0; i<45; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b00000010){ 
		for (i=45; i<90; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b00000100){ 
		for (i=90; i<135; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b00001000){ 
		for (i=135; i<180; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b00010000){ 
		for (i=180; i<225; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b00100000){ 
		for (i=225; i<270; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b01000000){ 
		for (i=270; i<315; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
	if (e.sp & 0b10000000){ 
		for (i=315; i<360; i++) 
			plot_point(	e.xo+(e.xa*my_sin(i)/SIN_FACTOR), 
						e.yo+(e.ya*my_sin(i+e.phase)/SIN_FACTOR));
	}
}

// *********************************  	 
void plot_char_new(int xo, int yo, uint8_t ci, uint8_t zoom)
{
	digit_element_t e;
	const digit_element_t *ptr;
	
	ptr = c[ci].c;
	e.end=0;
	while (!e.end) {
		e=*ptr;
		e.xo=e.xo*zoom+xo;
		e.yo=e.yo*zoom+yo;
		e.xa=e.xa*zoom;
		e.ya=e.ya*zoom;
		plot_lissajous_element(e);
		ptr++;
	}
}

// *********************************  	 
uint8_t get_char_width(uint8_t ci){
	return c[ci].width;	
}

// *********************************  	 
uint8_t get_char_height(uint8_t ci){
	return c[ci].height;		
}


// *********************************  	 
// Bresenham fuer Kreis...
void plot_circle(int x0, int y0, int radius)
{
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
 
	plot_point(x0, y0 + radius);
	plot_point(x0 + radius, y0);
	plot_point(x0 - radius, y0);
	plot_point(x0, y0 - radius);
	while(x < y) {
		if(f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
 
		plot_point(x0 + x, y0 + y);
		plot_point(x0 - x, y0 + y);
		plot_point(x0 + x, y0 - y);
		plot_point(x0 - x, y0 - y);
		plot_point(x0 + y, y0 + x);
		plot_point(x0 - y, y0 + x);
		plot_point(x0 + y, y0 - x);
		plot_point(x0 - y, y0 - x);
	}
}



/*   
 *    scope_graph.h
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
#ifndef __SCOPE_GRAPH_H__
#define __SCOPE_GRAPH_H__ 
 
#include <stdint.h>
#include "my_defs.h"
#include "my_math.h"
 
 // "Bildschirmaufloesung"
#define MAX_X				255
#define MAX_Y				255
#define DT_X				255/MAX_X
#define DT_Y				255/MAX_Y
//#define R					MAX_X/2 
#define R					100

// Zeichen-Definitionen
#define E			0xFF	// Zeichen Ende
#define F   		0xFE	// "Stift" absetzen
#define DDX 		3		// Zeichenbreite bei Zoom 1
#define DDY 		6		// Zeichenhöhe bei Zoom 1
#define DPX 		2		// Zeichenzwischenraum X
#define DPY 		10		// Zeichenzwischenraum Y
#define ZOOM_T		3		// Zoomfaktor Uhrzeit
#define ZOOM_D		1		// Zoomfaktor Datum
#define ZOOM_TEMP	3		// Zommfaktor Temperatur
static const uint8_t d[17][17][2] = {
	{{0,5},{0,1},{1,0},{2,0},{3,1},{3,5},{2,6},{1,6},{0,5},{3,1},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 0
	{{0,2},{2,0},{2,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 1
	{{0,1},{1,0},{2,0},{3,1},{3,2},{0,6},{3,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 2
	{{0,1},{1,0},{2,0},{3,1},{3,2},{2,3},{3,4},{3,5},{2,6},{1,6},{0,5},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 3
	{{3,3},{0,3},{2,0},{2,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 4
	{{3,0},{0,0},{0,3},{1,2},{2,2},{3,3},{3,5},{2,6},{1,6},{0,5},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 5
	{{3,1},{2,0},{1,0},{0,1},{0,5},{1,6},{2,6},{3,5},{3,4},{2,3},{1,3},{0,4},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 6
	{{0,0},{3,0},{3,1},{0,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 7
	{{1,3},{0,2},{0,1},{1,0},{2,0},{3,1},{3,2},{2,3},{1,3},{0,4},{0,5},{1,6},{2,6},{3,5},{3,4},{2,3},{E,E}},	// 8
	{{0,5},{1,6},{2,6},{3,5},{3,1},{2,0},{1,0},{0,1},{0,2},{1,3},{2,3},{3,2},{E,E},{E,E},{E,E},{E,E},{E,E}},	// 9
	{{1,6},{2,6},{2,5},{1,5},{1,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// .
	{{1,6},{2,6},{2,5},{1,5},{1,6},{F,F},{1,3},{2,3},{2,2},{1,2},{1,3},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// :
	{{1,0},{2,0},{2,1},{1,1},{1,0},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// °
	{{2,0},{1,1},{1,5},{2,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// (
	{{1,0},{2,1},{2,5},{1,6},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// )
	{{3,1},{2,0},{1,0},{0,1},{0,5},{1,6},{2,6},{3,5},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}},	// C
	{{0,6},{0,0},{3,0},{F,F},{0,2},{2,2},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E},{E,E}}		// F
};

// Definition Zeichen (neu)
typedef struct {
	int phase;		// Phasenverschiebung zwischen x und x
	int xo;			// Offset x
	int yo;			// Offset y
	int xa;			// Aplitudenhoehe x
	int ya;			// Aplitudenhoehe y
	uint8_t sp; 	// Kreissegmentpattern
	uint8_t end;	// letztes Element des Zeichen? (!=0 --> ja)
} digit_element_t;	

typedef struct {
	const digit_element_t * c;
	uint8_t height;
	uint8_t width;
} c_list_t;


// Blank
static const digit_element_t c_blank[] = {
	{0, 0, 0, 0, 0, 0, 1},				
};

// 0
static const digit_element_t c_0[] = {
	{90, 6, 10, 6, 10, 255, 1},				// Kreis, Elipse	
};

// 1
static const digit_element_t c_1[] = {
	{0, 6, 17, 3, 3, 0b00111100, 0},		// schraeger Strich (rechts)
	{90, 9, 10, 0, 10, 0b00001111, 1}		// senkrechter Strich
};

// 2
static const digit_element_t c_2[] = {
	{90, 6, 14, 6, 6, 0b11001111, 0},		// Kreissegment
	{90, 6, 0, 6, 8, 0b11000000, 0},		// Kreissegment
	{90, 6, 0, 6, 0, 0b00111100, 1}			// waagerechter Strich 
};

// 3
static const digit_element_t c_3[] = {
	{90, 6, 6, 6, 6, 0b00011111, 0},		// Keissegment	
	{0, 9, 16, 3, 4, 0b00111100, 0},		// schraeger Strich (rechts)
	{90, 7, 20, 5, 0, 0b00111100, 1}		// waagerechter Strich
};

// 4
static const digit_element_t c_4[] = {
	{90, 6, 6, 6, 0, 0b00111100, 0},		// waagerechter Strich
	{0, 5, 13, 5, 7, 0b00111100, 0},		// schraeger Strich (rechts)
	{90, 10, 10, 0, 10, 0b00001111, 1}		// senkrechter Strich
};

// 5
static const digit_element_t c_5[] = {
	{90, 6, 6, 6, 6, 0b10011111, 0},		// Kreissegment	
	{90, 8, 20, 4, 0, 0b00111100, 0},		// waagerechter Strich
	{0, 3, 15, 1, 5, 0b00111100, 1}			// schraeger Strich (rechts)
};

// 6
static const digit_element_t c_6[] = {
	{90, 6, 6, 6, 6, 255, 0},				// Kreis		
	{0, 5, 15, 3, 5, 0b00111100, 1}			// schraeger Strich (rechts)
};

// 7
static const digit_element_t c_7[] = {
	{0, 6, 10, 6, 10, 0b00111100, 0},		// schraeger Strich (rechts)
	{90, 6, 20, 6, 0, 0b00111100, 1}		// waagerechter Strich
};

// 8
static const digit_element_t c_8[] = {
	{90, 6, 16, 4, 4, 255, 0},				// Kreis
	{90, 6, 6, 6, 6, 255, 1}				// Kreis
};

// 9
static const digit_element_t c_9[] = {
	{90, 6, 14, 6, 6, 255, 0},				// Kreis
	{0, 7, 5, 3, 5, 0b00111100, 1}			// schraeger Strich (rechts) 
};

// Punkt
static const digit_element_t c_point[] = {
	{90, 2, 1, 1, 1, 255, 1}				// Kreis
};

// Doppelpunkt
static const digit_element_t c_dpoint[] = {
	{90, 2, 10, 1, 1, 255, 0},				// Kreis
	{90, 2, 1, 1, 1, 255, 1}				// Kreis
};

// Grad-Zeichen
static const digit_element_t c_degree[] = {
	{90, 4, 16, 3, 3, 255, 1}				// Kreis
};

// C
static const digit_element_t c_C[] = {
	{90, 6, 10, 6, 10, 0b11111001, 1}		// Kreissegment
};

// F
static const digit_element_t c_F[] = {
	{90, 0, 10, 0, 10, 0b00001111, 0},		// senkrechter Strich
	{90, 6, 20, 6, 0, 0b00111100, 0},		// waagerechter Strich 
	{90, 5, 12, 5, 0, 0b00111100, 1}		// waagerechter Strich 
};


static const c_list_t c[] = {
	{c_0, 20, 12}, {c_1, 20, 12}, {c_2, 20, 12},			// 0 1 2
	{c_3, 20, 12}, {c_4, 20, 12}, {c_5, 20, 12}, 			// 3 4 5
	{c_6, 20, 12}, {c_7, 20, 12}, {c_8, 20, 12}, 			// 6 7 8
	{c_9, 20, 12},											// 9
	{c_point, 20, 4}, {c_dpoint, 20, 4}, {c_degree, 20, 8},	// . : °
	{c_blank, 20, 12}, {c_blank, 20, 12},					//
	{c_C, 20, 12}, {c_F, 20, 12}						 	// C F
};


// *********************************
// richtet sich nach der Hardware, deshalb woanders zu definieren
extern void plot_point(uint8_t x, uint8_t y);

// Prototypen
void plot_line(int x0, int y0, int x1, int y1);
void plot_rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void plot_char (uint8_t px, uint8_t py, uint8_t c, uint8_t zoom);
void fat_hand (int val, int dist);
void digital_clock (uint8_t x, uint8_t y, datetime_t now, uint8_t mark);
void analog_clock  (datetime_t now);
//void draw_xbm(uint8_t x, uint8_t y, uint8_t width, uint8_t height, unsigned char xbm[]);
void draw_tux1(uint8_t x, uint8_t y);
void draw_temperature(uint8_t x, uint8_t y, temperature_t temp, uint8_t mode);
void draw_all_pixel(void);
void plot_circle(int x0, int y0, int radius);
void plot_circle_segm(int x, int y, int radius, int angle_begin, int angle_end);
void lissajous_figure(int fx, int fy, int phase);
void plot_char_new(int xo, int yo, uint8_t c, uint8_t zoom);
uint8_t get_char_width(uint8_t c);
uint8_t get_char_height(uint8_t c);

 
#endif

/*  
 *      Scopeclock
 *   ================
 *   Uwe Berger; 2013
 * 
 * Hardware:
 * ---------
 * 
 * --> Stellaris Launchpad von TI
 * 
 *     Taster an PF0 und PF4 werden zur Steuerung der Uhr
 *     verwendet...
 *     -Taster SW1 (PF4):
 *      -im Anzeige-Mode;
 *       Digital-Uhr -> Analog-Uhr -> ...
 *      -im Edit-Mode;
 *       angewaehlte Stelle von Zeit/Datum hochzaehlen
 *     -Taster SW2 (PF0):
 *      -im Digital-Uhr-Anzeige:
 *        Anzeige-Mode -> Edit Stunde -> Edit Minute -> Edit Sekunde
 *        -> Edit Tag -> Edit Monat -> Edit Jahr
 * 
 *     interner Temperatursensor (ADC0)
 *        
 * 
 * --> Digital-Analog-Wandler TLC7528 an 5V
 * 
 * 		Launchpad | TLC7528
 * 		----------+-----------
 *      PB0..7   --> DB0..7
 * 		PE1      --> DACA/DACB
 *      PE2      --> WR
 *      PE3      --> CS
 * 
 * 		-TCL7528 im "Voltage Mode" beschaltet (siehe Datenblatt)
 * 		-generierte Spannungen fuer DAC-Kanal A und B entsprechend 
 *		 an X- und Y-Eingang des Oszilloskop eingespeist...
 * 
 * 
 * --> RTC DS1307 (batteriegepuffert) an 5V
 * 
 * 		DS1307  | Launchpad 
 *      --------+----------
 * 		SCL    --> PA6      \
 * 		SDA   <--> PA7       - jeweils 4k7-Widerstand gegen 3,3V
 * 		SQE    --> PA5      /
 * 
 * 
 * 
 * ---------
 * Have fun!
 *  
 */
#include <stdint.h> 

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"

#include "my_defs.h"
#include "API_i2c.h"
#include "my_math.h"
#include "scope_graph.h"
#include "uartstdio.h"
#include "ustdlib.h" 


#define PERF		0		// mit Performance-Messung und Ergebnisausgabe an UART(0)
#define THROTTLE	0		// "Pixelbremse"

//     
// Hardware-Kofiguration
//

// --> Daten-Port DAC
#define DATA_OUT			GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
#define DATA_PORT_BASE		GPIO_PORTB_BASE
#define DATA_SYSCTL_PERIPH	SYSCTL_PERIPH_GPIOB

// --> Steuerleitungen DAC
#define DAC_PORT_BASE		GPIO_PORTE_BASE
#define DAC_SYSCTL_PERIPH	SYSCTL_PERIPH_GPIOE
#define DAC_AB				GPIO_PIN_1
#define DAC_WR				GPIO_PIN_2
#define DAC_CS				GPIO_PIN_3
#define DAC_CH_A			0
#define DAC_CH_B			DAC_AB

// --> I2C-Adresse DS1307
#define ADDR_DS1307			0x68

// --> LEDs auf Launchpad
#define LED_PORT_BASE		GPIO_PORTF_BASE
#define LED_SYSCTL_PERIPH	SYSCTL_PERIPH_GPIOF
#define LED_GREEN			GPIO_PIN_3
#define LED_RED				GPIO_PIN_1
#define LED_BLUE			GPIO_PIN_2
#define LED_OUT				LED_GREEN|LED_RED|LED_BLUE

// Taster
#define BUTTON_PORT_BASE		GPIO_PORTF_BASE
#define BUTTON_SYSCTL_PERIPH	SYSCTL_PERIPH_GPIOF
#define BUTTON_0				GPIO_PIN_4
#define BUTTON_1				GPIO_PIN_0
#define DEBOUNCE_TIME			0.2					// 200ms

// Input-Pin fuer Sekundentakt
#define CLOCK_IN_PORT_BASE		GPIO_PORTA_BASE
#define CLOCK_IN_SYSCTL_PERIPH	SYSCTL_PERIPH_GPIOA
#define CLOCK_IN_PIN			GPIO_PIN_5

// SysTick
#define SYSTICK_HZ			1000	// 1kHz

// Datum/Uhrzeit
volatile datetime_t now;
const uint8_t day_of_month[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char rtc_buffer[8]={0x00,0x50,0x20,0x04,0x30,0x10,0x13,0x00};

// Tasten
#define BUTTON_COUNT	2
volatile keymap_t key[BUTTON_COUNT];

// Anzeigemodi
#define MAX_DISPLAYMODE				6
#define DIGITAL_DISPLAY				0
#define ANALOG_DISPLAY				1
#define XBM_DISPLAY					2
#define TEMPERATURE_DISPLAY			3
#define SCREENSAVER_DISPLAY			4
#define LISSAJOUS_DISPLAY			5
#define TEST_DISPLAY				6
#define DEFAULT_DISPLAY				SCREENSAVER_DISPLAY
#define DEFAULT_DISPLAY_TIME 		60				// Sekunden
volatile uint8_t display_mode = DIGITAL_DISPLAY;
volatile uint32_t default_display_time = DEFAULT_DISPLAY_TIME * SYSTICK_HZ;

// Lissajous-Figur
#define LISSAJOUS_REFRESH_TIME	0.07					// Sekunden
volatile int lissajous_phase = 0;

// Modi zur Einstellung von Datum/Uhrzeit
#define MAX_EDITMODE		6
uint8_t edit_mode = EDIT_NONE;

// Temperaturmessung
#define MCU_TEMP_INTERVAL			1				// Sekunden
volatile temperature_t mcu_temp;
#define DEFAULT_TEMP				TEMP_CELSIUS
uint8_t temp_mode = DEFAULT_TEMP;

// Screensaver
#define SCREENSAVER_REFRESH_TIME	0.03			// Sekunden
#define SCREENSAVER_SPEED_MAX		3
#define SCREENSAVER_BALLS			23
volatile uint8_t calc_screensaver = 0;
screensaver_t ball[SCREENSAVER_BALLS];


#if PERF
// Performance-Messung (Pixel und Zeit pro Bild)
// SYSTICK_HZ=10000 sinnvoller Wert, um hinreichend genaue
// und auswertbare Zeiten zu bekommen...
#undef SYSTICK_HZ
#define SYSTICK_HZ	10000
volatile uint32_t sys_counter = 0;
uint32_t start, print = 0;
uint16_t pixel_counter = 0;
#define PERF_OUTPUT_INTERVAL	1					// Sekunden
#endif

#if THROTTLE
#define THROTTLE_FRQ		50000					// Hz
volatile uint8_t throttle = 1;
#endif

// Forwards...
void plot_point(uint8_t x, uint8_t y);
static void int_handler_ds1307_clock(void);
static void int_handler_button(void);
static void int_handler_systick(void);
static void int_handler_adc0(void);
#if THROTTLE
static void int_handler_throttle_timer(void);
#endif


// *********************************  	   
void plot_point(uint8_t x, uint8_t y)
{
	static uint8_t xo, yo;
	// wenn gleiche Koordinaten, wie vorher, dann nicht zeichnen!
	if ((xo==x) && (yo==y)) return;
#if THROTTLE
	// "Pixelbremse"
	while (throttle==0);
	throttle=0;
#endif
	// x
	GPIOPinWrite(DAC_PORT_BASE, DAC_AB, 0);
	GPIOPinWrite(DATA_PORT_BASE, DATA_OUT, x*DT_X);
	GPIOPinWrite(DAC_PORT_BASE, DAC_WR, 0);
	GPIOPinWrite(DAC_PORT_BASE, DAC_WR, DAC_WR);
	// y
	GPIOPinWrite(DAC_PORT_BASE, DAC_AB, DAC_AB);
	GPIOPinWrite(DATA_PORT_BASE, DATA_OUT, y*DT_Y);
	GPIOPinWrite(DAC_PORT_BASE, DAC_WR, 0);
	GPIOPinWrite(DAC_PORT_BASE, DAC_WR, DAC_WR);
	// gezeichnete Koordinaten merken
	xo=x;
	yo=y;
#if PERF
	pixel_counter++;
#endif
}

// *********************************  	   
static uint8_t getkey(uint8_t i)
{
	if (i<BUTTON_COUNT) {
		if (key[i].state && !key[i].time) {
			key[i].state=0;
			return 1;	
		}
	}
	return 0;
}

// ********************************* 
static uint8_t bcd2int(uint8_t val)
{
	return ((val>>4)*10+(val&15));	
}

// ********************************* 
static uint8_t int2bcd(uint8_t val)
{
	return ((val/10)<<4)|(val % 10);
	
}

// *********************************  	   
static void init_uart(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
				(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTStdioInit(0);
}

// *********************************  	   
static void init_gpio(void)
{
#if 1
	// LEDs
	SysCtlPeripheralEnable(LED_SYSCTL_PERIPH);
	GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED_OUT);
	GPIOPinWrite(GPIO_PORTF_BASE, LED_OUT, 0);
#endif
	// Taster 1
	SysCtlPeripheralEnable(BUTTON_SYSCTL_PERIPH);
	GPIOPinTypeGPIOInput(BUTTON_PORT_BASE, BUTTON_0);
	GPIOPadConfigSet(BUTTON_PORT_BASE, BUTTON_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	// Taster 2 (hat Spezialfunktion..., diese ausschalten) 
	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR)  |= 0x01;
	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = 0;
	GPIOPinTypeGPIOInput(BUTTON_PORT_BASE, BUTTON_1);
	GPIOPadConfigSet(BUTTON_PORT_BASE, BUTTON_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	// Tasten-Interrupt-Handler definieren
	GPIOPortIntRegister(BUTTON_PORT_BASE, int_handler_button);
	GPIOIntTypeSet(BUTTON_PORT_BASE, BUTTON_0|BUTTON_1, GPIO_FALLING_EDGE);
	GPIOPinIntEnable(BUTTON_PORT_BASE, BUTTON_0|BUTTON_1);
		// DAC-DATA-Port
	SysCtlPeripheralEnable(DATA_SYSCTL_PERIPH);
	GPIOPinTypeGPIOOutput(DATA_PORT_BASE, DATA_OUT);
	// DAC-Control-Pins
	SysCtlPeripheralEnable(DAC_SYSCTL_PERIPH);
	GPIOPinTypeGPIOOutput(DAC_PORT_BASE, DAC_AB|DAC_WR|DAC_CS);
	// ...initial CS=Low; WR=Hight
	GPIOPinWrite(DAC_PORT_BASE, DAC_WR|DAC_CS, DAC_WR);
	// Interrupt zu Sekundensignal von DS1307 an CLOCK_IN_PIN
	SysCtlPeripheralEnable(CLOCK_IN_SYSCTL_PERIPH);
	GPIOPortIntRegister(CLOCK_IN_PORT_BASE, int_handler_ds1307_clock);
	GPIOPinTypeGPIOInput(CLOCK_IN_PORT_BASE, CLOCK_IN_PIN);
	GPIOIntTypeSet(CLOCK_IN_PORT_BASE, CLOCK_IN_PIN, GPIO_RISING_EDGE);
	GPIOPinIntEnable(CLOCK_IN_PORT_BASE, CLOCK_IN_PIN);
}

// *********************************  	   
static void init_adc(void)
{
	// ADC0 fuer Temperaturmessung initialisieren
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ADCSequenceDisable(ADC0_BASE, 1);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);
	ADCIntEnable(ADC0_BASE, 1);
	ADCIntRegister(ADC_BASE, 1, int_handler_adc0);	
}

// *********************************  	   
static void init_i2c(void)
{
	// I2C-1 initialisieren
	I2CSetup(I2C1_MASTER_BASE, false);
}

// *********************************  	   
static void init_systick(void)
{
	// SysTick initialisieren
	SysTickIntRegister(int_handler_systick);
	unsigned long clk=(ROM_SysCtlClockGet()/SYSTICK_HZ)-1;
    SysTickPeriodSet(clk);
    SysTickEnable();
    SysTickIntEnable();	
}

// *********************************  	   
static screensaver_t calc_screensaver_xy(screensaver_t sc)
{
	sc.x += sc.speed_x;
	sc.y += sc.speed_y;
	// rechts/links	
	if (!(sc.x > (sc.mx+1)) || !(sc.x < (MAX_X-sc.mx-1))) {
		sc.x += -sc.speed_x;
		sc.speed_x = -(my_sign(sc.speed_x)*(my_abs(urand()%SCREENSAVER_SPEED_MAX)+1));
	}
	// oben/unten
	if (!(sc.y > (sc.my+1)) || !(sc.y < (MAX_Y-sc.my-1))) {
		sc.y += -sc.speed_y;
		sc.speed_y = -(my_sign(sc.speed_y)*(my_abs(urand()%SCREENSAVER_SPEED_MAX)+1));
	}
	sc.x += sc.speed_x;
	sc.y += sc.speed_y;
	return sc;
}


#if THROTTLE
// *********************************  	   
static void init_throttle_timer(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
	unsigned long clk = (ROM_SysCtlClockGet()/THROTTLE_FRQ)-1;
	TimerLoadSet(TIMER0_BASE, TIMER_A, clk);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntRegister(TIMER0_BASE, TIMER_A, int_handler_throttle_timer);
	TimerEnable(TIMER0_BASE, TIMER_A);	
}
#endif

// *********************************  	   
// *********************************  	   
// *********************************  	   
int main()
{
	uint8_t x=100;
	uint8_t y=70;

	// Systemtakt 80MHz...
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	init_gpio();
	init_systick();
	init_i2c();
	init_adc();
	init_uart();
	

#if THROTTLE
	init_throttle_timer();
#endif

	// DS1307 --> SQW-Ausgang im Sekundentakt: Register 0x07 --> 0b00010000
	// (siehe Datenblatt!)
	if (!I2CRegWrite(I2C1_MASTER_BASE, ADDR_DS1307, 0x07, 0x10)) {
		// im Fehlerfall rote LED
		GPIOPinWrite(LED_PORT_BASE, LED_RED, LED_RED);
	}
	// aktuelle Zeit aus DS1307 auslesen	
	if (!I2CReadData(I2C1_MASTER_BASE, ADDR_DS1307, 0, rtc_buffer, 8)) {
		// im Fehlerfall rote LED
		GPIOPinWrite(LED_PORT_BASE, LED_RED, LED_RED);
	} 
	now.year    = bcd2int(rtc_buffer[6]);
	now.month   = bcd2int(rtc_buffer[5]);
	now.day     = bcd2int(rtc_buffer[4]);
	now.wt      = bcd2int(rtc_buffer[3]);	
	now.hour    = bcd2int(rtc_buffer[2]);
	now.minute  = bcd2int(rtc_buffer[1]);
	now.secound = bcd2int(rtc_buffer[0]);

	// Anfangswerte Screensaver-Baelle initialisieren
	usrand(now.secound);	
	for (uint8_t i=0;i<SCREENSAVER_BALLS;i++) {
		ball[i].x=100;
		ball[i].y=150;
		ball[i].mx=10;
		ball[i].my=10;
		ball[i].speed_x = my_abs((urand()%SCREENSAVER_SPEED_MAX))+1;
		ball[i].speed_y = my_abs((urand()%SCREENSAVER_SPEED_MAX))+1;
	}

	// Interrupte generell einschalten
	IntMasterEnable();

	UARTprintf("\033[2J\033[HScopeClock; Uwe Berger, 2013\n");

	while (1) {

#if PERF
		// Start Performace-Messung		
		start = sys_counter;	
		pixel_counter=0;
#endif

		// welche Anzeigeart darstellen?
		switch (display_mode) {
			case ANALOG_DISPLAY:
				analog_clock(now);
				break;
			case DIGITAL_DISPLAY:
				x=0;
				y=130;
				digital_clock(x, y, now, edit_mode);		
				break;
			case XBM_DISPLAY:
				//draw_penguin(0, 255, 3);
				draw_tux1(0, 255);
				break;
			case TEMPERATURE_DISPLAY:
				x=33;
				y=98;
				draw_temperature(x, y, mcu_temp, temp_mode);
				break;
			case SCREENSAVER_DISPLAY:
				// Baelle zeichnen
				for (uint8_t i=0; i<SCREENSAVER_BALLS; i++) {
					plot_rectangle(ball[i].x-ball[i].mx, ball[i].y-ball[i].my, ball[i].x+ball[i].mx, ball[i].y+ball[i].my);
				}
				// Ball-Positionen neu berechnen
				if (calc_screensaver) {
					for (uint8_t i=0; i<SCREENSAVER_BALLS; i++) {
						ball[i]=calc_screensaver_xy(ball[i]);
					}
					calc_screensaver=0;
				}
				break;
			case LISSAJOUS_DISPLAY:
				lissajous_figure(3, 4, lissajous_phase);
				break;
			case TEST_DISPLAY:
				//plot_circle_segm(127, 127, 100, 0, 360);
				//plot_char_new(100, 100, 0);
				
				plot_char_new(0,   180, 0, 3);
				plot_char_new(40,  180, 1, 3);
				plot_char_new(80,  180, 2, 3);
				plot_char_new(120, 180, 3, 3);
				plot_char_new(160, 180, 4, 3);
				plot_char_new(200, 180, 5, 3);
			
				plot_char_new(0,   110, 6, 3);
				plot_char_new(40,  110, 7, 3);
				plot_char_new(80,  110, 8, 3);
				plot_char_new(120, 110, 9, 3);
				
				plot_char_new(0,   40, 10, 3);
				plot_char_new(40,  40, 11, 3);
				plot_char_new(80,  40, 12, 3);
				plot_char_new(120, 40, 15, 3);
				plot_char_new(160, 40, 16, 3);
				
				break;
		}
	
		// Tasten gedrueckt?
		if (getkey(0)) {
			if (!edit_mode) {
				display_mode++;
				if (display_mode > MAX_DISPLAYMODE) display_mode=0;
				if (display_mode != DEFAULT_DISPLAY) 
					default_display_time = DEFAULT_DISPLAY_TIME * SYSTICK_HZ;
				else 
					default_display_time = 0;
			} else {
				switch (edit_mode) {
					case EDIT_SECOUND:
						now.secound=0;
						break;	
					case EDIT_MINUTE:
						now.minute++;
						if (now.minute>59) now.minute=0;
						break;	
					case EDIT_HOUR:
						now.hour++;
						if (now.hour>23) now.hour=0;
						break;	
					case EDIT_DAY:
						now.day++;
						if (now.day>31) now.day=1;
						break;	
					case EDIT_MONTH:
						now.month++;
						if (now.month>12) now.month=1;
						break;	
					case EDIT_YEAR:
						now.year++;
						if (now.year>99) now.year=0;
						break;	
				}
				// in RTC schreiben...
				rtc_buffer[6] =	int2bcd(now.year);
				rtc_buffer[5] =	int2bcd(now.month);
				rtc_buffer[4] =	int2bcd(now.day);
				rtc_buffer[3] =	int2bcd(now.wt);
				rtc_buffer[2] =	int2bcd(now.hour);
				rtc_buffer[1] =	int2bcd(now.minute);
				rtc_buffer[0] =	int2bcd(now.secound);
				if (!I2CWriteData(I2C1_MASTER_BASE, ADDR_DS1307, 0, rtc_buffer, 7)) {
					// im Fehlerfall rote LED
					GPIOPinWrite(LED_PORT_BASE, LED_RED, LED_RED);
				} 
			}
		}
		if (getkey(1)) {
			if (display_mode==DIGITAL_DISPLAY) {
				edit_mode++;
				if (edit_mode>MAX_EDITMODE) edit_mode=0;
			}
			if (display_mode==TEMPERATURE_DISPLAY) {
				if (temp_mode==TEMP_CELSIUS)
					temp_mode=TEMP_FAHRENHEIT;
				else
					temp_mode=TEMP_CELSIUS;
			}
		}

#if PERF		
		// Ende/Ausgabe Performace-Messung		
		if (print) {	
			UARTprintf("%i --> %i\n", (sys_counter-start), pixel_counter);
			print=0;
		}
#endif
				
	} // while...
}

// *********************************  	   
static void int_handler_systick(void)
{
	static uint32_t counter=0;
	
	counter++;

#if PERF	
	sys_counter++;
	if (!(counter%(PERF_OUTPUT_INTERVAL*SYSTICK_HZ))) {
		print = 1;
	}
#endif	

	// Tasten behandeln (eventuelle Entprellung)
	for (uint8_t i=0; i<BUTTON_COUNT; i++) {
		if (key[i].time) {
			key[i].time--;
			if (!key[i].time) key[i].state=1;
		}
	}
	
	// ADCO zyklisch anstossen
	if (!(counter%(MCU_TEMP_INTERVAL*SYSTICK_HZ))) {
		ADCProcessorTrigger(ADC0_BASE, 1);
	}
	
	// Screensaver-Berechnung zyklisch anstossen
	if (!(counter%(uint32_t)(SCREENSAVER_REFRESH_TIME*SYSTICK_HZ))) {
		calc_screensaver = 1;
	}
	
	// Screensaver-Berechnung zyklisch anstossen
	if (!(counter%(uint32_t)(LISSAJOUS_REFRESH_TIME*SYSTICK_HZ))) {
		lissajous_phase++;
		if (lissajous_phase==361) lissajous_phase=0;
	}

	// Default-Mode-Timer
	if (default_display_time) {
		default_display_time--;
		if (!default_display_time)	display_mode = DEFAULT_DISPLAY;
	}
	
}

// *********************************  	   
static void int_handler_button(void)
{
	// Interrupt loeschen
	GPIOPinIntClear(BUTTON_PORT_BASE, BUTTON_0|BUTTON_1);
	// Taste 1?
	if(!GPIOPinRead(BUTTON_PORT_BASE, BUTTON_0) && !key[0].time && !key[0].state) {
			key[0].time=DEBOUNCE_TIME * SYSTICK_HZ;
	}			
	// Taste 2?
	if(!GPIOPinRead(BUTTON_PORT_BASE, BUTTON_1) && !key[1].time && !key[1].state) {
			key[1].time=DEBOUNCE_TIME * SYSTICK_HZ;
	}			
}

// *********************************  	   
static void int_handler_adc0(void)
{
	uint32_t adc_val[4];
	uint32_t temp_avg;
	ADCIntClear(ADC0_BASE, 1);
	ADCSequenceDataGet(ADC0_BASE, 1, adc_val);
	temp_avg = (adc_val[0] + adc_val[1] + adc_val[2] + adc_val[3] + 2)/4;
	mcu_temp.celsius = (1475 - ((2475 * temp_avg)) / 4096);
	mcu_temp.fahrenheit = ((mcu_temp.celsius * 9) + 160) / 5;
}

#if THROTTLE
// *********************************  	   
static void int_handler_throttle_timer(void)
{
	// Interrupt loeschen
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	throttle = 1;
}
#endif

// *********************************  	   
static void int_handler_ds1307_clock(void)
{
	// Interrupt loeschen
	GPIOPinIntClear(CLOCK_IN_PORT_BASE, CLOCK_IN_PIN);

#if 0	
	// Blaue LED schalten...	
	if(GPIOPinRead(LED_PORT_BASE, LED_GREEN)) {
		GPIOPinWrite(LED_PORT_BASE, LED_GREEN, 0);
	} else {
		GPIOPinWrite(LED_PORT_BASE, LED_GREEN, LED_GREEN);
	}
#endif

	// Datum/Zeit hochzaehlen
	now.secound++;
	if (now.secound>59) {
		now.secound=0;
		now.minute++;
		if (now.minute>59) {
			now.minute=0;
			now.hour++;
			if (now.hour>23) {
				now.hour=0;
				now.day++;
				if (now.day>day_of_month[now.month-1]) {
					now.day=1;
					now.month++;
					if (now.month>12) {
						now.month=1;
						now.year++;
					}
				}
			}
		}
	}
}

/******  Includes  *******************************************************************************/
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>

#include	<dht.h>																		// DHT11 temperature/humidity sensor libraries

#include	<Wire.h>																	// I2C communication libraries

#include	<Adafruit_GFX.h>															// SSD1306 OLED display module libraries
#include	<Adafruit_SSD1306.h>														// SSD1306 OLED display module libraries
/******  Macros  *********************************************************************************/
#define		DHT11_PIN				A0													// Analog input channel for DHT11 sensor
#define		MQ6_PIN					A1													// Analog input channel for MQ-6 sensor
#define		MQ8_PIN					A2													// Analog input channel for MQ-8 sensor
#define		MQ135_PIN				A3													// Analog input channel for MQ-135 sensor

#define		RL_VALUE				(20)												// (kOhm) Board load resistance
#define		R0_CLEAN_AIR_FACTOR		(10)												// (RS/R0)

#define		CALIB_SAMPLE_AMT		(100)												// Number of calibration samples to take
#define		CALIB_SAMPLE_INTRVL		(3)													// (ms) Amount of time to take each calibration sample
#define		READ_SAMPLE_AMT			(100)												// Number of read samples to take 
#define		READ_SAMPLE_INTRVL		(3)													// (ms) Amount of time to take each read sample

#define		GAS_LPG					(0)
#define		GAS_CH4					(1)
#define		GAS_H2					(2)
#define		GAS_Alcohol				(3)
#define		GAS_CO					(4)
#define		GAS_CO2					(5)
#define		GAS_NH4					(6)

#define		SSD1306_ADDR			0x3C												// 
#define		SSD1306_SCREEN_WIDTH	128													// (px) SSD1306 OLED display width
#define		SSD1306_SCREEN_HEIGHT	64													// (px) SSD1306 OLED display height
#define		SSD1306_RESET			-1													// -1 to share Arduino reset pin
/******  SSD1306 Globals  ************************************************************************/
Adafruit_SSD1306 SSD1306_Display(SSD1306_SCREEN_WIDTH, SSD1306_SCREEN_HEIGHT, &Wire, SSD1306_RESET);
/******  DHT11 Globals  **************************************************************************/
dht			DHT11;
float		CEL_Temp_Current;
float		FAR_Temp_Current;
float		HUM_Current;
/******  MQ-6 Globals  ***************************************************************************/
float		MQ6_LPG_Function[3]		= {3, 0,  -0.39794000867};							// (log(1000), log(1), log(0.4))
float		MQ6_CH4_Function[3]		= {3.30102999566, 0,  -0.15490195998};				// (log(2000), log(1), log(0.7))
float		MQ6_R0					= 10;
/******  MQ-135 Globals  *************************************************************************/
float		MQ135_CO_Function[3]	= {2.30102999566, 0.16136800223, 0.29666519026};	// (log(200), log(1.45), log(1.98))
float		MQ135_NH4_Function[3]	= {2.30102999566, -0.10790539731, 0};				// (log(200), log(0.78), log(1))
float		MQ135_R0				= 10;
/******  MQ-8 Globals  ***************************************************************************/
float		MQ8_H2_Function[3]		= {3, -0.0087739243, -1.52287874528};				// (log(1000), log(0.98), log(0.03))
float		MQ8_R0					= 10;
/******  Splash Screen  **************************************************************************/
const unsigned char BATMAN_LOGO [] PROGMEM = {											// Batman logo bitmap
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x1c, 0x38, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x3c, 0x3c, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x80, 0x00, 0x3f, 0xfc, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 
	0x00, 0x01, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xfc, 0x00, 0x0f, 0xff, 0xff, 0xff, 0x80, 0x00, 
	0x00, 0x03, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x7f, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 
	0x00, 0x38, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1c, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0x81, 0xff, 0xff, 0xff, 0xff, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x3f, 0xff, 0xff, 0xf8, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/*************************************************************************************************/

void setup()
{
	
	//						<SERIAL INIT>
	Serial.begin(9600);																	// Baud rate = 9600bps
	while(!Serial) { ; }																// Wait for serial port to connect
	//						</SERIAL INIT>
	
//	Serial.print("\n\n");																// Print two blank lines to serial monitor
	
	//						<SPLASH SCREEN>
	SSD1306_Initialize();																// Initialize SSD1306 OLED display module
	
	SSD1306_Display.drawBitmap(0, 0, BATMAN_LOGO, 128, 64, 1);							// Draw Batman logo on display
	SSD1306_Display.display();															// Update display
	delay(2000);
	SSD1306_Circles();																	// Draw concentric circles on display
	//						</SPLASH SCREEN>
	
	//						<SENSORS INIT>
	MQ6_Initialize();																	// Initialize MQ-6 sensor
	MQ8_Initialize();																	// Initialize MQ-8 sensor
	MQ135_Initialize();																	// Initialize MQ-135 sensor
	//						</SENSORS INIT>
  
}

void loop()
{
	
	//						<STARTUP LOOP>
	SSD1306_Display.clearDisplay();														// Clear the buffer
	SSD1306_Setup();																	// Run setup function for SSD1306 display
//	Serial.println("\n**************************************************\n");			// Print 50 asterisks--because this is America--to serial monitor as a divider 
	//						</STARTUP LOOP>
	
	//						<SENSOR LOOP>
	DHT11_ReadSensor();																	// Generate data from DHT11 sensor
	
	DHT11_SerialOutput();																// Output DHT11 sensor data to serial monitor and display
	
	SSD1306_Display.setCursor(0, 24);
	
	MQ6_SerialOutput();																	// Output MQ-6 sensor data to serial monitor and display
	MQ8_SerialOutput();																	// Output MQ-8 sensor data to serial monitor and display
	MQ135_SerialOutput();																// Output MQ-135 sensor data to serial monitor and display
	//						</SENSOR LOOP>
	
	delay(3000);
	
}

/******  General Functions  **********************************************************************/
float ResistanceCalculator(int RAW_ADC)													// Function to calculate resistance through MQ-# sensors
{
  
	return (((float)RL_VALUE * (1023 - RAW_ADC) / RAW_ADC));
  
}

float Calibration(int OUT_PIN)															// Function to perform a general calibration on MQ-# sensors
{
  
	int		i;
	float	R0_Value = 0;
  
	for (i = 0; i < CALIB_SAMPLE_AMT; i++)
	{
	
		R0_Value += ResistanceCalculator(analogRead(OUT_PIN));
		delay(CALIB_SAMPLE_INTRVL);
	
	}
  
	R0_Value = R0_Value / CALIB_SAMPLE_AMT;												// Calculate average value output by above FOR loop
	R0_Value = R0_Value / R0_CLEAN_AIR_FACTOR;											// Calculate R0 value
  
	return R0_Value;
  
}

float ReadGasSensor(int OUT_PIN)
{
  
	int		i;
	float	RS = 0;
  
	for (i = 0; i < READ_SAMPLE_AMT; i++)
	{
    
		RS += ResistanceCalculator(analogRead(OUT_PIN));
		delay(READ_SAMPLE_INTRVL);
    
	}
  
	RS = RS / READ_SAMPLE_AMT;
  
	return RS;
}

int GetPPM(float RSR0_Ratio, float *PNT_Curve)
{
  
	return (pow(10, (((log(RSR0_Ratio) - PNT_Curve[1]) / PNT_Curve[2]) + PNT_Curve[0])));
  
}

float ConvertCelsiusToFarenheit(float CEL_Temp)
{
  
	return ((CEL_Temp * (9.0 / 5.0)) + 32.0);
  
}
/******  MQ-6 Functions  *************************************************************************/
void MQ6_Initialize(void)
{
	
	SSD1306_Display.setCursor(0, 0);
//	Serial.println("Calibrating MQ-6...");
	SSD1306_Display.print("Calibrating MQ-6");
	SSD1306_Display.display();
	SSD1306_LoadingEllipses(4);
	MQ6_R0 = Calibration(MQ6_PIN);													// Calibrates sensor in clean air
	SSD1306_Display.setCursor(0, 17);
//	Serial.println("Calibration is complete!");
//	Serial.print("R0 = ");
//	Serial.print(MQ6_R0);
//	Serial.println("kOhm");
	SSD1306_Display.println("Calibration complete!");
	SSD1306_Display.print("R0 = ");
	SSD1306_Display.print(MQ6_R0);
	SSD1306_Display.println("kOhm");
	SSD1306_Display.display();
	delay(2000);
	SSD1306_Display.clearDisplay();													// Clear the buffer
	
}

int MQ6_GetGasPercentage(float RSR0_Ratio, int GAS_ID)
{
  
	if (GAS_ID == GAS_LPG)
	{
	
		return GetPPM(RSR0_Ratio, MQ6_LPG_Function);
	
	}
	else if (GAS_ID == GAS_CH4)
	{
	
		return GetPPM(RSR0_Ratio, MQ6_CH4_Function);
  
	}
  
	return 0;
}

void MQ6_SerialOutput(void)
{
	
//	Serial.print("LPG: ");
//	Serial.print(GetPPM(ReadGasSensor(MQ6_PIN) / MQ6_R0, GAS_LPG));
//	Serial.print("ppm");
//	Serial.print("    ");
//	Serial.print("CH4: ");
//	Serial.print(GetPPM(ReadGasSensor(MQ6_PIN) / MQ6_R0, GAS_CH4));
//	Serial.println("ppm");
	
	SSD1306_Display.print("LPG: ");
	SSD1306_Display.display();
	SSD1306_Display.print(GetPPM(ReadGasSensor(MQ6_PIN) / MQ6_R0, GAS_LPG));
	SSD1306_Display.println("ppm");
	SSD1306_Display.display();
	SSD1306_Display.print("CH4: ");
	SSD1306_Display.display();
	SSD1306_Display.print(GetPPM(ReadGasSensor(MQ6_PIN) / MQ6_R0, GAS_CH4));
	SSD1306_Display.println("ppm");
	SSD1306_Display.display();
	
}
/******  MQ-8 Functions  *************************************************************************/
void MQ8_Initialize(void)
{
	
	SSD1306_Display.setCursor(0, 0);
//	Serial.println("Calibrating MQ-8...");
	SSD1306_Display.print("Calibrating MQ-8");
	SSD1306_Display.display();
	SSD1306_LoadingEllipses(20);
	MQ8_R0 = Calibration(MQ8_PIN);													// Calibrates sensor in clean air
	SSD1306_Display.setCursor(0, 17);
//	Serial.println("Calibration is complete!");
//	Serial.print("R0 = ");
//	Serial.print(MQ8_R0);
//	Serial.println("kOhm");
	SSD1306_Display.println("Calibration complete!");
	SSD1306_Display.print("R0 = ");
	SSD1306_Display.print(MQ8_R0);
	SSD1306_Display.println("kOhm");
	SSD1306_Display.display();
	delay(2000);
	SSD1306_Display.clearDisplay();													// Clear the buffer
	
}

int MQ8_GetGasPercentage(float RSR0_Ratio, int GAS_ID)
{
  
	if (GAS_ID == GAS_H2)
	{
    
		return GetPPM(RSR0_Ratio, MQ8_H2_Function);
    
	}
  
	return 0;
}

void MQ8_SerialOutput(void)
{
	
//	Serial.print("H2: ");
//	Serial.print(GetPPM(ReadGasSensor(MQ8_PIN) / MQ8_R0, GAS_H2));
//	Serial.println("ppm");
	
	SSD1306_Display.print("H2:  ");
	SSD1306_Display.display();
	SSD1306_Display.print(GetPPM(ReadGasSensor(MQ8_PIN) / MQ8_R0, GAS_H2));
	SSD1306_Display.println("ppm");
	SSD1306_Display.display();
	
}
/******  MQ-135 Functions  ***********************************************************************/
void MQ135_Initialize(void)
{

	SSD1306_Display.setCursor(0, 0);
//	Serial.println("Calibrating MQ-135...");
	SSD1306_Display.print("Calibrating MQ-135");
	SSD1306_Display.display();
	SSD1306_LoadingEllipses(3);
	MQ135_R0 = Calibration(MQ135_PIN);												// Calibrates sensor in clean air
	SSD1306_Display.setCursor(0, 17);
//	Serial.println("Calibration is complete!");
//	Serial.print("R0 = ");
//	Serial.print(MQ135_R0);
//	Serial.println("kOhm");
	SSD1306_Display.println("Calibration complete!");
	SSD1306_Display.print("R0 = ");
	SSD1306_Display.print(MQ135_R0);
	SSD1306_Display.println("kOhm");
	SSD1306_Display.display();
	delay(2000);
	SSD1306_Display.clearDisplay();													// Clear the buffer
	
}

int MQ135_GetGasPercentage(float RSR0_Ratio, int GAS_ID)
{
  
	if (GAS_ID == GAS_CO)
	{
  
		return GetPPM(RSR0_Ratio, MQ135_CO_Function);
  
	}
	else if (GAS_ID == GAS_NH4)
	{
  
		return GetPPM(RSR0_Ratio, MQ135_NH4_Function);
  
	}
  
	return 0;
}

void MQ135_SerialOutput(void)
{
	
//	Serial.print("CO: ");
//	Serial.print(GetPPM(ReadGasSensor(MQ135_PIN) / MQ135_R0, GAS_CO));
//	Serial.print("ppm");
//	Serial.print("    ");
//	Serial.print("NH4: ");
//	Serial.print(GetPPM(ReadGasSensor(MQ135_PIN) / MQ135_R0, GAS_NH4));
//	Serial.println("ppm");
	
	SSD1306_Display.print("CO:  ");
	SSD1306_Display.display();
	SSD1306_Display.print(GetPPM(ReadGasSensor(MQ135_PIN) / MQ135_R0, GAS_CO));
	SSD1306_Display.println("ppm");
	SSD1306_Display.display();
	SSD1306_Display.print("NH4: ");
	SSD1306_Display.display();
	SSD1306_Display.print(GetPPM(ReadGasSensor(MQ135_PIN) / MQ135_R0, GAS_NH4));
	SSD1306_Display.println("ppm");
	SSD1306_Display.display();
	
}
/******  DHT11 Functions  ************************************************************************/
void DHT11_ReadSensor(void)
{
  
	int		Read_Pin = DHT11.read11(DHT11_PIN);
  
	CEL_Temp_Current = (float)DHT11.temperature;
	FAR_Temp_Current = (float)ConvertCelsiusToFarenheit(CEL_Temp_Current);
	HUM_Current = (float)DHT11.humidity;
	
}
void DHT11_SerialOutput(void)
{
	
	if ((HUM_Current <= 33.0) || (HUM_Current >= 85.0))
	{
	
	//	Serial.println("Current humidity readings may skew gas sensor readings.");
    
	}
	
//	Serial.print("Temperature: ");
//	Serial.print(CEL_Temp_Current);
//	Serial.print("??C / ");
//	Serial.print(FAR_Temp_Current);
//	Serial.print("??F");
//	Serial.print("    ");
//	Serial.print("Humidity: ");
//	Serial.print(HUM_Current);
//	Serial.println("%");
	
	SSD1306_Display.print("Temp: ");
	SSD1306_Display.print(CEL_Temp_Current);
	SSD1306_Display.print(" C/");
	SSD1306_Display.print(FAR_Temp_Current);
	SSD1306_Display.println(" F");
	SSD1306_Display.print("Humidity: ");
	SSD1306_Display.print(HUM_Current);
	SSD1306_Display.println("%");
	SSD1306_Display.display();
	
}
/******  SSD1306 Functions  **********************************************************************/
void SSD1306_Initialize(void)
{
	
	if(!SSD1306_Display.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDR))					// Address 0x3C for blue/yellow 128x64 screen
	{
		
	//	Serial.println(F("SSD1306 allocation failed"));
		for(;;);																	// Don't proceed, loop forever
		
	}

	SSD1306_Display.clearDisplay();													// Clear the buffer
	SSD1306_Setup();

//	Serial.print("\n\n");
	
}

void SSD1306_Setup(void)
{
	
	SSD1306_Display.setTextSize(1);
	SSD1306_Display.setTextColor(1);
	SSD1306_Display.setCursor(0, 0);
	
}

void SSD1306_LoadingEllipses(int REPEAT_NUM)
{
	
	int		i;
	
	if (REPEAT_NUM > 26)
	{
		
		int Extraneous_Delay = (REPEAT_NUM - 26) * 500;
		
		for (i = 0; i < 26; i++)
		{
			
			delay(500);
			SSD1306_Display.print(".");
			SSD1306_Display.display();
			
		}
		
		delay(Extraneous_Delay);
		
	}
	else
	{
	
		for (i = 0; i < REPEAT_NUM; i++)
		{
			
			delay(500);
			SSD1306_Display.print(".");
			SSD1306_Display.display();
			
		}
		
	}
}
void SSD1306_Circles(void)
{

	for(int16_t i = 0; i < max(SSD1306_Display.width(), SSD1306_Display.height()) / 2; i+= 2)
	{
	
		SSD1306_Display.drawCircle(SSD1306_Display.width() / 2, SSD1306_Display.height() / 2, i, 1);
		SSD1306_Display.display();
		delay(1);
		
	}
  
	for(int16_t i = 0; i < max(SSD1306_Display.width(), SSD1306_Display.height()); i+= 2)
	{
	
		SSD1306_Display.drawCircle(SSD1306_Display.width() / 2, SSD1306_Display.height() / 2, i, 0);
		SSD1306_Display.display();
		delay(1);
			
	}
	
	delay(1000);
	SSD1306_Display.clearDisplay();

}

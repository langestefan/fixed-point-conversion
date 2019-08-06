/******************************************************************************
 * File           : Main program
 * Author         : Stefan de Lange
 *****************************************************************************/
//#include "stm32f0xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"

void assert_strequal( int test, char* b, char* expected, char* message );

//comment the following line to test your fixed point solution instead of the given example
//#define RUN_FLOAT 1

char  buffer[20];  //for printing
int   passed = 0;

#ifdef RUN_FLOAT

typedef   float  fixed;     //note that still one testcase fails because float is not accurate enough!
//typedef   double  fixed;  //more precision floatint point type passes all tests

#define float_to_fixed(a)   (a)   //float does not need conversion

fixed calc_average( fixed* array, int8_t lenght) {
  int8_t hour;
  fixed sum = 0;
  for ( hour=0; hour<lenght; hour++) {
    sum += *array++;
  }
  return sum / lenght;   //float does not need rounding
}

void to_string( fixed number )
{
  sprintf(buffer, "%.1f", number);  //printing takes care of rounding
}

#else

// Q7.8 = 1 bit sign, 7 bit integer value, 8 bit fractional part
typedef   int16_t  fixed;   

#define SHIFT_AMOUNT 					(8)																			// 8 bit shift
#define SHIFT_MASK 						((1 << SHIFT_AMOUNT) - 1)								// 0000 0000 . 1111 1111 = 255

// make the correct conversion macro
#define float_to_fixed(a)     ((fixed)((a) * (1 << SHIFT_AMOUNT))) 		// Multiply with 256
	
#define frac_part(a)					((a) & SHIFT_MASK) 											// Retrieve fractional part of fixed point number
#define frac_fixed(a) 	      (frac_part(a) * 10)											// Multiply integer value fractional part by 10 to not lose it after reverting back to int
#define frac_rounded(a)	      (frac_fixed(a) + float_to_fixed(0.6)) 	// Add 0.6(fixed notation) to fixed point fractional part
#define frac_real(a)					(frac_rounded(a) / (1 << SHIFT_AMOUNT))	// Divide by 256 to convert back to notation presentable in int

// implement this code yourself
fixed calc_average( fixed* array, int8_t lenght) 
{
	// your code goes here
	int8_t hour;
  int32_t sum = 0;
	
  for ( hour=0; hour<lenght; hour++){ 					// Calculate every element of array
    sum += *array++; 														// Dereference every array position to retrieve value
  }
  return sum / lenght; 													// Calculate average
}

// implement this code yourself
void to_string( fixed number ) 
{
	sprintf(buffer, "%d.%1d",
					(number / (1 << SHIFT_AMOUNT)), 			// Devide by 256 to retrieve real value (7 bits value, 1 bit sign = 8 bits)
					frac_real(number)); 									// Use frac_real(a) macro to retrieve fractional part of number input (8 bit value + 8 bit fraction = 16 bits total)
}

#endif // RUN_FLOAT

struct testcase
{
	  int    aantalgetallen;
	  float  invoer[5];
	  char*  verwachtte_uitvoer;
	  char*  melding;
};

const int nrofTestcases = 9;

struct testcase testcases[] = {
  //nrvalues       input              output   test
      { 1, {1, 0, 0, 0, 0},           "1.0",   "simple" },
      { 1, {0.1, 0, 0, 0, 0},         "0.1",   "simple" },
      { 2, {36.3, 36.3, 0, 0, 0},     "36.3",  "average" },
      { 2, {36.3, 36.4, 0, 0, 0},     "36.4",  "average with rounding" },
      { 3, {36.3, 36.4, 0, 0, 0},     "24.2",  "average with rounding" },
      { 5, {100, 100, 100, 100, 100}, "100.0", "overflow" },
      { 5, {-20, -20, -20, -20, -20}, "-20.0", "underflow" },
      { 2, {-20, -19, 0, 0, 0},       "-19.5", "rounding negative values" },
      { 2, {-20, 20, 0, 0, 0},        "0.0",   "negative + positive number " }
};

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
int main(void)
{
  int test, j;
  fixed avg;
  fixed hourly_temperatures[ 24 ];
	
#ifdef RUN_FLOAT
  USART_putstr("This is testcode for floating point arithmetic\n");
#else
  USART_putstr("This is testcode for fixed point arithmetic\n");
#endif
  USART_putstr("The output of the testcases is\n\n");

  for ( test=0; test<nrofTestcases; test++)
  {
	 //do each test
	 struct testcase thistest = testcases[test];

	 //fill values array
	 for (j=0; j<thistest.aantalgetallen; j++)
	 {
        hourly_temperatures[j] = float_to_fixed(thistest.invoer[j]);
	 }

     avg = calc_average( hourly_temperatures, thistest.aantalgetallen );
     to_string( avg );
     assert_strequal( test, buffer, thistest.verwachtte_uitvoer, thistest.melding );
  }

  USART_putstr( "\nYou passed " );
  USART_putint( passed );
  USART_putstr( " of the tests\n" );
  USART_putstr("Ready");

  // Ready
  while(1);
}


void assert_strequal( int test, char* b, char* expected, char* message )
{
	USART_putstr( "TEST: " );
	USART_putint( test+1 );
	if ( strncmp( b, expected, 10 ) != 0 )   //check the first 10 digits for equality
	{
		USART_putstr( " FAIL: " );
		USART_putstr( b );
		USART_putstr( " != " );
		USART_putstr( expected );
		USART_putstr( " " );
		USART_putstr( message );
	}
	else
	{
		USART_putstr( " PASS: " );
		USART_putstr( b );
		USART_putstr( " = " );
		USART_putstr( expected );
		passed++;
	}
  USART_putstr( "\n" );
}

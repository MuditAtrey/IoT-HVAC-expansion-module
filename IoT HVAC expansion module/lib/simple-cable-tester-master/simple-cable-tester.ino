#include "pins.h"

int txPin[numTestPins];
int rxPin[numTestPins];
int enaPin[numTestPins];
bool ena[numTestPins];

bool testPassed = false;

void runCableTest()
{
	initializeCableTest();
	executeCableTest();
	finalizeCableTest();
}

void initializeCableTest()
{
  delay(50);
	testPassed = true;
	
	//check which pins are to be tested
	for (int i = 0; i < numTestPins; ++i)
	{
		if ( enaPinActiveState == digitalRead(enaPin[i]) )
			ena[i] = true;
		else
			ena[i] = false;
	}
}


void executeCableTest()
{
	for (int i = 0; i < numTestPins; ++i)
	{
		if (ena[i])
		{
			//send a signal through the test pin 
			digitalWrite(txPin[i], testPinActiveState);
			delay(10);
			
			//verify that the test pin received the signal on the other side
			if (digitalRead(rxPin[i]) != testPinActiveState)
				testPassed = false;
			
			//verify that the other test pins did *NOT* receive the signal
			for (int j = 0; j < numTestPins; ++j)
			{
				if (ena[j] && j != i)
				{
					if (digitalRead(rxPin[j]) != testPinInactiveState)
						testPassed = false;
				}
			}
			
			//turn off the signal
			digitalWrite(txPin[i], testPinInactiveState);
			delay(10);
		}
	}
}


void finalizeCableTest()
{
	if (testPassed)
	{
		digitalWrite(passLed, ledActiveState);
    digitalWrite(failLed, ledInactiveState);
	}
	else
	{
    digitalWrite(passLed, ledInactiveState);
		digitalWrite(failLed, ledActiveState);
	}
}

void setup()
{
  delay(200);
	txPin[0]  = P1TX ;
	txPin[1]  = P2TX ;
	txPin[2]  = P3TX ;
	txPin[3]  = P4TX ;
	txPin[4]  = P5TX ;
	txPin[5]  = P6TX ;
	txPin[6]  = P7TX ;
	txPin[7]  = P8TX ;
	txPin[8]  = P9TX ;
	txPin[9]  = P10TX;
	txPin[10] = P11TX;
	txPin[11] = P12TX;
	
	rxPin[0]  = P1RX ;
	rxPin[1]  = P2RX ;
	rxPin[2]  = P3RX ;
	rxPin[3]  = P4RX ;
	rxPin[4]  = P5RX ;
	rxPin[5]  = P6RX ;
	rxPin[6]  = P7RX ;
	rxPin[7]  = P8RX ;
	rxPin[8]  = P9RX ;
	rxPin[9]  = P10RX;
	rxPin[10] = P11RX;
	rxPin[11] = P12RX;
	
	enaPin[0]  = P1EnaPin ;
	enaPin[1]  = P2EnaPin ;
	enaPin[2]  = P3EnaPin ;
	enaPin[3]  = P4EnaPin ;
	enaPin[4]  = P5EnaPin ;
	enaPin[5]  = P6EnaPin ;
	enaPin[6]  = P7EnaPin ;
	enaPin[7]  = P8EnaPin ;
	enaPin[8]  = P9EnaPin ;
	enaPin[9]  = P10EnaPin;
	enaPin[10] = P11EnaPin;
	enaPin[11] = P12EnaPin;
	
	for (int i = 0; i < numTestPins; ++i)
	{
		pinMode(txPin[i], OUTPUT);
    digitalWrite(txPin[i], testPinInactiveState);
    
		pinMode(rxPin[i], INPUT_PULLUP);
		pinMode(enaPin[i], INPUT_PULLUP);
	}
	
	pinMode(passLed, OUTPUT);
	pinMode(failLed, OUTPUT);
	
	digitalWrite(passLed, ledInactiveState);
	digitalWrite(failLed, ledInactiveState);
}

void loop()
{
  runCableTest();
}


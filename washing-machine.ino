#include "FastLED.h"
#include "digits.h"


// How many leds in your strip?
#define NUM_LEDS_1 150
#define NUM_LEDS_2 98

#define NUM_LEDS NUM_LEDS_1 + NUM_LEDS_2

#define LEDS_PER_ROW 8
#define NUM_ROWS 31


// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN_1 7
#define DATA_PIN_2 8

#define BIKE1 A0
#define BIKE2 A3

enum {  WAITING, COUNT_DOWN, START_GAME, PLAYING, WIN, LOSE, END_GAME };

int state = WAITING;

bool CYCLING = false;

CRGB leds[NUM_LEDS];




unsigned char hue = 0;
unsigned int delay_time = 100;
unsigned char fade_rate = 180;

unsigned long cycling_time = millis();




void fadeAll()  {
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i].nscale8(fade_rate);

	}
	FastLED.show();
}

void light_row(int row, CRGB colour) {

	if (row > NUM_ROWS) {
		return;
	}

	for (int i = 0; i <  LEDS_PER_ROW; i++) {
		int pixel = (LEDS_PER_ROW * row) + i;
		leds[NUM_LEDS - pixel - 1] = colour;
	}

}


void show_digit(int digit, int start_row) {
	//FastLED.clear();

	for (int row = 0; row < 8; row++) {
		if (start_row + row >= 0 ) {
			byte line = DIGITS[digit][row];

			//Serial.println(line);
			int col = 0;
			for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
				//int pixel = ((start_row + row) * 8) +  col;
				int pixel  = ((start_row + row) * 8) + (7 - col);

				if ((start_row + row) % 2) {
					//pixel  = ((start_row + row) * 8) + (7 - col);
					pixel = ((start_row + row) * 8) +  col;//pixel += 1;
				}

				if (line & mask) {
					leds[pixel] = CRGB::Red;
					//Serial.print("*");
				} else {
					//Serial.print(" ");
					leds[pixel] = CRGB::Black;
				}
				//Serial.println(pixel);

				col++;
			}
		}
		//Serial.println();
	}
	FastLED.show();
}





void processSerial() {
	while (Serial.available()) {
		char inChar = (char) Serial.read();
		unsigned int val;
		switch (inChar) {
		case 's':
			val = (unsigned int) Serial.parseInt();

			Serial.print("s");
			if (val > 1) {
				delay_time = val;
				Serial.print("delay: ");
				Serial.println(val);
			}

			// get the new byte:
			break;
		case 'f':
			Serial.print("f");
			val = (unsigned int) Serial.parseInt();

			if (val < 256 && val > 1) {
				fade_rate = val;
				Serial.print("fade: ");
				Serial.println(val);
			}

			break;
		case 'd':
			FastLED.clear();
			val = (unsigned int) Serial.parseInt();

			if (val == 10) {
				Serial.println(0);
				FastLED.clear();
				show_digit(0, 8);
			}

			if (val > 0 && val < 10) {
				Serial.println(val);
				show_digit(val, 8);
			}

			break;

		case 'x':
			state = COUNT_DOWN;
			break;
		case 'q':
			break;
		case 'p':

			break;

		}


	}

}


void serialEvent() {
	processSerial();

}

void readADC() {

	int val = analogRead(BIKE1) / 6;

	if (val > 3) {
		if (!CYCLING) {
			CYCLING = true;
			cycling_time = millis();
			Serial.print(CYCLING);
			Serial.print(" ");
			Serial.print(val);
			Serial.print(" ");
			Serial.println(analogRead(BIKE1));
		}

	} else {
		if (CYCLING) {
			CYCLING = false;
			cycling_time = 0;
			Serial.print(CYCLING);
			Serial.print(" ");
			Serial.print(val);
			Serial.print(" ");
			Serial.println(analogRead(BIKE1));

		}
	}



}


void testScreen() {
	for (int i = 0; i < NUM_ROWS; i++) {
		hue = map(i, 0, NUM_ROWS, 0, 160);
		light_row(i, CHSV(hue, 255, 255));

	}
	FastLED.show();
	fade_rate = 240;

	//unsigned long start_time = millis();
	while (leds[0].r > 0 || leds[0].b > 0 || leds[0].g > 0) {
		fadeAll();
		delay(100);
	}
	fade_rate = 100;
}





void resetGame() {
	//Reset player positions
	cycling_time = 0;
	FastLED.clear();
	FastLED.show();
	fade_rate = 100;

}





void setup() {


	Serial.begin(115200);
	Serial.println("ON");

	//FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds, 0, NUM_LEDS_1);
	FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds, NUM_LEDS_1, NUM_LEDS_2);

	testScreen();
	FastLED.clear();
	FastLED.show();


}



void colours() {
	hue += 1;
	CRGB col = CHSV(hue, 255, 255);
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = col;
	}
	FastLED.show();
}

void wait(unsigned int dt) {
	unsigned long start_time = millis();
	while (millis() - start_time <  dt) {
		processSerial();
		delay(1);
	}
}

//Show 3 to 1 count_down
void count_down() {
	FastLED.clear();
	// FastLED.show();
	show_digit(3, 8);
	delay(1000);
	show_digit(2, 8);
	delay(1000);
	show_digit(1, 8);
	delay(1000);
	FastLED.clear();
	FastLED.show();


}





void loop() {


	switch (state) {

	case WAITING:
		fade_rate = 100;
		fadeAll();
		//matrix();
		//noisyFire();
		readADC();

		if (CYCLING) {
			state = START_GAME;

		}
		wait(100);
		break;

	case START_GAME:

		resetGame();
		Serial.println("Playing");
		state = PLAYING;
		//drawPlayer(RED_PLAYER);
		//drawPlayer(BLUE_PLAYER);
		cycling_time = millis();
		break;

	case PLAYING:
		//FastLED.clear();
		readADC();

		if (!CYCLING) {
			state = WAITING;
			break;
		}
		if (CYCLING) {
			FastLED.clear();
			int lines = map(millis() - cycling_time, 0, 30000, 1, 31);
			//int hue = 160;

			for (int i = lines; i > 0; i--) {
				hue = map(i, 1, 31, 160, 96);
				light_row(i, CHSV(hue, 255, 255));
			}
			FastLED.show();
		}

		if (millis() - cycling_time > 30000) {
			state = WIN;
			hue = 96;
			FastLED.clear();

			for (int i = 0; i < NUM_ROWS; i++) {
				//hue = map(i, 0, NUM_ROWS, 0, 160);
				light_row(i, CHSV(hue, 255, 255));

			}

			FastLED.show();
			Serial.println("Winning relay on");

		}

		wait(delay_time);

		break;

	case WIN:
		readADC();
		//TODO Power on relay
		//colours();
		if (!CYCLING) {
			//power off relay
			state = WAITING;
			Serial.println("Relay off");
		}
		wait(delay_time);

		break;
	case END_GAME:

		break;
	}


}



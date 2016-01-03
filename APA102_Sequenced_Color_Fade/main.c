#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define PIN_DATA 0
#define PIN_CLOCK 1

#define PIXEL_COUNT  10
#define PIXEL_BRIGHTNESS 0xff

#define COLOR_STEP_INCREMENT 8
#define COLOR_STEP_DELAY 5
#define COLOR_STEP_MAX 255

/*
1 0 0   BLUE
0 1 0   GREEN
0 0 1   RED
1 0 0   BLUE

1 1 0   CYAN
0 1 1   YELLOW
1 0 1   MAGENTA
*/

uint8_t counter = 0;

void spi_write(uint8_t spi_data) {
	for (uint8_t i = 0; i < 8; i++) {
		if (spi_data & 0x80) {
			PORTB |= _BV(PIN_DATA);  // Data high.
			} else {
			PORTB &= ~_BV(PIN_DATA);  // Data low.
		}
		PORTB |= _BV(PIN_CLOCK);    // Clock high.
		spi_data = spi_data << 1;
		_delay_ms(1);
		//asm("NOP");
		//asm("NOP");
		PORTB &= ~_BV(PIN_CLOCK);    // Clock low.
		//_delay_ms(1);
	}
}

void start_frame() {
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
}

void end_frame() {
	spi_write(0xff);
	spi_write(0xff);
	spi_write(0xff);
	spi_write(0xff);
}

const uint8_t start_sequence[6] PROGMEM = { 4, 6, 2, 3, 1, 5 };
const uint8_t set_sequence[6]   PROGMEM = { 0, 1, 1, 2, 2, 0 };
const uint8_t fade_sequence[6]  PROGMEM = { 1, 0, 2, 1, 0, 2 };

void led_show_init() {
	start_frame();
	for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
		volatile uint8_t data1[3] = { 0, 0, 0 };
		uint8_t data_bits = pgm_read_byte(&(start_sequence[i % 6]));
		if (data_bits & 4) {
			data1[0] = COLOR_STEP_MAX;
		}
		if (2  & data_bits) {
			data1[1] = COLOR_STEP_MAX;
		}
		if (1  & data_bits) {
			data1[2] = COLOR_STEP_MAX;
		}
		spi_write(PIXEL_BRIGHTNESS);
		spi_write(data1[0]);
		spi_write(data1[1]);
		spi_write(data1[2]);
	}
	end_frame();
}

void led_show(uint8_t fade_progress) {
	start_frame();
	for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
		uint8_t data[3] = {0, 0, 0};
		uint8_t index = counter + i;
		index = index % 6;
		uint8_t fade_channel = pgm_read_byte(&(fade_sequence[index]));
		uint8_t set_channel = pgm_read_byte(&(set_sequence[index]));
		if (index % 2 == 0) {
			data[fade_channel] = fade_progress;  // fade up
			} else {
			data[fade_channel] = COLOR_STEP_MAX - fade_progress;
		}
		data[set_channel] = COLOR_STEP_MAX;
		spi_write(PIXEL_BRIGHTNESS);
		spi_write(data[0]);
		spi_write(data[1]);
		spi_write(data[2]);
	}
	end_frame();
}

int main(void) {
	// Configure I/O ports.
	DDRB = 0x03;  // Set PB0 and PB1 to output.
	PORTB = 0x01;  // Set PB0 and PB1 low.

	led_show_init();
	_delay_ms(COLOR_STEP_DELAY * 50);

	while(1) {
		for (int i = COLOR_STEP_INCREMENT - 1; i <= COLOR_STEP_MAX + 1; i = i + COLOR_STEP_INCREMENT) {
			led_show(i);
			_delay_ms(COLOR_STEP_DELAY);
		}
		counter++;
		_delay_ms(COLOR_STEP_DELAY * 50);
	}

	return 1;
}

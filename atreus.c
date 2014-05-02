#include <stdlib.h>
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define DEBOUNCE_PASSES 3

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))



void reset(void);

// set this for layer changes that need to persist beyond one cycle
int current_layer_number = 0;
int *current_layer;

// layout.h must define:
// * ROW_COUNT, COL_COUNT, KEY_COUNT
// * layers: array of int[KEY_COUNT]
// * layer_functions: array of void function pointers
// ... plus any functions included in layer_functions
#include "layout.h"

int pressed_count = 0;
int presses[KEY_COUNT];
int debouncing_presses[KEY_COUNT];



void record(int col, int row) {
  presses[pressed_count++] = (row * COL_COUNT) + col;
};

void activate_row(int row) {
  PORTD = (char)(~(1 << row)) | 32; // leave the LED on
  _delay_us(50);
};

// TODO: 0,0 never registers
void scan_row(int row) {
  // if(((~PINF) & 64) && row == 3) reset();
  unsigned int col_bits = ((~PINF << 4) & (1024 | 512 | 256)) | (~PINB & 255);
  for(int col = 0; col < COL_COUNT; col++) {
    if(col_bits & 1024) record(col, row);
    col_bits = col_bits << 1;
  }
};

void scan_rows() {
  for(int i = 0; i < ROW_COUNT; i++) {
    activate_row(i);
    scan_row(i);
  };
};



void debounce(int passes_remaining) {
  while(passes_remaining) {
    memcpy(presses, debouncing_presses, KEY_COUNT);
    pressed_count = 0;
    scan_rows();
    _delay_ms(1);
    if(!pressed_count || memcmp(presses, debouncing_presses, pressed_count)) {
      passes_remaining--;
    } else {
      passes_remaining = DEBOUNCE_PASSES;
    }
  }
};

void pre_invoke_functions() {
  for(int i = 0; i < pressed_count; i++) {
    int keycode = current_layer[presses[pressed_count]];
    if(keycode >= 200 && keycode < 300) {
      (layer_functions[keycode - 200])();
    }
  }
};

void calculate_presses() {
  int keycode = 0;
  int usb_presses = 0;
  for(int i = 0; i < pressed_count; i++) {
    keycode = current_layer[presses[i]];
    if(keycode >= 300) {
      (layer_functions[keycode - 200])();
    } else if(keycode >= 200) {
      // pre-invoke functions have already been processed
    } else if(keycode > 100) {
      keyboard_modifier_keys |= (keycode - 100);
    } else if(keycode > 108 && usb_presses < 6) {
      keyboard_modifier_keys |= KEY_SHIFT;
      keyboard_keys[usb_presses++] = (keycode - 108);
    } else if(usb_presses < 6){
      keyboard_keys[usb_presses++] = keycode;
    };
  };
};



void init() {
  CPU_PRESCALE(0);
  DDRD = 255;
  DDRB = DDRF = 0;
  PORTB = PORTF = 255;
  usb_init();
  while (!usb_configured()) /* wait */ ;
  _delay_ms(1000);
};

void clear_keys() {
  current_layer = layers[current_layer_number];
  keyboard_modifier_keys = 0;
  for(int i = 0; i < 6; i++) {
    keyboard_keys[i] = 0;
  };
};

int main() {
  init();
  while(1) {
    clear_keys();
    debounce(DEBOUNCE_PASSES);
    pre_invoke_functions();
    calculate_presses();
    usb_keyboard_send();
  };
};

void reset(void) {
  UDCON = 1; USBCON = (1<<FRZCLK); UCSR1B = 0;
  _delay_ms(5);
  EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
  TIMSK0 = 0; TIMSK1 = 0; TIMSK3 = 0; TIMSK4 = 0; UCSR1B = 0; TWCR = 0;
  DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0; TWCR = 0;
  PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
  asm volatile("jmp 0x7E00");
};

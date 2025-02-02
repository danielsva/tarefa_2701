#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "hardware/clocks.h"

#define LED_R 13
#define LED_G 11
#define LED_B 12

#define GPIO_BOTAO_A 5
#define GPIO_BOTAO_B 6
#define WS2812_PIN 7
#define NUM_LEDS 25

#ifndef LED_DELAY
#define LED_DELAY 50
#endif

void pico_led_rgb_init(void) {
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
}

void pico_set_led_rgb(bool red, bool green, bool blue) {
    gpio_put(LED_R, red);
    gpio_put(LED_G, green);
    gpio_put(LED_B, blue);
}

// Variáveis para controlar a LED vermelha e o piscar
void piscarLed() {
        pico_set_led_rgb(true, false, false); // Acende LED vermelha
        sleep_ms(LED_DELAY);
        pico_set_led_rgb(false, false, false); // Apaga LED vermelha
        sleep_ms(LED_DELAY);
}

void config_button() {
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_A);
    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_B);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

uint32_t led_buffer[NUM_LEDS] = {0};

// Padrões para números de 0 a 9
const uint32_t numbers[10][25] = {
    {1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1}, // zero
    {1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0}, // um
    {1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1}, // dois
    {1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1}, // três
    {1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1}, // quatro
    {1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1}, // cinco
    {1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1}, // seis
    {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1}, // sete
    {1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1}, // oito
    {1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1}  // nove
};

void display_number(int num) {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (numbers[num][i]) {
            put_pixel(urgb_u32(0, 0, 80)); // Azul
        } else {
            put_pixel(urgb_u32(0, 0, 0));  // Apagado
        }
    }
}

int main() {
    stdio_init_all();

    pico_led_rgb_init();
    config_button();

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    int current_number = 9;
    bool last_button_state_A = true;
    bool last_button_state_B = true;

    

    while (true) {
        piscarLed();
        
        if (gpio_get(GPIO_BOTAO_A) == 0 && last_button_state_A) {
            current_number = (current_number + 1) % 10; // Incremento
            display_number(current_number);
            sleep_ms(300); // Debounce
            }
        last_button_state_A = !gpio_get(GPIO_BOTAO_A);
        
        
        if (gpio_get(GPIO_BOTAO_B) == 0 && last_button_state_B) {
            current_number = (current_number == 0) ? 9 : current_number - 1; // Decremento
            display_number(current_number);
            sleep_ms(300); // Debounce
            }       
        last_button_state_B = !gpio_get(GPIO_BOTAO_B);
    
    }

    return 0;
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define IS_RGBW false

/* Definições dos pinos */
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define RED_LED_PIN 13
#define WS2812_PIN 7

#define NUM_PIXELS 25    // Matriz 5x5
#define DEBOUNCE_DELAY_MS 100

// Cores para exibição (ajuste conforme necessário)
uint8_t led_r = 0;   // vermelho
uint8_t led_g = 0;   // verde
uint8_t led_b = 20;  // azul – intensidade baixa, por exemplo

// Buffer que define quais LEDs estão ligados (1) ou desligados (0)
bool led_buffer[NUM_PIXELS];

// Padrões para os dígitos de 0 a 9 em uma matriz 5x5  
// Cada padrão é representado por 25 valores booleanos (linha a linha)
static const bool digit_patterns[10][NUM_PIXELS] = {
    // 0
    {
        0,1,1,1,0,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        0,1,1,1,0
    },
    // 1
    {
        0,0,1,0,0,
        0,1,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,1,1,1,0
    },
    // 2
    {
        0,1,1,1,0,
        1,0,0,0,1,
        0,0,0,1,0,
        0,0,1,0,0,
        1,1,1,1,1
    },
    // 3
    {
        1,1,1,1,0,
        0,0,0,0,1,
        0,1,1,1,0,
        0,0,0,0,1,
        1,1,1,1,0
    },
    // 4
    {
        0,0,0,1,0,
        0,0,1,1,0,
        0,1,0,1,0,
        1,1,1,1,1,
        0,0,0,1,0
    },
    // 5
    {
        1,1,1,1,1,
        1,0,0,0,0,
        1,1,1,1,0,
        0,0,0,0,1,
        1,1,1,1,0
    },
    // 6
    {
        0,1,1,1,0,
        1,0,0,0,0,
        1,1,1,1,0,
        1,0,0,0,1,
        0,1,1,1,0
    },
    // 7
    {
        1,1,1,1,1,
        0,0,0,1,0,
        0,0,1,0,0,
        0,1,0,0,0,
        0,1,0,0,0
    },
    // 8
    {
        0,1,1,1,0,
        1,0,0,0,1,
        0,1,1,1,0,
        1,0,0,0,1,
        0,1,1,1,0
    },
    // 9
    {
        0,1,1,1,0,
        1,0,0,0,1,
        0,1,1,1,1,
        0,0,0,0,1,
        0,1,1,1,0
    }
};

/* Variáveis para controle de debounce e estado dos botões */
volatile uint32_t last_debounce_A = 0;
volatile uint32_t last_debounce_B = 0;
volatile bool button_A_pressed = false;
volatile bool button_B_pressed = false;

/* Variável que guarda o dígito atual */
volatile int current_digit = 0;

/* Variáveis para controle do PIO WS2812 */
static PIO ws2812_pio = pio0;
static uint ws2812_sm = 0;
static uint ws2812_offset;

/* --- Funções para o controle da matriz WS2812 --- */

// Função para enviar um pixel via PIO (semelhante ao exemplo original)
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel_grb << 8u);
}

// Converte componentes R, G, B em um valor de 32 bits no formato GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

// Função que envia o buffer para a matriz, aplicando cor nos LEDs ativos
void set_matrix_leds(uint8_t r, uint8_t g, uint8_t b) {
    // Aplica um fator de brilho, se necessário (aqui, 10% da intensidade máxima)
    const float brightness = 0.1f;
    r = (uint8_t)(r * brightness);
    g = (uint8_t)(g * brightness);
    b = (uint8_t)(b * brightness);

    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer[i])
            put_pixel(color);  // LED ligado
        else
            put_pixel(0);      // LED desligado
    }
}

// Atualiza o buffer global "led_buffer" com o padrão do dígito e envia para a matriz
// Aqui, o padrão é invertido verticalmente (linha 0 torna-se linha 4, etc.)
void update_matrix_display(int digit) {
    if (digit < 0 || digit > 9) return;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            // Inverte verticalmente: a linha 'row' do buffer recebe a linha (4 - row) do padrão
            led_buffer[row * 5 + col] = digit_patterns[digit][(4 - row) * 5 + col];
        }
    }
    set_matrix_leds(led_r, led_g, led_b);
}

/* --- Funções para os botões com debouncing --- */
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (gpio == BUTTON_A_PIN) {
        bool level = gpio_get(BUTTON_A_PIN); // true = solto, false = pressionado
        if (!level && !button_A_pressed && (now - last_debounce_A >= DEBOUNCE_DELAY_MS)) {
            button_A_pressed = true;
            last_debounce_A = now;
            current_digit = (current_digit + 1) % 10;
            update_matrix_display(current_digit);
            printf("Botão A pressionado. Dígito: %d\n", current_digit);
        } else if (level) {
            button_A_pressed = false;
        }
    } else if (gpio == BUTTON_B_PIN) {
        bool level = gpio_get(BUTTON_B_PIN); // true = solto, false = pressionado
        if (!level && !button_B_pressed && (now - last_debounce_B >= DEBOUNCE_DELAY_MS)) {
            button_B_pressed = true;
            last_debounce_B = now;
            // Decrementa de forma cíclica: se 0, vai para 9
            current_digit = (current_digit + 9) % 10;
            update_matrix_display(current_digit);
            printf("Botão B pressionado. Dígito: %d\n", current_digit);
        } else if (level) {
            button_B_pressed = false;
        }
    }
}

/* --- Função de callback do timer --- */
bool repeating_timer_callback(struct repeating_timer *t) {
    static bool red_led_state = false;
    red_led_state = !red_led_state;
    gpio_put(RED_LED_PIN, red_led_state);
    return true;
}

/* --- Funções para inicializar os WS2812 via PIO --- */
void ws2812_init(uint gpio) {
    ws2812_offset = pio_add_program(ws2812_pio, &ws2812_program);
    ws2812_program_init(ws2812_pio, ws2812_sm, ws2812_offset, gpio, 800000, IS_RGBW);
}

/* --- Função principal --- */
int main() {
    stdio_init_all();

    // Configuração dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configura interrupções para os botões (borda de subida e descida)
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    // Configuração do LED RGB
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);

    // Inicializa a matriz de LEDs WS2812
    ws2812_init(WS2812_PIN);

    // Exibe inicialmente o dígito 0
    update_matrix_display(current_digit);

    // Configuração do timer para piscar o LED RGB
    struct repeating_timer timer;
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);

    // Loop principal: a lógica é gerenciada via interrupções
    while (true) {
        tight_loop_contents();
    }
    
    return 0;
}

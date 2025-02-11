#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BTN_A_PIN 5
#define VRX_PIN 27    // Define o pino GP26 para o eixo X do joystick (Canal ADC0).
#define VRY_PIN 26    // Define o pino GP27 para o eixo Y do joystick (Canal ADC1).
#define SW_PIN 22     // Define o pino GP22 para o botão do joystick (entrada digital).
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define ADC_MAX_VALUE 4096

void init_led(uint8_t led_pin);
void init_btn(uint8_t btn_pin);
void init_joystick();
void init_i2c();
void init_display(ssd1306_t *ssd);
void update_joystick_values(uint16_t *x_value, uint16_t *y_value, bool *sw_value);

int main()
{
    uint16_t adc_x_value;
    uint16_t adc_y_value;
    ssd1306_t ssd; // Inicializa a estrutura do display
    bool adc_sw_value;

    bool color = true;

    stdio_init_all();

    // Inicializa os LEDs
    init_led(GREEN_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(RED_LED_PIN);

    // Inicializa os botões
    init_btn(BTN_A_PIN);

    // Inicializa a comunicação I2C
    init_i2c();

    // Inicializa o display
    init_display(&ssd);

    // Inicializa o ADC e o Joystick
    adc_init();
    init_joystick();

    while (true)
    {
        update_joystick_values(&adc_x_value, &adc_y_value, &adc_sw_value);

        float tmp_x = DISPLAY_WIDTH * (float) adc_x_value / ADC_MAX_VALUE;
        float tmp_y = DISPLAY_HEIGHT * (1 - (float) adc_y_value / ADC_MAX_VALUE);
        tmp_x = tmp_x > DISPLAY_WIDTH - 8 ? DISPLAY_WIDTH - 8 : tmp_x;
        tmp_y = tmp_y > DISPLAY_HEIGHT - 8 ? DISPLAY_HEIGHT - 8 : tmp_y;

        // Imprime os valores lidos na comunicação serial.
        printf("VRX: %f, VRY: %f, SW: %d\n", tmp_x,  tmp_y, adc_sw_value);

        //ssd1306_draw_string(&ssd, "X, Y");
        ssd1306_fill(&ssd, !color); // Limpa o display
        ssd1306_rect(&ssd, (int) tmp_y, (int) tmp_x, 8, 8, color, true); // Desenha um retângulo
        //ssd1306_draw_string(&ssd, "Teste", 60, 28);
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(200);
    }
}

// Inicializa um led em um pino específico
void init_led(uint8_t led_pin)
{
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
}

// Inicializa um botão em um pino específico
void init_btn(uint8_t btn_pin)
{
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
}

// Inicializa a comunicação I2C
void init_i2c()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Inicializa o display OLED
void init_display(ssd1306_t *ssd)
{
    ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

void init_joystick() {
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    init_btn(SW_PIN);
}

void update_joystick_values(uint16_t *x_value, uint16_t *y_value, bool *sw_value) {
    adc_select_input(1); // Seleciona o ADC para eixo X. O pino 26 como entrada analógica
    *x_value = adc_read();

    adc_select_input(0); // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    *y_value = adc_read();

    // Leitura do estado do botão do joystick (SW)
    *sw_value = gpio_get(SW_PIN) == 0; // 0 indica que o botão está pressionado.
}

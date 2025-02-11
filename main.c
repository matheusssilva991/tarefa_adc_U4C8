#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

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
#define BTN_B_PIN 6
#define VRX_PIN 27 // Define o pino GP26 para o eixo X do joystick (Canal ADC0).
#define VRY_PIN 26 // Define o pino GP27 para o eixo Y do joystick (Canal ADC1).
#define SW_PIN 22  // Define o pino GP22 para o botão do joystick (entrada digital).
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define ADC_MAX_VALUE 4096
#define ADC_MIDDLE_VALUE ADC_MAX_VALUE / 2
#define RECT_XY_SHAPE 8
#define VRXY_VALUE_ERROR 100

uint init_led_pwm(uint8_t led_pin);
void init_btn(uint8_t btn_pin);
void init_joystick();
void init_i2c();
void init_display(ssd1306_t *ssd);
void update_joystick_xy_values(uint16_t *x_value, uint16_t *y_value);
void process_joystick_xy_values(uint16_t x_value_raw, uint16_t y_value_raw, float *x_value, float *y_value);
void gpio_irq_handler(uint gpio, uint32_t events);

static volatile bool sw_value = true;
static volatile uint32_t last_time_btn_a = 0; // Armazena o tempo do último evento do btn A (em microssegundos)
static volatile uint32_t last_time_btn_sw = 0; // Armazena o tempo do último evento do btn B(em microssegundos)
static volatile uint green_led_slice;
static volatile uint blue_led_slice;
static volatile uint red_led_slice;
static volatile bool is_leds_pwm_enabled = false;
static volatile bool is_green_led_on = false;

int main()
{
    uint16_t vrx_value_raw;
    uint16_t vry_value_raw;
    float vrx_value_pos; // Valor de X após processamento
    float vry_value_pos; // Valor de Y após processamento
    ssd1306_t ssd; // Inicializa a estrutura do display
    bool color = true;

    stdio_init_all();

    // Inicializa os LEDs
    green_led_slice = init_led_pwm(GREEN_LED_PIN);
    blue_led_slice = init_led_pwm(BLUE_LED_PIN);
    red_led_slice = init_led_pwm(RED_LED_PIN);

    // Inicializa os botões
    init_btn(BTN_A_PIN);
    init_btn(BTN_B_PIN);

    // Inicializa a comunicação I2C
    init_i2c();

    // Inicializa o display
    init_display(&ssd);

    // Inicializa o ADC e o Joystick
    adc_init();
    init_joystick();

    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        update_joystick_xy_values(&vrx_value_raw, &vry_value_raw);
        process_joystick_xy_values(vrx_value_raw, vry_value_raw, &vrx_value_pos, &vry_value_pos);

        // Imprime os valores lidos na comunicação serial.
        printf("VRX: %u, VRY: %u, SW: %d\n", vrx_value_raw, vry_value_raw, sw_value);

        /* bool condition;
        if (vrx_value_raw > ADC_MIDDLE_VALUE - VRXY_VALUE_ERROR && vrx_value_raw < ADC_MIDDLE_VALUE + VRXY_VALUE_ERROR) {

        } */

        ssd1306_fill(&ssd, !sw_value);
        ssd1306_rect(&ssd, 0, 0, 128, 64, sw_value, !sw_value); // Desenha um retângulo
                                                               // Limpa o display
        ssd1306_rect(&ssd, (int)vry_value_pos, (int)vrx_value_pos, RECT_XY_SHAPE, RECT_XY_SHAPE, color, true); // Desenha um retângulo
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(100);
    }
}

// Inicializa um led em um pino específico
uint init_led_pwm(uint8_t led_pin)
{
    gpio_set_function(led_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(led_pin);
    pwm_set_wrap(slice, ADC_MAX_VALUE);
    pwm_set_enabled(slice, true);
    return slice;
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

// Inicializa o joystick
void init_joystick()
{
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    init_btn(SW_PIN);
}

// Realiza a amostragem e atualiza os valores X e Y do joystick
void update_joystick_xy_values(uint16_t *x_value, uint16_t *y_value)
{
    adc_select_input(1); // Seleciona o ADC para eixo X. O pino 26 como entrada analógica
    *x_value = adc_read();

    adc_select_input(0); // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
    *y_value = adc_read();
}

// Processa os valores de X e Y para a posição correto no display
void process_joystick_xy_values(uint16_t x_value_raw, uint16_t y_value_raw, float *x_value, float *y_value) {
    *x_value = DISPLAY_WIDTH * (float) x_value_raw / ADC_MAX_VALUE;
    *y_value = DISPLAY_HEIGHT * (1 - (float)y_value_raw / ADC_MAX_VALUE);
    *x_value = *x_value > DISPLAY_WIDTH - RECT_XY_SHAPE ? DISPLAY_WIDTH - RECT_XY_SHAPE : *x_value;
    *y_value = *y_value > DISPLAY_HEIGHT - RECT_XY_SHAPE ? DISPLAY_HEIGHT - RECT_XY_SHAPE : *y_value;
}

// Função para manipular os eventos dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == 6)
    {
        reset_usb_boot(0, 0);

    } else if (gpio == BTN_A_PIN && (current_time - last_time_btn_a > 200000))
    {
        // Atualiza o tempo do último evento do botão A.
        last_time_btn_a = current_time;

        pwm_set_gpio_level(GREEN_LED_PIN, 0);
        pwm_set_gpio_level(BLUE_LED_PIN, 0);
        pwm_set_gpio_level(RED_LED_PIN, 0);

        pwm_set_enabled(green_led_slice, !is_leds_pwm_enabled);
        pwm_set_enabled(blue_led_slice, !is_leds_pwm_enabled);
        pwm_set_enabled(red_led_slice, !is_leds_pwm_enabled);

        is_leds_pwm_enabled = !is_leds_pwm_enabled;
    }
    else if (gpio == SW_PIN && (current_time - last_time_btn_sw > 200000))
    {
        // Atualiza o tempo do último evento do botão B.
        last_time_btn_sw = current_time;

        sw_value = !sw_value;

        if (is_green_led_on)
        {
            pwm_set_gpio_level(GREEN_LED_PIN, 0);
            is_green_led_on = false;
        }
        else
        {
            pwm_set_gpio_level(GREEN_LED_PIN, ADC_MIDDLE_VALUE);
            is_green_led_on = true;
        }

    }

}

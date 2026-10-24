#include <stdlib.h>

#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7796.h"
#include "esp_log.h"
#include "driver/gpio.h"

/* Panlee ZX3D50CE08S-V16 / WT32-SC01 Plus: ST7796UI, 8080 8-bit bus. */
#define LCD_WIDTH  480
#define LCD_HEIGHT 320
#define LCD_DC_GPIO     0
#define LCD_WR_GPIO    47
#define LCD_RST_GPIO    4
#define LCD_BL_GPIO    45

static const int lcd_data_gpios[8] = {9, 46, 3, 8, 18, 17, 16, 15};
static const char *TAG = "sc01_lcd";

static void fill_band(esp_lcd_panel_handle_t panel, int y, int height, uint16_t color)
{
    const size_t pixels = LCD_WIDTH * height;
    uint16_t *line = heap_caps_malloc(pixels * sizeof(*line), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (line == NULL) {
        ESP_LOGE(TAG, "Sem memoria DMA para faixa da tela");
        return;
    }
    for (size_t i = 0; i < pixels; ++i) {
        line[i] = color;
    }
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_WIDTH, y + height, line));
    free(line);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Teste direto ST7796UI / barramento i80");

    gpio_config_t backlight = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_BL_GPIO,
    };
    ESP_ERROR_CHECK(gpio_config(&backlight));
    ESP_ERROR_CHECK(gpio_set_level(LCD_BL_GPIO, 1));

    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = LCD_DC_GPIO,
        .wr_gpio_num = LCD_WR_GPIO,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            lcd_data_gpios[0], lcd_data_gpios[1], lcd_data_gpios[2], lcd_data_gpios[3],
            lcd_data_gpios[4], lcd_data_gpios[5], lcd_data_gpios[6], lcd_data_gpios[7],
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_WIDTH * 40 * sizeof(uint16_t),
        .dma_burst_size = 64,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = -1,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 1,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    /* Faixas inequívocas: vermelho, verde, azul e branco. */
    fill_band(panel,   0, 80, 0xF800);
    fill_band(panel,  80, 80, 0x07E0);
    fill_band(panel, 160, 80, 0x001F);
    fill_band(panel, 240, 80, 0xFFFF);
    ESP_LOGI(TAG, "Padrao enviado. A tela deve mostrar quatro faixas horizontais.");
}

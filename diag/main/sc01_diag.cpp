#include "esp_log.h"
#include "esp_psram.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "smartpanlee-sc01-plus.hpp"

static constexpr const char *TAG = "SC01_DIAG";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "SC01_DIAG v0.1.0 starting");
    ESP_LOGI(TAG, "PSRAM initialized: %s; size: %u bytes",
             esp_psram_is_initialized() ? "yes" : "no", esp_psram_get_size());

    auto &board = espp::SmartPanleeSc01Plus::get();
    const bool lcd_ok = board.initialize_lcd();
    ESP_LOGI(TAG, "LCD initialization: %s", lcd_ok ? "OK" : "FAILED");
    if (lcd_ok) {
        board.brightness(70.0f);
        const bool display_ok = board.initialize_display(board.display_width() * 40);
        ESP_LOGI(TAG, "LVGL display initialization: %s", display_ok ? "OK" : "FAILED");
        if (display_ok) {
            lv_obj_t *screen = lv_screen_active();
            lv_obj_set_style_bg_color(screen, lv_color_hex(0x003366), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_t *label = lv_label_create(screen);
            lv_label_set_text(label, "SC01 DIAG\nLCD + PSRAM OK");
            lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
            lv_obj_center(label);
        }
    }

    const bool touch_ok = board.initialize_touch();
    ESP_LOGI(TAG, "Touch initialization: %s", touch_ok ? "OK" : "FAILED");

    while (true) {
        ESP_LOGI(TAG, "alive; free heap: %lu", (unsigned long)esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"
#include "esp_log.h"


#define RMT_RX_GPIO    18  // IR Receiver Pin
#define RMT_TX_GPIO    14  // IR Transmitter Pin
#define MAX_SYMBOLS    128 // Max symbols buffer size

static const char* TAG = "IR_Remote";
static rmt_symbol_word_t raw_data[MAX_SYMBOLS];

// Function to transmit IR signal
void send_ir_signal(rmt_symbol_word_t* data, size_t length)
{
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = RMT_TX_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000, // 1 MHz resolution
        .mem_block_symbols = 64,
        .trans_queue_depth = 1
    };

    rmt_channel_handle_t tx_channel;
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &tx_channel));

    rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = {.level0 = 0, .duration0 = 500, .level1 = 1, .duration1 = 500},
        .bit1 = {.level0 = 0, .duration0 = 1000, .level1 = 1, .duration1 = 1000},
        .flags.msb_first = 0
    };

    rmt_encoder_handle_t bytes_encoder = NULL;
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&encoder_config, &bytes_encoder));

    ESP_ERROR_CHECK(rmt_enable(tx_channel));

    rmt_transmit_config_t tx_config_struct = {
        .loop_count = 1 // Send once
    };

    ESP_ERROR_CHECK(
        rmt_transmit(tx_channel, bytes_encoder, data, length * sizeof(rmt_symbol_word_t), &tx_config_struct));

    ESP_LOGI(TAG, "IR signal sent!");
}

// Function to receive IR signals
void rmt_rx_task(void* arg)
{
    rmt_rx_channel_config_t rx_config = {
        .gpio_num = RMT_RX_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000, // 1 MHz (1 tick = 1 µs)
        .mem_block_symbols = 64,
        .flags = {
            .invert_in = false,
            .with_dma = false
        }
    };
    rmt_channel_handle_t rx_channel;
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_config, &rx_channel));

    rmt_receive_config_t receive_config = {
        .signal_range_min_ns = 1000, // Min pulse width (500 µs)
        .signal_range_max_ns = 10000000 // Max pulse width (10 ms)
    };

    ESP_ERROR_CHECK(rmt_enable(rx_channel));
    ESP_LOGI(TAG, "Waiting for IR signal...");

    while (1)
    {
        rmt_rx_done_event_data_t rx_data;
        if (rmt_receive(rx_channel, raw_data, sizeof(raw_data), &receive_config) == ESP_OK)
        {
            ESP_LOGI(TAG, "IR signal received! Sending it back in 5 seconds...");
            vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
            send_ir_signal(raw_data, MAX_SYMBOLS);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Avoid CPU overload
    }
}

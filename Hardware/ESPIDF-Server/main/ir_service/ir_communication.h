#ifndef IR_COMMUNICATION_H
#define IR_COMMUNICATION_H

#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"

#define RMT_RX_GPIO    18
#define RMT_TX_GPIO    19
#define MAX_SYMBOLS    128

void rmt_rx_task(void *arg);


void send_ir_signal(rmt_symbol_word_t *data, size_t length);

#endif

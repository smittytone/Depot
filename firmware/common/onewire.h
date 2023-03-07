/*
 * Depot RP2040 Bus Host Firmware - 1-Wire functions
 *
 * @version     1.2.0
 * @author      Tony Smith (@smittytone)
 * @copyright   2023
 * @licence     MIT
 *
 */
#ifndef _HEADER_ONE_WIRE_
#define _HEADER_ONE_WIRE_


/*
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// Pico SDK Includes
#include "pico/stdlib.h"
// App Includes
#include "serial.h"


/*
 * CONSTANTS
 */
#define     DELAY_STANDARD_A_US             10      // Host pre-read/write signal period
#define     DELAY_STANDARD_B_US             70      // Host write 1 period
#define     DELAY_STANDARD_C_US             60      // Host write 0 period
#define     DELAY_STANDARD_D_US             20      // Host write 0 post write period
#define     DELAY_STANDARD_E_US             4       // Host read pre-sample period (A+E <= 15us)
#define     DELAY_STANDARD_F_US             66      // Host read post-sample period
#define     DELAY_STANDARD_G_US             0       // Host pre-reset period
#define     DELAY_STANDARD_H_US             485     // Reset signal period
#define     DELAY_STANDARD_I_US             55      // Host peripheral presence pre-sample period (TPDLOW)
#define     DELAY_STANDARD_J_US             430     // Host peripheral presence post-sample period (TPDHIGH)
#define     DELAY_STANDARD_R_US             1

#define     DELAY_OVERDRIVE_A_US            1.0
#define     DELAY_OVERDRIVE_B_US            7.5
#define     DELAY_OVERDRIVE_C_US            7.5
#define     DELAY_OVERDRIVE_D_US            2.5
#define     DELAY_OVERDRIVE_E_US            1.0
#define     DELAY_OVERDRIVE_F_US            7.0
#define     DELAY_OVERDRIVE_G_US            2.5
#define     DELAY_OVERDRIVE_H_US            70.0
#define     DELAY_OVERDRIVE_I_US            8.5
#define     DELAY_OVERDRIVE_J_US            40.0

#define     BIT_VALUE_1                     1
#define     BIT_VALUE_0                     0

#define     OW_CMD_SKIP_ROM                 0xCC
#define     OW_CMD_READ_ROM                 0x33
#define     OW_CMD_SEARCH_ROM               0xF0
#define     OW_CMD_MATCH_ROM                0x55

#define     DEFAULT_DATA_PIN                10


/*
 * STRUCTURES
 */
typedef struct {
    bool        is_ready;
    uint8_t     data_pin;
    uint32_t    device_count;
    uint32_t    write_byte_count;
    uint32_t    read_byte_count;
    uint32_t    current_device;             // Index into following array
    uint64_t    device_ids[64];
} OneWireState;


/*
 * PROTOTYPES
 */
void        ow_init(OneWireState* ows);
bool        ow_reset(OneWireState* ows);
bool        ow_configure(OneWireState* ows, uint32_t pin);
void        ow_write_byte(OneWireState* ows, uint8_t byte_value);
uint8_t     ow_read_byte(OneWireState* ows);
void        ow_send_state(OneWireState* ows);
void        ow_send_scan(OneWireState* ows);
bool        is_pin_in_use_by_ow(OneWireState* ows, uint8_t pin);


#endif      // _HEADER_ONE_WIRE_

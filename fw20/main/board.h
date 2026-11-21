/*
 * Board pin map: Waveshare ESP32-C6-Touch-AMOLED-1.32
 *
 * Source: waveshareteam/ESP32-C6-Touch-AMOLED-1.32, Example/ESP-IDF/08_FactoryProgram
 * (main/user_config.h, components/codec_board/board_cfg.txt, button_bsp, user_app).
 * Verify against the schematic before relying on the marked (?) entries.
 */
#pragma once

#include "driver/gpio.h"

/* ---- AMOLED 1.32" 466x466, CO5300 over QSPI (SPI2) ---- */
#define BOARD_LCD_HOST          SPI2_HOST
#define BOARD_LCD_H_RES         466
#define BOARD_LCD_V_RES         466
#define BOARD_LCD_CS            GPIO_NUM_22
#define BOARD_LCD_PCLK          GPIO_NUM_18
#define BOARD_LCD_D0            GPIO_NUM_19
#define BOARD_LCD_D1            GPIO_NUM_20
#define BOARD_LCD_D2            GPIO_NUM_10
#define BOARD_LCD_D3            GPIO_NUM_11
#define BOARD_LCD_RST           GPIO_NUM_21
#define BOARD_LCD_TE            GPIO_NUM_NC

/* ---- Shared I2C bus (touch + ES8311 codec) ---- */
#define BOARD_I2C_PORT          I2C_NUM_0
#define BOARD_I2C_SDA           GPIO_NUM_8
#define BOARD_I2C_SCL           GPIO_NUM_7

/* ---- Touch CST820 ---- */
#define BOARD_TOUCH_I2C_ADDR    0x15
#define BOARD_TOUCH_RST         GPIO_NUM_NC
#define BOARD_TOUCH_INT         GPIO_NUM_NC

/* ---- Audio ES8311 (I2S) ---- */
#define BOARD_I2S_MCLK          GPIO_NUM_15
#define BOARD_I2S_BCLK          GPIO_NUM_3
#define BOARD_I2S_WS            GPIO_NUM_5
#define BOARD_I2S_DIN           GPIO_NUM_4   /* codec -> ESP (mic) */
#define BOARD_I2S_DOUT          GPIO_NUM_6   /* ESP -> codec (speaker) */
#define BOARD_PA_EN             GPIO_NUM_23  /* speaker amplifier enable */

/* ---- Power / battery ---- */
/* Verified against ESP32-C6-Touch-AMOLED-1.32-Schematic.pdf (BAT block, GPIO table) */
#define BOARD_SYS_POWER_HOLD    GPIO_NUM_2   /* BAT_EN: high = keep battery->VSYS path on
                                                (Q1 via T1); soft power latch, NOT charger enable */
#define BOARD_BATT_ADC_UNIT     ADC_UNIT_1
#define BOARD_BATT_ADC_CHANNEL  ADC_CHANNEL_0 /* GPIO0 BAT_ADC, divider 200K/100K -> Vbat = 3 * Vadc */
/* Charger ETA6098: no MCU control (no enable, no I2C). Green LED1 is driven by its
 * STAT pin from VSYS - hardware only, cannot be switched off in firmware. */

/* ---- Buttons (active low, pull-up) ---- */
#define BOARD_BTN_BOOT          GPIO_NUM_9   /* shared with BOARD_DS18B20_DQ - no button handler */
#define BOARD_BTN_PWR           GPIO_NUM_1   /* PWR_KEY; pressing also powers on via D2 */

/* ---- External sensors on the 12-pin header J1 (see README) ---- */
#define BOARD_HX711_DOUT        GPIO_NUM_17  /* J1-12 ESP_RXD (U0RXD) */
#define BOARD_HX711_SCK         GPIO_NUM_16  /* J1-11 ESP_TXD (U0TXD); ROM log toggles it at reset */
#define BOARD_DS18B20_DQ        GPIO_NUM_9   /* J1-4, open-drain 1-wire + 4k7 pull-up to 3V3 */

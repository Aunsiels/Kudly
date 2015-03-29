/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for Olimex STM32-P407 board.
 * NOTE: Part of JTAG signals are used for other functions, this board can be
 *       used using SWD only.
 */

/*
 * Board identifier.
 */
#define BOARD_OLIMEX_STM32_P407
#define BOARD_NAME              "Olimex STM32-P407"

/*
 * Ethernet PHY type.
 */
#define BOARD_PHY_ID            MII_KS8721_ID
#define BOARD_PHY_RMII

/*
 * Board frequencies.
 * NOTE: The LSE crystal is not fitted by default on the board.
 */
#define STM32_LSECLK            32768
#define STM32_HSECLK            8000000

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD               330

/*
 * MCU type as defined in the ST header.
 */
#define STM32F40_41xxx

/*
 * IO pins assignments.
 */
#define LED1_R                  0
#define LED1_G                  1
#define LED1_B                  2
#define GPIOA_3                 3
#define CAMERA_SYNC             4
#define HAND_SENSOR_OUT         5
#define CAMERA_PIXCLK           6
#define GPIOA_7                 7
#define CAMERA_XCLK             8
#define USB_VBUS                9
#define GPIOA_10                10
#define USB_DM                  11
#define USB_DP                  12
#define SWDIO                   13
#define SWCLK                   14
#define GPIOA_15                15

#define LED2_G                  0
#define LED2_B                  1
#define GPIOB_2                 2
#define GPIOB_3                 3
#define GPIOB_4                 4
#define GPIOB_5                 5
#define CAMERA_D5               6
#define CAMERA_VSYNC            7
#define CAMERA_D6               8
#define CAMERA_D7               9
#define I2C_SCL                 10
#define I2C_SDA                 11
#define SPI2_CS_WIFI            12
#define SPI2_SCK                13
#define SPI2_MISO               14
#define SPI2_MOSI               15

#define HUG_SENS_OUT            0
#define HUG_SENS1_IN            1
#define HUG_SENS2_IN            2
#define GPIOC_3                 3
#define HAND_SENSOR1_IN         4
#define HAND_SENSOR2_IN         5
#define CAMERA_D0               6
#define CAMERA_D1               7
#define CAMERA_SDA              8
#define CAMERA_SCL              9
#define GPIOC_10                10
#define CAMERA_D4               11
#define GPIOC_12                12
#define GPIOC_13                13
#define CAMERA_ENABLE           14
#define GPIOC_15                15 

#define GPIOD_0                 0
#define GPIOD_1                 1
#define GPIOD_2                 2
#define GPIOD_3                 3
#define GPIOD_4                 4
#define GPIOD_5                 5
#define GPIOD_6                 6
#define GPIOD_7                 7
#define WIFI_UART_TX            8
#define WIFI_UART_RX            9
#define SPI2_CS_SD              10
#define WIFI_UART_CTS           11
#define WIFI_UART_RTS           12
#define WIFI_WAKEUP             13
#define PIR                     14
#define GPIOD_15                15

#define CAMERA_D2               0
#define CAMERA_D3               1
#define GPIOE_2                 2
#define GPIOE_3                 3
#define GPIOE_4                 4
#define GPIOE_5                 5
#define GPIOE_6                 6
#define GPIOE_7                 7
#define LED2_R                  8
#define SPI4_XDCS               9
#define CODEC_DREQ              10
#define SPI4_XCS                11
#define SPI4_SCK                12
#define SPI4_MISO               13
#define SPI4_MOSI               14
#define IMU_INT                 15

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUDR_FLOATING(n)        (0U << ((n) * 2))
#define PIN_PUDR_PULLUP(n)          (1U << ((n) * 2))
#define PIN_PUDR_PULLDOWN(n)        (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * Port A setup.
 *
 * PA0  - LED1_R                  (input floating).
 * PA1  - LED1_G                  (alternate 11).
 * PA2  - LED1_B                  (alternate 11).
 * PA3  - GPIOA_3                 (input floating).
 * PA4  - CAMERA_SYNC             (input pull-up).
 * PA5  - HAND_SENSOR_OUT         (output push-pull).
 * PA6  - CAMERA_PIXCLK           (input pull-up).
 * PA7  - GPIOA_7                 (alternate 11).
 * PA8  - CAMERA_XCLK             (alternate 0).
 * PA9  - USB_VBUS                (input pull-up).
 * PA10 - GPIOA_10                (input pull-up).
 * PA11 - USB_DM                  (alternate 10).
 * PA12 - USB_DP                  (alternate 10).
 * PA13 - SWDIO                   (alternate 0).
 * PA14 - SWCLK                   (alternate 0, pull-down).
 * PA15 - GPIOA_15                (alternate 6).
 */
#define VAL_GPIOA_MODER     (PIN_MODE_INPUT(LED1_R) |                       \
                             PIN_MODE_ALTERNATE(LED1_G) |                   \
                             PIN_MODE_ALTERNATE(LED1_B) |                   \
                             PIN_MODE_INPUT(GPIOA_3) |                      \
                             PIN_MODE_INPUT(CAMERA_SYNC) |                  \
                             PIN_MODE_OUTPUT(HAND_SENSOR_OUT) |             \
                             PIN_MODE_INPUT(CAMERA_PIXCLK) |                \
                             PIN_MODE_ALTERNATE(GPIOA_7) |                  \
                             PIN_MODE_ALTERNATE(CAMERA_XCLK) |              \
                             PIN_MODE_INPUT(USB_VBUS) |                     \
                             PIN_MODE_INPUT(GPIOA_10) |                     \
                             PIN_MODE_ALTERNATE(USB_DM) |                   \
                             PIN_MODE_ALTERNATE(USB_DP) |                   \
                             PIN_MODE_ALTERNATE(SWDIO) |                    \
                             PIN_MODE_ALTERNATE(SWCLK) |                    \
                             PIN_MODE_ALTERNATE(GPIOA_15))
#define VAL_GPIOA_OTYPER    0x00000000
#define VAL_GPIOA_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOA_PUPDR     (PIN_PUDR_PULLUP(CAMERA_SYNC) |                 \
                             PIN_PUDR_PULLUP(CAMERA_PIXCLK) |               \
                             PIN_PUDR_PULLDOWN(USB_VBUS) |                  \
                             PIN_PUDR_PULLUP(GPIOA_10) |                    \
                             PIN_PUDR_PULLDOWN(SWCLK))
#define VAL_GPIOA_ODR       0xFFFFFFDF
#define VAL_GPIOA_AFRL      (PIN_AFIO_AF(LED1_G, 11) |                      \
                             PIN_AFIO_AF(LED1_B, 11) |                      \
                             PIN_AFIO_AF(GPIOA_7, 11))
#define VAL_GPIOA_AFRH      (PIN_AFIO_AF(CAMERA_XCLK, 0) |                  \
                             PIN_AFIO_AF(USB_DM, 10) |                      \
                             PIN_AFIO_AF(USB_DP, 10) |                      \
                             PIN_AFIO_AF(SWDIO, 0) |                        \
                             PIN_AFIO_AF(SWCLK, 0) |                        \
                             PIN_AFIO_AF(GPIOA_15, 6))

/*
 * Port B setup.
 *
 * PB0  - LED2_G                (output push-pull).
 * PB1  - LED2_B                (output push-pull).
 * PB2  - GPIOB_2               (input floating).
 * PB3  - GPIOB_3               (alternate 6).
 * PB4  - GPIOB_4               (input floating).
 * PB5  - GPIOB_5               (alternate 6).
 * PB6  - CAMERA_D5             (input pull-up).
 * PB7  - CAMERA_VSYNC          (input pull-up).
 * PB8  - CAMERA_D6             (alternate 9).
 * PB9  - CAMERA_D7             (alternate 9).
 * PB10 - I2C_SCL               (input floating).
 * PB11 - I2C_SDA               (alternate 11).
 * PB12 - SPI2_CS_WIFI          (alternate 12).
 * PB13 - SPI2_SCK              (input pull-up).
 * PB14 - SPI2_MISO             (alternate 12).
 * PB15 - SPI2_MOSI             (alternate 12).
 */
#define VAL_GPIOB_MODER     (PIN_MODE_OUTPUT(LED2_G) |                      \
                             PIN_MODE_OUTPUT(LED2_B) |                      \
                             PIN_MODE_INPUT(GPIOB_2) |                      \
                             PIN_MODE_ALTERNATE(GPIOB_3) |                  \
                             PIN_MODE_INPUT(GPIOB_4) |                      \
                             PIN_MODE_ALTERNATE(GPIOB_5) |                  \
                             PIN_MODE_INPUT(CAMERA_D5) |                    \
                             PIN_MODE_INPUT(CAMERA_VSYNC) |                 \
                             PIN_MODE_ALTERNATE(CAMERA_D6) |                \
                             PIN_MODE_ALTERNATE(CAMERA_D7) |                \
                             PIN_MODE_INPUT(I2C_SCL) |                      \
                             PIN_MODE_ALTERNATE(I2C_SDA) |                  \
                             PIN_MODE_ALTERNATE(SPI2_CS_WIFI) |             \
                             PIN_MODE_INPUT(SPI2_SCK) |                     \
                             PIN_MODE_ALTERNATE(SPI2_MISO) |                \
                             PIN_MODE_ALTERNATE(SPI2_MOSI))
#define VAL_GPIOB_OTYPER    0x00000000
#define VAL_GPIOB_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOB_PUPDR     (PIN_PUDR_PULLUP(CAMERA_D5) |                   \
                             PIN_PUDR_PULLUP(CAMERA_VSYNC) |                \
                             PIN_PUDR_PULLDOWN(SPI2_SCK))
#define VAL_GPIOB_ODR       0xFFFFFFFC
#define VAL_GPIOB_AFRL      (PIN_AFIO_AF(GPIOB_3, 6) |                      \
                             PIN_AFIO_AF(GPIOB_5, 6))
#define VAL_GPIOB_AFRH      (PIN_AFIO_AF(CAMERA_D6, 9) |                    \
                             PIN_AFIO_AF(CAMERA_D7, 9) |                    \
                             PIN_AFIO_AF(I2C_SDA, 11) |                     \
                             PIN_AFIO_AF(SPI2_CS_WIFI, 12) |                \
                             PIN_AFIO_AF(SPI2_MISO, 12) |                   \
                             PIN_AFIO_AF(SPI2_MOSI, 12))

/*
 * Port C setup.
 *
 * PC0  - HUG_SENS_OUT          (input floating).
 * PC1  - HUG_SENS1_IN          (alternate 11).
 * PC2  - HUG_SENS2_IN          (output push-pull).
 * PC3  - GPIOC_3               (output push-pull).
 * PC4  - HAND_SENSOR1_IN       (alternate 11).
 * PC5  - HAND_SENSOR2_IN       (alternate 11).
 * PC6  - CAMERA_D0             (alternate 8).
 * PC7  - CAMERA_D1             (alternate 6).
 * PC8  - CAMERA_SDA            (input pull-up).
 * PC9  - CAMERA_SCL            (input pull-up).
 * PC10 - GPIOC_10              (alternate 6).
 * PC11 - CAMERA_D4             (alternate 6).
 * PC12 - GPIOC_12              (alternate 6).
 * PC13 - GPIOC_13              (input floating).
 * PC14 - CAMERA_ENABLE         (input floating).
 * PC15 - GPIOC_15              (input floating).
 */
#define VAL_GPIOC_MODER     (PIN_MODE_INPUT(HUG_SENS_OUT) |                 \
                             PIN_MODE_ALTERNATE(HUG_SENS1_IN) |             \
                             PIN_MODE_OUTPUT(HUG_SENS2_IN) |                \
                             PIN_MODE_OUTPUT(GPIOC_3) |                     \
                             PIN_MODE_ALTERNATE(HAND_SENSOR1_IN) |          \
                             PIN_MODE_ALTERNATE(HAND_SENSOR2_IN) |          \
                             PIN_MODE_ALTERNATE(CAMERA_D0) |                \
                             PIN_MODE_ALTERNATE(CAMERA_D1) |                \
                             PIN_MODE_INPUT(CAMERA_SDA) |                   \
                             PIN_MODE_INPUT(CAMERA_SCL) |                   \
                             PIN_MODE_ALTERNATE(GPIOC_10) |                 \
                             PIN_MODE_ALTERNATE(CAMERA_D4) |                \
                             PIN_MODE_ALTERNATE(GPIOC_12) |                 \
                             PIN_MODE_INPUT(GPIOC_13) |                     \
                             PIN_MODE_INPUT(CAMERA_ENABLE) |                \
                             PIN_MODE_INPUT(GPIOC_15))
#define VAL_GPIOC_OTYPER    0x00000000
#define VAL_GPIOC_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOC_PUPDR     (PIN_PUDR_PULLUP(CAMERA_SDA) |                  \
                             PIN_PUDR_PULLUP(CAMERA_SCL))
#define VAL_GPIOC_ODR       0xFFFFFFF3
#define VAL_GPIOC_AFRL      (PIN_AFIO_AF(HUG_SENS1_IN, 11) |                \
                             PIN_AFIO_AF(HAND_SENSOR1_IN, 11) |             \
                             PIN_AFIO_AF(HAND_SENSOR2_IN, 11) |             \
                             PIN_AFIO_AF(CAMERA_D0, 8) |                    \
                             PIN_AFIO_AF(CAMERA_D1, 6))
#define VAL_GPIOC_AFRH      (PIN_AFIO_AF(GPIOC_10, 6) |                     \
                             PIN_AFIO_AF(CAMERA_D4, 6) |                    \
                             PIN_AFIO_AF(GPIOC_12, 6))

/*
 * Port D setup.
 *
 * PD0  - GPIOD_0               (input pull-up).
 * PD1  - GPIOD_1               (input pull-up).
 * PD2  - GPIOD_2               (output opendrain).
 * PD3  - GPIOD_3               (output push-pull).
 * PD4  - GPIOD_4               (input pull-up).
 * PD5  - GPIOD_5               (input pull-up).
 * PD6  - GPIOD_6               (output push-pull).
 * PD7  - GPIOD_7               (input pull-up).
 * PD8  - WIFI_UART_TX          (alternate 8).
 * PD9  - WIFI_UART_RX          (alternate 8).
 * PD10 - SPI2_CS_SD            (input pull-up).
 * PD11 - WIFI_UART_CTS         (alternate 8).
 * PD12 - WIFI_UART_RTS         (alternate 8).
 * PD13 - WIFI_WAKEUP           (input floating).
 * PD14 - PIR                   (input pull-up).
 * PD15 - GPIOD_15              (input pull-up).
 */
#define VAL_GPIOD_MODER     (PIN_MODE_INPUT(GPIOD_0) |                      \
                             PIN_MODE_INPUT(GPIOD_1) |                      \
                             PIN_MODE_OUTPUT(GPIOD_2) |                     \
                             PIN_MODE_OUTPUT(GPIOD_3) |                     \
                             PIN_MODE_INPUT(GPIOD_4) |                      \
                             PIN_MODE_INPUT(GPIOD_5) |                      \
                             PIN_MODE_OUTPUT(GPIOD_6) |                     \
                             PIN_MODE_INPUT(GPIOD_7) |                      \
                             PIN_MODE_ALTERNATE(WIFI_UART_TX) |             \
                             PIN_MODE_ALTERNATE(WIFI_UART_RX) |             \
                             PIN_MODE_INPUT(SPI2_CS_SD) |                   \
                             PIN_MODE_ALTERNATE(WIFI_UART_CTS) |            \
                             PIN_MODE_ALTERNATE(WIFI_UART_RTS) |            \
                             PIN_MODE_INPUT(WIFI_WAKEUP) |                  \
                             PIN_MODE_INPUT(PIR) |                          \
                             PIN_MODE_INPUT(GPIOD_15))
#define VAL_GPIOD_OTYPER    PIN_OTYPE_OPENDRAIN(GPIOD_2)
#define VAL_GPIOD_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOD_PUPDR     (PIN_PUDR_PULLUP(GPIOD_0) |                     \
                             PIN_PUDR_PULLUP(GPIOD_1) |                     \
                             PIN_PUDR_PULLUP(GPIOD_4) |                     \
                             PIN_PUDR_PULLUP(GPIOD_5) |                     \
                             PIN_PUDR_PULLUP(GPIOD_7) |                     \
                             PIN_PUDR_PULLUP(SPI2_CS_SD) |                  \
                             PIN_PUDR_PULLUP(PIR) |                         \
                             PIN_PUDR_PULLUP(GPIOD_15))
#define VAL_GPIOD_ODR       0xFFFFFFFF
#define VAL_GPIOD_AFRL      0x00000000
#define VAL_GPIOD_AFRH      (PIN_AFIO_AF(WIFI_UART_TX, 7) |                 \
                             PIN_AFIO_AF(WIFI_UART_RX, 7) |                 \
                             PIN_AFIO_AF(WIFI_UART_CTS, 7) |                \
                             PIN_AFIO_AF(WIFI_UART_RTS, 7))

/*
 * Port E setup.
 *
 * PE0  - CAMERA_D2             (input pull-up).
 * PE1  - CAMERA_D3             (input pull-up).
 * PE2  - GPIOE_2               (input floating).
 * PE3  - GPIOE_3               (output push-pull).
 * PE4  - GPIOE_4               (input pull-up).
 * PE5  - GPIOE_5               (input pull-up).
 * PE6  - GPIOE_6               (input pull-up).
 * PE7  - GPIOE_7               (input pull-up).
 * PE8  - LED2_R                (input pull-up).
 * PE9  - SPI4_XDCS             (input pull-up).
 * PE10 - CODEC_DREQ            (input pull-up).
 * PE11 - SPI4_XCS              (input pull-up).
 * PE12 - SPI4_SCK              (input pull-up).
 * PE13 - SPI4_MISO             (input pull-up).
 * PE14 - SPI4_MOSI             (input pull-up).
 * PE15 - IMU_INT              (input pull-up).
 */
#define VAL_GPIOE_MODER     (PIN_MODE_INPUT(CAMERA_D2) |                    \
                             PIN_MODE_INPUT(CAMERA_D3) |                    \
                             PIN_MODE_INPUT(GPIOE_2) |                      \
                             PIN_MODE_OUTPUT(GPIOE_3) |                     \
                             PIN_MODE_INPUT(GPIOE_4) |                      \
                             PIN_MODE_INPUT(GPIOE_5) |                      \
                             PIN_MODE_INPUT(GPIOE_6) |                      \
                             PIN_MODE_INPUT(GPIOE_7) |                      \
                             PIN_MODE_INPUT(LED2_R) |                       \
                             PIN_MODE_INPUT(SPI4_XDCS) |                    \
                             PIN_MODE_INPUT(CODEC_DREQ) |                   \
                             PIN_MODE_INPUT(SPI4_XCS) |                     \
                             PIN_MODE_INPUT(SPI4_SCK) |                     \
                             PIN_MODE_INPUT(SPI4_MISO) |                    \
                             PIN_MODE_INPUT(SPI4_MOSI) |                    \
                             PIN_MODE_INPUT(IMU_INT))
#define VAL_GPIOE_OTYPER    0x00000000
#define VAL_GPIOE_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOE_PUPDR     (PIN_PUDR_PULLUP(CAMERA_D2) |                   \
                             PIN_PUDR_PULLUP(CAMERA_D3) |                   \
                             PIN_PUDR_PULLUP(GPIOE_4) |                     \
                             PIN_PUDR_PULLUP(GPIOE_5) |                     \
                             PIN_PUDR_PULLUP(GPIOE_6) |                     \
                             PIN_PUDR_PULLUP(GPIOE_7) |                     \
                             PIN_PUDR_PULLUP(LED2_R) |                      \
                             PIN_PUDR_PULLUP(SPI4_XDCS) |                   \
                             PIN_PUDR_PULLUP(CODEC_DREQ) |                  \
                             PIN_PUDR_PULLUP(SPI4_XCS) |                    \
                             PIN_PUDR_PULLUP(SPI4_SCK) |                    \
                             PIN_PUDR_PULLUP(SPI4_MISO) |                   \
                             PIN_PUDR_PULLUP(SPI4_MOSI) |                   \
                             PIN_PUDR_PULLUP(IMU_INT))
#define VAL_GPIOE_ODR       0xFFFFFFF7
#define VAL_GPIOE_AFRL      0x00000000
#define VAL_GPIOE_AFRH      0x00000000


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */

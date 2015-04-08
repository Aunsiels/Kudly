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
 * Setup for Kudly STM32-F427 board.
 * NOTE: Part of JTAG signals are used for other functions, this board can be
 *       used using SWD only.
 */

/*
 * Board identifier.
 */
#define BOARD_KUDLY_STM32_F427
#define BOARD_NAME              "Kudly STM32-F427"

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
#define STM32F427_437xx

/*
 * IO pins assignments.
 */
#define GPIOA_LED1_R            0
#define GPIOA_LED1_G            1
#define GPIOA_LED1_B            2
#define GPIOA_3                 3
#define GPIOA_CAMERA_HSYNC      4
#define GPIOA_HAND_SENSOR_OUT   5
#define GPIOA_CAMERA_PIXCLK     6
#define GPIOA_7                 7
#define GPIOA_CAMERA_XCLK       8
#define GPIOA_USB_VBUS          9
#define GPIOA_10                10
#define GPIOA_USB_DM            11
#define GPIOA_USB_DP            12
#define GPIOA_SWDIO             13
#define GPIOA_SWCLK             14
#define GPIOA_15                15

#define GPIOB_LED2_G            0
#define GPIOB_LED2_R            1
#define GPIOB_2                 2
#define GPIOB_3                 3
#define GPIOB_4                 4
#define GPIOB_5                 5
#define GPIOB_CAMERA_D5         6
#define GPIOB_CAMERA_VSYNC      7
#define GPIOB_CAMERA_D6         8
#define GPIOB_CAMERA_D7         9
#define GPIOB_I2C_SCL           10
#define GPIOB_I2C_SDA           11
#define GPIOB_SPI2_CS_WIFI      12
#define GPIOB_SPI2_SCK          13
#define GPIOB_SPI2_MISO         14
#define GPIOB_SPI2_MOSI         15

#define GPIOC_HUG_SENS_OUT      0
#define GPIOC_HUG_SENS1_IN      1
#define GPIOC_HUG_SENS2_IN      2
#define GPIOC_3                 3
#define GPIOC_HAND_SENSOR1_IN   4
#define GPIOC_HAND_SENSOR2_IN   5
#define GPIOC_CAMERA_D0         6
#define GPIOC_CAMERA_D1         7
#define GPIOC_CAMERA_SDA        8
#define GPIOC_CAMERA_SCL        9
#define GPIOC_10                10
#define GPIOC_CAMERA_D4         11
#define GPIOC_12                12
#define GPIOC_13                13
#define GPIOC_CAMERA_ENABLE     14
#define GPIOC_15                15 

#define GPIOD_0                 0
#define GPIOD_1                 1
#define GPIOD_2                 2
#define GPIOD_3                 3
#define GPIOD_4                 4
#define GPIOD_5                 5
#define GPIOD_6                 6
#define GPIOD_7                 7
#define GPIOD_WIFI_UART_TX      8
#define GPIOD_WIFI_UART_RX      9
#define GPIOD_SPI2_CS_SD        10
#define GPIOD_WIFI_UART_CTS     11
#define GPIOD_WIFI_UART_RTS     12
#define GPIOD_WIFI_WAKEUP       13
#define GPIOD_PIR               14
#define GPIOD_15                15

#define GPIOE_CAMERA_D2         0
#define GPIOE_CAMERA_D3         1
#define GPIOE_2                 2
#define GPIOE_3                 3
#define GPIOE_4                 4
#define GPIOE_5                 5
#define GPIOE_6                 6
#define GPIOE_7                 7
#define GPIOE_LED2_B            8
#define GPIOE_SPI4_XDCS         9
#define GPIOE_CODEC_DREQ        10
#define GPIOE_SPI4_XCS          11
#define GPIOE_SPI4_SCK          12
#define GPIOE_SPI4_MISO         13
#define GPIOE_SPI4_MOSI         14
#define GPIOE_IMU_INT           15

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
 * PA0  - GPIOA_LED1_R            (alternate 2).
 * PA1  - GPIOA_LED1_G            (alternate 2).
 * PA2  - GPIOA_LED1_B            (alternate 2).
 * PA3  - GPIOA_3                 (input pull-up).
 * PA4  - GPIOA_CAMERA_HSYNC      (alternate 13).
 * PA5  - GPIOA_HAND_SENSOR_OUT   (output push-pull).
 * PA6  - GPIOA_CAMERA_PIXCLK     (alternate 13).
 * PA7  - GPIOA_7                 (input pull-up).
 * PA8  - GPIOA_CAMERA_XCLK       (alternate 0).
 * PA9  - GPIOA_USB_VBUS          (alternate 10).
 * PA10 - GPIOA_10                (input pull-up).
 * PA11 - GPIOA_USB_DM            (alternate 10).
 * PA12 - GPIOA_USB_DP            (alternate 10).
 * PA13 - GPIOA_SWDIO             (alternate 0).
 * PA14 - GPIOA_SWCLK             (alternate 0, pull-down).
 * PA15 - GPIOA_15                (input pull-up).
 */
#define VAL_GPIOA_MODER     0xA8000000
#define VAL_GPIOA_OTYPER    0x00000000
#define VAL_GPIOA_OSPEEDR   0x00000000
#define VAL_GPIOA_PUPDR     0x64000000
#define VAL_GPIOA_ODR       0x00000000
#define VAL_GPIOA_AFRL      0x00000000
#define VAL_GPIOA_AFRH      0x00000000

/*
 * Port B setup.
 *
 * PB0  - GPIOB_LED2_G          (alternate 1).
 * PB1  - GPIOB_LED2_B          (alternate 1).
 * PB2  - GPIOB_2               (input pull-up).
 * PB3  - GPIOB_3               (input pull-up).
 * PB4  - GPIOB_4               (input pull-up).
 * PB5  - GPIOB_5               (input pull-up).
 * PB6  - GPIOB_CAMERA_D5       (alternate 13).
 * PB7  - GPIOB_CAMERA_VSYNC    (alternate 13).
 * PB8  - GPIOB_CAMERA_D6       (alternate 13).
 * PB9  - GPIOB_CAMERA_D7       (alternate 13).
 * PB10 - GPIOB_I2C_SCL         (alternate 4).
 * PB11 - GPIOB_I2C_SDA         (alternate 4).
 * PB12 - GPIOB_SPI2_CS_WIFI    (output push-pull).
 * PB13 - GPIOB_SPI2_SCK        (alternate 5).
 * PB14 - GPIOB_SPI2_MISO       (alternate 5).
 * PB15 - GPIOB_SPI2_MOSI       (alternate 5).
 */
#define VAL_GPIOB_MODER     0x00000280
#define VAL_GPIOB_OTYPER    0x00000000
#define VAL_GPIOB_OSPEEDR   0x000000C0
#define VAL_GPIOB_PUPDR     0x00000100
#define VAL_GPIOB_ODR       0x00000000
#define VAL_GPIOB_AFRL      0x00000000
#define VAL_GPIOB_AFRH      0x00000000

/*
 * Port C setup.
 *
 * PC0  - GPIOC_HUG_SENS_OUT    (output push-pull).
 * PC1  - GPIOC_HUG_SENS1_IN    (analog).
 * PC2  - GPIOC_HUG_SENS2_IN    (analog).
 * PC3  - GPIOC_3               (input pull-up).
 * PC4  - GPIOC_HAND_SENSOR1_IN (analog).
 * PC5  - GPIOC_HAND_SENSOR2_IN (analog).
 * PC6  - GPIOC_CAMERA_D0       (alternate 13).
 * PC7  - GPIOC_CAMERA_D1       (alternate 13).
 * PC8  - GPIOC_CAMERA_SDA      (output open-drain).
 * PC9  - GPIOC_CAMERA_SCL      (output open-drain).
 * PC10 - GPIOC_10              (input pull-up).
 * PC11 - GPIOC_CAMERA_D4       (alternate 13).
 * PC12 - GPIOC_12              (input pull-up).
 * PC13 - GPIOC_13              (input pull-up).
 * PC14 - GPIOC_CAMERA_ENABLE   (output push-pull).
 * PC15 - GPIOC_15              (input pull-up).
 */
#define VAL_GPIOC_MODER     0x00000000
#define VAL_GPIOC_OTYPER    0x00000000
#define VAL_GPIOC_OSPEEDR   0x00000000
#define VAL_GPIOC_PUPDR     0x00000000
#define VAL_GPIOC_ODR       0x00000000
#define VAL_GPIOC_AFRL      0x00000000
#define VAL_GPIOC_AFRH      0x00000000

/*
 * Port D setup.
 *
 * PD0  - GPIOD_0               (input pull-up).
 * PD1  - GPIOD_1               (input pull-up).
 * PD2  - GPIOD_2               (input pull-up).
 * PD3  - GPIOD_3               (input pull-up).
 * PD4  - GPIOD_4               (input pull-up).
 * PD5  - GPIOD_5               (input pull-up).
 * PD6  - GPIOD_6               (input pull-up).
 * PD7  - GPIOD_7               (input pull-up).
 * PD8  - GPIOD_WIFI_UART_TX    (alternate 7).
 * PD9  - GPIOD_WIFI_UART_RX    (alternate 7).
 * PD10 - GPIOD_SPI2_CS_SD      (output push-pull).
 * PD11 - GPIOD_WIFI_UART_CTS   (alternate 7).
 * PD12 - GPIOD_WIFI_UART_RTS   (alternate 7).
 * PD13 - GPIOD_WIFI_WAKEUP     (output push-pull).
 * PD14 - GPIOD_PIR             (input pull-up).
 * PD15 - GPIOD_15              (input pull-up).
 */
#define VAL_GPIOD_MODER     0x00000000
#define VAL_GPIOD_OTYPER    0x00000000
#define VAL_GPIOD_OSPEEDR   0x00000000
#define VAL_GPIOD_PUPDR     0x00000000
#define VAL_GPIOD_ODR       0x00000000
#define VAL_GPIOD_AFRL      0x00000000
#define VAL_GPIOD_AFRH      0x00000000

/*
 * Port E setup.
 *
 * PE0  - GPIOE_CAMERA_D2       (alternate 13).
 * PE1  - GPIOE_CAMERA_D3       (alternate 13).
 * PE2  - GPIOE_2               (input pull-up).
 * PE3  - GPIOE_3               (input pull-up).
 * PE4  - GPIOE_4               (input pull-up).
 * PE5  - GPIOE_5               (input pull-up).
 * PE6  - GPIOE_6               (input pull-up).
 * PE7  - GPIOE_7               (input pull-up).
 * PE8  - GPIOE_LED2_R          (alternate 1).
 * PE9  - GPIOE_SPI4_XDCS       (output push-pull).
 * PE10 - GPIOE_CODEC_DREQ      (input pull-up).
 * PE11 - GPIOE_SPI4_XCS        (output push-pull).
 * PE12 - GPIOE_SPI4_SCK        (alternate 5).
 * PE13 - GPIOE_SPI4_MISO       (alternate 5).
 * PE14 - GPIOE_SPI4_MOSI       (alternate 5).
 * PE15 - GPIOE_IMU_INT         (input pull-up).
 */
#define VAL_GPIOE_MODER     0x00000000
#define VAL_GPIOE_OTYPER    0x00000000
#define VAL_GPIOE_OSPEEDR   0x00000000
#define VAL_GPIOE_PUPDR     0x00000000
#define VAL_GPIOE_ODR       0x00000000
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

/**
******************************************************************************
* @file    platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides all MICO Peripherals mapping table and platform
*          specific funcgtions.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "stdio.h"
#include "string.h"

#include "platform.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "PlatformLogging.h"
#include "MicoPlatform.h"
#include "wlan_platform_common.h"
#include "spi_flash_platform_interface.h"

/******************************************************
*                      Macros
******************************************************/


/******************************************************
*                    Constants
******************************************************/

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/
extern WEAK void PlatformEasyLinkButtonClickedCallback(void);
extern WEAK void PlatformStandbyButtonClickedCallback(void);
extern WEAK void PlatformEasyLinkButtonLongPressedCallback(void);
extern WEAK void bootloader_start(void);

/******************************************************
*               Variables Definitions
******************************************************/

/* This table maps STM32 pins to GPIO definitions on the schematic
* A full pin definition is provided in <WICED-SDK>/include/platforms/BCM943362WCD4/platform.h
*/

static uint32_t _default_start_time = 0;
static mico_timer_t _button_EL_timer;

const platform_gpio_t platform_gpio_pins[] =
{
  /* Common GPIOs for internal use */
  [STDIO_UART_TX]                       = { IOPORT_CREATE_PIN( PIOA, 10 ),  false, 0, 0 },
  [STDIO_UART_RX]                       = { IOPORT_CREATE_PIN( PIOA,  9 ),  false, 0, 0 },  

  [FLASH_PIN_SPI_CS  ]                  = { IOPORT_CREATE_PIN( PIOA, 30 ),  false, 0, 0 }, 
  [FLASH_PIN_SPI_CLK ]                  = { IOPORT_CREATE_PIN( PIOA, 29 ),  false, 0, 0 },
  [FLASH_PIN_SPI_MOSI]                  = { IOPORT_CREATE_PIN( PIOA, 28 ),  false, 0, 0 },
  [FLASH_PIN_SPI_MISO]                  = { IOPORT_CREATE_PIN( PIOA, 27 ),  false, 0, 0 },

  [MICO_SYS_LED]                        = { IOPORT_CREATE_PIN( PIOA, 16 ),  false, 0, 0 },
  [MICO_RF_LED]                         = { IOPORT_CREATE_PIN( PIOA, 24 ),  false, 0, 0 },
  [EasyLink_BUTTON]                     = { IOPORT_CREATE_PIN( PIOA,  2 ),  false, 0, 0 },
  
  [BOOT_SEL]                            = { IOPORT_CREATE_PIN( PIOB, 14 ),  false, 0, 0 },
  [MFG_SEL]                             = { IOPORT_CREATE_PIN( PIOB, 15 ),  false, 0, 0 },
 

  /* GPIOs for external use */
  [Arduino_RXD]                         = { IOPORT_CREATE_PIN( PIOB,  1 ),  false, 0, 0  },
  [Arduino_TXD]                         = { IOPORT_CREATE_PIN( PIOB,  0 ),  false, 0, 0 },
  [Arduino_D2]                          = { IOPORT_CREATE_PIN( PIOA, 21 ),  false, 0, 0  },
  [Arduino_D3]                          = { IOPORT_CREATE_PIN( PIOA, 22 ),  false, 0, 0  }, 
  [Arduino_D4]                          = { IOPORT_CREATE_PIN( PIOA, 23 ),  false, 0, 0  }, 
  [Arduino_D5]                          = { IOPORT_CREATE_PIN( PIOA,  6 ),  false, 0, 0  }, 
  [Arduino_D6]                          = { IOPORT_CREATE_PIN( PIOA, 25 ),  false, 0, 0  }, 
  [Arduino_D7]                          = { IOPORT_CREATE_PIN( PIOB, 13 ),  false, 0, 0  }, 
  [Arduino_D8]                          = { IOPORT_CREATE_PIN( PIOA,  0 ),  false, 0, 0  }, 
  [Arduino_D9]                          = { IOPORT_CREATE_PIN( PIOA,  1 ),  false, 0, 0  }, 
  [Arduino_CS]                          = { IOPORT_CREATE_PIN( PIOA, 31 ),  false, 0, 0  }, 
  [Arduino_SI]                          = { IOPORT_CREATE_PIN( PIOA, 28 ),  false, 0, 0  }, 
  [Arduino_SO]                          = { IOPORT_CREATE_PIN( PIOA, 27 ),  false, 0, 0  }, 
  [Arduino_SCK]                         = { IOPORT_CREATE_PIN( PIOA, 29 ),  false, 0, 0  }, 
  [Arduino_SDA]                         = { IOPORT_CREATE_PIN( PIOA,  3 ),  false, 0, 0  }, 
  [Arduino_SCL]                         = { IOPORT_CREATE_PIN( PIOA,  4 ),  false, 0, 0  }, 

};

const platform_adc_t *platform_adc_peripherals = NULL;


/* PWM mappings */
const platform_pwm_t *platform_pwm_peripherals = NULL;

const platform_spi_t platform_spi_peripherals [] =  
{
  [MICO_SPI_1] =
  {
    .spi_id                       = 7,
    .port                         = SPI7,
    .flexcom_base                 = FLEXCOM7,
    .peripheral_id                = ID_FLEXCOM7,
    .mosi_pin                     = &platform_gpio_pins[FLASH_PIN_SPI_MOSI],
    .mosi_pin_mux_mode            = IOPORT_MODE_MUX_B,
    .miso_pin                     = &platform_gpio_pins[FLASH_PIN_SPI_MISO],
    .miso_pin_mux_mode            = IOPORT_MODE_MUX_B,
    .clock_pin                    = &platform_gpio_pins[FLASH_PIN_SPI_CLK],
    .clock_pin_mux_mode           = IOPORT_MODE_MUX_B,
  },
};

platform_spi_driver_t platform_spi_drivers[MICO_SPI_MAX];

const platform_uart_t platform_uart_peripherals[] =
{
  [MICO_UART_1] =
  {
    .uart_id          = 0,
    .port             = USART0,
    .flexcom_base     = FLEXCOM0,
    .peripheral_id    = ID_FLEXCOM0,
    .tx_pin           = &platform_gpio_pins[STDIO_UART_TX],
    .tx_pin_mux_mode  = IOPORT_MODE_MUX_A,
    .rx_pin           = &platform_gpio_pins[STDIO_UART_RX],
    .rx_pin_mux_mode  = IOPORT_MODE_MUX_A,
    .cts_pin          = NULL, /* flow control isn't supported */
    .cts_pin_mux_mode = IOPORT_MODE_MUX_A,
    .rts_pin          = NULL, /* flow control isn't supported */
    .rts_pin_mux_mode = IOPORT_MODE_MUX_A,
  },
  [MICO_UART_2] =
  {
    .uart_id          = 6,
    .port             = USART6,
    .flexcom_base     = FLEXCOM6,
    .peripheral_id    = ID_FLEXCOM6,
    .tx_pin           = &platform_gpio_pins[Arduino_TXD],
    .tx_pin_mux_mode  = IOPORT_MODE_MUX_B,
    .rx_pin           = &platform_gpio_pins[Arduino_RXD],
    .rx_pin_mux_mode  = IOPORT_MODE_MUX_B,
    .cts_pin          = NULL, /* flow control isn't supported */
    .cts_pin_mux_mode = IOPORT_MODE_MUX_B,
    .rts_pin          = NULL, /* flow control isn't supported */
    .rts_pin_mux_mode = IOPORT_MODE_MUX_B,
  },
};

platform_uart_driver_t platform_uart_drivers[MICO_UART_MAX];


const platform_i2c_t platform_i2c_peripherals[] = 
{
  [MICO_I2C_1] =
  {
    .i2c_id           = 3,
    .port             = TWI3,
    .flexcom_base     = FLEXCOM3,
    .peripheral_id    = ID_FLEXCOM3,
    .sda_pin          = &platform_gpio_pins[Arduino_SDA],
    .sda_pin_mux_mode = IOPORT_MODE_MUX_A,  
    .scl_pin          = &platform_gpio_pins[Arduino_SCL],
    .scl_pin_mux_mode = IOPORT_MODE_MUX_A,  
  },
};

const platform_flash_t platform_flash_peripherals[] =
{
  [MICO_SPI_FLASH] =
  {
    .flash_type                   = FLASH_TYPE_SPI,
    .flash_start_addr             = 0x000000,
    .flash_length                 = 0x200000,
  },
  [MICO_INTERNAL_FLASH] =
  {
    .flash_type                   = FLASH_TYPE_INTERNAL,
    .flash_start_addr             = 0x00400000,
    .flash_length                 = 0x80000,
  },
};

platform_flash_driver_t platform_flash_drivers[MICO_FLASH_MAX];

/* Wi-Fi control pins. Used by platform/MCU/wlan_platform_common.c
* SDIO: EMW1062_PIN_BOOTSTRAP[1:0] = b'00
* gSPI: EMW1062_PIN_BOOTSTRAP[1:0] = b'01
*/
const platform_gpio_t wifi_control_pins[] =
{
  [WIFI_PIN_RESET      ] = { IOPORT_CREATE_PIN( PIOA, 26 ),  false, 0, 0 },
};

/* Wi-Fi gSPI bus pins. Used by platform/MCU/STM32F2xx/EMW1062_driver/wlan_spi.c */
const platform_gpio_t wifi_spi_pins[] =
{
  [WIFI_PIN_SPI_IRQ ] = { IOPORT_CREATE_PIN( PIOB, 10 ),  false, 0, 0 },
  [WIFI_PIN_SPI_CS  ] = { IOPORT_CREATE_PIN( PIOA, 11 ),  false, 0, 0 },
  [WIFI_PIN_SPI_CLK ] = { IOPORT_CREATE_PIN( PIOA, 14 ),  false, 0, 0 },
  [WIFI_PIN_SPI_MOSI] = { IOPORT_CREATE_PIN( PIOA, 13 ),  false, 0, 0 },
  [WIFI_PIN_SPI_MISO] = { IOPORT_CREATE_PIN( PIOA, 12 ),  false, 0, 0 },
};

const platform_spi_t wifi_spi =
{
  .spi_id                       = 5,
  .port                         = SPI5,
  .flexcom_base                 = FLEXCOM5,
  .mosi_pin                     = &wifi_spi_pins[WIFI_PIN_SPI_MOSI],
  .mosi_pin_mux_mode            = IOPORT_MODE_MUX_A,
  .miso_pin                     = &wifi_spi_pins[WIFI_PIN_SPI_MISO],
  .miso_pin_mux_mode            = IOPORT_MODE_MUX_A,
  .clock_pin                    = &wifi_spi_pins[WIFI_PIN_SPI_CLK],
  .clock_pin_mux_mode           = IOPORT_MODE_MUX_A,
};

#if defined ( USE_MICO_SPI_FLASH )
const mico_spi_device_t mico_spi_flash =
{
  .port        = MICO_SPI_1,
  .chip_select = FLASH_PIN_SPI_CS,
  .speed       = 40000000,
  .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST),
  .bits        = 8,
};
#endif


/******************************************************
*           Interrupt Handler Definitions
******************************************************/

MICO_RTOS_DEFINE_ISR( FLEXCOM0_Handler )
{
    platform_uart_irq( &platform_uart_drivers[MICO_UART_1] );
}

MICO_RTOS_DEFINE_ISR( FLEXCOM6_Handler )
{
    platform_uart_irq( &platform_uart_drivers[MICO_UART_2] );
}

#ifndef BOOTLOADER
MICO_RTOS_DEFINE_ISR( FLEXCOM5_Handler )
{
    platform_wifi_spi_rx_dma_irq( );
}
#endif


/******************************************************
*               Function Definitions
******************************************************/

static void _button_EL_irq_handler( void* arg )
{
  (void)(arg);
  int interval = -1;
  
  if ( MicoGpioInputGet( (mico_gpio_t)EasyLink_BUTTON ) == 0 ) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
    MicoGpioEnableIRQ( (mico_gpio_t)EasyLink_BUTTON, IRQ_TRIGGER_RISING_EDGE, _button_EL_irq_handler, NULL );
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
    }
    MicoGpioEnableIRQ( (mico_gpio_t)EasyLink_BUTTON, IRQ_TRIGGER_FALLING_EDGE, _button_EL_irq_handler, NULL );
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}


static void _button_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  _default_start_time = 0;
  PlatformEasyLinkButtonLongPressedCallback();
}

void platform_init_peripheral_irq_priorities( void )
{
  NVIC_SetPriority  ( PIOA_IRQn,      14 );
  NVIC_SetPriority  ( PIOB_IRQn,      14 );
  NVIC_SetPriority  ( FLEXCOM0_IRQn,   6 );  /* STDIO UART  */
  NVIC_SetPriority  ( FLEXCOM5_IRQn,   3 );  /* WLAN SPI    */
  NVIC_SetPriority  ( FLEXCOM3_IRQn,   3 );  /* I2C  on Arduino  */
  NVIC_SetPriority  ( RTT_IRQn,        1 );  /* RTT Wake-up event */
}

void init_platform( void )
{
  MicoGpioInitialize( MICO_SYS_LED, OUTPUT_PUSH_PULL );
  MicoGpioOutputHigh( MICO_SYS_LED );
  MicoGpioInitialize( MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
  MicoGpioOutputHigh( MICO_RF_LED );
//  
  //  Initialise EasyLink buttons
  MicoGpioInitialize( EasyLink_BUTTON, INPUT_PULL_UP );
  mico_init_timer(&_button_EL_timer, RestoreDefault_TimeOut, _button_EL_Timeout_handler, NULL);
  MicoGpioEnableIRQ( EasyLink_BUTTON, IRQ_TRIGGER_FALLING_EDGE, _button_EL_irq_handler, NULL );

#if defined ( USE_MICO_SPI_FLASH )
  MicoFlashInitialize( MICO_SPI_FLASH );
#endif
}

void init_platform_bootloader( void )
{
  MicoGpioInitialize( MICO_SYS_LED, OUTPUT_PUSH_PULL );
  MicoGpioOutputHigh( MICO_SYS_LED );
  MicoGpioInitialize( MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
  MicoGpioOutputHigh( MICO_RF_LED );
  
  MicoGpioInitialize(BOOT_SEL, INPUT_PULL_UP);
  MicoGpioInitialize(MFG_SEL, INPUT_PULL_UP);
  
#if defined ( USE_MICO_SPI_FLASH )
  MicoFlashInitialize( MICO_SPI_FLASH );
#endif
}

void MicoSysLed(bool onoff)
{
    if (onoff) {
        MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
    } else {
        MicoGpioOutputHigh( (mico_gpio_t)MICO_SYS_LED );
    }
}

void MicoRfLed(bool onoff)
{
    if (onoff) {
        MicoGpioOutputLow( (mico_gpio_t)MICO_RF_LED );
    } else {
        MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );
    }
}

bool MicoShouldEnterMFGMode(void)
{
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==false)
    return true;
  else
    return false;
}

bool MicoShouldEnterBootloader(void)
{
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==true)
    return true;
  else
   return false;
}



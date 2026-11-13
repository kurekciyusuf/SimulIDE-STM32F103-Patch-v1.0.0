/*
 * STM32F103 Bare-metal Blink & UART Test for SimulIDE 2
 *
 * Toggles PC13 LED at ~1 Hz and transmits messages on USART1 (PA9).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdint.h>

/* Registers */
#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

#define GPIOA_BASE      0x40010800
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))

#define GPIOC_BASE      0x40011000
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))
#define GPIOC_BSRR      (*(volatile uint32_t *)(GPIOC_BASE + 0x10))
#define GPIOC_BRR       (*(volatile uint32_t *)(GPIOC_BASE + 0x14))

#define USART1_BASE     0x40013800
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

/* Linker symbols */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void Default_Handler(void);

__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    &_estack,
    Reset_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    0, 0, 0, 0,
    Default_Handler,
    Default_Handler,
    0,
    Default_Handler,
    Default_Handler
};

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile("nop");
    }
}

static void uart_send_char(char c)
{
    while (!(USART1_SR & (1 << 7))); // Wait for TXE
    USART1_DR = (uint32_t)c;
}

static void uart_send_string(const char *s)
{
    while (*s) {
        uart_send_char(*s++);
    }
}

void Reset_Handler(void)
{
    /* Copy .data from Flash to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero out .bss */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* Enable clocks for GPIOA, GPIOC, and USART1 */
    RCC_APB2ENR |= (1 << 2) | (1 << 4) | (1 << 14);

    /* Configure PC13 as Output Push-Pull 50MHz: CRH bits [23:20] = 0b0011 */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);

    /* Configure PA9 (USART1 TX) as Alternate Function Push-Pull 50MHz: CRH bits [7:4] = 0b1011 */
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |= (0xB << 4);

    /* Configure USART1: 9600 baud @ 24MHz PCLK2 (BRR = 24000000 / 9600 = 2500 = 0x09C4) */
    USART1_BRR = 0x09C4;
    USART1_CR1 = (1 << 13) | (1 << 3); // UE | TE

    uart_send_string("\r\n=== STM32F103 SimulIDE 2 Boot ===\r\n");

    uint32_t cycle = 0;
    while (1) {
        /* Turn LED ON (Active Low on PC13: Reset bit 13) */
        GPIOC_BRR = (1 << 13);
        uart_send_string("[STM32] LED ON\r\n");
        delay(300000);

        /* Turn LED OFF (Set bit 13 via BSRR) */
        GPIOC_BSRR = (1 << 13);
        uart_send_string("[STM32] LED OFF\r\n");
        delay(300000);

        cycle++;
    }
}

void Default_Handler(void)
{
    while (1);
}


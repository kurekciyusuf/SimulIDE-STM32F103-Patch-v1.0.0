/*
 * STM32F103 USART1 RXNE Interrupt Echo for SimulIDE 2
 *
 * M4 Acceptance Test (b)
 * A keystroke in SimulIDE's Serial Monitor triggers USART1 RXNE interrupt;
 * the guest ISR echoes it back.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdint.h>

#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

#define GPIOA_BASE      0x40010800
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))

#define GPIOC_BASE      0x40011000
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#define USART1_BASE     0x40013800
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

/* NVIC ISER1: IRQs 32..63 (USART1 is IRQ 37 -> bit 37-32 = 5) */
#define NVIC_ISER1      (*(volatile uint32_t *)0xE000E104)

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void);
void Default_Handler(void);
void USART1_IRQHandler(void);

__attribute__((section(".isr_vector")))
const void *vector_table[54] = {
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
    Default_Handler,
    /* IRQ 0..36 filled with Default_Handler */
    [16 ... 52] = Default_Handler,
    /* IRQ 37: USART1 */
    [53] = USART1_IRQHandler
};

static void uart_putc(char c)
{
    while (!(USART1_SR & (1 << 7))); // Wait for TXE
    USART1_DR = (uint32_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

void USART1_IRQHandler(void)
{
    if (USART1_SR & (1 << 5)) { // RXNE
        char rx_char = (char)(USART1_DR & 0xFF); // Reading DR clears RXNE

        /* Toggle PC13 on each received character */
        GPIOC_ODR ^= (1 << 13);

        /* Echo back character with framing */
        uart_putc('[');
        uart_putc(rx_char);
        uart_putc(']');
    }
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    /* Enable GPIOA, GPIOC, and USART1 clocks */
    RCC_APB2ENR |= (1 << 2) | (1 << 4) | (1 << 14);

    /* PC13 = Output Push-Pull 50MHz */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);
    GPIOC_ODR |= (1 << 13); // LED off

    /* PA9 (TX) = AF Push-Pull 50MHz: CRH bits [7:4] = 0b1011 */
    /* PA10 (RX) = Input Floating: CRH bits [11:8] = 0b0100 */
    GPIOA_CRH &= ~(0xFF << 4);
    GPIOA_CRH |= (0x4B << 4);

    /* USART1: 9600 baud @ 24MHz PCLK2 (BRR = 0x09C4) */
    USART1_BRR = 0x09C4;

    /* Enable UE (13), RXNEIE (5), TE (3), RE (2) */
    USART1_CR1 = (1 << 13) | (1 << 5) | (1 << 3) | (1 << 2);

    /* NVIC: Enable USART1 IRQ (IRQ 37 -> bit 5 of ISER1) */
    NVIC_ISER1 = (1 << 5);

    uart_puts("\r\n=== STM32 USART1 RXNE Echo Ready ===\r\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}

void Default_Handler(void)
{
    while (1);
}


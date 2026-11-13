/*
 * STM32F103 I2C Master -> 24C02 EEPROM Test for SimulIDE 2
 *
 * M4 Acceptance Test (c)
 * Writes 0x5A to EEPROM address 0x10, reads it back, verifies match,
 * and logs the result over USART1 (PA9).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdint.h>

#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))

#define GPIOA_BASE      0x40010800
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))

#define GPIOB_BASE      0x40010C00
#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00))

#define GPIOC_BASE      0x40011000
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#define USART1_BASE     0x40013800
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

#define I2C1_BASE       0x40005400
#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))

#define EEPROM_ADDR     0xA0 // 0x50 << 1

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

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

static void delay(volatile uint32_t n)
{
    while (n--) __asm__ volatile("nop");
}

static void uart_putc(char c)
{
    while (!(USART1_SR & (1 << 7)));
    USART1_DR = (uint32_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

static void uart_puthex(uint8_t b)
{
    const char hex[] = "0123456789ABCDEF";
    uart_putc(hex[(b >> 4) & 0x0F]);
    uart_putc(hex[b & 0x0F]);
}

static void i2c_init(void)
{
    /* PB6 (SCL) & PB7 (SDA) = Alternate Function Open-Drain 50MHz: bits [31:24] = 0b11111111 */
    GPIOB_CRL &= ~(0xFF << 24);
    GPIOB_CRL |= (0xFF << 24);

    /* I2C1 Reset */
    I2C1_CR1 = (1 << 15);
    I2C1_CR1 = 0;

    /* PCLK1 = 24MHz */
    I2C1_CR2 = 24;

    /* Standard Mode 100kHz: CCR = 24MHz / (2 * 100kHz) = 120 */
    I2C1_CCR = 120;

    /* Maximum Rise Time: 1000ns / (1/24MHz) + 1 = 25 */
    I2C1_TRISE = 25;

    /* Enable I2C */
    I2C1_CR1 = (1 << 0);
}

static int i2c_write_byte(uint8_t dev_addr, uint8_t mem_addr, uint8_t data)
{
    /* Generate START */
    I2C1_CR1 |= (1 << 8);
    uint32_t to = 100000;
    while (!(I2C1_SR1 & (1 << 0)) && --to);
    if (!to) return -1;

    /* Send device write address */
    I2C1_DR = dev_addr & 0xFE;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 1)) && --to);
    if (!to) return -2;
    (void)I2C1_SR2; // Clear ADDR

    /* Send memory address */
    I2C1_DR = mem_addr;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 7)) && --to);
    if (!to) return -3;

    /* Send data byte */
    I2C1_DR = data;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 2)) && --to); // Wait for BTF
    if (!to) return -4;

    /* Generate STOP */
    I2C1_CR1 |= (1 << 9);
    return 0;
}

static int i2c_read_byte(uint8_t dev_addr, uint8_t mem_addr, uint8_t *data)
{
    /* START */
    I2C1_CR1 |= (1 << 8);
    uint32_t to = 100000;
    while (!(I2C1_SR1 & (1 << 0)) && --to);
    if (!to) return -1;

    /* Send device write address to set memory pointer */
    I2C1_DR = dev_addr & 0xFE;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 1)) && --to);
    if (!to) return -2;
    (void)I2C1_SR2;

    /* Send memory address */
    I2C1_DR = mem_addr;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 7)) && --to);
    if (!to) return -3;

    /* Repeated START */
    I2C1_CR1 |= (1 << 8);
    to = 100000;
    while (!(I2C1_SR1 & (1 << 0)) && --to);
    if (!to) return -4;

    /* Send device read address */
    I2C1_DR = dev_addr | 0x01;
    to = 100000;
    while (!(I2C1_SR1 & (1 << 1)) && --to);
    if (!to) return -5;

    /* Single byte read: clear ACK before clearing ADDR */
    I2C1_CR1 &= ~(1 << 10);
    (void)I2C1_SR2; // Clear ADDR

    /* Program STOP */
    I2C1_CR1 |= (1 << 9);

    /* Wait for RXNE */
    to = 100000;
    while (!(I2C1_SR1 & (1 << 6)) && --to);
    if (!to) return -6;

    *data = (uint8_t)I2C1_DR;
    return 0;
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    /* Enable Clocks: GPIOA, GPIOB, GPIOC, USART1, I2C1 */
    RCC_APB2ENR |= (1 << 2) | (1 << 3) | (1 << 4) | (1 << 14);
    RCC_APB1ENR |= (1 << 21); // I2C1

    /* PC13 LED */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);
    GPIOC_ODR |= (1 << 13); // LED off

    /* PA9 (TX) = AF Push-Pull 50MHz */
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |= (0xB << 4);

    /* USART1 9600 baud */
    USART1_BRR = 0x09C4;
    USART1_CR1 = (1 << 13) | (1 << 3); // UE | TE

    uart_puts("\r\n=== STM32 I2C 24C02 EEPROM Test ===\r\n");

    i2c_init();
    delay(50000);

    /* Write 0x5A to address 0x10 */
    uart_puts("[I2C] Writing 0x5A to EEPROM addr 0x10...\r\n");
    int w_res = i2c_write_byte(EEPROM_ADDR, 0x10, 0x5A);
    if (w_res != 0) {
        uart_puts("[I2C] Write error code: ");
        uart_puthex((uint8_t)(-w_res));
        uart_puts("\r\n");
    }

    delay(100000);

    /* Read back from address 0x10 */
    uint8_t rx_data = 0;
    int r_res = i2c_read_byte(EEPROM_ADDR, 0x10, &rx_data);
    if (r_res != 0) {
        uart_puts("[I2C] Read error code: ");
        uart_puthex((uint8_t)(-r_res));
        uart_puts("\r\n");
    } else {
        uart_puts("[I2C] Read back value: 0x");
        uart_puthex(rx_data);
        uart_puts("\r\n");

        if (rx_data == 0x5A) {
            uart_puts("[I2C] >>> ACCEPTANCE TEST PASS: 0x5A MATCH! <<<\r\n");
            GPIOC_ODR &= ~(1 << 13); // Turn LED ON!
        } else {
            uart_puts("[I2C] >>> MISMATCH! <<<\r\n");
        }
    }

    while (1) {
        delay(500000);
    }
}

void Default_Handler(void)
{
    while (1);
}


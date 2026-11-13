/*
 * STM32F103 Button -> EXTI0 -> ISR Toggle LED (PC13)
 *
 * M4 Acceptance Test (a)
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdint.h>

#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

#define GPIOA_BASE      0x40010800
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_IDR       (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))

#define GPIOC_BASE      0x40011000
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#define AFIO_BASE       0x40010000
#define AFIO_EXTICR1    (*(volatile uint32_t *)(AFIO_BASE + 0x08))

#define EXTI_BASE       0x40010400
#define EXTI_IMR        (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_RTSR       (*(volatile uint32_t *)(EXTI_BASE + 0x08))
#define EXTI_FTSR       (*(volatile uint32_t *)(EXTI_BASE + 0x0C))
#define EXTI_PR         (*(volatile uint32_t *)(EXTI_BASE + 0x14))

#define NVIC_ISER0      (*(volatile uint32_t *)0xE000E100)

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void);
void Default_Handler(void);
void EXTI0_IRQHandler(void);

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
    Default_Handler,
    /* External Interrupts: IRQ 0..6 */
    Default_Handler, // IRQ 0: WWDG
    Default_Handler, // IRQ 1: PVD
    Default_Handler, // IRQ 2: TAMPER
    Default_Handler, // IRQ 3: RTC
    Default_Handler, // IRQ 4: FLASH
    Default_Handler, // IRQ 5: RCC
    EXTI0_IRQHandler // IRQ 6: EXTI0
};

static volatile uint32_t g_exti_count = 0;

void EXTI0_IRQHandler(void)
{
    if (EXTI_PR & (1 << 0)) {
        EXTI_PR = (1 << 0); // Clear pending bit by writing 1

        /* Toggle PC13 LED */
        GPIOC_ODR ^= (1 << 13);
        g_exti_count++;
    }
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    /* Enable Clocks: AFIO, GPIOA, GPIOC */
    RCC_APB2ENR |= (1 << 0) | (1 << 2) | (1 << 4);

    /* PC13 = Output Push-Pull 50MHz: CRH bits [23:20] = 0b0011 */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |= (0x3 << 20);
    GPIOC_ODR |= (1 << 13); // LED off initially

    /* PA0 = Input Pull-down: CRL bits [3:0] = 0b1000 (CNF=10, MODE=00), ODR bit 0 = 0 */
    GPIOA_CRL &= ~(0xF << 0);
    GPIOA_CRL |= (0x8 << 0);
    GPIOA_ODR &= ~(1 << 0);

    /* AFIO EXTICR1: Map EXTI0 to PA0 (bits [3:0] = 0b0000) */
    AFIO_EXTICR1 &= ~(0xF << 0);

    /* EXTI0: Unmask line 0, trigger on rising edge */
    EXTI_IMR |= (1 << 0);
    EXTI_RTSR |= (1 << 0);

    /* NVIC: Enable EXTI0 IRQ (IRQ 6) */
    NVIC_ISER0 = (1 << 6);

    while (1) {
        __asm__ volatile("wfi");
    }
}

void Default_Handler(void)
{
    while (1);
}


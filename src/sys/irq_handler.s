//
// auntflora - Port of Aunt Flora's Mansion to Gameboy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

    .section    .iwram, "ax"
    .global     irq_handler
    .global     irq_vblank
    .global     irq_hblank
    .global     irq_vcount
    .global     irq_timer0
    .global     irq_timer1
    .global     irq_timer2
    .global     irq_timer3
    .global     irq_serial
    .global     irq_dma0
    .global     irq_dma1
    .global     irq_dma2
    .global     irq_dma3
    .global     irq_keypad
    .global     irq_gamepak
    .cpu        arm7tdmi
    .arm

irq_vblank:
    .space 4
irq_hblank:
    .space 4
irq_vcount:
    .space 4
irq_timer0:
    .space 4
irq_timer1:
    .space 4
irq_timer2:
    .space 4
irq_timer3:
    .space 4
irq_serial:
    .space 4
irq_dma0:
    .space 4
irq_dma1:
    .space 4
irq_dma2:
    .space 4
irq_dma3:
    .space 4
irq_keypad:
    .space 4
irq_gamepak:
    .space 4
irq_handler:
    // offsets relative to 0x04000000
    .set IE    , 0x200
    .set IF    , 0x202
    .set IFBIOS, -8    // mirrored at 0x03fffff8
    .set IME   , 0x208

    // r0 already contains 0x04000000 from the BIOS
    // r1 = REG_IE & REG_IF
    ldr   r1, [r0, #IE]
    and   r1, r1, r1, lsr #16

    // check interrupt handlers in priority order

    //
    // NOTE: If you remove a check (because you know that the IRQ is never used), then you need to
    //       adjust the add instructions (r3), since they are dependant on the previous value
    //

    // HBLANK
    ldr   r3, =irq_hblank
    mov   r2, #1 << 1
    tst   r1, r2
    bne   interrupt_found

    // VCOUNT
    add   r3, #irq_vcount - irq_hblank
    mov   r2, #1 << 2
    tst   r1, r2
    bne   interrupt_found

    // VBLANK
    add   r3, #irq_vblank - irq_vcount
    mov   r2, #1 << 0
    tst   r1, r2
    bne   interrupt_found

    // TIMER0
    add   r3, #irq_timer0 - irq_vblank
    mov   r2, #1 << 3
    tst   r1, r2
    bne   interrupt_found

    // TIMER1
    add   r3, #irq_timer1 - irq_timer0
    mov   r2, #1 << 4
    tst   r1, r2
    bne   interrupt_found

    // TIMER2
    add   r3, #irq_timer2 - irq_timer1
    mov   r2, #1 << 5
    tst   r1, r2
    bne   interrupt_found

    // TIMER3
    add   r3, #irq_timer3 - irq_timer2
    mov   r2, #1 << 6
    tst   r1, r2
    bne   interrupt_found

    // SERIAL
    add   r3, #irq_serial - irq_timer3
    mov   r2, #1 << 7
    tst   r1, r2
    bne   interrupt_found

    // DMA0
    add   r3, #irq_dma0 - irq_serial
    mov   r2, #1 << 8
    tst   r1, r2
    bne   interrupt_found

    // DMA1
    add   r3, #irq_dma1 - irq_dma0
    mov   r2, #1 << 9
    tst   r1, r2
    bne   interrupt_found

    // DMA2
    add   r3, #irq_dma2 - irq_dma1
    mov   r2, #1 << 10
    tst   r1, r2
    bne   interrupt_found

    // DMA3
    add   r3, #irq_dma3 - irq_dma2
    mov   r2, #1 << 11
    tst   r1, r2
    bne   interrupt_found

    // KEYPAD
    add   r3, #irq_keypad - irq_dma3
    mov   r2, #1 << 12
    tst   r1, r2
    bne   interrupt_found

    // GAMEPAK
    add   r3, #irq_gamepak - irq_keypad
    mov   r2, #1 << 13
    tst   r1, r2
    bne   interrupt_found

    // no interrupt handlers have to be called

    // clear IF by setting it to IF
    // (strangely, setting a bit in IF actually clears it)
    add   r3, r0, #IF & 0xff00
    orr   r3, #IF & 0xff
    ldrh  r1, [r3]
    strh  r1, [r3]

    // clear IFBIOS
    ldrh  r2, [r0, #IFBIOS]
    orr   r2, r1
    strh  r2, [r0, #IFBIOS]

    bx    lr
    .pool

interrupt_found:
    // r0 = 0x04000000
    // r1 = unused
    // r2 = 1 << handler
    // r3 = handler address

    // clear IF for this handler
    // (strangely, setting a bit in IF actually clears it)
    add   r1, r0, #IF & 0xff00
    orr   r1, #IF & 0xff
    strh  r2, [r1]

    // clear IFBIOS for this handler, which is mirrored at 0x03fffff8
    ldrh  r1, [r0, #IFBIOS]
    orr   r1, r2
    strh  r1, [r0, #IFBIOS]

    // load the handler (exit if NULL)
    ldr   r3, [r3]
    cmp   r3, #0
    bxeq  lr

    // we're ready to call the handler in r3

    // when the IRQ is called, the CPSR.I flag is set, which disables
    // any future interrupts... we want to clear this flag, so that
    // interrupts can be nested
    //
    // in order to do that, we first need to disable interrupts using
    // IME
    //
    // this gives the option to the handler to resume interrupts by
    // setting IME (and not messing with CPSR.I)
    //
    // we also need to save the old IME, SPSR, and LR registers, so we
    // can restore them, if a nested interrupt occurs

    // write 0 to IME, and read the old value at the same time
    add   r2, r0, #IME & 0xff00
    orr   r2, #IME & 0xff
    mov   r1, #0
    swp   r1, r1, [r2]

    // get SPSR
    mrs   r2, spsr

    // save old IME, SPSR, and LR
    push  {r1-r2, lr}

    .set MODE_IRQ   , 0x12
    .set MODE_SYSTEM, 0x1f
    .set MODE_MASK  , 0x1f
    .set IRQ_DISABLE, 1 << 7

    // set the CPU mode to system, which restores the user stack, and
    // allows us to revert it later
    mrs   r2, cpsr
    bic   r2, #IRQ_DISABLE
    orr   r2, #MODE_SYSTEM
    msr   cpsr, r2

    // call interrupt handler, using user stack
    push  {lr}
    mov   lr, pc
    bx    r3
    pop   {lr}

    // disable interrupts while switching modes
    mov   r0, #0x04000000
    str   r0, [r0, #IME]

    // revert CPU mode back to IRQ with interrupts disabled
    mrs   r2, cpsr
    bic   r2, #MODE_MASK
    orr   r2, #MODE_IRQ | IRQ_DISABLE
    msr   cpsr, r2

    // restore IME, SPSR, and LR
    pop   {r1-r2, lr}
    msr   spsr, r2
    str   r1, [r0, #IME]

    bx    lr

    .align 4
    .pool
    .end

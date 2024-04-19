//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

    .section    .iwram, "ax"
    .global     g_snd
    .extern     g_snd
    .global     sys__snd_timer1_handler
    .include    "snd_offsets.inc"
    .include    "reg.inc"
    .cpu        arm7tdmi
    .arm

sys__snd_timer1_handler:
    push  {r4-r5}

    // prepare to swap out DMAs
    ldr   r0, =g_snd + SND_BUFFER_ADDR
    ldr   r1, =0
    ldr   r2, [r0, #4]
    ldr   r3, =0xb640
    ldr   r4, =REG_DMA1CNT_H
    ldr   r5, =REG_DMA1SAD

    // disable DMA
    strh  r1, [r4]
    // set source
    str   r2, [r5]
    // enable DMA
    strh  r3, [r4]

    // rotate the addresses and decrease next_buffer_index
    ldr   r1, [r0, #0]
    ldr   r3, [r0, #8]
    str   r2, [r0, #0]
    str   r3, [r0, #4]
    str   r1, [r0, #8]
    ldr   r0, =g_snd + SND_NEXT_BUFFER_INDEX
    ldr   r1, [r0]
    subs  r1, #4
    cmp   r1, #4
    movlt r1, #4
    str   r1, [r0]

    pop   {r4-r5}
    bx    lr

    .align 4
    .pool
    .end

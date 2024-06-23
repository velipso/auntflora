//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

    .section    .iwram, "ax"
    .global     memcpy32
    .global     memcpy16
    .global     memcpy8
    .global     memset32
    .global     memset8
    .cpu        arm7tdmi
    .arm

memcpy32:
    subs  r2, #32
    blt   L3
    stmfd sp!, {r3-r10}
L1: // copy 32 bytes at a time (unrolled x 4)
    ldmia r1!, {r3-r10}
    stmia r0!, {r3-r10}
    subs  r2, #32
    blt   L2
    ldmia r1!, {r3-r10}
    stmia r0!, {r3-r10}
    subs  r2, #32
    blt   L2
    ldmia r1!, {r3-r10}
    stmia r0!, {r3-r10}
    subs  r2, #32
    blt   L2
    ldmia r1!, {r3-r10}
    stmia r0!, {r3-r10}
    subs  r2, #32
    bge   L1
L2:
    ldmfd sp!, {r3-r10}
L3:
    adds  r2, #32
    bxle  lr
L4: // copy 4 bytes at a time
    subs  r2, #4
    ldrge r12, [r1], #4
    strge r12, [r0], #4
    bgt   L4
    bxeq  lr
    add   r2, #4
    // fallthrough to memcpy16
memcpy16:
    subs  r2, #2
    blt   L5
    ldrh  r12, [r1]
    strh  r12, [r0]
    adds  r1, #2
    adds  r0, #2
    b     memcpy16
L5:
    add   r2, #2
    bxle  lr
    // copy last byte
    ldrb  r12, [r1]
    strb  r12, [r0]
    bx    lr

memcpy8:
    subs  r2, #1
    bxlt  lr
    ldrb  r12, [r1], #1
    strb  r12, [r0], #1
    b     memcpy8

memset32:
    subs  r2, #32
    blt   M3
    stmfd sp!, {r3-r9}
    mov   r3, r1
    mov   r4, r1
    mov   r5, r1
    mov   r6, r1
    mov   r7, r1
    mov   r8, r1
    mov   r9, r1
M1: // set 32 bytes at a time (unrolled x 4)
    stmia r0!, {r1, r3-r9}
    subs  r2, #32
    blt   M2
    stmia r0!, {r1, r3-r9}
    subs  r2, #32
    blt   M2
    stmia r0!, {r1, r3-r9}
    subs  r2, #32
    blt   M2
    stmia r0!, {r1, r3-r9}
    subs  r2, #32
    bge   M1
M2:
    ldmfd sp!, {r3-r9}
M3:
    adds  r2, #32
    bxle  lr
M4: // set 4 bytes at a time
    subs  r2, #4
    blt   M5
    str   r1, [r0], #4
    b     M4
M5:
    adds  r2, #4
    // fallthrough to memset16
memset16:
    subs  r2, #2
    blt   M6
    strh  r1, [r0]
    adds  r0, #2
    b     memset16
M6:
    adds  r2, #2
    bxle  lr
    // set last byte
    strb  r1, [r0]
    bx    lr

memset8:
    subs  r2, #1
    bxlt  lr
    strb  r1, [r0], #1
    b     memset8

    .align 4
    .pool
    .end

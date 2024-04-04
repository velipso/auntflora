//
// auntflora - Port of Aunt Flora's Mansion to Gameboy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

    .section    .gba_crt0, "ax"
    .global     entrypoint
    .cpu        arm7tdmi
    .arm

entrypoint:
    b       start_vector

    // Nintendo logo
    .byte   0x24, 0xff, 0xae, 0x51, 0x69, 0x9a, 0xa2, 0x21
    .byte   0x3d, 0x84, 0x82, 0x0a, 0x84, 0xe4, 0x09, 0xad
    .byte   0x11, 0x24, 0x8b, 0x98, 0xc0, 0x81, 0x7f, 0x21
    .byte   0xa3, 0x52, 0xbe, 0x19, 0x93, 0x09, 0xce, 0x20
    .byte   0x10, 0x46, 0x4a, 0x4a, 0xf8, 0x27, 0x31, 0xec
    .byte   0x58, 0xc7, 0xe8, 0x33, 0x82, 0xe3, 0xce, 0xbf
    .byte   0x85, 0xf4, 0xdf, 0x94, 0xce, 0x4b, 0x09, 0xc1
    .byte   0x94, 0x56, 0x8a, 0xc0, 0x13, 0x72, 0xa7, 0xfc
    .byte   0x9f, 0x84, 0x4d, 0x73, 0xa3, 0xca, 0x9a, 0x61
    .byte   0x58, 0x97, 0xa3, 0x27, 0xfc, 0x03, 0x98, 0x76
    .byte   0x23, 0x1d, 0xc7, 0x61, 0x03, 0x04, 0xae, 0x56
    .byte   0xbf, 0x38, 0x84, 0x00, 0x40, 0xa7, 0x0e, 0xfd
    .byte   0xff, 0x52, 0xfe, 0x03, 0x6f, 0x95, 0x30, 0xf1
    .byte   0x97, 0xfb, 0xc0, 0x85, 0x60, 0xd6, 0x80, 0x25
    .byte   0xa9, 0x63, 0xbe, 0x03, 0x01, 0x4e, 0x38, 0xe2
    .byte   0xf9, 0xa2, 0x34, 0xff, 0xbb, 0x3e, 0x03, 0x44
    .byte   0x78, 0x00, 0x90, 0xcb, 0x88, 0x11, 0x3a, 0x94
    .byte   0x65, 0xc0, 0x7c, 0x63, 0x87, 0xf0, 0x3c, 0xaf
    .byte   0xd6, 0x25, 0xe4, 0x8b, 0x38, 0x0a, 0xac, 0x72
    .byte   0x21, 0xd4, 0xf8, 0x07

    // game title (12 characters, padded with 0)
    .ascii  "AUNTFLORA\0\0\0"

    // game code, maker code
    .ascii  "CAFE77"

    // hardcoded
    .byte   150
    .fill   9, 1, 0

    // version
    .byte   0

    // complement check (fixed via xform)
    .byte   0

    // reserved
    .byte   0, 0

    // ensure ROM isn't interpetted as multi-boot
    b       entrypoint

    // tell emulators we want 32K SRAM
    .ascii  "SRAM_Vnnn"
    .align  4

start_vector:
    // Disable interrupts
    mov     r0, #0x4000000
    mov     r1, #0
    str     r1, [r0, #0x208] // IME

    // set cartridge wait state for faster access
    .set REG_WAITCNT, 0x04000204
    ldr     r0, =REG_WAITCNT
    ldr     r1, =0x4317
    strh    r1, [r0]

    // Setup IRQ mode stack
    mov     r0, #0x12
    msr     cpsr, r0
    ldr     sp, =__STACK_IRQ_END__

    // Setup system mode stack
    mov     r0, #0x1F
    msr     cpsr, r0
    ldr     sp, =__STACK_USR_END__

    // Switch to Thumb mode
    add     r0, pc, #1
    bx      r0

    .thumb

    // Clear IWRAM
    ldr     r0, =#0x3000000
    ldr     r1, =#(32 * 1024)
    bl      mem_zero

    // Copy data section from ROM to RAM
    ldr     r0, =__DATA_LMA__
    ldr     r1, =__DATA_START__
    ldr     r2, =__DATA_SIZE__
    bl      mem_copy

    // Copy IWRAM data from ROM to RAM
    ldr     r0, =__IWRAM_LMA__
    ldr     r1, =__IWRAM_START__
    ldr     r2, =__IWRAM_SIZE__
    bl      mem_copy

    // Clear EWRAM
    ldr     r0, =#0x2000000
    ldr     r1, =#(256 * 1024)
    bl      mem_zero

    // Copy EWRAM data from ROM to RAM
    ldr     r0, =__EWRAM_LMA__
    ldr     r1, =__EWRAM_START__
    ldr     r2, =__EWRAM_SIZE__
    bl      mem_copy

    // Global constructors
    ldr     r2, =__libc_init_array
    bl      blx_r2_trampoline

    // Call main()
    mov     r0, #0 // int argc
    mov     r1, #0 // char *argv[]
    ldr     r2, =main
    bl      blx_r2_trampoline

    // Global destructors
    ldr     r2, =__libc_fini_array
    bl      blx_r2_trampoline

    // If main() returns, reboot the GBA using SoftReset
    swi     #0x00

// r0 = Base address
// r1 = Size
mem_zero:
    and     r1, r1
    beq     2f // Return if size is 0

    mov     r2, #0
1:
    stmia   r0!, {r2}
    sub     r1, #4
    bne     1b

2:
    bx      lr

// r0 = Source address
// r1 = Destination address
// r2 = Size
mem_copy:
    and     r2, r2
    beq     2f // Return if size is 0

1:
    ldmia   r0!, {r3}
    stmia   r1!, {r3}
    sub     r2, #4
    bne     1b

2:
    bx      lr

// r2 = Address to jump to
blx_r2_trampoline:
    bx      r2

    .align 4
    .pool
    .end

//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

    .section    .iwram, "ax"
    .global     g_snd
    .extern     g_snd
    .global     _binary_snd_bend_bin_start
    .extern     _binary_snd_bend_bin_start
    .global     _binary_snd_tempo_bin_start
    .extern     _binary_snd_tempo_bin_start
    .global     _binary_snd_dphase_bin_start
    .extern     _binary_snd_dphase_bin_start
    .global     _binary_snd_slice_bin_start
    .extern     _binary_snd_slice_bin_start
    .global     _binary_snd_osc_bin_start
    .extern     _binary_snd_osc_bin_start
    .global     _binary_snd_offsets_bin_start
    .extern     _binary_snd_offsets_bin_start
    .global     _binary_snd_wavs_bin_start
    .extern     _binary_snd_wavs_bin_start
    .global     _binary_snd_sizes_bin_start
    .extern     _binary_snd_sizes_bin_start
    .global     debug_print_number
    .extern     debug_print_number
    .global     sys__snd_timer1_handler
    .global     sys__snd_frame
    .include    "snd_offsets.inc"
    .include    "reg.inc"
    .set        pitchDivisionBits, 4
    .set        maxPhaseBits, 11
    .set        maxPhaseQBits, maxPhaseBits + 5
    .set        maxRndSample, 32768
    .cpu        arm7tdmi
    .arm

//
// void sys__snd_frame();
//
// Renders next frame to the output buffers
//
sys__snd_frame:
    // if we don't have any room left, don't render anything
    ldr   r0, =g_snd + SND_NEXT_BUFFER_INDEX
    ldr   r1, [r0]
    cmp   r1, #12
    bxge  lr

    // setup frame
    push  {r4-r11, lr}
    #define sOutputBuffer  0
    #define sDidClear      4
    #define sChannelLeft   8
    sub   sp, #12

    // sOutputBuffer = g_snd.buffer_addr[g_snd.next_buffer_index];
    ldr   r2, =g_snd + SND_BUFFER_ADDR
    ldr   r2, [r2, r1]
    str   r2, [sp, #sOutputBuffer]
    // g_snd.next_buffer_index += 4;
    adds  r1, #4
    str   r1, [r0]

    // check for muted sound immediately so we don't access the cart if
    // sound is completely muted
    #define rSndPtr        r4
    ldr   rSndPtr, =g_snd
    ldr   r0, [rSndPtr, #SND_MASTER_VOLUME]
    cmp   r0, #0
    beq   mute_all_sound
    // check for no song loaded
    ldr   r0, [rSndPtr, #SND_SYNTH + SND_SYNTH_SONG_BASE]
    cmp   r0, #0
    beq   mute_all_sound

    // advance tick counter
    #define rTickStart     r5
    #define rTickLeft      r6
    ldr   rTickLeft, [rSndPtr, #SND_SYNTH + SND_SYNTH_TICK_LEFT]
    subs  rTickLeft, #256
    str   rTickLeft, [rSndPtr, #SND_SYNTH + SND_SYNTH_TICK_LEFT]
    bgt   done_tick
run_tick:
    bl    snd_tick
    // snd_tick could change tick_start/tick_left
    ldr   rTickStart, [rSndPtr, #SND_SYNTH + SND_SYNTH_TICK_START]
    muls  r0, rTickStart
    ldr   rTickLeft, [rSndPtr, #SND_SYNTH + SND_SYNTH_TICK_LEFT]
    adds  rTickLeft, r0
    str   rTickLeft, [rSndPtr, #SND_SYNTH + SND_SYNTH_TICK_LEFT]
    cmp   rTickLeft, #0
    ble   run_tick
    #undef rSndPtr
    #undef rTickStart
    #undef rTickLeft
done_tick:

    #define rChannelPtr    r1
    #define rChannelLeft   r2
    #define rSampleLeft    r3
    #define rOutputBuffer  r4
    #define rFinalVolume   r5
    #define rPhase         r6
    #define rDPhase        r7
    #define rInstPtr       r8

    ldr   rChannelPtr, =g_snd + SND_SYNTH + SND_SYNTH_CHANNEL
    ldr   r0, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   r0, [r0]
    ldrb  rChannelLeft, [r0, #SND_SONG_CHANNEL_COUNT]
    movs  r0, #0
    str   r0, [sp, #sDidClear]

    //
    // Render each channel to rOutputBuffer
    //
render_channel:
    ldr   r0, [rChannelPtr, #SND_CHANNEL_STATE]
    cmp   r0, #0 // is note completely off?
    beq   render_channel_continue

    ldr   rOutputBuffer, =g_snd + SND_BUFFER_TEMP
    movs  rSampleLeft, #608

    // check for PCM
    ldr   ip, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    cmp   ip, #0x80 << pitchDivisionBits
    bge   render_pcm_inst

    // check for instrument 0
    ldr   rInstPtr, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    cmp   rInstPtr, #0
    beq   render_channel_continue

    // calculate final volume
    ldrh  rPhase, [rInstPtr, #SND_SONGINST_VOLUME_ENV_OFFSET]
    adds  rPhase, rInstPtr
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    ldrsb rFinalVolume, [rPhase, r0]
    ldr   r0, [rChannelPtr, #SND_CHANNEL_CHAN_VOLUME]
    muls  rFinalVolume, r0

    // apply pitch envelope
    ldrh  rPhase, [rInstPtr, #SND_SONGINST_PITCH_ENV_OFFSET]
    adds  rPhase, rInstPtr
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    ldrsb r0, [rPhase, r0]
    adds  ip, r0
    lsls  ip, #1

    // load phase and dphase
    ldr   rPhase, [rChannelPtr, #SND_CHANNEL_PHASE]
    ldr   rDPhase, =_binary_snd_dphase_bin_start
    ldrh  rDPhase, [rDPhase, ip]

    ldrh  r0, [rInstPtr, #SND_SONGINST_WAVE]
    cmp   r0, #0
    beq   render_rnd_inst
    subs  r0, #1
    #undef rInstPtr

    // otherwise, oscillator
    #define rSample0    r8
    #define rSample1    r9
    #define rPhaseMask  r10
    #define rWave       r11
    lsrs  ip, #pitchDivisionBits + 1
    lsls  ip, #1
    lsls  r0, #8
    adds  r0, ip
    ldr   rWave, =_binary_snd_slice_bin_start
    ldrh  r0, [rWave, r0]
    lsls  r0, #maxPhaseBits + 1
    ldr   rWave, =_binary_snd_osc_bin_start
    adds  rWave, r0

    // cache rChannelLeft and load sDidClear
    str   rChannelLeft, [sp, #sChannelLeft]
    ldr   rChannelLeft, [sp, #sDidClear]

    ldr   rPhaseMask, =0xffe0
next_osc_sample: // core loop!
    // rWave data is 15bit
    // rOutputBuffer is 16bit
    // rFinalVolume is 8bit (0-256)
    cmp   rChannelLeft, #0
    moveq rSample0, #0
    ldrne rSample0, [rOutputBuffer]
    asrs  rSample1, rSample0, #16
    lsls  rSample0, #16
    asrs  rSample0, #16

    // render oscillator instrument
    ands  r0, rPhase, rPhaseMask
    lsrs  r0, #4
    ldrsh r0, [rWave, r0]
    adds  rPhase, rDPhase
    muls  r0, rFinalVolume
    asrs  r0, #7
    adds  rSample0, r0

    // render another sample, since we're here
    ands  r0, rPhase, rPhaseMask
    lsrs  r0, #4
    ldrsh r0, [rWave, r0]
    adds  rPhase, rDPhase
    muls  r0, rFinalVolume
    asrs  r0, #7
    adds  rSample1, r0

    // save results
    strh  rSample0, [rOutputBuffer, #0]
    strh  rSample1, [rOutputBuffer, #2]
    adds  rOutputBuffer, #4

    subs  rSampleLeft, #2
    bne   next_osc_sample
    // end core loop

    lsls  rPhase, #32 - maxPhaseQBits
    lsrs  rPhase, #32 - maxPhaseQBits
    str   rPhase, [rChannelPtr, #SND_CHANNEL_PHASE]

    // uncache rChannelLeft and load didClear
    movs  rChannelLeft, #1
    str   rChannelLeft, [sp, #sDidClear]
    ldr   rChannelLeft, [sp, #sChannelLeft]
    b     render_channel_continue

render_rnd_inst:
    ldr   rWave, =_binary_snd_osc_bin_start

    // cache rChannelLeft and load didClear
    str   rChannelLeft, [sp, #sChannelLeft]
    ldr   rChannelLeft, [sp, #sDidClear]

    ldr   rPhaseMask, =(maxRndSample - 1) << 12
next_rnd_sample: // core loop!
    cmp   rChannelLeft, #0
    moveq rSample0, #0
    ldrne rSample0, [rOutputBuffer]
    asrs  rSample1, rSample0, #16
    lsls  rSample0, #16
    asrs  rSample0, #16

    // render rnd instrument
    ands  r0, rPhase, rPhaseMask
    lsrs  r0, #11
    ldrsh r0, [rWave, r0]
    adds  rPhase, rDPhase
    muls  r0, rFinalVolume
    asrs  r0, #10
    adds  rSample0, r0

    // render another sample, since we're here
    ands  r0, rPhase, rPhaseMask
    lsrs  r0, #11
    ldrsh r0, [rWave, r0]
    adds  rPhase, rDPhase
    muls  r0, rFinalVolume
    asrs  r0, #10
    adds  rSample1, r0

    // save results
    strh  rSample0, [rOutputBuffer, #0]
    strh  rSample1, [rOutputBuffer, #2]
    adds  rOutputBuffer, #4

    subs  rSampleLeft, #2
    bne   next_rnd_sample
    // end core loop

    str   rPhase, [rChannelPtr, #SND_CHANNEL_PHASE]

    // uncache rChannelLeft and load didClear
    movs  rChannelLeft, #1
    str   rChannelLeft, [sp, #sDidClear]
    ldr   rChannelLeft, [sp, #sChannelLeft]
    b     render_channel_continue
    #undef rDPhase
    #undef rPhaseMask
    #undef rWave

render_pcm_inst:
    #define rSamplePtr  r7
    // convert basePitch to rSamplePtr
    lsrs  r0, ip, #pitchDivisionBits
    ands  r0, #0x7f
    lsls  r0, #1
    ldr   rSample0, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   rSample0, [rSample0]
    ldr   rSample1, =SND_SONG_SAMP_TABLE
    adds  rSample0, rSample1
    ldrh  r0, [rSample0, r0]
    lsrs  rFinalVolume, r0, #12
    adds  rFinalVolume, #1
    lsls  r0, #20
    lsrs  r0, #18
    ldr   rSample0, =_binary_snd_offsets_bin_start
    ldr   r0, [rSample0, r0]
    ldr   rSamplePtr, =_binary_snd_wavs_bin_start
    adds  rSamplePtr, r0
    ldr   r0, [rChannelPtr, #SND_CHANNEL_CHAN_VOLUME]
    muls  rFinalVolume, r0
    ldr   rPhase, [rChannelPtr, #SND_CHANNEL_PHASE]
    lsls  rPhase, #1

    // cache rChannelLeft and load sDidClear
    str   rChannelLeft, [sp, #sChannelLeft]
    ldr   rChannelLeft, [sp, #sDidClear]

next_pcm_sample: // core loop!
    // rFinalVolume is 8bit (0-256)
    // rOutputBuffer is 16bit
    // rSamplePtr is 16bit
    cmp   rChannelLeft, #0
    moveq rSample0, #0
    ldrne rSample0, [rOutputBuffer]
    asrs  rSample1, rSample0, #16
    lsls  rSample0, #16
    asrs  rSample0, #16

    // render PCM sample
    ldrsh r0, [rSamplePtr, rPhase]
    muls  r0, rFinalVolume
    asrs  r0, #8
    adds  rSample0, r0
    adds  rPhase, #2

    // render another sample, since we're here
    ldrsh r0, [rSamplePtr, rPhase]
    muls  r0, rFinalVolume
    asrs  r0, #8
    adds  rSample1, r0
    adds  rPhase, #2

    // save results
    strh  rSample0, [rOutputBuffer, #0]
    strh  rSample1, [rOutputBuffer, #2]
    adds  rOutputBuffer, #4

    subs  rSampleLeft, #2
    bne   next_pcm_sample
    // end core loop

    lsrs rPhase, #1
    str  rPhase, [rChannelPtr, #SND_CHANNEL_PHASE]

    // uncache rChannelLeft and load sDidClear
    movs  rChannelLeft, #1
    str   rChannelLeft, [sp, #sDidClear]
    ldr   rChannelLeft, [sp, #sChannelLeft]
    b     render_channel_continue
    #undef rSamplePtr
    #undef rSample0
    #undef rSample1

render_channel_continue:
    adds  rChannelPtr, #SIZEOF_SND_CHANNEL_ST
    subs  rChannelLeft, #1
    bne   render_channel
    #undef rChannelPtr
    #undef rChannelLeft
    #undef rSampleLeft
    #undef rOutputBuffer
    #undef rFinalVolume
    #undef rPhase
    //
    // Done rendering channels!
    //

    //
    // Copy to final buffer
    //
    #define rSfxPtr0         r1
    #define rSfxPtr1         r2
    #define rSfxPtr2         r3
    #define rSfxPtr3         r4
    #define rSfxVolumes      r5
    #define rSfxSample       r6
    #define rSfxSampleIdx    r7
    #define rPackedVolumes   r8
    #define rSynthSample     r9
    #define rInput           r10
    #define rOutput          r11
    #define rSampleLeft      ip
    #define rDidClear        lr

    ldr   r0, =g_snd + SND_SFX
    // pack sfx volumes into register
    ldr   rSfxVolumes, [r0, #SND_SFX_WAV_VOLUME + SIZEOF_SND_SFX_ST * 0]
    ldr   rSfxPtr1, [r0, #SND_SFX_WAV_VOLUME + SIZEOF_SND_SFX_ST * 1]
    lsls  rSfxPtr1, #8
    orrs  rSfxVolumes, rSfxPtr1
    ldr   rSfxPtr1, [r0, #SND_SFX_WAV_VOLUME + SIZEOF_SND_SFX_ST * 2]
    lsls  rSfxPtr1, #16
    orrs  rSfxVolumes, rSfxPtr1
    ldr   rSfxPtr1, [r0, #SND_SFX_WAV_VOLUME + SIZEOF_SND_SFX_ST * 3]
    lsls  rSfxPtr1, #24
    orrs  rSfxVolumes, rSfxPtr1
    // load sfx wave pointers
    ldr   rSfxPtr0, [r0, #SND_SFX_WAV_BASE + SIZEOF_SND_SFX_ST * 0]
    ldr   rSfxPtr1, [r0, #SND_SFX_WAV_BASE + SIZEOF_SND_SFX_ST * 1]
    ldr   rSfxPtr2, [r0, #SND_SFX_WAV_BASE + SIZEOF_SND_SFX_ST * 2]
    ldr   rSfxPtr3, [r0, #SND_SFX_WAV_BASE + SIZEOF_SND_SFX_ST * 3]

    // load volumes into rPackedVolumes 0xAABBCC (AA = Master, BB = SFX, CC = Synth)
    ldr   r0, =g_snd
    ldr   rPackedVolumes, [r0, #SND_SYNTH + SND_SYNTH_VOLUME]
    ldr   rSampleLeft, [r0, #SND_SFX_VOLUME]
    lsls  rSampleLeft, #8
    orrs  rPackedVolumes, rSampleLeft
    ldr   rSampleLeft, [r0, #SND_MASTER_VOLUME]
    lsls  rSampleLeft, #16
    orrs  rPackedVolumes, rSampleLeft

    ldr   rDidClear, [sp, #sDidClear]
    ldr   rOutput, [sp, #sOutputBuffer]
    ldr   rInput, =g_snd + SND_BUFFER_TEMP
    movs  rSfxSampleIdx, #0
    movs  rSampleLeft, #608
copy_next_sample:
    // each rPackedVolume is 4bit (0-16)
    // each rSfxVolumes is 4bit (0-16)
    // rInput is 16bit
    // rOutput is 8bit
    // rSfxPtrX is 16bit

    // sfx 0
    movs  rSfxSample, #0
    cmp   rSfxPtr0, #0
    ldrnesh r0, [rSfxPtr0, rSfxSampleIdx]
    andne rSynthSample, rSfxVolumes, #0xff
    mulne r0, rSynthSample
    addne rSfxSample, r0

    // sfx 1
    cmp   rSfxPtr1, #0
    ldrnesh r0, [rSfxPtr1, rSfxSampleIdx]
    andne rSynthSample, rSfxVolumes, #0xff00
    lsrne rSynthSample, #8
    mulne r0, rSynthSample
    addne rSfxSample, r0

    // sfx 2
    cmp   rSfxPtr2, #0
    ldrnesh r0, [rSfxPtr2, rSfxSampleIdx]
    andne rSynthSample, rSfxVolumes, #0xff0000
    lsrne rSynthSample, #16
    mulne r0, rSynthSample
    addne rSfxSample, r0

    // sfx 3
    cmp   rSfxPtr3, #0
    ldrnesh r0, [rSfxPtr3, rSfxSampleIdx]
    andne rSynthSample, rSfxVolumes, #0xff000000
    lsrne rSynthSample, #24
    mulne r0, rSynthSample
    addne rSfxSample, r0

    // apply sfx volume
    ands  r0, rPackedVolumes, #0xff00
    lsrs  r0, #8
    muls  rSfxSample, r0
    lsrs  rSfxSample, #4

    // synth
    movs  rSynthSample, #0
    cmp   rDidClear, #0
    // read synth sample and apply synth volume
    ldrnesh rSynthSample, [rInput]
    ands  r0, rPackedVolumes, #0xff
    muls  rSynthSample, r0

    // final sample
    adds  rSynthSample, rSfxSample

    // apply master volume
    ands  r0, rPackedVolumes, #0xff0000
    lsrs  r0, #16
    muls  rSynthSample, r0
    asrs  r0, rSynthSample, #16

    // clamp r0 between -128 and 127 (magic)
    // rSfxSample and rSynthSample are just work registers that are available
    mov   rSfxSample, #0x7f
    mov   rSynthSample, r0, lsl #24    // clear any potential overflow bits
    cmp   r0, rSynthSample, asr #24    // check if anything changed
    eorne r0, rSfxSample, r0, asr #32  // set the clamp based on the sign of r0

    // write output
    strb  r0, [rOutput]

    adds  rInput, #2
    adds  rOutput, #1
    adds  rSfxSampleIdx, #2
    subs  rSampleLeft, #1
    bne   copy_next_sample

    #undef rSfxPtr0
    #undef rSfxPtr1
    #undef rSfxPtr2
    #undef rSfxPtr3
    #undef rSfxVolumes
    #undef rSfxSample
    #undef rSfxSampleIdx
    #undef rPackedVolumes
    #undef rSynthSample
    #undef rInput
    #undef rOutput
    #undef rSampleLeft
    #undef rDidClear

    //
    // Advance sfx
    //
    ldr   r0, =g_snd + SND_SFX
    movs  r4, #4
advance_sfx:
    ldr   r1, [r0, #SND_SFX_WAV_BASE]
    cmp   r1, #0
    beq   advance_sfx_continue
    adds  r1, #608 * 2
    ldr   r2, [r0, #SND_SFX_SAMPLES_LEFT]
    subs  r2, #608
    moveq r1, #0
    str   r1, [r0, #SND_SFX_WAV_BASE]
    str   r2, [r0, #SND_SFX_SAMPLES_LEFT]
advance_sfx_continue:
    adds  r0, #SIZEOF_SND_SFX_ST
    subs  r4, #1
    bne   advance_sfx

    //
    // Advance envelopes
    //
    #define rChannelPtr   r4
    #define rChannelLeft  r5
    ldr   rChannelPtr, =g_snd + SND_SYNTH + SND_SYNTH_CHANNEL
    ldr   r0, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   r0, [r0]
    ldrb  rChannelLeft, [r0, #SND_SONG_CHANNEL_COUNT]
env_channel:
    ldr   r0, [rChannelPtr, #SND_CHANNEL_STATE]
    cmp   r0, #0
    beq   env_channel_continue
    cmp   r0, #1
    beq   released

    // otherwise, note is on
    ldr   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    cmp   r0, #0x80 << pitchDivisionBits
    bge   check_pcm_size
    ldr   r1, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    cmp   r1, #0
    beq   env_channel_continue
    // advance volume envelope and loop for sustain
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    adds  r0, #1
    ldrb  r2, [r1, #SND_SONGINST_VOLUME_ENV_SUSTAIN]
    cmp   r0, r2
    ldrgeb r0, [r1, #SND_SONGINST_VOLUME_ENV_ATTACK]
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    // advance pitch envelope and loop for sustain
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    adds  r0, #1
    ldrb  r2, [r1, #SND_SONGINST_PITCH_ENV_SUSTAIN]
    cmp   r0, r2
    ldrgeb r0, [r1, #SND_SONGINST_PITCH_ENV_ATTACK]
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    b     env_channel_continue

check_pcm_size:
    // convert base_pitch to sample size
    lsrs  r0, #pitchDivisionBits
    ands  r0, #0x7f
    lsls  r0, #1
    ldr   r1, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   r1, [r1]
    ldr   r2, =SND_SONG_SAMP_TABLE
    adds  r1, r2
    ldrh  r0, [r1, r0]
    lsls  r0, #20
    lsrs  r0, #18
    ldr   r1, =_binary_snd_sizes_bin_start
    ldr   r0, [r1, r0]
    ldr   r1, [rChannelPtr, #SND_CHANNEL_PHASE]
    cmp   r1, r0
    // if ran over sample size, so turn note off
    bge   mute_note
    b     env_channel_continue

released:
    ldr   r1, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    cmp   r1, #1
    beq   mute_note
    // advance volume envelope until it runs off
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    adds  r0, #1
    ldr   r1, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    ldrb  r2, [r1, #SND_SONGINST_VOLUME_ENV_LENGTH]
    cmp   r0, r2
    bge   mute_note
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    // advance pitch envelope
    ldr   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    adds  r0, #1
    ldr   r1, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    ldrb  r2, [r1, #SND_SONGINST_PITCH_ENV_LENGTH]
    cmp   r0, r2
    subge r0, #1
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    b     env_channel_continue

env_channel_continue:
    // check for delayed note on
    ldr   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_LEFT]
    cmp   r0, #0
    beq   skip_delayed_note_on
    subs  r0, #1
    str   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_LEFT]
    bne   skip_delayed_note_on
    // delay has been hit, so note on
    // (copied from below)
    ldr   r2, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_NOTE]
    cmp   r2, #7
    beq   note_stop1
    movs  r0, #2
    str   r0, [rChannelPtr, #SND_CHANNEL_STATE]
    // TODO: maybe some instruments shouldn't reset phase?
    // ex: saw, tri, sin, pcm => reset   sqX, rnd => no reset
    movs  r0, #0
    str   r0, [rChannelPtr, #SND_CHANNEL_PHASE]
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    str   r0, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    lsls  r0, r2, #pitchDivisionBits
    str   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r0, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    b     skip_delayed_note_on
note_stop1:
    movs  r0, #0
    str   r0, [rChannelPtr, #SND_CHANNEL_STATE]
skip_delayed_note_on:

    // check for delayed note off
    ldr   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_OFF_LEFT]
    cmp   r0, #0
    beq   skip_delayed_note_off
    subs  r0, #1
    str   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_OFF_LEFT]
    bne   skip_delayed_note_off
    // delay has been hit, so note off
    // (copied from below)
    ldr   r0, [rChannelPtr, #SND_CHANNEL_STATE]
    cmp   r0, #2
    bne   skip_delayed_note_off
    movs  r0, #1
    str   r0, [rChannelPtr, #SND_CHANNEL_STATE]
skip_delayed_note_off:

    // check for delayed bends
    ldr   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_LEFT]
    cmp   r0, #0
    beq   skip_delayed_bend
    subs  r0, #1
    str   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_LEFT]
    bne   skip_delayed_bend
    // delay has been hit, so start bend
    // (copied from below)
    #define rPayload  r3
    ldr   r0, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_NOTE]
    ldr   rPayload, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_DURATION]

    lsls  r0, #pitchDivisionBits
    str   r0, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    cmp   rPayload, #0
    beq   bend_immediately1
    ldr   r1, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    subs  r0, r1
    rsblt r0, #0
    lsrs  r0, #pitchDivisionBits
    beq   skip_bend1
    subs  r0, #1
    ldr   ip, =g_snd + SND_SYNTH + SND_SYNTH_TEMPO_INDEX
    ldr   ip, [ip]
    lsls  ip, #7 + 1
    lsls  r0, #1
    adds  ip, r0
    ldr   r0, =_binary_snd_bend_bin_start
    ldrh  ip, [r0, ip]
    muls  rPayload, ip
    str   rPayload, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER_MAX]
    movs  r0, #0
    str   r0, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER]
    b     skip_delayed_bend
bend_immediately1:
    str   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    b     skip_delayed_bend
skip_bend1:
    ldr   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r0, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    b     skip_delayed_bend
    #undef rPayload
skip_delayed_bend:

    // pitch bend
    ldr   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    ldr   r1, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    cmp   r0, r1
    beq   skip_pitch_bend
    ldr   r2, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER]
    adds  r2, #65536
    ldr   r3, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER_MAX]
perform_pitch_bend:
    cmp   r2, r3
    blt   finish_pitch_bend
    subs  r2, r3
    cmp   r0, r1
    addlt r0, #1
    subgt r0, #1
    cmp   r0, r1
    bne   perform_pitch_bend
finish_pitch_bend:
    str   r0, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r2, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER]
skip_pitch_bend:

    // done with channel envelope
    adds  rChannelPtr, #SIZEOF_SND_CHANNEL_ST
    subs  rChannelLeft, #1
    bne   env_channel

    // all done!

snd_frame_return:
    add   sp, #12
    pop   {r4-r11}
    pop   {r0}
    bx    r0
mute_note:
    movs  r0, #0
    str   r0, [rChannelPtr, #SND_CHANNEL_STATE]
    b     env_channel_continue
    #undef rChannelPtr
    #undef rChannelLeft
mute_all_sound:
    ldr   r0, [sp, #sOutputBuffer]
    movs  r1, #608
    movs  r2, #0
mute_next_sample:
    str   r2, [r0]
    adds  r0, #4
    subs  r1, #4
    bne   mute_next_sample
    b     snd_frame_return
    #undef sOutputBuffer
    #undef sDidClear
    #undef sChannelLeft
    .align 4
    .pool

//
// int snd_tick();
//
// Advances through the pattern for each channel; returns number of 16th notes to wait
//
snd_tick:
    push  {r4-r7, lr}

    #define rEndFlag      r4
    #define rChannelLeft  r5
    #define rPatPtr       r6
    #define rChannelPtr   r7
    movs  rEndFlag, #0
    ldr   r0, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   r0, [r0]
    ldrb  rChannelLeft, [r0, #SND_SONG_CHANNEL_COUNT]
    ldr   rChannelPtr, =g_snd + SND_SYNTH + SND_SYNTH_CHANNEL
    ldr   rPatPtr, =g_snd + SND_SYNTH + SND_SYNTH_PAT
    ldr   rPatPtr, [rPatPtr]
snd_tick_next_channel:
    ldrh  r0, [rPatPtr]
    movs  r1, rChannelPtr
    cmp   r0, #0
    blne  snd_tick_channel
    cmp   r0, #0
    addne rEndFlag, #1
    adds  rPatPtr, #2
    adds  rChannelPtr, #SIZEOF_SND_CHANNEL_ST
    subs  rChannelLeft, #1
    bne   snd_tick_next_channel
    #undef rChannelLeft
    #undef rChannelPtr

    #define rDelta  r7
    // rDelta = *rPatPtr++
    ldrh  rDelta, [rPatPtr]
    adds  rPatPtr, #2
    ldr   r1, =g_snd + SND_SYNTH + SND_SYNTH_PAT
    str   rPatPtr, [r1]

    // check if last entry in pattern
    cmp   rEndFlag, #0
    bne   snd_tick_next_seq
snd_tick_done_next_seq:

    // return delta
    movs  r0, rDelta

snd_tick_return:
    pop   {r4-r7}
    pop   {r1}
    bx    r1
    #undef rEndFlag
    #undef rPatPtr
    #undef rDelta
snd_tick_next_seq:
    // end of pattern, load next one
    #define rSongBase     r2
    #define rPatTable     r3
    #define rSeqBase      r4
    #define rSeqIndex     r5
    #define rSeqIndexPtr  r6
    #define rDelta        r7
    ldr   rSongBase, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   rSongBase, [rSongBase]
    ldr   rSeqBase, [rSongBase, #SND_SONG_SEQ_TABLE_OFFSET]
    adds  rSeqBase, rSongBase
    ldr   r0, =g_snd + SND_SYNTH + SND_SYNTH_SEQUENCE
    ldr   r0, [r0]
    lsls  r0, #2
    adds  rSeqBase, r0
    ldr   rSeqBase, [rSeqBase]
    adds  rSeqBase, rSongBase
    ldr   rPatTable, [rSongBase, #SND_SONG_PAT_TABLE_OFFSET]
    adds  rPatTable, rSongBase
    ldr   rSeqIndexPtr, =g_snd + SND_SYNTH + SND_SYNTH_SEQ_INDEX
    ldr   rSeqIndex, [rSeqIndexPtr]
    adds  rSeqIndex, #1
    ldrh  r0, [rSeqBase, #SND_SONGSEQ_EXIT]
    cmp   rSeqIndex, r0
    ldrgeh rSeqIndex, [rSeqBase, #SND_SONGSEQ_LOOP_INDEX]
    str   rSeqIndex, [rSeqIndexPtr]
    lsls  rSeqIndex, #1
    adds  rSeqBase, #SND_SONGSEQ_PATTERNS
    ldrh  r0, [rSeqBase, rSeqIndex]
    lsls  r0, #2
    ldr   r0, [rPatTable, r0]
    adds  r0, rSongBase
    ldr   r1, =g_snd + SND_SYNTH + SND_SYNTH_PAT
    str   r0, [r1]
    b     snd_tick_done_next_seq
    #undef rSongBase
    #undef rPatTable
    #undef rSeqBase
    #undef rSeqIndex
    #undef rSeqIndexPtr
    #undef rDelta
    .align 4
    .pool

//
// bool snd_tick_channel(int instruction, struct channel_st *channel);
//
// Executes a single instruction for a channel; returns true if END was executed
//
snd_tick_channel: //(rInstruction, rChannelPtr) => end?
    push  {r4-r5}
    #define rInstruction  r0
    #define rChannelPtr   r1
    #define rPayload      r2
    #define rEndFlag      r3
    // apply effect
    movs  rEndFlag, #0
    lsrs  r4, rInstruction, #8
    ands  rPayload, r4, #0x3f
    lsrs  r4, #6
    lsls  r4, #2
    ldr   r5, =effect_jump_table
    ldr   r4, [r5, r4]
    bx    r4
effect_jump_table:
    .long set_command_delay
    .long set_volume_instrument
    .long set_bend
    .long set_tempo
set_command_delay:
    cmp   rPayload, #0
    beq   done_effect
    cmp   rPayload, #1
    beq   flag_end1
    cmp   rPayload, #2
    beq   set_bend0
    // otherwise, set delay
    subs  rPayload, #3
    str   rPayload, [rChannelPtr, #SND_CHANNEL_DELAY]
    b     done_effect
flag_end1:
    movs  rEndFlag, #1
    b     done_effect
set_volume_instrument:
    cmp   rPayload, #16
    ble   set_volume
    beq   set_instrument0
    // otherwise, set instrument
    subs  rPayload, #18
    movs  r4, #0
    str   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    str   r4, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_LEFT]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_OFF_LEFT]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_LEFT]
    // set instrument base pointer
    ldr   r4, =g_snd + SND_SYNTH + SND_SYNTH_SONG_BASE
    ldr   r4, [r4]
    ldr   r5, [r4, #SND_SONG_INST_TABLE_OFFSET]
    adds  r5, r4
    lsls  rPayload, #2
    ldr   r5, [r5, rPayload]
    adds  r5, r4
    str   r5, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    b     done_effect
set_volume:
    str   rPayload, [rChannelPtr, #SND_CHANNEL_CHAN_VOLUME]
    b     done_effect
set_instrument0:
    movs  r4, #0
    str   r4, [rChannelPtr, #SND_CHANNEL_INST_BASE]
    str   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    str   r4, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_LEFT]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_OFF_LEFT]
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_LEFT]
    b     done_effect
set_bend:
    adds  rPayload, #1
set_bend_body:
    ldr   r4, [rChannelPtr, #SND_CHANNEL_DELAY]
    cmp   r4, #0
    bne   delayed_bend
    ands  r4, rInstruction, #0x7f
    // (copied above for delay)
    lsls  r4, #pitchDivisionBits
    str   r4, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    cmp   rPayload, #0
    beq   bend_immediately2
    ldr   r5, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    subs  r4, r5
    rsblt r4, #0
    lsrs  r4, #pitchDivisionBits
    beq   skip_bend2
    subs  r4, #1
    ldr   ip, =g_snd + SND_SYNTH + SND_SYNTH_TEMPO_INDEX
    ldr   ip, [ip]
    lsls  ip, #7 + 1
    lsls  r4, #1
    adds  ip, r4
    ldr   r4, =_binary_snd_bend_bin_start
    ldrh  ip, [r4, ip]
    muls  rPayload, ip
    str   rPayload, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER_MAX]
    movs  r4, #0
    str   r4, [rChannelPtr, #SND_CHANNEL_BEND_COUNTER]
    movs  r0, #0
    b     snd_tick_channel_return
bend_immediately2:
    str   r4, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    movs  r0, #0
    b     snd_tick_channel_return
skip_bend2:
    ldr   r4, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    movs  r0, #0
    b     snd_tick_channel_return
set_bend0:
    movs  rPayload, #0
    b     set_bend_body
delayed_bend:
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_LEFT]
    str   rPayload, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_DURATION]
    ands  r4, rInstruction, #0x7f
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_BEND_NOTE]
    movs  r0, #0
    b     snd_tick_channel_return
set_tempo:
    // tempoIndex = payload
    // tickStart = tempoTable[tempoIndex]
    // tickLeft = 0
    ldr   r4, =g_snd + SND_SYNTH + SND_SYNTH_TEMPO_INDEX
    str   rPayload, [r4]
    lsls  rPayload, #1
    ldr   r5, =_binary_snd_tempo_bin_start
    ldrh  r5, [r5, rPayload]
    ldr   r4, =g_snd + SND_SYNTH + SND_SYNTH_TICK_START
    str   r5, [r4]
    ldr   r4, =g_snd + SND_SYNTH + SND_SYNTH_TICK_LEFT
    movs  r5, #0
    str   r5, [r4]
    b     done_effect
done_effect:
    #undef rPayload
    #define rNote      r2
    // apply note
    ands  rNote, rInstruction, #0xff
    cmp   rNote, #0
    beq   done_note
    cmp   rNote, #2
    beq   flag_end2
    cmp   rNote, #1
    beq   note_off

    // otherwise, note on
    // (copied above for delay)
    ldr   r4, [rChannelPtr, #SND_CHANNEL_DELAY]
    cmp   r4, #0
    bne   delayed_note_on
    cmp   rNote, #7
    beq   note_stop2
    movs  r4, #2
    str   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    // TODO: maybe some instruments shouldn't reset phase?
    // ex: saw, tri, sin, pcm => reset   sqX, rnd => no reset
    movs  r4, #0
    str   r4, [rChannelPtr, #SND_CHANNEL_PHASE]
    str   r4, [rChannelPtr, #SND_CHANNEL_ENV_VOLUME_INDEX]
    str   r4, [rChannelPtr, #SND_CHANNEL_ENV_PITCH_INDEX]
    lsls  r4, rNote, #pitchDivisionBits
    str   r4, [rChannelPtr, #SND_CHANNEL_BASE_PITCH]
    str   r4, [rChannelPtr, #SND_CHANNEL_TARGET_PITCH]
    b     done_note
note_stop2:
    movs  r4, #0
    str   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    b     done_note
delayed_note_on:
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_LEFT]
    str   rNote, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_ON_NOTE]
    b     done_note
flag_end2:
    movs  rEndFlag, #1
    b     done_note
note_off:
    ldr   r4, [rChannelPtr, #SND_CHANNEL_DELAY]
    cmp   r4, #0
    bne   delayed_note_off
    // otherwise, note off
    // (copied above for delay)
    ldr   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    cmp   r4, #2
    bne   done_note
    movs  r4, #1
    str   r4, [rChannelPtr, #SND_CHANNEL_STATE]
    b     done_note
delayed_note_off:
    str   r4, [rChannelPtr, #SND_CHANNEL_DELAYED_NOTE_OFF_LEFT]
    b     done_note
done_note:
    movs  r0, rEndFlag
snd_tick_channel_return:
    pop   {r4-r5}
    bx    lr
    #undef rInstruction
    #undef rChannelPtr
    #undef rNote
    #undef rEndFlag
    .align 4
    .pool

//
// void sys__snd_timer1_handler();
//
// Handles timer1, which is used to rotate buffers for the DMA.
//
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

//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once

#define REG_DISPCNT     *((volatile u16 *)0x04000000)
#define REG_DISPSTAT    *((volatile u16 *)0x04000004)
#define REG_VCOUNT      *((volatile u16 *)0x04000006)
#define REG_BG0CNT      *((volatile u16 *)0x04000008)
#define REG_BG1CNT      *((volatile u16 *)0x0400000a)
#define REG_BG2CNT      *((volatile u16 *)0x0400000c)
#define REG_BG3CNT      *((volatile u16 *)0x0400000e)
#define REG_BG0HOFS     *((volatile u16 *)0x04000010)
#define REG_BG0VOFS     *((volatile u16 *)0x04000012)
#define REG_BG1HOFS     *((volatile u16 *)0x04000014)
#define REG_BG1VOFS     *((volatile u16 *)0x04000016)
#define REG_BG2HOFS     *((volatile u16 *)0x04000018)
#define REG_BG2VOFS     *((volatile u16 *)0x0400001a)
#define REG_BG3HOFS     *((volatile u16 *)0x0400001c)
#define REG_BG3VOFS     *((volatile u16 *)0x0400001e)
#define REG_BG2PA       *((volatile u16 *)0x04000020)
#define REG_BG2PB       *((volatile u16 *)0x04000022)
#define REG_BG2PC       *((volatile u16 *)0x04000024)
#define REG_BG2PD       *((volatile u16 *)0x04000026)
#define REG_BG2X        *((volatile i32 *)0x04000028)
#define REG_BG2Y        *((volatile i32 *)0x0400002c)
#define REG_BG3PA       *((volatile u16 *)0x04000030)
#define REG_BG3PB       *((volatile u16 *)0x04000032)
#define REG_BG3PC       *((volatile u16 *)0x04000034)
#define REG_BG3PD       *((volatile u16 *)0x04000036)
#define REG_BG3X        *((volatile i32 *)0x04000038)
#define REG_BG3Y        *((volatile i32 *)0x0400003c)
#define REG_WIN0H       *((volatile u16 *)0x04000040)
#define REG_WIN1H       *((volatile u16 *)0x04000042)
#define REG_WIN0V       *((volatile u16 *)0x04000044)
#define REG_WIN1V       *((volatile u16 *)0x04000046)
#define REG_WININ       *((volatile u16 *)0x04000048)
#define REG_WINOUT      *((volatile u16 *)0x0400004a)
#define REG_MOSAIC      *((volatile u16 *)0x0400004c)
#define REG_BLDCNT      *((volatile u16 *)0x04000050)
#define REG_BLDALPHA    *((volatile u16 *)0x04000052)
#define REG_BLDY        *((volatile u16 *)0x04000054)
#define REG_BLDVAL      *((volatile u16 *)0x04000054)
#define REG_SOUND1CNT_L *((volatile u16 *)0x04000060)
#define REG_SOUND1CNT_H *((volatile u16 *)0x04000062)
#define REG_SOUND1CNT_X *((volatile u16 *)0x04000064)
#define REG_SOUND2CNT_L *((volatile u16 *)0x04000068)
#define REG_SOUND2CNT_H *((volatile u16 *)0x0400006c)
#define REG_SOUND3CNT_L *((volatile u16 *)0x04000070)
#define REG_SOUND3CNT_H *((volatile u16 *)0x04000072)
#define REG_SOUND3CNT_X *((volatile u16 *)0x04000074)
#define REG_SOUND4CNT_L *((volatile u16 *)0x04000078)
#define REG_SOUND4CNT_H *((volatile u16 *)0x0400007c)
#define REG_SOUNDCNT_L  *((volatile u16 *)0x04000080)
#define REG_SOUNDCNT_H  *((volatile u16 *)0x04000082)
#define REG_SOUNDCNT_X  *((volatile u16 *)0x04000084)
#define REG_SOUNDBIAS   *((volatile u16 *)0x04000088)
#define REG_WAVE_RAM    *((volatile u16 *)0x04000090)
#define REG_FIFO_A      *((volatile u32 *)0x040000a0)
#define REG_FIFO_B      *((volatile u32 *)0x040000a4)
#define REG_DMA0SAD     *((volatile u32 *)0x040000b0)
#define REG_DMA0SRC     *((volatile u32 *)0x040000b0)
#define REG_DMA0DAD     *((volatile u32 *)0x040000b4)
#define REG_DMA0DST     *((volatile u32 *)0x040000b4)
#define REG_DMA0CNT_L   *((volatile u16 *)0x040000b8)
#define REG_DMA0LEN     *((volatile u16 *)0x040000b8)
#define REG_DMA0CNT_H   *((volatile u16 *)0x040000ba)
#define REG_DMA0CNT     *((volatile u16 *)0x040000ba)
#define REG_DMA1SAD     *((volatile u32 *)0x040000bc)
#define REG_DMA1SRC     *((volatile u32 *)0x040000bc)
#define REG_DMA1DAD     *((volatile u32 *)0x040000c0)
#define REG_DMA1DST     *((volatile u32 *)0x040000c0)
#define REG_DMA1CNT_L   *((volatile u16 *)0x040000c4)
#define REG_DMA1LEN     *((volatile u16 *)0x040000c4)
#define REG_DMA1CNT_H   *((volatile u16 *)0x040000c6)
#define REG_DMA1CNT     *((volatile u16 *)0x040000c6)
#define REG_DMA2SAD     *((volatile u32 *)0x040000c8)
#define REG_DMA2SRC     *((volatile u32 *)0x040000c8)
#define REG_DMA2DAD     *((volatile u32 *)0x040000cc)
#define REG_DMA2DST     *((volatile u32 *)0x040000cc)
#define REG_DMA2CNT_L   *((volatile u16 *)0x040000d0)
#define REG_DMA2LEN     *((volatile u16 *)0x040000d0)
#define REG_DMA2CNT_H   *((volatile u16 *)0x040000d2)
#define REG_DMA2CNT     *((volatile u16 *)0x040000d2)
#define REG_DMA3SAD     *((volatile u32 *)0x040000d4)
#define REG_DMA3SRC     *((volatile u32 *)0x040000d4)
#define REG_DMA3DAD     *((volatile u32 *)0x040000d8)
#define REG_DMA3DST     *((volatile u32 *)0x040000d8)
#define REG_DMA3CNT_L   *((volatile u16 *)0x040000dc)
#define REG_DMA3LEN     *((volatile u16 *)0x040000dc)
#define REG_DMA3CNT_H   *((volatile u16 *)0x040000de)
#define REG_DMA3CNT     *((volatile u16 *)0x040000de)
#define REG_TM0CNT_L    *((volatile u16 *)0x04000100)
#define REG_TM0D        *((volatile u16 *)0x04000100)
#define REG_TM0VAL      *((volatile u16 *)0x04000100)
#define REG_TM0CNT_H    *((volatile u16 *)0x04000102)
#define REG_TM0CNT      *((volatile u16 *)0x04000102)
#define REG_TM1CNT_L    *((volatile u16 *)0x04000104)
#define REG_TM1D        *((volatile u16 *)0x04000104)
#define REG_TM1VAL      *((volatile u16 *)0x04000104)
#define REG_TM1CNT_H    *((volatile u16 *)0x04000106)
#define REG_TM1CNT      *((volatile u16 *)0x04000106)
#define REG_TM2CNT_L    *((volatile u16 *)0x04000108)
#define REG_TM2D        *((volatile u16 *)0x04000108)
#define REG_TM2VAL      *((volatile u16 *)0x04000108)
#define REG_TM2CNT_H    *((volatile u16 *)0x0400010a)
#define REG_TM2CNT      *((volatile u16 *)0x0400010a)
#define REG_TM3CNT_L    *((volatile u16 *)0x0400010c)
#define REG_TM3D        *((volatile u16 *)0x0400010c)
#define REG_TM3VAL      *((volatile u16 *)0x0400010c)
#define REG_TM3CNT_H    *((volatile u16 *)0x0400010e)
#define REG_TM3CNT      *((volatile u16 *)0x0400010e)
#define REG_SIODATA32   *((volatile u32 *)0x04000120)
#define REG_SIOMULTI0   *((volatile u16 *)0x04000120)
#define REG_SIOMULTI1   *((volatile u16 *)0x04000122)
#define REG_SIOMULTI2   *((volatile u16 *)0x04000124)
#define REG_SIOMULTI3   *((volatile u16 *)0x04000126)
#define REG_SIOCNT      *((volatile u16 *)0x04000128)
#define REG_SIOMLT_SEND *((volatile u16 *)0x0400012a)
#define REG_SIODATA8    *((volatile u16 *)0x0400012a)
#define REG_KEYINPUT    *((volatile u16 *)0x04000130)
#define REG_KEYCNT      *((volatile u16 *)0x04000132)
#define REG_RCNT        *((volatile u16 *)0x04000134)
#define REG_SIOMODE2    *((volatile u16 *)0x04000134)
#define REG_JOYCNT      *((volatile u16 *)0x04000140)
#define REG_HS_CTRL     *((volatile u16 *)0x04000140)
#define REG_JOY_RECV    *((volatile u32 *)0x04000150)
#define REG_JOYRE       *((volatile u32 *)0x04000150)
#define REG_JOY_TRANS   *((volatile u32 *)0x04000154)
#define REG_JOYTR       *((volatile u32 *)0x04000154)
#define REG_JOYSTAT     *((volatile u16 *)0x04000158)
#define REG_JSTAT       *((volatile u16 *)0x04000158)
#define REG_IE          *((volatile u16 *)0x04000200)
#define REG_IF          *((volatile u16 *)0x04000202)
#define REG_WAITCNT     *((volatile u16 *)0x04000204)
#define REG_IME         *((volatile u8  *)0x04000208)
#define REG_POSTFLG     *((volatile u8  *)0x04000300)
#define REG_HALTCNT     *((volatile u8  *)0x04000301)
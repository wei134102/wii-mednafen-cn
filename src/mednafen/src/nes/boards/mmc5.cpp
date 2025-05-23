/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* None of this code should use any of the iNES bank switching wrappers. */

#include "mapinc.h"
#include "../nsf.h"

static void (*sfun)(int P);
static void (*psfun)(void);

void MMC5RunSoundHQ(void);

static INLINE void MMC5SPRVROM_BANK1(uint32 A,uint32 V) 
{
 if(CHRptr[0])
 {
  V&=CHRmask1[0];
  MMC5SPRVPage[(A)>>10]=&CHRptr[0][(V)<<10]-(A);
 }
}

static INLINE void MMC5BGVROM_BANK1(uint32 A,uint32 V) {if(CHRptr[0]){V&=CHRmask1[0];MMC5BGVPage[(A)>>10]=&CHRptr[0][(V)<<10]-(A);}}

static INLINE void MMC5SPRVROM_BANK2(uint32 A,uint32 V) {if(CHRptr[0]){V&=CHRmask2[0];MMC5SPRVPage[(A)>>10]=MMC5SPRVPage[((A)>>10)+1]=&CHRptr[0][(V)<<11]-(A);}}
static INLINE void MMC5BGVROM_BANK2(uint32 A,uint32 V) {if(CHRptr[0]){V&=CHRmask2[0];MMC5BGVPage[(A)>>10]=MMC5BGVPage[((A)>>10)+1]=&CHRptr[0][(V)<<11]-(A);}}

static INLINE void MMC5SPRVROM_BANK4(uint32 A,uint32 V) {if(CHRptr[0]){V&=CHRmask4[0];MMC5SPRVPage[(A)>>10]=MMC5SPRVPage[((A)>>10)+1]= MMC5SPRVPage[((A)>>10)+2]=MMC5SPRVPage[((A)>>10)+3]=&CHRptr[0][(V)<<12]-(A);}}
static INLINE void MMC5BGVROM_BANK4(uint32 A,uint32 V) {if(CHRptr[0]){V&=CHRmask4[0];MMC5BGVPage[(A)>>10]=MMC5BGVPage[((A)>>10)+1]=MMC5BGVPage[((A)>>10)+2]=MMC5BGVPage[((A)>>10)+3]=&CHRptr[0][(V)<<12]-(A);}}

static INLINE void MMC5SPRVROM_BANK8(uint32 V) {if(CHRptr[0]){V&=CHRmask8[0];MMC5SPRVPage[0]=MMC5SPRVPage[1]=MMC5SPRVPage[2]=MMC5SPRVPage[3]=MMC5SPRVPage[4]=MMC5SPRVPage[5]=MMC5SPRVPage[6]=MMC5SPRVPage[7]=&CHRptr[0][(V)<<13];}}
static INLINE void MMC5BGVROM_BANK8(uint32 V) {if(CHRptr[0]){V&=CHRmask8[0];MMC5BGVPage[0]=MMC5BGVPage[1]=MMC5BGVPage[2]=MMC5BGVPage[3]=MMC5BGVPage[4]=MMC5BGVPage[5]=MMC5BGVPage[6]=MMC5BGVPage[7]=&CHRptr[0][(V)<<13];}}

static uint8 PRGBanks[4];
static uint8 WRAMPage;
static uint32 CHRBanksA[8], CHRBanksB[4];
static uint8 WRAMMaskEnable[2];
static uint8 ABMode;		/* A=0, B=1 */

static uint8 IRQScanline,IRQEnable;
static uint8 CHRMode, NTAMirroring, NTFill, ATFill;

static uint8 MMC5IRQR;
static uint8 MMC5LineCounter;
static uint8 mmc5psize, mmc5vsize;
static uint8 MMC5BigCHRSelect;
static uint8 mul[2];

static uint8 *WRAM=NULL;
static uint8 *MMC5fill=NULL;
static uint8 *ExRAM=NULL;

static uint8 MMC5WRAMsize;
static uint8 MMC5WRAMIndex[8];

static uint8 MMC5ROMWrProtect[4];
static uint8 MMC5MemIn[5];

static void MMC5CHRA(void);
static void MMC5CHRB(void);

typedef struct __cartdata {
        uint32 crc32;
        uint8 size;
} cartdata;


// ETROM seems to have 16KB of WRAM, ELROM seems to have 8KB
// EWROM seems to have 32KB of WRAM

#define MMC5_NOCARTS 14
cartdata MMC5CartList[MMC5_NOCARTS]=
{
 {0x9c18762b,2},         /* L'Empereur */
 {0x26533405,2},
 {0x6396b988,2},

 {0xaca15643,2},        /* Uncharted Waters */
 {0xfe3488d1,2},        /* Dai Koukai Jidai */

 {0x15fe6d0f,2},        /* BKAC             */
 {0x39f2ce4b,2},        /* Suikoden              */

 {0x8ce478db,2},        /* Nobunaga's Ambition 2 */
 {0xeee9a682,2},

 {0x1ced086f,2},	/* Ishin no Arashi */

 {0xf540677b,4},        /* Nobunaga...Bushou Fuuun Roku */

 {0x6f4e4312,4},        /* Aoki Ookami..Genchou */

 {0xf011e490,4},        /* Romance of the 3 Kingdoms 2 */
 {0x184c2124,4},        /* Sangokushi 2 */
};


int DetectMMC5WRAMSize(uint32 crc32)
{
 int x;

 for(x=0;x<MMC5_NOCARTS;x++)
  if(crc32==MMC5CartList[x].crc32)
  {
    MDFN_printf(_(">8KB external WRAM present.  Use UNIF if you hack the ROM image.\n"));
    return(MMC5CartList[x].size*8);
  }
 return(8);
}

static void BuildWRAMSizeTable(void)
{
 int x;

 for(x=0;x<8;x++)
 {
  switch(MMC5WRAMsize)
  {
    case 0:MMC5WRAMIndex[x]=255;break;
    case 1:MMC5WRAMIndex[x]=(x>3)?255:0;break;
    case 2:MMC5WRAMIndex[x]=(x&4)>>2;break;
    case 4:MMC5WRAMIndex[x]=(x>3)?255:(x&3);break;
  }
 }
}

static void MMC5CHRA(void)
{
	int x;
	switch(mmc5vsize&3)
	{
	 case 0:setchr8(CHRBanksA[7]);
		MMC5SPRVROM_BANK8(CHRBanksA[7]);
        	break;
	 case 1:setchr4(0x0000,CHRBanksA[3]);
	        setchr4(0x1000,CHRBanksA[7]);
		MMC5SPRVROM_BANK4(0x0000,CHRBanksA[3]);
                MMC5SPRVROM_BANK4(0x1000,CHRBanksA[7]);
        	break;
 	 case 2:setchr2(0x0000,CHRBanksA[1]);
        	setchr2(0x0800,CHRBanksA[3]);
	        setchr2(0x1000,CHRBanksA[5]);
	        setchr2(0x1800,CHRBanksA[7]);
		MMC5SPRVROM_BANK2(0x0000,CHRBanksA[1]);
                MMC5SPRVROM_BANK2(0x0800,CHRBanksA[3]);
                MMC5SPRVROM_BANK2(0x1000,CHRBanksA[5]);
                MMC5SPRVROM_BANK2(0x1800,CHRBanksA[7]);
        	break;
	 case 3:
	 	for(x=0;x<8;x++)
	        {
	         setchr1(x<<10,CHRBanksA[x]);
	         MMC5SPRVROM_BANK1(x<<10,CHRBanksA[x]);
		}
	        break;
 }
}

static void MMC5CHRB(void)
{
 int x;
 switch(mmc5vsize&3)
 {
	case 0:	
		setchr8(CHRBanksB[3]);
		MMC5BGVROM_BANK8(CHRBanksB[3]);
	        break;

	case 1: 
		setchr4(0x0000,CHRBanksB[3]);
	       	setchr4(0x1000,CHRBanksB[3]);
	        MMC5BGVROM_BANK4(0x0000,CHRBanksB[3]);
	        MMC5BGVROM_BANK4(0x1000,CHRBanksB[3]);
	        break;

	case 2:	
		setchr2(0x0000,CHRBanksB[1]);
		setchr2(0x0800,CHRBanksB[3]);
		setchr2(0x1000,CHRBanksB[1]);
		setchr2(0x1800,CHRBanksB[3]);
		MMC5BGVROM_BANK2(0x0000,CHRBanksB[1]);
	        MMC5BGVROM_BANK2(0x0800,CHRBanksB[3]);
	        MMC5BGVROM_BANK2(0x1000,CHRBanksB[1]);
	        MMC5BGVROM_BANK2(0x1800,CHRBanksB[3]);
	        break;
	case 3:
        	for(x=0;x<8;x++)
		{
		 setchr1(x<<10,CHRBanksB[x&3]);
	         MMC5BGVROM_BANK1(x<<10,CHRBanksB[x&3]);
		}
	        break;
 }
}

static void MMC5WRAM(uint32 A, uint32 V)
{
   //printf("%02x\n",V);
   V=MMC5WRAMIndex[V&7];
   if(V!=255)
   {
    setprg8r(0x10,A,V);
    MMC5MemIn[(A-0x6000)>>13]=1;
   }
   else
    MMC5MemIn[(A-0x6000)>>13]=0;
}

static void MMC5PRG(void)
{
 int x;

 switch(mmc5psize&3)
 {
  case 0:
           MMC5ROMWrProtect[0]=MMC5ROMWrProtect[1]=
           MMC5ROMWrProtect[2]=MMC5ROMWrProtect[3]=1;
           setprg32(0x8000,((PRGBanks[3]&0x7F)>>2));
	   for(x=0;x<4;x++)
 	    MMC5MemIn[1+x]=1;
           break;
  case 1:
         if(PRGBanks[1]&0x80)
          {
           MMC5ROMWrProtect[0]=MMC5ROMWrProtect[1]=1;
           setprg16(0x8000,(PRGBanks[1]>>1));
	   MMC5MemIn[1]=MMC5MemIn[2]=1;
          }
         else
          {
           MMC5ROMWrProtect[0]=MMC5ROMWrProtect[1]=0;
           MMC5WRAM(0x8000,PRGBanks[1]&7&0xFE);
           MMC5WRAM(0xA000,(PRGBanks[1]&7&0xFE)+1);
          }
	  MMC5MemIn[3]=MMC5MemIn[4]=1;
          MMC5ROMWrProtect[2]=MMC5ROMWrProtect[3]=1;
          setprg16(0xC000,(PRGBanks[3]&0x7F)>>1);
         break;
  case 2:
         if(PRGBanks[1]&0x80)
          {
	   MMC5MemIn[1]=MMC5MemIn[2]=1;
           MMC5ROMWrProtect[0]=MMC5ROMWrProtect[1]=1;
           setprg16(0x8000,(PRGBanks[1]&0x7F)>>1);
          }
         else
          {
           MMC5ROMWrProtect[0]=MMC5ROMWrProtect[1]=0;
           MMC5WRAM(0x8000,PRGBanks[1]&7&0xFE);
           MMC5WRAM(0xA000,(PRGBanks[1]&7&0xFE)+1);
          }
         if(PRGBanks[2]&0x80)
          {MMC5ROMWrProtect[2]=1;MMC5MemIn[3]=1;setprg8(0xC000,PRGBanks[2]&0x7F);}
         else
          {MMC5ROMWrProtect[2]=0;MMC5WRAM(0xC000,PRGBanks[2]&7);}
	 MMC5MemIn[4]=1;
         MMC5ROMWrProtect[3]=1;
         setprg8(0xE000,PRGBanks[3]&0x7F);
         break;
  case 3:
	 for(x=0;x<3;x++)
	  if(PRGBanks[x]&0x80)
	  {
	   MMC5ROMWrProtect[x]=1;
	   setprg8(0x8000+(x<<13),PRGBanks[x]&0x7F);
	   MMC5MemIn[1+x]=1;
	  }
	  else
	  {
	   MMC5ROMWrProtect[x]=0;
	   MMC5WRAM(0x8000+(x<<13),PRGBanks[x]&7);
	  }
	 MMC5MemIn[4]=1;
         MMC5ROMWrProtect[3]=1;
         setprg8(0xE000,PRGBanks[3]&0x7F);
         break;
  }
}

static DECLFW(Mapper5_write)
{
 if(A >= 0x5120 && A<=0x5127)
 {
  MDFNPPU_LineUpdate();
  ABMode = 0;
  CHRBanksA[A&7] = V | (MMC5BigCHRSelect << 8);
  MMC5CHRA();
 }
 else if(A >= 0x5128 && A <= 0x512B)
 {
  MDFNPPU_LineUpdate();
  ABMode=1;
  CHRBanksB[A&3] = V | (MMC5BigCHRSelect << 8);
  MMC5CHRB();
 }
 else switch(A)
 {
   default:
	   //printf("$%04x, $%02x\n",A,V);
	   break;
   case 0x5105:
		MDFNPPU_LineUpdate();
                {
                int x;
                for(x=0;x<4;x++)
                {
                 switch((V>>(x<<1))&3)
                 {
                  case 0:PPUNTARAM|=1<<x;vnapage[x]=NTARAM;break;
                  case 1:PPUNTARAM|=1<<x;vnapage[x]=NTARAM+0x400;break;
                  case 2:PPUNTARAM|=1<<x;vnapage[x]=ExRAM;break;
                  case 3:PPUNTARAM&=~(1<<x);vnapage[x]=MMC5fill;break;
                 }
                }
               }
	       NTAMirroring=V;
               break;

   case 0x5113:WRAMPage=V;
	       MMC5WRAM(0x6000,V&7);
	       break;

   case 0x5100:mmc5psize=V;
	       MMC5PRG();
	       break;

   case 0x5101:MDFNPPU_LineUpdate();
	       mmc5vsize=V;
               if(!ABMode)
                {MMC5CHRB();MMC5CHRA();}
               else
                {MMC5CHRA();MMC5CHRB();}
               break;

   case 0x5114:
   case 0x5115:
   case 0x5116:
   case 0x5117:
	       PRGBanks[A&3]=V;
	       MMC5PRG();
	       break;

   case 0x5102:WRAMMaskEnable[0]=V;
	       break;

   case 0x5103:WRAMMaskEnable[1]=V;
	       break;

   case 0x5104:MDFNPPU_LineUpdate();
	       CHRMode=V;
	       MMC5HackCHRMode=V&3;
	       break;

   case 0x5106:if(V!=NTFill)
               {
		uint32 t;

		MDFNPPU_LineUpdate();
		t=V|(V<<8)|(V<<16)|(V<<24);
                memset(MMC5fill,t,0x3c0);
               }
	       NTFill=V;
               break;

   case 0x5107:if(V!=ATFill)
               {
                unsigned char moop;
                uint32 t;

		MDFNPPU_LineUpdate();
                moop=V|(V<<2)|(V<<4)|(V<<6);
		t=moop|(moop<<8)|(moop<<16)|(moop<<24);
                memset(MMC5fill+0x3c0,t,0x40);
               }
	       ATFill=V;
               break;

   case 0x5130: MMC5HackCHRBank = MMC5BigCHRSelect = V & 0x3;
	        //printf("Write: %04x %02x\n", A, V);
		break;

   case 0x5200:MMC5HackSPMode=V;break;
   case 0x5201:MMC5HackSPScroll=(V>>3)&0x1F;break;
   case 0x5202:MMC5HackSPPage=V&0x3F;break;
   case 0x5203:X6502_IRQEnd(MDFN_IQEXT);IRQScanline=V;break;
   case 0x5204:X6502_IRQEnd(MDFN_IQEXT);IRQEnable=V&0x80;break;
   case 0x5205:mul[0]=V;break;
   case 0x5206:mul[1]=V;break;
  }
}

static DECLFR(MMC5_ReadROMRAM)
{
	if(MMC5MemIn[(A-0x6000)>>13])
         return Page[A>>11][A];
	else
	 return X.DB;
}

static DECLFW(MMC5_WriteROMRAM)
{
       if(A>=0x8000)
        if(MMC5ROMWrProtect[(A-0x8000)>>13])
         return;
       if(MMC5MemIn[(A-0x6000)>>13])
        if(((WRAMMaskEnable[0]&3)|((WRAMMaskEnable[1]&3)<<2)) == 6)
         Page[A>>11][A]=V;
}

static DECLFW(MMC5_ExRAMWr)
{
 if(MMC5HackCHRMode!=3)
  ExRAM[A&0x3ff]=V;
}

static DECLFR(MMC5_ExRAMRd)
{
 /* Not sure if this is correct, so I'll comment it out for now. */
 //if(MMC5HackCHRMode>=2)
  return ExRAM[A&0x3ff];
 //else
 // return(X.DB);
}

static DECLFR(MMC5_read)
{
 switch(A)
 {
  //default:printf("$%04x\n",A);break;
  case 0x5204:X6502_IRQEnd(MDFN_IQEXT);
              {
		uint8 x;
		x=MMC5IRQR;
		if(!fceuindbg)
		 MMC5IRQR&=0x40;
		return x;
	      }
  case 0x5205:return (mul[0]*mul[1]);
  case 0x5206:return ((mul[0]*mul[1])>>8);
 }
 return(X.DB);
}

void MMC5Synco(void)
{
 int x;

 MMC5PRG();
 for(x=0;x<4;x++)
 {
  switch((NTAMirroring>>(x<<1))&3)
   {
    case 0:PPUNTARAM|=1<<x;vnapage[x]=NTARAM;break;
    case 1:PPUNTARAM|=1<<x;vnapage[x]=NTARAM+0x400;break;
    case 2:PPUNTARAM|=1<<x;vnapage[x]=ExRAM;break;
    case 3:PPUNTARAM&=~(1<<x);vnapage[x]=MMC5fill;break;
   }
 }
 MMC5WRAM(0x6000,WRAMPage&7);
 if(!ABMode)
 {
  MMC5CHRB();
  MMC5CHRA();
 }
 else
 {
   MMC5CHRA();
   MMC5CHRB();
 }
  {
    uint32 t;
    t=NTFill|(NTFill<<8)|(NTFill<<16)|(NTFill<<24);
    memset(MMC5fill,t,0x3c0);
  }
  {
   unsigned char moop;
   uint32 t;
   moop=ATFill|(ATFill<<2)|(ATFill<<4)|(ATFill<<6);
   t=moop|(moop<<8)|(moop<<16)|(moop<<24);
   memset(MMC5fill+0x3c0,t,0x40);
  }
  X6502_IRQEnd(MDFN_IQEXT);
  MMC5HackCHRMode=CHRMode&3;
}

void MMC5_hb(int cur_scanline)
{
  //printf("%d:%d, ",scanline,MMC5LineCounter);
  if(cur_scanline==240)
  {
  // printf("\n%d:%d\n",scanline,MMC5LineCounter);
   MMC5LineCounter=0;
   MMC5IRQR=0x40;
   return;
  }

  if(MMC5LineCounter<240)
  {
   if(MMC5LineCounter==IRQScanline) 
   {
    MMC5IRQR|=0x80;
    if(IRQEnable&0x80)
     X6502_IRQBegin(MDFN_IQEXT);
   }
   MMC5LineCounter++;
  }
//  printf("%d:%d\n",MMC5LineCounter,scanline);
 if(MMC5LineCounter==240)
   MMC5IRQR=0;
}

typedef struct {
        uint16 wl[2];
        uint8 env[2];
        uint8 enable;
	uint8 running;
        uint8 raw;
        uint8 rawcontrol;

        int32 dcount[2];

        uint32 BC[3];
        int32 vcount[2];
} MMC5APU;

static MMC5APU MMC5Sound;

static void Do5PCMHQ()
{
   uint32 V;
   if(!(MMC5Sound.rawcontrol&0x40) && MMC5Sound.raw)
    for(V=MMC5Sound.BC[2];V<SOUNDTS;V++)
     WaveHiEx[V]-=MMC5Sound.raw<<5;
   MMC5Sound.BC[2]=SOUNDTS;
} 

void MMC5HiSync(int32 ts)
{
 int x;
 for(x=0;x<3;x++) MMC5Sound.BC[x]=ts;
}

static DECLFW(Mapper5_SW)
{
 A&=0x1F;

 switch(A)
 {
  case 0x10:if(psfun) psfun();MMC5Sound.rawcontrol=V;break;
  case 0x11:if(psfun) psfun();MMC5Sound.raw=V;break;

  case 0x0:
  case 0x4://printf("%04x:$%02x\n",A,V&0x30);
	   if(sfun) sfun(A>>2);
           MMC5Sound.env[A>>2]=V;
           break;
  case 0x2:
  case 0x6:if(sfun) sfun(A>>2);
           MMC5Sound.wl[A>>2]&=~0x00FF;
           MMC5Sound.wl[A>>2]|=V&0xFF;
           break;
  case 0x3:
  case 0x7://printf("%04x:$%02x\n",A,V>>3);
	   MMC5Sound.wl[A>>2]&=~0x0700;
           MMC5Sound.wl[A>>2]|=(V&0x07)<<8;           
	   MMC5Sound.running|=1<<(A>>2);
	   break;
  case 0x15:if(sfun)
            {
             sfun(0);
             sfun(1);
            }
	    MMC5Sound.running&=V;
            MMC5Sound.enable=V;
		//printf("%02x\n",V);
            break;
 }
}

static void Do5SQHQ(int P)
{
 static const int tal[4]={1,2,4,6};
 uint32 V;
 int32 amp,rthresh,wl;
  
 wl=MMC5Sound.wl[P]+1;
 amp=((MMC5Sound.env[P]&0xF)<<8);
 rthresh=tal[(MMC5Sound.env[P]&0xC0)>>6];
     
 if(wl>=8 && (MMC5Sound.running&(P+1)))
 {
  int dc,vc;
  
  wl<<=1;

  dc=MMC5Sound.dcount[P];   
  vc=MMC5Sound.vcount[P];
  for(V=MMC5Sound.BC[P];V<SOUNDTS;V++)
  {
    if(dc<rthresh)
     WaveHiEx[V]-=amp;
    vc--;
    if(vc<=0)   /* Less than zero when first started. */
    {
     vc=wl;
     dc=(dc+1)&7;
    }
  }  
  MMC5Sound.dcount[P]=dc;
  MMC5Sound.vcount[P]=vc;
 }
 MMC5Sound.BC[P]=SOUNDTS;
}

void MMC5RunSoundHQ(void)
{  
  Do5SQHQ(0);
  Do5SQHQ(1);
  Do5PCMHQ();
}

static void Mapper5_ESI(EXPSOUND *ep)
{
 sfun = Do5SQHQ;
 psfun = Do5PCMHQ;   

 memset(MMC5Sound.BC,0,sizeof(MMC5Sound.BC));
 memset(MMC5Sound.vcount,0,sizeof(MMC5Sound.vcount));

 ep->HiSync = MMC5HiSync;
 ep->HiFill = MMC5RunSoundHQ;
}

int NSFMMC5_Init(EXPSOUND *ep, bool MultiChip)
{
  memset(&MMC5Sound,0,sizeof(MMC5Sound));
  mul[0]=mul[1]=0;
  if(!(ExRAM=(uint8*)MDFN_malloc(1024, _("MMC5 EXRAM"))))
   return(0);
  Mapper5_ESI(ep);
  NSFECSetWriteHandler(0x5c00,0x5fef,MMC5_ExRAMWr);
  SetReadHandler(0x5c00,0x5fef,MMC5_ExRAMRd); 
  MMC5HackCHRMode=2;
  NSFECSetWriteHandler(0x5000,0x5015,Mapper5_SW);
  NSFECSetWriteHandler(0x5205,0x5206,Mapper5_write);
  SetReadHandler(0x5205,0x5206,MMC5_read);
  return(1);
}

void NSFMMC5_Close(void)
{
 if(ExRAM)
  MDFN_free(ExRAM);
 ExRAM=NULL;
}

static void GenMMC5Reset(CartInfo *info)
{
 int x;

 memset(ExRAM, 0, 0x400);

 MMC5BigCHRSelect = 0;

 for(x=0;x<4;x++) PRGBanks[x]=~0;
 for(x=0;x<8;x++) CHRBanksA[x]=~0;
 for(x=0;x<4;x++) CHRBanksB[x]=~0;
 WRAMMaskEnable[0]=WRAMMaskEnable[1]=~0;

 mmc5psize=mmc5vsize=3;
 CHRMode=0;

 NTAMirroring=NTFill=ATFill=0xFF;

 MMC5Synco();

 //GameHBIRQHook=MMC5_hb;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
	SFORMAT StateRegs[]=
	{
	 SFARRAY(WRAM, MMC5WRAMsize * 8192),
	 SFARRAY(ExRAM, 1024),
         SFARRAYN(PRGBanks, 4, "PRGB"),
         SFARRAY32N(CHRBanksA, 8, "CHRA"),
         SFARRAY32N(CHRBanksB, 4, "CHRB"),

         SFVARN(WRAMPage, "WRMP"), 
	 SFARRAYN(WRAMMaskEnable, 2, "WRME"),
	 SFVARN(ABMode, "ABMD"),
	 SFVARN(IRQScanline, "IRQS"),
	 SFVARN(IRQEnable, "IRQE"),
	 SFVARN(CHRMode, "CHRM"),
	 SFVARN(NTAMirroring, "NTAM"),
	 SFVARN(NTFill, "NTFL"),
	 SFVARN(ATFill, "ATFL"),

	 SFVARN(MMC5HackSPMode, "SPLM"),
	 SFVARN(MMC5HackSPScroll, "SPLS"),
	 SFVARN(MMC5HackSPPage, "SPLP"),

         SFVARN(MMC5Sound.wl[0], "SDW0"),
         SFVARN(MMC5Sound.wl[1], "SDW1"),
         SFARRAYN(MMC5Sound.env, 2, "SDEV"),
         SFVARN(MMC5Sound.enable, "SDEN"),
	 SFVARN(MMC5Sound.running, "SDRU"),
         SFVARN(MMC5Sound.raw, "SDRW"),
         SFVARN(MMC5Sound.rawcontrol, "SDRC"),
	 SFVARN(MMC5Sound.vcount[0], "SDV0"),
	 SFVARN(MMC5Sound.vcount[1], "SDV1"),
         SFVARN(MMC5Sound.dcount[0], "SDD0"),
         SFVARN(MMC5Sound.dcount[1], "SDD1"),
         SFEND
	};

	int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MMC5");

	if(load)
	 MMC5Synco();

	return(ret);
}

static void GenMMC5_Close(void)
{
 if(WRAM)
 {
  MDFN_free(WRAM);
  WRAM = NULL;
 }

 if(MMC5fill)
 {
  MDFN_free(MMC5fill);
  MMC5fill = NULL;
 }

 if(ExRAM)
 {
  MDFN_free(ExRAM);
  ExRAM = NULL;
 }
}

static int GenMMC5_Init(CartInfo *info, int wsize, int battery)
{
 info->StateAction = StateAction;

 if(wsize)
 {
  if(!(WRAM=(uint8*)MDFN_malloc(wsize*1024, _("WRAM"))))
  {
   GenMMC5_Close();
   return(0);
  }
  memset(WRAM, 0x00, wsize * 1024);
  SetupCartPRGMapping(0x10,WRAM,wsize*1024,1);
 }

 if(!(MMC5fill=(uint8*)MDFN_malloc(1024, _("MMC5 Fill"))))
 {
  GenMMC5_Close();
  return(0);
 }

 if(!(ExRAM = (uint8*)MDFN_malloc(1024, _("MMC5 EXRAM"))))
 {
  GenMMC5_Close();
  return(0);
 }

 MMC5WRAMsize=wsize/8; 
 BuildWRAMSizeTable();
 info->Power=GenMMC5Reset;

 if(battery)
 {
  info->SaveGame[0]=WRAM;
  if(wsize<=16)
   info->SaveGameLen[0]=8192;
  else
   info->SaveGameLen[0]=32768;
 }

 MMC5HackVROMMask=CHRmask4[0];
 MMC5HackExNTARAMPtr=ExRAM;
 MMC5Hack=1;
 MMC5HackVROMPTR=CHRptr[0];
 MMC5HackCHRMode=0;
 MMC5HackSPMode=MMC5HackSPScroll=MMC5HackSPPage=0;


 Mapper5_ESI(&info->CartExpSound);

 SetWriteHandler(0x4020,0x5bff,Mapper5_write);
 SetReadHandler(0x4020,0x5bff,MMC5_read);

 SetWriteHandler(0x5c00,0x5fff,MMC5_ExRAMWr);
 SetReadHandler(0x5c00,0x5fff,MMC5_ExRAMRd);

 SetWriteHandler(0x6000,0xFFFF,MMC5_WriteROMRAM);
 SetReadHandler(0x6000,0xFFFF,MMC5_ReadROMRAM);

 SetWriteHandler(0x5000,0x5015,Mapper5_SW);
 SetWriteHandler(0x5205,0x5206,Mapper5_write);
 SetReadHandler(0x5205,0x5206,MMC5_read);

 MDFNMP_AddRAM(8192, 0x6000, WRAM);
 MDFNMP_AddRAM(1024, 0x5c00, ExRAM);

 return(1);
}

int Mapper5_Init(CartInfo *info)
{
 return(GenMMC5_Init(info, DetectMMC5WRAMSize(info->CRC32), info->battery));
}

// ELROM seems to have 0KB of WRAM
// EKROM seems to have 8KB of WRAM
// ETROM seems to have 16KB of WRAM
// EWROM seems to have 32KB of WRAM

// ETROM and EWROM are battery-backed, EKROM isn't.

int ETROM_Init(CartInfo *info)
{
 return(GenMMC5_Init(info, 16,info->battery));
}

int ELROM_Init(CartInfo *info)
{
 return(GenMMC5_Init(info,0,0));
}

int EWROM_Init(CartInfo *info)
{
 return(GenMMC5_Init(info,32,info->battery));
}

int EKROM_Init(CartInfo *info)
{
 return(GenMMC5_Init(info,8,info->battery));
}

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

#include        "share.h"

typedef struct {
        uint32 mzx,mzy,mzb;
        int zap_readbit;
        int bogo;
        int zappo;
        uint64 zaphit;
} ZAPPER;

static ZAPPER ZD;

static void ZapperFrapper(uint8 *bg, uint8 *spr, uint32  linets, int final)
{
 int xs,xe;   
 int zx,zy;
 
 xs=linets; 
 xe=final;

 zx=ZD.mzx;
 zy=ZD.mzy;
 
 if(xe>256) xe=256;
 
 if(scanline>=(zy-4) && scanline<=(zy+4))
 {
  while(xs<xe)
  {
    uint8 a1,a2;
    uint32 sum;
    if(xs<=(zx+4) && xs>=(zx-4))
    {
     a1=bg[xs];
     if(spr)
     {
      a2=spr[xs];  
 
      if(!(a2&0x80))
       if(!(a2&0x40) || (a1&64))
        a1=a2; 
     }
     a1&=63;
 
     sum=ActiveNESPalette[a1].r+ActiveNESPalette[a1].g+ActiveNESPalette[a1].b;
#ifndef WII
     if(sum>=100*3)
#else
     if(sum>=220*3)
#endif
     {
      ZD.zaphit=timestampbase + timestamp;
      goto endo;
     }
    }
   xs++;
  }  
 }
 endo:
 ZD.zappo=final;
}

static INLINE int CheckColor(void)
{ 
 MDFNPPU_LineUpdate();
 
 if((ZD.zaphit+10)>=(timestampbase+timestamp)) return(0);
 
 return(1);   
}


static uint8 ReadZapper(int w, uint8 ret)
{
		if(w)
		{
		 ret&=~0x18;
                 if(ZD.bogo)
                  ret|=0x10;
                 if(CheckColor())
                  ret|=0x8;
		}
		else
		{
		 //printf("Kayo: %d\n",ZD.zap_readbit);
		 ret&=~2;
		 //if(ZD.zap_readbit==4) ret|=ZD.mzb&2;
		 ret|=(ret&1)<<1;
		 //ZD.zap_readbit++;
		}
                return ret;
}

static void DrawZapper(MDFN_Surface *surface)
{
  MDFN_DrawGunSight(surface, ZD.mzx, ZD.mzy);
}

static void UpdateZapper(void *data)
{
  uint8 *ptr=(uint8*)data;

  if(ZD.bogo)
   ZD.bogo--;

  if(ptr[8]&7 && (!(ZD.mzb&7)))
   ZD.bogo=5;

  ZD.mzx=(uint32)MDFN_de32lsb(ptr + 0);
  ZD.mzy=(uint32)MDFN_de32lsb(ptr + 4);
  ZD.mzb=ptr[8];

  NESPPU_TranslateMouseXY(ZD.mzx, ZD.mzy);
}

static void StrobeShadow(void)
{
 ZD.zap_readbit=0;
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(ZD.mzx),
  SFVAR(ZD.mzy),
  SFVAR(ZD.mzb),
  SFVAR(ZD.bogo),
  SFVAR(ZD.zaphit),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}

static INPUTCFC SHADOWC={ReadZapper,0,StrobeShadow,UpdateZapper,ZapperFrapper, DrawZapper, StateActionFC, 3, sizeof(uint32) };

INPUTCFC *MDFN_InitSpaceShadow(void)
{
  memset(&ZD,0,sizeof(ZAPPER));
  return(&SHADOWC);
}



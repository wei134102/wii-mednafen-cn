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

#include	"share.h"

typedef struct {
	uint32 mzx,mzb;
	uint32 readbit;
} ARK;

static ARK NESArk[2];
static ARK FCArk;

static void StrobeARKFC(void)
{
	FCArk.readbit=0;
}


static uint8 ReadARKFC(int w,uint8 ret)
{
 ret&=~2;

 if(w)  
 {
  if(FCArk.readbit>=8) 
   ret|=2;
  else
  {
   ret|=((FCArk.mzx>>(7-FCArk.readbit))&1)<<1;
   if(!fceuindbg)
    FCArk.readbit++;
  }
 }
 else
  ret|=FCArk.mzb<<1;
 return(ret);
}

static uint32 FixX(uint32 in_x)
{
 uint32 dummy = 0;
 int32 x;

 NESPPU_TranslateMouseXY(in_x, dummy);

 x = (int32)in_x + 98 - 32;


#ifdef WII
 if(x < 98 - 32)
  x = 98 - 32;
#else
 if(x < 98)
  x = 98;
#endif

 if(x > 242)
  x = 242;

 x=~x;
 return(x);
}

static void UpdateARKFC(void *data)
{
#ifndef WII
 uint32 *ptr=(uint32 *)data;
 FCArk.mzx=FixX(ptr[0]);
 FCArk.mzb=ptr[2]?1:0;
#else
 uint8 *ptr = (uint8*)data;
 FCArk.mzx=FixX(MDFN_de32lsb(ptr + 0));
 FCArk.mzb=ptr[4]?1:0;
#endif
}

static int StateAction(int w, StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(NESArk[w].mzx),
   SFVAR(NESArk[w].mzb),
   SFVAR(NESArk[w].readbit),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, w ? "INP1" : "INP0");
 if(load)
 {

 }
 return(ret);
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(FCArk.mzx),
   SFVAR(FCArk.mzb),
   SFVAR(FCArk.readbit),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}


static INPUTCFC ARKCFC={ReadARKFC,0,StrobeARKFC,UpdateARKFC,0,0, StateActionFC, 3, sizeof(uint32) };

INPUTCFC *MDFN_InitArkanoidFC(void)
{
 FCArk.mzx=98;
 FCArk.mzb=0;
 return(&ARKCFC);
}

static uint8 ReadARK(int w)
{
 uint8 ret=0;

 if(NESArk[w].readbit>=8)
  ret|=1<<4;
 else
 {
  ret|=((NESArk[w].mzx>>(7-NESArk[w].readbit))&1)<<4;
  if(!fceuindbg)
   NESArk[w].readbit++;
 }
 ret|=(NESArk[w].mzb&1)<<3;
 return(ret);
}


static void StrobeARK(int w)
{
	NESArk[w].readbit=0;
}

static void UpdateARK(int w, void *data)
{
 uint8 *ptr = (uint8*)data;

 NESArk[w].mzx=FixX(MDFN_de32lsb(ptr + 0));
 NESArk[w].mzb=ptr[4]?1:0;
}

static INPUTC ARKC={ReadARK, 0, StrobeARK, UpdateARK, 0, 0, StateAction, 3, sizeof(uint32) };

INPUTC *MDFN_InitArkanoid(int w)
{
 NESArk[w].mzx=98;
 NESArk[w].mzb=0;
 return(&ARKC);
}

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

#include "mapinc.h"

static uint8 latch;
static void Sync(void)
{
 setchr8((latch&3)|((latch>>4)&4));
 setprg32(0x8000, (latch >> 4) & 3);
}

static DECLFW(Mapper86_write)
{
 if(A>=0x6000 && A<=0x6fFF)
 {
  latch = V;
  Sync();
 }
}

static void Power(CartInfo *info)
{
 latch = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latch), SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper86_Init(CartInfo *info)
{
 info->StateAction = StateAction;
 info->Power = Power;
 SetWriteHandler(0x6000,0x6fff,Mapper86_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}

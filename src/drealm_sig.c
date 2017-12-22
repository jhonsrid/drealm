#if 0
drealmBBS - Bulletin Board System for Linux
Copyright (C) 1994, 1995, 1996  Inge Cubitt and Peter Jones

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

The GNU General Public License should be in a file called COPYING.
#endif
#include "drealm_sig.h"
void (*Dsigset(int signum, void (*handler)(int)))(int) {
	struct sigaction sa,sb;
	sa.sa_handler = handler;
#if defined(SVR42)
	sa.sa_mask.sa_sigbits[0] = 0;
	sa.sa_mask.sa_sigbits[1] = 0;
	sa.sa_mask.sa_sigbits[2] = 0;
	sa.sa_mask.sa_sigbits[3] = 0;
#else
	sa.sa_mask = 0;
	sa.sa_restorer = 0;
#endif
	sa.sa_flags = 0;

	if (sigaction(signum,&sa,&sb)) {
		return sb.sa_handler;
	} else {
		return SIG_ERR;
	}
}

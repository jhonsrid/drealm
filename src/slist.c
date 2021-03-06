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
#include <stdlib.h>
#include <string.h>
#include "slist.h"

void push_slist(struct slist **const list, const char *v) {
	struct slist *m;

	m = (struct slist *)malloc(sizeof (struct slist));
	m->this = strdup(v);
	m->next = 0;

	if (*list) {
		struct slist *l = *list;
		while(l->next) l = l->next;
		l->next = m;
		m->prev = l;
	} else {
		m->prev = 0;
		*list = m;
	}
}

struct slist *pop_slist(struct slist **const list, struct slist *const l) {
	if (*list) {
		struct slist *k = l->prev;
		struct slist *m = l->next;

		if (k) k->next = m;
		if (m) m->prev = k;
		if (*list == l) *list = m;
		free(l->this);
		free(l);
		return m;
	} else {
		return 0;
	}
	/* NOTREACHED */
}

void free_slist(struct slist **const list) {
	struct slist *l = *list;
	while(l) l = pop_slist(list, l);
}

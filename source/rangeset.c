/* $Id: rangeset.c,v 1.3 2002/12/08 09:29:40 yooden Exp $ */
/*******************************************************************************
*									       *
* rangeset.c	 -- Nirvana Editor rangest functions			       *
*									       *
* Copyright (C) 1999 Mark Edel						       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.								       *
*									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	       *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License	       *
* for more details.							       *
*									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA	02111-1307 USA				       *
*									       *
* Nirvana Text Editor							       *
* Sep 26, 2002								       *
*									       *
* Written by Tony Balinski with contributions from Andrew Hood		       *
*									       *
* Modifications:							       *
*									       *
*									       *
*******************************************************************************/
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "textBuf.h"
#include "textDisp.h"
#include "rangeset_fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_DEBUG_H
#include "../debug.h"
#endif

/* -------------------------------------------------------------------------- */

#define N_RANGESETS	26

struct _Range {
    int start, end;			/* range from [start-]end */
};

typedef Rangeset *RangesetUpdateFn(Rangeset *p, int pos, int ins, int del);

struct _Rangeset {
    RangesetUpdateFn *update_fn;	/* modification update function */
    char *update_name;			/* update function name */
    int maxpos;				/* text buffer maxpos */
    int last_index;			/* a place to start looking */
    int n_ranges;			/* how many ranges in ranges */
    Range *ranges;			/* the ranges table */
    char label;				/* a letter A-Z */

    int color_set;			/* 0: unset; 1: set; -1: invalid */
    char *color_name;			/* the name of an assigned color */
    Pixel color;			/* the value of a particular color */
    textBuffer *buf;			/* the text buffer of the rangeset */
};

struct _RangesetTable {
    int n_set;				/* how many sets are active */
    int macro_last_set;			/* last set accessed by macro call */
    int macro_last_range;		/* index of range found in last set */
    int macro_last_start;		/* start of range in last set used */
    int macro_last_end;			/* end of range in last set used */
    int macro_last_n;			/* number of ranges in last set */
    char *macro_last_color;		/* color name in last range set */
    char *macro_last_modify;		/* modify name for last range */
    textBuffer *buf;			/* the text buffer of the rangeset */
    Rangeset set[N_RANGESETS];		/* the rangeset table */
    char order[N_RANGESETS];		/* inds of set[]s ordered by depth */
    char active[N_RANGESETS];		/* entry true if corresp. set active */
    char depth[N_RANGESETS];		/* depth[i]: pos of set[i] in order[] */
    char list[N_RANGESETS + 1];		/* string of labels in depth order */
};

/* -------------------------------------------------------------------------- */

#define SWAPval(T,a,b) { T t; t = *(a); *(a) = *(b); *(b) = t; }

static char rangeset_labels[N_RANGESETS + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* -------------------------------------------------------------------------- */

static RangesetUpdateFn rangesetInsDelMaintain;
static RangesetUpdateFn rangesetInclMaintain;
static RangesetUpdateFn rangesetDelInsMaintain;
static RangesetUpdateFn rangesetExclMaintain;
static RangesetUpdateFn rangesetBreakMaintain;

#define DEFAULT_UPDATE_FN_NAME	"maintain"

static struct {
    char *name;
    RangesetUpdateFn *update_fn;
} RangesetUpdateMap[] = {
    {DEFAULT_UPDATE_FN_NAME,	rangesetInsDelMaintain},
    {"ins_del",			rangesetInsDelMaintain},
    {"include",			rangesetInclMaintain},
    {"del_ins",			rangesetDelInsMaintain},
    {"exclude",			rangesetExclMaintain},
    {"break",			rangesetBreakMaintain},
    {(char *)0,			(RangesetUpdateFn *)0}
};

/* -------------------------------------------------------------------------- */

static Range *RangesNew(int n)
{
    int sz;
    Range *new_p;

    if (n) {
	/* We use a blocked allocation scheme here, with a block size of factor.
	   Only allocations of multiples of factor will be allowed.
	   Be sure to allocate at least one more than we really need, and
	   round up to next higher multiple of factor, ie
		n = (((n + 1) + factor - 1) / factor) * factor
	   If we choose factor = (1 << factor_bits), we can use shifts
	   instead of multiply/divide, ie
	        n = ((n + (1 << factor_bits)) >> factor_bits) << factor_bits
	   or
		n = (1 + (n >> factor_bits)) << factor_bits
	   Since the shifts just strip the end 1 bits, we can even get away
	   with
	   	n = ((1 << factor_bits) + n) & (~0 << factor_bits);
	   Finally, we decide on factor_bits according to the size of n:
	   if n >= 256, we probably want less reallocation on growth than
	   otherwise; choose some arbitrary values thus:
		factor_bits = (n >= 256) ? 6 : 4
	   so
	   	n = (n >= 256) ? (n + (1<<6)) & (~0<<6) : (n + (1<<4)) & (~0<<4)
	   or
	   	n = (n >= 256) ? ((n + 64) & ~63) : ((n + 16) & ~15)
	 */
	n = (n >= 256) ? ((n + 64) & ~63) : ((n + 16) & ~15);
	sz = n * sizeof (Range);
	new_p = (Range *)XtMalloc(sz);
	return new_p;
    }

    return (Range *)0;
}

/* -------------------------------------------------------------------------- */

static Range* RangesRealloc(Range* range, int n)
{
    int size;
    Range* newRange;

    if (n > 0)
    {
        /* see RangesNew() for comments */
        n = (n >= 256) ? ((n + 64) & ~63) : ((n + 16) & ~15);
        size = n * sizeof (Range);
        newRange = (Range*) (range != NULL
                ? XtRealloc((char *)range, size)
                : XtMalloc(size));
        return newRange;
    } else if (range != NULL)
    {
        XtFree((char*) range);
    }

    return (Range*) 0;
}

/* -------------------------------------------------------------------------- */

static Range *RangesFree(Range *p)
{
    if (p)
	XtFree((char *)p);

    return (Range *)0;
}

/* -------------------------------------------------------------------------- */

/*
** Refresh the given range on the screen. If the range indicated is null, we
** refresh the screen for the whole file.
*/

void RangesetRefreshRange(Rangeset *p, int start, int end)
{
    if (p->buf)
	BufCheckDisplay(p->buf, start, end);
}

void RangesetRefreshAllRanges(Rangeset *p)
{
    int i;

    for (i = 0; i < p->n_ranges; i++)
	RangesetRefreshRange(p, p->ranges[i].start, p->ranges[i].end);
}

/* -------------------------------------------------------------------------- */

/*
** Remove all ranges from a range set.
*/

void RangesetEmpty(Rangeset *r)
{
    Range *p_r = r->ranges;
    int start, end;

    if (r->color_name && r->color_set > 0) {
	/* this range is colored: we need to clear it */
	r->color_set = -1;

	while (r->n_ranges--) {
	    start = p_r[r->n_ranges].start;
	    end = p_r[r->n_ranges].end;
	    RangesetRefreshRange(r, start, end);
	}
    }

    if (r->color_name)
	XtFree(r->color_name);

    r->color_name = (char *)0;
    r->ranges = RangesFree(r->ranges);
}

/* -------------------------------------------------------------------------- */

/*
** Initialise a new range set.
*/

void RangesetInit(Rangeset *r, char label, textBuffer *buf)
{
    r->label = label;			/* a letter A-Z */
    r->maxpos = 0;			/* text buffer maxpos */
    r->last_index = 0;			/* a place to start looking */
    r->n_ranges = 0;			/* how many ranges in ranges */
    r->ranges = (Range *)0;		/* the ranges table */

    r->color_name = (char *)0;
    r->color_set = 0;
    r->buf = buf;

    r->maxpos = buf->gapEnd - buf->gapStart + buf->length;

    RangesetChangeModifyResponse(r, DEFAULT_UPDATE_FN_NAME);
}

/*
** Change a range set's modification behaviour. Returns true (non-zero)
** if the update function name was found, else false.
*/

int RangesetChangeModifyResponse(Rangeset *r, char *name)
{
    int i;

    if (!name)
	name = DEFAULT_UPDATE_FN_NAME;

    for (i = 0; RangesetUpdateMap[i].name; i++)
	if (strcmp(RangesetUpdateMap[i].name, name) == 0) {
	    r->update_fn = RangesetUpdateMap[i].update_fn;
	    r->update_name = RangesetUpdateMap[i].name;
	    return 1;
	}

    return 0;
}

/* -------------------------------------------------------------------------- */

/*
** Find the index of the first integer in tab greater than or equal to pos.
** Fails with len (the total number of entries). The start index base can be
** chosen appropriately.
*/

static int at_or_before(int *tab, int base, int len, int val)
{
    int lo, mi=0, hi;

    if (base >= len)
	return len;		/* not sure what this means! */

    lo = base;			/* first valid index */
    hi = len - 1;		/* last valid index */

    while (lo <= hi) {
	mi = (lo + hi) / 2;
	if (val == tab[mi])
	    return mi;
	if (val < tab[mi])
	    hi = mi - 1;
	else
	    lo = mi + 1;
    }
    /* if we get here, we didn't find val itself */
    if (val > tab[mi])
	mi++;

    return mi;
}

static int weighted_at_or_before(int *tab, int base, int len, int val)
{
    int lo, mi=0, hi;
    int min, max;

    if (base >= len)
	return len;		/* not sure what this means! */

    lo = base;			/* first valid index */
    hi = len - 1;		/* last valid index */

    min = tab[lo];		/* establish initial min/max */
    max = tab[hi];

    if (val <= min)		/* initial range checks */
	return lo;		/* needed to avoid out-of-range mi values */
    else if (val > max)
	return len;
    else if (val == max)
	return hi;

    while (lo <= hi) {
	mi = lo + (hi - lo) * (max - val) / (max - min);
	/* we won't worry about min == max - values should be unique */

	if (val == tab[mi])
	    return mi;
	if (val < tab[mi]) {
	    hi = mi - 1;
	    max = tab[mi];
	}
	else { /* val > tab[mi] */
	    lo = mi + 1;
	    min = tab[mi];
	}
    }

    /* if we get here, we didn't find val itself */
    if (val > tab[mi])
	return mi + 1;

    return mi;
}

/* -------------------------------------------------------------------------- */

/*
** Find out whether the position pos is included in one of the ranges of the
** rangeset p. Returns the containing range's index if true, -1 otherwise.
*/

int RangesetFindRangeNo(Rangeset *p, int ind, int *start, int *end)
{
    if (!p || ind < 0 || p->n_ranges <= ind || !p->ranges)
	return 0;

    *start = p->ranges[ind].start;
    *end   = p->ranges[ind].end;

    return 1;
}

/*
** Find out whether the position pos is included in one of the ranges of the
** rangeset p. Returns the containing range's index if true, -1 otherwise.
*/

int RangesetFindRangeOfPos(Rangeset *p, int pos, int incl_end)
{
    int *tab;
    int len, ind;

    if (!p || !p->n_ranges || !p->ranges)
	return -1;

    tab = (int *)p->ranges;		/* { s1,e1, s2,e2, s3,e3,... } */
    len = p->n_ranges * 2;
    ind = at_or_before(tab, 0, len, pos);

    if (ind == len)
	return -1;			/* beyond end */

    if (ind & 1) {			/* ind odd: references an end marker */
	if (pos < tab[ind] || (incl_end && pos == tab[ind]))
	    return ind / 2;		/* return the range index */
    }
    else {				/* ind even: references start marker */
	if (pos == tab[ind])
	    return ind / 2;		/* return the range index */
    }
    return -1;				/* not in any range */
}

/*
** Find out whether the position pos is included in one of the ranges of the
** rangeset p. Returns the containing range's index if true, -1 otherwise.
** Essentially the same as the RangesetFindRangeOfPos() function, but uses the
** last_index member of the rangeset and weighted_at_or_before() for speedy
** lookup in refresh tasks. The rangeset is assumed to be valid, as is the
** position. We also don't allow checking of the endpoint.
** Returns the including range index, or -1 if not found.
*/

int RangesetCheckRangeOfPos(Rangeset *p, int pos)
{
    int *tab;
    int len, ind, last;

    len = p->n_ranges;
    if (len == 0)
	return -1;			/* no ranges */

    tab = (int *)p->ranges;		/* { s1,e1, s2,e2, s3,e3,... } */
    last = p->last_index;

    /* try to profit from the last lookup by using its index */
    if (last >= len || last < 0) {
	last = (len > 0) ? len - 1 : 0;	/* make sure last is in range */
	p->last_index = last;
    }

    len *= 2;
    last *= 2;

    if (pos >= tab[last]) {		/* last even: this is a start */
	if (pos < tab[last + 1])	/* checking an end here */
	    return last / 2;		/* no need to change p->last_index */
	else
	    last += 2;			/* not in this range: move on */

	if (last == len)
	    return -1;			/* moved on too far */

	/* find the entry in the upper portion of tab */
	ind = weighted_at_or_before(tab, last, len, pos); /* search end only */
    }
    else if (last > 0) {
	ind = weighted_at_or_before(tab, 0, last, pos); /* search front only */
    }
    else
	ind = 0;

    p->last_index = ind / 2;

    if (ind == len)
	return -1;			/* beyond end */

    if (ind & 1) {			/* ind odd: references an end marker */
	if (pos < tab[ind])
	    return ind / 2;		/* return the range index */
    }
    else {				/* ind even: references start marker */
	if (pos == tab[ind])
	    return ind / 2;		/* return the range index */
    }
    return -1;				/* not in any range */
}

/* -------------------------------------------------------------------------- */

/*
** Merge the ranges in rangeset q into rangeset p.
*/

int RangesetAdd(Rangeset *p, Rangeset *q)
{
    Range *p_p, *p_q, *p_r, *p_old;
    int n_p, n_q;
    int is_old;

    p_p = p->ranges;
    n_p = p->n_ranges;

    p_q = q->ranges;
    n_q = q->n_ranges;

    if (n_q == 0)
	return n_p;			/* no ranges in q - nothing to do */

    p_r = RangesNew(n_p + n_q);

    if (n_p == 0) {
	/* no ranges in destination: just copy the ranges from the other set */
	memcpy(p_r, p_q, n_q * sizeof (Range));
	RangesFree(p->ranges);
	p->ranges = p_r;
	p->n_ranges = n_q;
	for (n_p = 0; n_p < n_q; n_p++) {
	    RangesetRefreshRange(p, p_r->start, p_r->end);
	    p_r++;
	}
	return n_q;
    }

    p_old = p_p;
    p->ranges = p_r;
    p->n_ranges = 0;

    /* in the following we merrily swap the pointers/counters of the two input
       ranges (from p and q) - don't worry, they're both consulted read-only -
       building the merged set in p_r */

    is_old = 1;		/* true if p_p points to a range in p_old[] */

    while (n_p > 0 || n_q > 0) {
	/* make the range with the lowest start value the p_p range */
	if (n_p == 0 || (n_q > 0 && p_p->start > p_q->start)) {
	    SWAPval(Range *, &p_p, &p_q);
	    SWAPval(int, &n_p, &n_q);
	    is_old = !is_old;
	}

	p->n_ranges++;			/* we're using a new result range */

	*p_r = *p_p++;
	n_p--;
	if (!is_old)
	    RangesetRefreshRange(p, p_r->start, p_r->end);

	/* now we must cycle over p_q, merging in the overlapped ranges */
	while (n_q > 0 && p_r->end >= p_q->start) {
	    do {
		if (p_r->end < p_q->end) {
		    if (is_old)
			RangesetRefreshRange(p, p_r->end, p_q->end);
		    p_r->end = p_q->end;
		}
		p_q++;
		n_q--;
	    } while (n_q > 0 && p_r->end >= p_q->start);

	    /* by now, p_r->end may have extended to overlap more ranges in p_p,
	       so swap and start again */
	    SWAPval(Range *, &p_p, &p_q);
	    SWAPval(int, &n_p, &n_q);
	    is_old = !is_old;
	}

	/* OK: now *p_r holds the result of merging all the first ranges from
	   p_p and p_q - now we have a break in contiguity, so move on to the
	   next p_r in the result */
	p_r++;
    }

    /* finally, forget the old rangeset values, and reallocate the new ones */
    RangesFree(p_old);
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    return p->n_ranges;
}

/* -------------------------------------------------------------------------- */

/*
** Subtract the ranges of q from rangeset p.
*/

int RangesetRemove(Rangeset *p, Rangeset *q)
{
    Range *p_p, *p_q, *p_r, *p_old;
    int n_p, n_q;

    p_p = p->ranges;
    n_p = p->n_ranges;

    p_q = q->ranges;
    n_q = q->n_ranges;

    if (n_p == 0 || n_q == 0)
	return 0;		/* no ranges in p or q - nothing to do */

    /* we must provide more space: each range in q might split a range in p */
    p_r = RangesNew(p->n_ranges + q->n_ranges);

    p_old = p_p;
    p->ranges = p_r;
    p->n_ranges = 0;

    /* consider each range in p_p - we do not change any of p_q's data, but we
       may change p_p's - it will be discarded at the end */

    while (n_p > 0) {
	do {
	    /* skip all p_q ranges strictly in front of *p_p */
	    while (n_q > 0 && p_q->end <= p_p->start) {
		p_q++;		/* *p_q in front of *p_p: move onto next *p_q */
		n_q--;
	    }

	    if (n_q > 0) {
		/* keep all p_p ranges strictly in front of *p_q */
		while (n_p > 0 && p_p->end <= p_q->start) {
		    *p_r++ = *p_p++;   /* *p_q beyond *p_p: save *p_p in *p_r */
		    n_p--;
		    p->n_ranges++;
		}
	    }
	    else {
		/* no more p_q ranges to remove - save the rest of p_p */
		while (n_p > 0) {
		    *p_r++ = *p_p++;
		    n_p--;
		    p->n_ranges++;
	      }
	    }
	} while (n_q > 0 && p_q->end <= p_p->start); /* any more non-overlaps */

	/* when we get here either we're done, or we have overlap */
	if (n_p > 0) {
	    if (p_q->start <= p_p->start) {
		/* p_p->start inside *p_q */
		if (p_q->end < p_p->end) {
		    RangesetRefreshRange(p, p_p->start, p_q->end);
		    p_p->start = p_q->end;  /* cut off front of original *p_p */
		    p_q++;		    /* dealt with this *p_q: move on */
		    n_q--;
		}
		else {
		    /* all *p_p inside *p_q */
		    RangesetRefreshRange(p, p_p->start, p_p->end);
		    p_p++;			/* all of *p_p can be skipped */
		    n_p--;
		}
	    }
	    else {
		/* p_q->start inside *p_p: save front, adjust or skip rest */
		p_r->start = p_p->start;	/* save front of *p_p in *p_r */
		p_r->end = p_q->start;
		p_r++;
		p->n_ranges++;

		if (p_q->end < p_p->end) {
		    /* all *p_q inside *p_p */
		    RangesetRefreshRange(p, p_q->start, p_q->end); 
		    p_p->start = p_q->end; /* cut front of *p_p upto end *p_q */
		    p_q++;		   /* dealt with this *p_q: move on */
		    n_q--;
		}
		else {
		    /* p_q->end beyond *p_p */
		    RangesetRefreshRange(p, p_q->start, p_p->end); 
		    p_p++;			/* skip rest of *p_p */
		    n_p--;
		}
	    }
	}
    }

    /* finally, forget the old rangeset values, and reallocate the new ones */
    RangesFree(p_old);
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    return p->n_ranges;
}

/* -------------------------------------------------------------------------- */

/*
** Get number of ranges in rangeset p.
*/

int RangesetGetNRanges(Rangeset *p)
{
    return p ? p->n_ranges : 0;
}

/* -------------------------------------------------------------------------- */

/*
** Set maxpos for rangeset p.
*/

void RangesetSetMaxpos(Rangeset *p, int maxpos)
{
    if (p)
	p->maxpos = maxpos;
}

/* -------------------------------------------------------------------------- */

static Rangeset *RangesetFixMaxpos(Rangeset *p, int pos, int ins, int del)
{
    p->maxpos += ins - del;
    return p;
}

/* -------------------------------------------------------------------------- */

/*
** Allocate and initialise, or empty and free a ranges set table.
*/

RangesetTable *RangesetTableAlloc(textBuffer *buf)
{
    int i;
    RangesetTable *tab = (RangesetTable *)XtMalloc(sizeof (RangesetTable));

    if (!tab)
	return tab;

    tab->buf = buf;

    for (i = 0; i < N_RANGESETS; i++) {
	RangesetInit(&tab->set[i], rangeset_labels[i], buf);
	tab->order[i] = i;
	tab->active[i] = 0;
	tab->depth[i] = i;
    }

    tab->n_set = 0;
    return tab;
}

RangesetTable *RangesetTableFree(RangesetTable *tab)
{
    int i;

    for (i = 0; i < N_RANGESETS; i++)
	RangesetEmpty(&tab->set[i]);
    if (tab)
	XtFree((char *)tab);
    return (RangesetTable *)0;
}

/*
** Find a range set given its label in the table.
*/

int RangesetFindIndex(RangesetTable *tab, char label, int must_be_active)
{
    int i;
    char *p_label;

    if (tab) {
	p_label = strchr(rangeset_labels, toupper(label));
	if (p_label) {
	    i = p_label - rangeset_labels;
	    if (!must_be_active || tab->active[i])
		return i;
	}
    }

    return -1;
}

/*
** Assign the range set table list.
*/

static void RangesetTableListSet(RangesetTable *tab)
{
    int i;

    for (i = 0; i < tab->n_set; i++)
	tab->list[i] = rangeset_labels[(int)tab->order[i]];
    tab->list[tab->n_set] = '\0';
}

/*
** Return true if label is a valid identifier for a range set.
*/

int RangesetLabelOK(char label)
{
    return strchr(rangeset_labels, toupper(label)) != (char *)0;
}

/*
** Helper routines for managing the order and depth tables.
*/

static int activateRangeset(RangesetTable *tab, int active)
{
    int depth, i, j;

    if (tab->active[active])
	return 0;			/* already active */

    depth = tab->depth[active];

    /* we want to make the "active" set the most recent (lowest depth value):
       shuffle tab->order[0..depth-1] to tab->order[1..depth]
       readjust the entries in tab->depth[] accordingly */
    for (i = depth; i > 0; i--) {
	j = tab->order[i] = tab->order[i - 1];
	tab->depth[j] = i;
    }
    /* insert the new one: first in order, of depth 0 */
    tab->order[0] = active;
    tab->depth[active] = 0;

    /* and finally... */
    tab->active[active] = 1;
    tab->n_set++;

    RangesetTableListSet(tab);

    return 1;
}

static int deactivateRangeset(RangesetTable *tab, int active)
{
    int depth, n, i, j;

    if (!tab->active[active])
	return 0;			/* already inactive */

    /* we want to start by swapping the deepest entry in order with active
       shuffle tab->order[depth+1..n_set-1] to tab->order[depth..n_set-2]
       readjust the entries in tab->depth[] accordingly */
    depth = tab->depth[active];
    n = tab->n_set - 1;

    for (i = depth; i < n; i++) {
	j = tab->order[i] = tab->order[i + 1];
	tab->depth[j] = i;
    }
    /* reinsert the old one: at max (active) depth */
    tab->order[n] = active;
    tab->depth[active] = n;

    /* and finally... */
    tab->active[active] = 0;
    tab->n_set--;

    RangesetTableListSet(tab);

    return 1;
}

/*
** Forget the rangeset identified by label - clears it, renders it inactive.
*/

Rangeset *RangesetForget(RangesetTable *tab, char label)
{
    int set_ind = RangesetFindIndex(tab, label, 1);

    if (set_ind < 0)
	return (Rangeset *)0;

    if (deactivateRangeset(tab, set_ind))
	RangesetEmpty(&tab->set[set_ind]);

    return &tab->set[set_ind];
}

/*
** Fetch the rangeset identified by label - initialise it if not active and
** make_active is true, and make it the most visible.
*/

Rangeset *RangesetFetch(RangesetTable *tab, char label, int make_active)
{
    int set_ind = RangesetFindIndex(tab, label, 0);

    if (set_ind < 0)
	return (Rangeset *)0;

    if (tab->active[set_ind])
	return &tab->set[set_ind];

    if (!make_active)
	return (Rangeset *)0;

    if (activateRangeset(tab, set_ind))
	RangesetInit(&tab->set[set_ind], rangeset_labels[set_ind], tab->buf);

    return &tab->set[set_ind];
}

/*
** Assign values to the macro variable fields of the table, report them.
*/

void RangesetTableClearMacroVars(RangesetTable *tab)
{
    tab->macro_last_set    = -1;
    tab->macro_last_range  = -1;
    tab->macro_last_start  = -1;
    tab->macro_last_end    = -1;
    tab->macro_last_n	   = 0;
    tab->macro_last_color  = (char *)0;
    tab->macro_last_modify = (char *)0;
}

void RangesetTableAssignMacroVars(RangesetTable *tab, Rangeset *p, int range)
{
    int set;

    RangesetTableClearMacroVars(tab);

    if (!p || p < tab->set || p >= tab->set + N_RANGESETS)
	set = -1;			/* the rangeset is not in the table */
    else
	set = p - tab->set;

    if (set >= 0 && tab->active[set]) {
	tab->macro_last_set = set;
	tab->macro_last_n = p->n_ranges;
	if (p->color_set >= 0)
	    tab->macro_last_color = p->color_name;

	if (RangesetFindRangeNo(p, range,
	    &tab->macro_last_start, &tab->macro_last_end))
	    tab->macro_last_range = range;

	tab->macro_last_modify = p->update_name;
    }
}

char *RangesetTableGetMacroSetLabel(RangesetTable *tab)
{
    static char label_string[2] = { '\0', '\0' };
    if (tab && tab->macro_last_set >= 0)
	label_string[0] = rangeset_labels[tab->macro_last_set];
    else
	label_string[0] = '\0';
    return label_string;
}

int RangesetTableGetMacroRangeIndex(RangesetTable *tab)
{
    return tab ? tab->macro_last_range : -1;
}

int RangesetTableGetMacroRangeStart(RangesetTable *tab)
{
    return tab ? tab->macro_last_start : -1;
}

int RangesetTableGetMacroRangeEnd(RangesetTable *tab)
{
    return tab ? tab->macro_last_end : -1;
}

int RangesetTableGetMacroRangeN(RangesetTable *tab)
{
    return tab ? tab->macro_last_n : -1;
}

char *RangesetTableGetMacroRangeList(RangesetTable *tab)
{
    return tab ? tab->list : "";
}

char *RangesetTableGetMacroRangeColor(RangesetTable *tab)
{
    return (tab && tab->macro_last_color) ? tab->macro_last_color : "";
}

char *RangesetTableGetMacroRangeMod(RangesetTable *tab)
{
    return (tab && tab->macro_last_modify) ? tab->macro_last_modify : "";
}

/* -------------------------------------------------------------------------- */

void RangesetTableUpdatePos(RangesetTable *tab, int pos, int ins, int del)
{
    int i;
    Rangeset *p;

    if (!tab || (ins == 0 && del == 0))
	return;

    for (i = 0; i < tab->n_set; i++) {
	p = &tab->set[(int)tab->order[i]];
	p->update_fn(p, pos, ins, del);
    }
}

void RangesetBufModifiedCB(int pos, int nInserted, int nDeleted, int nRestyled,
	char *deletedText, void *cbArg)
{
    textBuffer *buf = (textBuffer *)cbArg;

    RangesetTableUpdatePos(buf->rangesetTable, pos,
			   nInserted, nDeleted);
}

/* -------------------------------------------------------------------------- */

/*
** Find the index of the first range in depth order which includes the position.
** Returns the index of the rangeset in the rangeset table + 1 if an including
** rangeset was found, 0 otherwise. If needs_color is true, "colorless" ranges
** will be skipped.
*/

int RangesetIndex1ofPos(RangesetTable *tab, int pos, int needs_color)
{
    int i;
    Rangeset *p;

    if (!tab)
	return 0;

    for (i = 0; i < tab->n_set; i++) {
	p = &tab->set[(int)tab->order[i]];
	if (RangesetCheckRangeOfPos(p, pos) >= 0) {
	    if (needs_color && p->color_set >= 0 && p->color_name)
		return tab->order[i] + 1;
	}
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

/*
** Assign a color name to a rangeset via the rangeset table.
*/

int RangesetAssignColorName(Rangeset *p, char *color_name)
{
    char *cp;
    int refresh = 0;

    if (color_name && color_name[0] == '\0')
	color_name = (char *)0;				/* "" invalid */

    if (p->color_name && color_name &&
	strcmp(p->color_name, color_name) != 0)
	refresh = 1;					/* different */
    else if (!p->color_name ^ !color_name)
	refresh = 1;					/* one is NULL */

    /* store new color name value */
    if (color_name) {
	cp = XtMalloc(strlen(color_name) + 1);
	strcpy(cp, color_name);
    }
    else
	cp = color_name;

    /* free old color name value */
    if (p->color_name)
	XtFree(p->color_name);

    p->color_name = cp;
    p->color_set = 0;

    RangesetRefreshAllRanges(p);
    return 1;
}

/*
** Assign a color pixel value to a rangeset via the rangeset table. If ok is
** false, the color_set flag is set to an invalid (negative) value.
*/

int RangesetAssignColorPixel(Rangeset *p, Pixel color, int ok)
{
    p->color_set = ok ? 1 : -1;
    p->color = color;
    return 1;
}

/*
** Return the color name, if any.
*/

char *RangesetGetColorName(Rangeset *p)
{
    return p->color_name;
}

/*
** Return the color validity, if any, and the value in *color.
*/

int RangesetGetColorValid(Rangeset *p, Pixel *color)
{
    *color = p->color;
    return p->color_set;
}

/*
** Return the color name, if any.
*/

char *RangesetTableGetColorName(RangesetTable *tab, int ind)
{
    Rangeset *p = &tab->set[ind];
    return p->color_name;
}

/*
** Return the color color validity, if any, and the value in *color.
*/

int RangesetTableGetColorValid(RangesetTable *tab, int ind, Pixel *color)
{
    Rangeset *p = &tab->set[ind];
    *color = p->color;
    return p->color_set;
}

/*
** Assign a color pixel value to a rangeset via the rangeset table. If ok is
** false, the color_set flag is set to an invalid (negative) value.
*/

int RangesetTableAssignColorPixel(RangesetTable *tab, int ind, Pixel color,
	int ok)
{
    Rangeset *p = &tab->set[ind];
    p->color_set = ok ? 1 : -1;
    p->color = color;
    return 1;
}

/* -------------------------------------------------------------------------- */

#define is_start(i)	!((i) & 1)	/* true if i is even */
#define is_end(i)	((i) & 1)	/* true if i is odd */

/*
** Find the index of the first entry in the range set's ranges table (viewed as
** an int array) whose value is equal to or greater than pos. As a side effect,
** update the lasi_index value of the range set. Return's the index value. This
** will be twice p->n_ranges if pos is beyond the end.
*/

static int rangesetWeightedAtOrBefore(Rangeset *p, int pos)
{
    int i, last, n, *tab = (int *)p->ranges;

    n = p->n_ranges;
    if (n == 0)
	return 0;

    last = p->last_index;

    if (last >= n || last < 0)
	last = 0;

    n *= 2;
    last *= 2;

    if (pos >= tab[last])		/* ranges[last_index].start */
	i = weighted_at_or_before(tab, last, n, pos); /* search end only */
    else
	i = weighted_at_or_before(tab, 0, last, pos); /* search front only */

    p->last_index = i / 2;

    return i;
}

/*
** Adjusts values in tab[] by an amount delta, perhaps moving them meanwhile.
*/

static int rangesetShuffleToFrom(int *tab, int to, int from, int n, int delta)
{
    int end, diff = from - to;

    if (n <= 0)
	return 0;

    if (delta != 0) {
	if (diff > 0) {			/* shuffle entries down */
	    for (end = to + n; to < end; to++)
		tab[to] = tab[to + diff] + delta;
	}
	else if (diff < 0) {		/* shuffle entries up */
	    for (end = to, to += n; --to >= end;)
		tab[to] = tab[to + diff] + delta;
	}
	else {				/* diff == 0: just run through */
	    for (end = n; end--;)
		tab[to++] += delta;
	}
    }
    else {
	if (diff > 0) {			/* shuffle entries down */
	    for (end = to + n; to < end; to++)
		tab[to] = tab[to + diff];
	}
	else if (diff < 0) {		/* shuffle entries up */
	    for (end = to, to += n; --to >= end;)
		tab[to] = tab[to + diff];
	}
	/* else diff == 0: nothing to do */
    }

    return n;
}

/*
** Functions to adjust a rangeset to include new text or remove old.
** *** NOTE: No redisplay: that's outside the responsability of these routines.
*/

/* "Insert/Delete": if the start point is in or at the end of a range
** (start < pos && pos <= end), any text inserted will extend that range.
** Insertions appear to occur before deletions. This will never add new ranges.
*/

static Rangeset *rangesetInsDelMaintain(Rangeset *p, int pos, int ins, int del)
{
    int i, j, n, *tab = (int *)p->ranges;
    int end_del, movement;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, pos);

    if (i == n)
	return RangesetFixMaxpos(p, pos, ins, del);	/* all beyond the end */

    end_del = pos + del;
    movement = ins - del;

    /* the idea now is to determine the first range not concerned with the
       movement: its index will be j. For indices j to n-1, we will adjust
       position by movement only. (They may need shuffling up or down, depending
       on whether ranges have been deleted or created by the change.) */
    j = i;
    while (j < n && tab[j] <= end_del)	/* skip j to first ind beyond changes */
	j++;

    /* if j moved forward, we have deleted over tab[i] - reduce it accordingly,
       accounting for inserts. */
    if (j > i)
	tab[i] = pos + ins;

    /* If i and j both index starts or ends, just copy all the tab[] values down
       by j - i spaces, adjusting on the way. Otherwise, move beyond tab[i]
       before doing this. */

    if (is_start(i) != is_start(j))
	i++;

    rangesetShuffleToFrom(tab, i, j, n - j, movement);

    n -= j - i;
    p->n_ranges = n / 2;
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    /* final adjustments */
    return RangesetFixMaxpos(p, pos, ins, del);
}

/* "Inclusive": if the start point is in, at the start of, or at the end of a
** range (start <= pos && pos <= end), any text inserted will extend that range.
** Insertions appear to occur before deletions. This will never add new ranges.
** (Almost identical to rangesetInsDelMaintain().)
*/

static Rangeset *rangesetInclMaintain(Rangeset *p, int pos, int ins, int del)
{
    int i, j, n, *tab = (int *)p->ranges;
    int end_del, movement;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, pos);

    if (i == n)
	return RangesetFixMaxpos(p, pos, ins, del);	/* all beyond the end */

    /* if the insert occurs at the start of a range, the following lines will
       extend the range, leaving the start of the range at pos. */

    if (is_start(i) && tab[i] == pos && ins > 0)
	i++;

    end_del = pos + del;
    movement = ins - del;

    /* the idea now is to determine the first range not concerned with the
       movement: its index will be j. For indices j to n-1, we will adjust
       position by movement only. (They may need shuffling up or down, depending
       on whether ranges have been deleted or created by the change.) */
    j = i;
    while (j < n && tab[j] <= end_del)	/* skip j to first ind beyond changes */
	j++;

    /* if j moved forward, we have deleted over tab[i] - reduce it accordingly,
       accounting for inserts. */
    if (j > i)
	tab[i] = pos + ins;

    /* If i and j both index starts or ends, just copy all the tab[] values down
       by j - i spaces, adjusting on the way. Otherwise, move beyond tab[i]
       before doing this. */

    if (is_start(i) != is_start(j))
	i++;

    rangesetShuffleToFrom(tab, i, j, n - j, movement);

    n -= j - i;
    p->n_ranges = n / 2;
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    /* final adjustments */
    return RangesetFixMaxpos(p, pos, ins, del);
}

/* "Delete/Insert": if the start point is in a range (start < pos &&
** pos <= end), and the end of deletion is also in a range
** (start <= pos + del && pos + del < end) any text inserted will extend that
** range. Deletions appear to occur before insertions. This will never add new
** ranges.
*/

static Rangeset *rangesetDelInsMaintain(Rangeset *p, int pos, int ins, int del)
{
    int i, j, n, *tab = (int *)p->ranges;
    int end_del, movement;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, pos);

    if (i == n)
	return RangesetFixMaxpos(p, pos, ins, del);	/* all beyond the end */

    end_del = pos + del;
    movement = ins - del;

    /* the idea now is to determine the first range not concerned with the
       movement: its index will be j. For indices j to n-1, we will adjust
       position by movement only. (They may need shuffling up or down, depending
       on whether ranges have been deleted or created by the change.) */
    j = i;
    while (j < n && tab[j] <= end_del)	/* skip j to first ind beyond changes */
	j++;

    /* if j moved forward, we have deleted over tab[i] - reduce it accordingly,
       accounting for inserts. (Note: if tab[j] is an end position, inserted
       text will belong to the range that tab[j] closes; otherwise inserted
       text does not belong to a range.) */
    if (j > i)
	tab[i] = (is_end(j)) ? pos + ins : pos;

    /* If i and j both index starts or ends, just copy all the tab[] values down
       by j - i spaces, adjusting on the way. Otherwise, move beyond tab[i]
       before doing this. */

    if (is_start(i) != is_start(j))
	i++;

    rangesetShuffleToFrom(tab, i, j, n - j, movement);

    n -= j - i;
    p->n_ranges = n / 2;
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    /* final adjustments */
    return RangesetFixMaxpos(p, pos, ins, del);
}

/* "Exclusive": if the start point is in, but not at the end of, a range
** (start < pos && pos < end), and the end of deletion is also in a range
** (start <= pos + del && pos + del < end) any text inserted will extend that
** range. Deletions appear to occur before insertions. This will never add new
** ranges. (Almost identical to rangesetDelInsMaintain().)
*/

static Rangeset *rangesetExclMaintain(Rangeset *p, int pos, int ins, int del)
{
    int i, j, n, *tab = (int *)p->ranges;
    int end_del, movement;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, pos);

    if (i == n)
	return RangesetFixMaxpos(p, pos, ins, del);	/* all beyond the end */

    /* if the insert occurs at the end of a range, the following lines will
       skip the range, leaving the end of the range at pos. */

    if (is_end(i) && tab[i] == pos && ins > 0)
	i++;

    end_del = pos + del;
    movement = ins - del;

    /* the idea now is to determine the first range not concerned with the
       movement: its index will be j. For indices j to n-1, we will adjust
       position by movement only. (They may need shuffling up or down, depending
       on whether ranges have been deleted or created by the change.) */
    j = i;
    while (j < n && tab[j] <= end_del)	/* skip j to first ind beyond changes */
	j++;

    /* if j moved forward, we have deleted over tab[i] - reduce it accordingly,
       accounting for inserts. (Note: if tab[j] is an end position, inserted
       text will belong to the range that tab[j] closes; otherwise inserted
       text does not belong to a range.) */
    if (j > i)
	tab[i] = (is_end(j)) ? pos + ins : pos;

    /* If i and j both index starts or ends, just copy all the tab[] values down
       by j - i spaces, adjusting on the way. Otherwise, move beyond tab[i]
       before doing this. */

    if (is_start(i) != is_start(j))
	i++;

    rangesetShuffleToFrom(tab, i, j, n - j, movement);

    n -= j - i;
    p->n_ranges = n / 2;
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    /* final adjustments */
    return RangesetFixMaxpos(p, pos, ins, del);
}

/* "Break": if the modification point pos is strictly inside a range, that range
** may be broken in two if the deletion point pos+del does not extend beyond the
** end. Inserted text is never included in the range.
*/

static Rangeset *rangesetBreakMaintain(Rangeset *p, int pos, int ins, int del)
{
    int i, j, n, *tab = (int *)p->ranges;
    int end_del, movement, need_gap;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, pos);

    if (i == n)
	return RangesetFixMaxpos(p, pos, ins, del);	/* all beyond the end */

    /* if the insert occurs at the end of a range, the following lines will
       skip the range, leaving the end of the range at pos. */

    if (is_end(i) && tab[i] == pos && ins > 0)
	i++;

    end_del = pos + del;
    movement = ins - del;

    /* the idea now is to determine the first range not concerned with the
       movement: its index will be j. For indices j to n-1, we will adjust
       position by movement only. (They may need shuffling up or down, depending
       on whether ranges have been deleted or created by the change.) */
    j = i;
    while (j < n && tab[j] <= end_del)	/* skip j to first ind beyond changes */
	j++;

    if (j > i)
	tab[i] = pos;

    /* do we need to insert a gap? yes if pos is in a range and ins > 0 */

    /* The logic for the next statement: if i and j are both range ends, range
       boundaries indicated by index values between i and j (if any) have been
       "skipped". This means that tab[i-1],tab[j] is the current range. We will
       be inserting in that range, splitting it. */

    need_gap = (is_end(i) && is_end(j) && ins > 0);

    /* if we've got start-end or end-start, skip tab[i] */
    if (is_start(i) != is_start(j)) {	/* one is start, other is end */
	if (is_start(i)) {
	    if (tab[i] == pos)
		tab[i] = pos + ins;	/* move the range start */
	}
	i++;				/* skip to next index */
    }

    /* values tab[j] to tab[n-1] must be adjusted by movement and placed in
       position. */

    if (need_gap)
	i += 2;				/* make space for the break */

    /* adjust other position values: shuffle them up or down if necessary */
    rangesetShuffleToFrom(tab, i, j, n - j, movement);

    if (need_gap) {			/* add the gap informations */
	tab[i - 2] = pos;
	tab[i - 1] = pos + ins;
    }

    n -= j - i;
    p->n_ranges = n / 2;
    p->ranges = RangesRealloc(p->ranges, p->n_ranges);

    /* final adjustments */
    return RangesetFixMaxpos(p, pos, ins, del);
}

/* -------------------------------------------------------------------------- */

/*
** Invert the rangeset (replace it with its complement in the range 0-maxpos).
** Returns the number of ranges if successful, -1 otherwise. Never adds more
** than one range.
*/

int RangesetInverse(Rangeset *p)
{
    int *tab;
    int n, has_zero, has_end;

    if (!p)
	return -1;

    tab = (int *)p->ranges;

    if (p->n_ranges == 0) {
	tab[0] = 0;
	tab[1] = p->maxpos;
	n = 2;
    }
    else {
	n = p->n_ranges * 2;

	/* find out what we have */
        has_zero = (tab[0] == 0);
	has_end = (tab[n - 1] == p->maxpos);

	/* fill the entry "beyond the end" with the buffer's length */
	tab[n + 1] = tab[n] = p->maxpos;

	if (has_zero) {
	  /* shuffle down by one */
	  rangesetShuffleToFrom(tab, 0, 1, n, 0);
	  n -= 1;
	}
	else {
	  /* shuffle up by one */
	  rangesetShuffleToFrom(tab, 1, 0, n, 0);
	  tab[0] = 0;
	  n += 1;
	}
	if (has_end)
	  n -= 1;
	else
	  n += 1;
    }

    p->n_ranges = n / 2;
    p->ranges = RangesRealloc((Range *)tab, p->n_ranges);

    RangesetRefreshRange(p, 0, p->maxpos);
    return p->n_ranges;
}

/* -------------------------------------------------------------------------- */

/*
** Remove the range indicated by the positions start and end. Returns the
** new number of ranges in the set.
*/

int RangesetRemoveFromTo(Rangeset *p, int start, int end)
{
    int i, j, n, *tab = (int *)p->ranges;

    n = 2 * p->n_ranges;

    i = rangesetWeightedAtOrBefore(p, start);

    if (i == n)
	return p->n_ranges;		/* beyond last range */

    j = i;
    while (j < n && tab[j] <= end)	/* skip j to first ind beyond changes */
	j++;

    if (i == j) {
	/* removal occurs in front of tab[i] */
	if (is_start(i))
	    return p->n_ranges;		/* no change */
	else {
	    /* is_end(i): need to make a gap in range tab[i-1], tab[i] */
	    i--;			/* start of current range */
	    rangesetShuffleToFrom(tab, i + 2, i, n - i, 0); /* shuffle up */
	    tab[i + 1] = start;		/* change end of current range */
	    tab[i + 2] = end;		/* change start of new range */
	    p->n_ranges++;		/* we've just created a new range */
	    p->ranges = RangesRealloc(p->ranges, p->n_ranges);
	}
    }
    else {
	/* removal occurs in front of tab[j]: we'll be shuffling down */
	if (is_end(i))
	    tab[i++] = start;
	if (is_end(j))
	     tab[--j] = end;
	if (i < j)
	    rangesetShuffleToFrom(tab, i, j, n - j, 0);
	n -= j - i;
	p->n_ranges = n / 2;
	p->ranges = RangesRealloc(p->ranges, p->n_ranges);
    }

    RangesetRefreshRange(p, start, end);
    return p->n_ranges;
}

/*
** Subtract the range from start to end from rangeset p.
*/

int RangesetRemoveBetween(Rangeset *p, int start, int end)
{
    return RangesetRemoveFromTo(p, start, end);
}

/* -------------------------------------------------------------------------- */

/*
** Add the range indicated by the positions start and end. Returns the
** new number of ranges in the set.
*/

int RangesetAddFromTo(Rangeset *p, int start, int end)
{
    int i, j, n, *tab = (int *)p->ranges;

    n = 2 * p->n_ranges;

    if (n == 0) {			/* make sure we have space */
	p->ranges = RangesNew(1);
	tab = (int *)p->ranges;
	i = 0;
    }
    else
	i = rangesetWeightedAtOrBefore(p, start);

    if (i == n) {			/* beyond last range: just add it */
	tab[n] = start;
	tab[n + 1] = end;
	p->n_ranges++;
	p->ranges = RangesRealloc(p->ranges, p->n_ranges);

	RangesetRefreshRange(p, start, end);
	return p->n_ranges;
    }

    j = i;
    while (j < n && tab[j] <= end)	/* skip j to first ind beyond changes */
	j++;

    if (i == j) {
	if (is_start(i)) {
	    /* is_start(i): need to make a gap in range tab[i-1], tab[i] */
	    rangesetShuffleToFrom(tab, i + 2, i, n - i, 0);	/* shuffle up */
	    tab[i] = start;		/* load up new range's limits */
	    tab[i + 1] = end;
	    p->n_ranges++;		/* we've just created a new range */
	    p->ranges = RangesRealloc(p->ranges, p->n_ranges);
	}
	else {
	    return p->n_ranges;		/* no change */
	}
    }
    else {
	/* we'll be shuffling down */
	if (is_start(i))
	    tab[i++] = start;
	if (is_start(j))
	     tab[--j] = end;
	if (i < j)
	    rangesetShuffleToFrom(tab, i, j, n - j, 0);
	n -= j - i;
	p->n_ranges = n / 2;
	p->ranges = RangesRealloc(p->ranges, p->n_ranges);
    }

    RangesetRefreshRange(p, start, end);
    return p->n_ranges;
}

/*
** Add the range from start to end from rangeset p.
*/

int RangesetAddBetween(Rangeset *p, int start, int end)
{
    return RangesetAddFromTo(p, start, end);
}

/*******************************************************************************
*                                                                              *
* refString.c -- Nirvana editor string handling                                *
*                                                                              *
* Copyright (C) 200 Scott Tringali                                             *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version. In addition, you may distribute versions of this program linked to  *
* Motif or Open Motif. See README for details.                                 *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* July, 1993                                                                   *
*                                                                              *
* Written by Mark Edel                                                         *
*                                                                              *
*******************************************************************************/

#include "refString.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RCS_SIZE 10000

struct rcs;

struct rcs_stats
{
    int talloc, tshar, tgiveup, tbytes, tbyteshared;
};

struct rcs
{
    struct rcs *next;
    char       *string;
    int         usage;
};

static struct rcs       *Rcs[RCS_SIZE];
static struct rcs_stats  RcsStats;

/*      Compute hash address from a string key */
unsigned StringHashAddr(const char *key)
{
    unsigned s=strlen(key);
    unsigned a=0,x=0,i;
    
    for (i=0; (i+3)<s; i += 4) {
        strncpy((char*)&a,&key[i],4);
        x += a;
    }
    
    for (a=1; i<(s+1); i++, a *= 256)
        x += key[i] * a;
        
    return x;
}

/*
** Take a normal string, create a shared string from it if need be,
** and return pointer to that shared string.
**
** Returned strings are const because they are shared.  Do not modify them!
*/

const char *RefStringDup(const char *str)
{
    int bucket;
    size_t len;
    struct rcs *rp;
    struct rcs *prev = NULL;
  
    char *newstr = NULL;
    
    if (str == NULL)
        return NULL;
        
    bucket = StringHashAddr(str) % RCS_SIZE;
    len = strlen(str);
    
    RcsStats.talloc++;

#if 0  
    /* Don't share if it won't save space.
    
       Doesn't save anything - if we have lots of small-size objects,
       it's beneifical to share them.  We don't know until we make a full
       count.  My tests show that it's better to leave this out.  */
    if (len <= sizeof(struct rcs))
    {
        new_str = strdup(str); /* GET RID OF strdup() IF EVER ENABLED (not ANSI) */ 
        RcsStats.tgiveup++;
        return;
    }
#endif

    /* Find it in hash */
    for (rp = Rcs[bucket]; rp; rp = rp->next)
    {
        if (!strcmp(str, rp->string))
            break;
        prev = rp;
    }

    if (rp)  /* It exists, return it and bump ref ct */
    {
        rp->usage++;
        newstr = rp->string;

        RcsStats.tshar++;
        RcsStats.tbyteshared += len;
    }
    else     /* Doesn't exist, conjure up a new one. */
    {
        struct rcs* newrcs;
        if (NULL == (newrcs = (struct rcs*) malloc(sizeof(struct rcs)))) {
            /*  Not much to fall back to here.  */
            fprintf(stderr, "nedit: rcs_strdup(): out of heap space!\n");
/*            XBell(TheDisplay, 0); */
            exit(EXIT_FAILURE);
        }

        if (NULL == (newrcs->string = (char*) malloc(len + 1))) {
            /*  Not much to fall back to here.  */
            fprintf(stderr, "nedit: rcs_strdup(): out of heap space!\n");
/*            XBell(TheDisplay, 0); */
            exit(EXIT_FAILURE);
        }
        strcpy(newrcs->string, str);
        newrcs->usage = 1;
        newrcs->next = NULL;

        if (Rcs[bucket])
            prev->next = newrcs;
        else
            Rcs[bucket] = newrcs;
            
        newstr = newrcs->string;
    }

    RcsStats.tbytes += len;
    return newstr;
}

/*
** Decrease the reference count on a shared string.  When the reference
** count reaches zero, free the master string.
*/

void RefStringFree(const char *rcs_str)
{
    int bucket;
    struct rcs *rp;
    struct rcs *prev = NULL;

    if (rcs_str == NULL)
        return;
        
    bucket = StringHashAddr(rcs_str) % RCS_SIZE;

    /* find it in hash */
    for (rp = Rcs[bucket]; rp; rp = rp->next)
    {
        if (rcs_str == rp->string)
            break;
        prev = rp;
    }

    if (rp)  /* It's a shared string, decrease ref count */
    {
        rp->usage--;
        
        if (rp->usage < 0) /* D'OH! */
        {
            fprintf(stderr, "NEdit: internal error deallocating shared string.");
            return;
        }

        if (rp->usage == 0)  /* Last one- free the storage */
        {
            free(rp->string);
            if (prev)
                prev->next = rp->next;
            else
                Rcs[bucket] = rp->next;
            free(rp);
        }
    }
    else    /* Doesn't appear to be a shared string */
    {
        fprintf(stderr, "NEdit: attempt to free a non-shared string.");
        return;
    }
}


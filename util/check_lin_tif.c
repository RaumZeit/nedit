/*******************************************************************************
*                                                                              *
* check_lin_tif.c:  A small test program to detect bad versions of LessTif and *
* Open Motif on Linux.                                                         *
*                                                                              *
* Copyright (C) 2003 Nathaniel Gray                                            *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.                                                                     *
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
* July 28, 1992                                                                *
*                                                                              *
* Written by Nathaniel Gray                                                    *
*                                                                              *
*******************************************************************************/

/*
 * About the different #defines that Motif gives us:
 * All Motifs #define several values.  These are the values in
 * Open Motif 2.1.30, for example:
 *     #define XmVERSION       2
 *     #define XmREVISION      1
 *     #define XmUPDATE_LEVEL  30
 *     #define XmVersion       (XmVERSION * 1000 + XmREVISION)
 *     #define XmVERSION_STRING "@(#)Motif Version 2.1.30"
 * 
 * In addition, LessTif #defines several values as shown here for
 * version 0.93.0:
 *     #define LESSTIF_VERSION  0
 *     #define LESSTIF_REVISION 93
 *     #define LesstifVersion   (LESSTIF_VERSION * 1000 + LESSTIF_REVISION)
 *     #define LesstifVERSION_STRING \
 *             "@(#)GNU/LessTif Version 2.1 Release 0.93.0"
 *
 * Also, in LessTif the XmVERSION_STRING is identical to the 
 * LesstifVERSION_STRING.  Unfortunately, the only way to find out the
 * "update level" of a LessTif release is to parse the LesstifVERSION_STRING.
 *
 * To test:
        gcc -I /usr/X11R6/include/ -o check_tif check_tif.c
        gcc -I /usr/X11R6/LessTif/Motif2.1/include/ -o check_tif check_tif.c
        gcc -I /usr/X11R6/LessTif/Motif1.2/include/ -o check_tif check_tif.c
 */
#include <Xm/Xm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 
 * These are versions of LessTif that are known to be stable with NEdit in
 * Motif 1.2 mode.
 */
char *good_lesstif_1_2_versions[] = {
    "0.92.32",
    "0.93.0",
    "0.93.12",
    "0.93.18",
    NULL
};

/* 
 * These are versions of LessTif that are known to be stable with NEdit in
 * Motif 2.1 mode.
 */
char *good_lesstif_2_1_versions[] = {
    "0.92.32",
    "0.93.0",
    "0.93.12",
    "0.93.18",
    NULL
};

/* 
 * These are versions of LessTif that are known NOT to be stable with NEdit in
 * Motif 1.2 mode.
 */
char *bad_lesstif_1_2_versions[] = {
    NULL
};

/* 
 * These are versions of LessTif that are known NOT to be stable with NEdit in
 * Motif 2.1 mode.
 */
char *bad_lesstif_2_1_versions[] = {
    NULL
};

/* Print out a message listing the known-good versions */
void good_versions() {
    int i;
    fprintf(stderr, "\nNEdit is known to work with LessTif versions:\n");
    for(i=0; good_lesstif_2_1_versions[i]; i++) {
        fprintf(stderr, "\t%s\n", good_lesstif_2_1_versions[i]);
    }
    fprintf(stderr, 
        "and is known to be very stable with Open Motif version 2.1.30,\n"
        "which is freely available from several sources, including:\n"
        "\thttp://www.opengroup.org/openmotif/\n"
        "\thttp://www.metrolink.com/products/motif/open_motif2-1-30.html\n"
        "\thttp://www.ist.co.uk/DOWNLOADS/motif_download.html\n"
        "\nAlso, unless you need a customized NEdit you should STRONGLY\n"
        "consider downloading a pre-built binary from http://www.nedit.org,\n"
        "since these are the most stable versions.\n");
}

/* We assume that the lesstif version is the string after the last
    space. */
char* get_lesstif_rev(char *vs) {
    char *rev;
    
    rev = strrchr(vs, ' ');
    if (rev == NULL) {
        fprintf(stderr, "ERROR: Can't get LessTif Version Substring!\n");
        exit(1);
    }
    return rev+1;
}

/* Check to see if the user has overridden our warnings.  If they haven't,
    tell them how to do so if they're brave (or foolish :-). */
void finish(int exitcode, char *tif) {
    char buf[2];
    
    good_versions();
    if (exitcode == 1) {
#ifdef BUILD_BROKEN_NEDIT
        fprintf(stderr,
            "\n========================== WARNING ===========================\n"
            "You have chosen to build NEdit with a known-bad version of %s,\n"
            "risking instability and probable data loss.  You are very brave!\n"
            "Please do not report bugs to the NEdit developers unless you can\n"
            "reproduce them with a known-good NEdit binary downloaded from:\n"
            "\thttp://www.nedit.org\n"
            "\nHIT ENTER TO CONTINUE\n", tif);
        fgets(buf, 2, stdin);
        exit(0);
#else
        fprintf(stderr,
            "\nIf you really want to build a known-bad version of NEdit you\n"
            "can override this sanity check by adding -DBUILD_BROKEN_NEDIT\n"
            "to the CFLAGS variable in makefiles/Makefile.linux\n");
        exit(1);
#endif
    } else if (exitcode == 2) {
#ifdef BUILD_UNTESTED_NEDIT
        fprintf(stderr,
            "\n========================== WARNING ===========================\n"
            "You have chosen to build NEdit with an untested version of %s.\n"
            "Please report your success or failure with this version to:\n"
            "\tdevelop@nedit.org\n"
            "\nHIT ENTER TO CONTINUE\n", tif);
        fgets(buf, 2, stdin);
        exit(0);
#else
        fprintf(stderr,
            "\nIf you really want to build an untested version of NEdit you\n"
            "can override this sanity check by adding -DBUILD_UNTESTED_NEDIT\n"
            "to the CFLAGS variable in makefiles/Makefile.linux\n");
        exit(2);
#endif
    }
}

int main() {
    char *vs = XmVERSION_STRING, *tif, **v_good, **v_bad, *lesstif_rev;
    int i, force_bad = 0;  /* This is just for debugging */
    
#ifdef LESSTIF_VERSION
    fprintf(stderr, "LessTif detected.\n");
    fprintf(stderr, "%s\n", vs);
    tif = "LessTif";
    lesstif_rev = get_lesstif_rev(vs);
    
    if (XmVersion == 1002) {
        v_good = good_lesstif_1_2_versions;
        v_bad = bad_lesstif_1_2_versions;
    } else if (XmVersion == 2001) {
        v_good = good_lesstif_2_1_versions;
        v_bad = bad_lesstif_2_1_versions;
    } else {
        fprintf(stderr, "Unexpected LessTif Version\n");
        finish(2, tif);
    }
    
    /* Check for known good LessTif versions */
    for(i=0; v_good[i]; i++) {
        if(!strcmp(lesstif_rev, v_good[i]))
            exit(0);
    }
    /* Check for known bad LessTif versions */
    for(i=0; v_bad[i]; i++) {
        if(!strcmp(lesstif_rev, v_bad[i])) {
            fprintf(stderr,
                "\nYou are attempting to compile NEdit with a version of "
                "LessTif that\nis known to interact badly with NEdit.  "
                "Please use a different\nversion of LessTif or Open Motif.\n");
            finish(1, tif);
        }
    }
#else

    fprintf(stderr, "Open Motif detected.\n");
    fprintf(stderr, "%s\n", vs);
    tif = "Open Motif";
    
    /* Check for Open Motif 2.1.30 */
    if (!force_bad && XmVERSION == 2 && XmREVISION == 1 && XmUPDATE_LEVEL == 30)
        exit(0);
    
    /* Check for the dreaded Open Motif 2.2.2 */
    if (force_bad || 
            (XmVERSION == 2 && XmREVISION == 2 && XmUPDATE_LEVEL <= 2)) {
        fprintf(stderr, "ERROR:  Bad Open Motif Version:\n\t%s\n", vs);
        fprintf(stderr, 
            "\nThis version of Open Motif is known to be broken and is\n"
            "thus unsupported by the NEdit developers.  It will probably\n"
            "cause NEdit to crash frequently.  Check these pages for a more\n"
            "detailed description of the problems with this version:\n"
            "\thttp://www.motifdeveloper.com/tips/tip22.html\n"
            "\thttp://www.motifdeveloper.com/tips/Motif22Review.pdf\n");
        finish(1, tif);
    }    
#endif
    
    /* This version is neither known-good nor known-bad */
    fprintf(stderr, "ERROR:  Untested %s Version:\n\t%s\n", tif, vs);
    fprintf(stderr, 
        "You are attempting to build NEdit with a version of %s that\n"
        "has not been verified to work well with NEdit.  This could be fine,\n"
        "but it could also lead to crashes and instability.  Historically, \n"
        "older versions of Linux Motifs have quite often been more stable\n"
        "than newer versions when used with NEdit.\n",
            tif);
    finish(2, tif);
    return 1;
}

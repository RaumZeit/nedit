/*******************************************************************************
*									       *
* system.h -- Compile time configuration               			       *
*									       *
* Copyright (C) 2001 Scott Tringali					       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.							               *
* 									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
* for more details.							       *
* 									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                       *
*									       *
* Nirvana Text Editor	    						       *
* July 23, 2001         						       *
*									       *
* Written by Scott Tringali, http://www.tringali.org			       *
*									       *
*******************************************************************************/

/* Determine which machine we were compiled with.  This isn't as accurate
   as calling uname(), which is preferred.  However, this gets us very close
   for a majority of the machines out there, and doesn't require any games
   with make.

   A better, but trickier solution, is to run uname at compile time, capture
   the string, and place it in the executable.
      
   Please update this with the proper symbols for your compiler/CPU.  It
   may take a little sleuthing to find out what the correct symbol is.
   Better compilers/OSs document the symbols they define, but not all do.
   Usually, the correct ones are prepended with an _ or __, as this is
   namespace is reserved by ANSI C for the compiler implementation.
*/

#if defined(__alpha)
#   define COMPILE_MACHINE "Alpha"
#elif defined(__mips)
#   define COMPILE_MACHINE "MIPS"
#elif defined(__sparc)
#   define COMPILE_MACHINE "Sparc"
#elif defined(__hppa)
#   define COMPILE_MACHINE "PA-RISC"
#elif defined(__PPC__) || defined(_POWER)
#   define COMPILE_MACHINE "PowerPC"
#elif defined(__ia64)
#   define COMPILE_MACHINE "Intel IA64"
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#   define COMPILE_MACHINE "Intel x86"
#elif defined(__VAX)
#   define COMPILE_MACHINE "VAX"        /* Untested, please verify */
#else	
#   define COMPILE_MACHINE "Unknown"
#endif

#if defined(__osf__)
#   define COMPILE_OS "OSF/1"
#elif defined(__sun)
#   define COMPILE_OS "Solaris"
#elif defined(__hpux)
#   define COMPILE_OS "HP/UX"
#elif defined(_WIN32)
#   define COMPILE_OS "Win32"
#elif defined(__sgi)
#   define COMPILE_OS "IRIX"
#elif defined(__linux__)
#   define COMPILE_OS "Linux"
#elif defined(_AIX)
#   define COMPILE_OS "AIX"
#elif defined(__VMS)                    /* Untested, please verify */
#   define COMPILE_OS "VMS"
#elif defined(__FreeBSD__)              /* Untested, please verify */
#   define COMPILE_OS "FreeBSD"
#elif defined(__OpenBSD__)              /* Untested, please verify */
#   define COMPILE_OS "OpenBSD"
#elif defined(__NetBSD__)               /* Untested, please verify */
#   define COMPILE_OS "NetBSD"
#elif defined(__bsdi)                   /* Untested, please verify */
#   define COMPILE_OS "BSDI"
#elif defined(__ultrix)                 /* Untested, please verify */
#   define COMPILE_OS "Ultrix"
#else
#   define COMPILE_OS "Unknown"
#endif

#if defined(__GNUC__)
#   define COMPILE_COMPILER "GNU C"
#elif defined (__DECC)
#   define COMPILE_COMPILER "DEC C"
#elif defined (__DECCXX)
#   define COMPILE_COMPILER "DEC C++"
#elif defined (__APOGEE)
#   define COMPILE_COMPILER "Apogee"
#else
#   define COMPILE_COMPILER "Unknown"
#endif

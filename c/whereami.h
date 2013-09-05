#ifndef EXC_whereami
#define EXC_whereami 1
/*
 * Written in 2005, 2006, 2013 by Gregor Richards
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty. 
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */ 
/* Figure out where a binary is installed
 * argvz: argv[0]
 * dir: Where to put the directory component
 * fil: Where to put the filename component
 * returns a pointer to dir or NULL for failure */
 char *whereAmI(const char *argvz, char **dir, char **fil);
#endif

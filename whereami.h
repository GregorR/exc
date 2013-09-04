#ifndef EXC_whereami
#define EXC_whereami 1

/* Figure out where a binary is installed
 * argvz: argv[0]
 * dir: Where to put the directory component
 * fil: Where to put the filename component
 * returns a pointer to dir or NULL for failure */
 char *whereAmI(const char *argvz, char **dir, char **fil);
#endif

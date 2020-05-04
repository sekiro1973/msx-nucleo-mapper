#ifndef TOOLS_H
#define TOOLS_H

/* Import a binary file */
#define	IMPORT_BIN(sect, file, sym) asm (\
  ".section " #sect "\n"                  /* Change section */\
  ".balign 4\n"                           /* Word alignment */\
  ".global " #sym "\n"                    /* Export the object address to other modules */\
  #sym ":\n"                              /* Define the object label */\
  ".incbin \"" file "\"\n"                /* Import the file */\
  ".global _sizeof_" #sym "\n"            /* Export the object size to oher modules */\
  ".set _sizeof_" #sym ", . - " #sym "\n" /* Define the object size */\
  ".balign 4\n"                           /* Word alignment */\
  ".section \".text\"\n")                 /* Restore section */

#endif

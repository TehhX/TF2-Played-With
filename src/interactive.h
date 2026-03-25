#ifndef INTERACTIVE_H
#define INTERACTIVE_H

/*
    interactive.h
    -------------
    Contains functionality for interactive user input for use in both collecting live data and as an initial argp option
*/

// Begins an interactive session. Will essentially take over process and stdin, be careful
extern void interactive_enter();

#endif // INTERACTIVE_H

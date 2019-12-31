/****************************************************/
/* File: util.h                                     */
/* The interface of general tools for the           */
/* C-Minus compiler                                 */
/* Programming designed by Zhiyao Liang             */
/* MUST  2016 Fall                                  */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "libs.h"

typedef enum{
  FALSE,
  TRUE
}Boolean;


/* Print the message msg, then wait for the user to hit the enter/return key.
 * The input queue is cleared before this function returns.
 */
void pause_msg(const char * msg);

/* <Parameter:>
 * str:  a character string.
 * <Return:>
 * A copy (a clone) of the input string str, including the ending '\0'. The space of the clone does not overlap with the space of str.
 * */
char * string_clone(const char* str);

void clear_input_queue(void);

void *checked_malloc(int len);


int read_file_to_char_array( char * array, int arrayLength, FILE * stream);


char *  clone_string_section(const char * str, int begin, int end);

#endif

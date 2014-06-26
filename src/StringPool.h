/* The StringPool reserves memory at its loading time.
 * Its string elements can be accessed with the function reserveString().
 * The pool is fixed size, hence if all elements are reserved the caller of
 * reserveString() will block.
 * Use releaseString() to mark a String as free again. */

#ifndef STRING_POOL_H
#define STRING_POOL_H

#define STRING_SIZE 50

int initStringPool();
int destroyStringPool();

/* This function can block if no string is available */
char* reserveString();
void releaseString(char *p_string);

#endif
/*	VGATERM.H -	Debug printing interface for VGA 80x25 text console
 *
 *	Author:	Crupette
 * */

#ifndef VGATERM_H
#define VGATERM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/*	Initializes the 80x25 VGA buffer for printing.
 *	Also initializes internal variables to allow for printing
 *	*	Resets col and row to 0
 *	*	Sets color to white (0xF)
 *	*	Initializes VGA buffer pointer
 *	*	Fills VGA data with NULL
 * */
void vgaterm_init(void);

/*	Sets the cursor color to c
 * */
void vgaterm_setc(uint8_t c);

/*	Sets character at column (col) row (row) to (c) with color (color)
 * */
void vgaterm_setent(uint8_t col, uint8_t row, int8_t c, uint8_t color);

/*	Sets character at next column and row to (c) and color (color)
 * */
void vgaterm_putent(int8_t c, uint8_t color);

/*	Puts entry with character (c) and cursor color
 * */
void vgaterm_putc(int8_t c);

/*	Puts a string of characters starting at (str) and ending with nullptr
 * */
void vgaterm_putstr(char *str);

#endif

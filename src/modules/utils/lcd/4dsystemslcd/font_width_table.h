#ifndef FONT_WIDTH_TABLE
#define FONT_WIDTH_TABLE

// Starts with " " and ends with "~"
#define FIRST_CHAR_TABLE ' '
#define LAST_CHAR_TABLE '~'

const word font_width_table[][LAST_CHAR_TABLE-FIRST_CHAR_TABLE] =
{
	{4,4,5,9,7,12,8,3,5,5,7,8,4,4,4,5,7,7,7,7,7,7,7,7,7,7,4,4,9,9,9,6,11,8,7,7,8,7,6,8,8,4,5,7,6,9,8,9,7,9,7,7,8,8,8,12,7,8,7,5,5,5,9,7,7,6,7,6,7,7,4,7,7,2,3,6,2,10,7,7,7,7,4,5,5,7,6,10,6,6,5,6,5,6},
	{4,5,7,12,10,18,12,4,7,7,10,12,5,6,5,9,10,10,10,10,10,10,10,10,10,10,5,5,12,12,12,9,14,10,10,10,11,9,9,11,11,7,8,10,9,13,12,12,10,12,11,10,9,11,10,15,10,11,9,7,9,7,12,10,8,9,9,8,9,9,6,9,9,5,5,9,5,13,9,9,9,9,7,8,6,9,9,13,9,9,8,9,10,9},
	{6,7,10,16,13,24,16,6,9,9,13,16,6,9,6,12,13,13,13,13,13,13,13,13,13,13,7,7,16,16,16,11,18,14,14,13,15,12,12,15,15,10,10,14,11,18,15,15,13,15,15,13,12,15,14,21,14,13,12,9,12,9,16,13,11,12,13,11,13,12,8,13,13,6,7,12,6,19,13,12,13,13,9,10,8,13,12,18,12,12,11,12,13,12}
};

// degrees (º) is char 186 in ISO-8859-1
const word degrees_width[] = {6, 8, 11};
// cube (³) is char 179 in ISO-8859-1
const word cube_width[] = {6, 8, 11};

const word font_height_table[] = {14, 18, 24};
const word max_numbers_width[] = {7, 10, 13};
const word max_char_alphanum_width[] = {12, 15, 21};

// Returns the width from the given char and size. It NEVER asks the LCD module what is the size.
// The reason is so that this macro can be used for preprocessor directives. If valid it returns
// the width from the table. Otherwise, it returns a value that follows more or less the proportion
// of the characters width for that font size.
#define CHAR_WIDTH(s,c) ( (c >= FIRST_CHAR_TABLE && c <= LAST_CHAR_TABLE)? (font_width_table[s][c - FIRST_CHAR_TABLE]) : (c == 'º') ? degrees_width[s] : (c == '³') ? cube_width[s] : max_char_alphanum_width[s] )
#define HOUR_WIDTH(s) (7*max_numbers_width[s]+1*CHAR_WIDTH(s, ':'))
#define PERCENT_WIDTH(s) (3*max_numbers_width[s]+CHAR_WIDTH(s, '%'))
#define TEMP_WIDTH(s) (3*max_numbers_width[s]+get_char_width(s, 'º'))

#endif // FONT_WIDTH_TABLE


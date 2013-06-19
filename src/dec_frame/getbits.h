#ifndef __GET_BITS_H__
#define __GET_BITS_H__


extern struct bit_info bi;

extern void backbits(int number_of_bits);
extern int getbitoffset(void);
extern int getbyte(void);
extern unsigned int getbits(int number);
extern unsigned int get1bit(void);

#endif

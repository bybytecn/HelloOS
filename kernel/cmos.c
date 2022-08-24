#include "./include/cmos.h"
#include "./include/ioutil.h"
static uint32_t bcd2bin(uint32_t bcd)
{
	return ((bcd >> 4) * 10 + (bcd & 0x0f));
}
uint32_t get_curtime()
{
	outb(0x70, 0x00);
	int sec = bcd2bin(inb(0x71));
	outb(0x70, 0x02);
	int min = bcd2bin(inb(0x71));
	outb(0x70, 0x04);
	int hour = bcd2bin(inb(0x71));
	outb(0x70, 0x07);
	int day = bcd2bin(inb(0x71));
	outb(0x70, 0x08);
	int month = bcd2bin(inb(0x71));
	outb(0x70, 0x09);
	int year = bcd2bin(inb(0x71)) + 30 + 1970;
	int timestamp = hour * 3600 + min * 60 + sec + day * 86400 + month * 2592000 + (year - 1970) * 31536000;
	return timestamp;
}
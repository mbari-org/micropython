/*
 * utils.c
 *
 *  Created on: Aug 9, 2022
 *      Author: sjensen
 */

#include "utils.h"
#include "lpuartISR.h"

void hextoascii (unsigned char byte, unsigned char *high, unsigned char *low)
  {
      *low = byte&0x0f;
      *high = (byte>>4);
      if (*low<=9) *low += '0';
      else *low += ('A'-10);
      if (*high<=9) *high += '0';
      else *high += ('A'-10);
  }

signed char sendstring(char *string)
{
    while (*string != 0x0)
    {
        if (utxaddbyte(*string) >= 0x0) string++;
    }
    return (signed char) 1;
}

signed char sendstringnr(void)
{
    utxaddbyte('\n');
    utxaddbyte('\r');

    return 0;
}

void clearwmess(char *string)
{
    unsigned char rchar;
    while ((urxgetbyte(&rchar)>0))
        if ((rchar==13)||(rchar==10)) break;
    sendstring(string);
    return;
}
signed char asciitohex(unsigned char nchar)
{
    if (nchar<'0') return -1;
    if (nchar>'F') return -1;
    if (nchar<='9') return ((signed char)nchar-(signed char)'0');
    if (nchar< 'A') return -1;
    return ((signed char)nchar-(signed char)'A'+10);
}

signed char sendhex(unsigned char hex)
{
    unsigned char hascii,lascii;
            hextoascii((unsigned char)hex,&hascii,&lascii);
            utxaddbyte(hascii);
            utxaddbyte(lascii);
            return 0;
}

signed char sendhexstring(unsigned int idata)
{

            sendhex(((unsigned char *)&idata)[1]);
            sendhex(((unsigned char *)&idata)[0]);
            return 0;

}

signed char sendhexerror(signed char error)
{
            error=-error;
            sendstringnr();
            utxaddbyte('-');
            error=sendhex((unsigned char)error);
            sendstringnr();
            return 0;
}


#ifndef __SERIAL_IO__
#define __SERIAL_IO__


int   serialRecv();
char *getSerialComType();
char  getSerialComSub();
char *getSerialComBody();
char  getSerialComLen();
bool  getSerialComEof();

#endif

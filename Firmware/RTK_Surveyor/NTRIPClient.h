#ifndef NTRIP_CLIENT
#define NTRIP_CLIENT

#include <base64.h>

class NTRIPClient : public WiFiClient{
  public :
  bool reqSrcTbl(char* host,uint16_t &port);   //request MountPoints List serviced the NTRIP Caster 
  bool reqRaw(char* host,uint16_t &port,char* mntpnt,char* user,char* psw);      //request RAW data from Caster 
  bool reqRaw(char* host,uint16_t &port,char* mntpnt); //non user
  int readLine(char* buffer,int size);

  
};

#endif


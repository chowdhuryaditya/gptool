/*
 *
 */

#ifndef  SOFTCORR_H
#define  SOFTCORR_H

#define  DasHeaderKey   2031
#define  DasBufferKey   2032

#define  DasHeaderKey_SIM   3031
#define  DasBufferKey_SIM   3032

//#include  <sys/ipc.h>
//#include  <sys/shm.h>
//#include  <sys/types.h>

// which is equivalent of 21.47483648s = 512MB.
// 32 of 0.671s buffers combine to form one SHM block of 512MB
// Total SHM (343.59738368s) size is 512MB * 16 = 8GB
#define  DataSize (256*512*4096) 
#define  MaxDataBlocks  16
#define  TimeSize       sizeof(double)

typedef struct
{
  unsigned int flag, curBlock, curRecord, blockSize;
  int overFlow;
  double pcTime[MaxDataBlocks],dataTime[MaxDataBlocks];
  unsigned char data[(long)(DataSize)*(long)(MaxDataBlocks)];
} DataBuffer;

typedef struct
{
  unsigned int active, status;
  double pcTime, dataTime, refTime;
	struct timeval timestamp[MaxDataBlocks];
  struct timeval timestamp_gps[MaxDataBlocks];
  double blk_nano[MaxDataBlocks];
} DataHeader;

//DataHeader *dataHdr;
//DataBuffer *dataBuf;

#endif  // SOFTCORR_H

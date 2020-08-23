/* acqbuf.h : changed for 30 station correlator with PCI DAS */
# ifndef __ACQBUF__
# define __ACQBUF__
#include <sys/time.h>
#include "gmrt_newcorr.h"


/* Words stand for complex words stored in sizeof(int)==4bytes */
/* Doubled MaxSamples to check 16 integ in acqpul -- 12-6-01 */
/* Doubled MaxSamples to check 08 integ in acqpul -- 14-6-01 */

#define CHANNEL 2048
#define MAX_BLOCKS 8
enum { MaxPols=4, MaxSamples=(16*MAX_BLOCKS), Channels=CHANNEL,
       WordSize=sizeof(short), WordMask=0xffff,
       TotalWords=MaxPols*MaxSamples*Channels, RecSize=TotalWords*WordSize,
     };

/* Following flag bits are used for both, acq_flag of global info,
   and rec_flag with each recsize data block.
   acq_flag uses UnInitialized, Marked, AcqOver, GPSpresent
   rec_flag uses UnInitialized, Marked, BlockErr, TimeErr
*/
enum { UnInitialized=1<<0, Marked=1<<1, GoodData=1<<2, AcqOver=1<<3,
       GPSpresent=1<<8,
       BlockErr=1<<13, TimeErr=1<<14, /* Used as recoverable Err indicator  */
       SimData=1<<15
     };
enum { MaxGPS=16, MaxBLK=64, MaxRecs=8,
       ExtraWords=32, ExtraBuf=ExtraWords*WordSize
     };

enum { PC2IST, PCref, ISTref, TimeParams,
       PC2BLK=PC2IST, BLKref=ISTref, SeqParams=TimeParams
     };
# define ToIST(pctime) ((pctime )-ISTeqn[PCref ])*ISTeqn[PC2IST]+ISTeqn[ISTref]
# define ToPC(isttime) ((isttime)-ISTeqn[ISTref])/ISTeqn[PC2IST]+ISTeqn[PCref ]
/* In order to use these #defined conv., one should declare double *ISTeqn
   and make ISTeqn point to InfoTypeVar.pc2ist
*/
# define ToBLK(pctime) ((pctime )-BLKeqn[PCref ])*BLKeqn[PC2BLK]+BLKeqn[BLKref]
# define ToPCT(blk_num) ((blk_num)-BLKeqn[BLKref])/BLKeqn[PC2BLK]+BLKeqn[PCref]
 
enum { SetCount=100, SetTime, GetTime }; /* ioctl cmds for irqtimer */
typedef struct
{ unsigned short flag, count; /* flag is dummy */
  int  seq;
  long tv[2];                 /* struct timeval tv; */
} TimeType;                   /* User time info struct */
enum { TimeSz = sizeof(TimeType) };

typedef struct
{ unsigned data_flag, dbuf_seq, time_err, blocks;
  double   prev_time;
  TimeType block_time;
} RackInfoType; /* No apparent use for acqhost etc. */

typedef struct
{ unsigned rec_flag, rec_seq;
  unsigned beg_off;
  unsigned short *begp;
  double   pc_time,  rec_time;
    /* pc_time = time of day in sec, unix_time=pc_time+ref_time */ 
    /* rec_time = IST */
  struct timeval timestamp_gps;
  double blk_nano;
  int     AcqSeqNo;
  struct timeval timestamp_pc;
} RecType;

typedef struct
{ unsigned acq_flag, mark_num, dummy;
  void     *shmp;
  long     ref_time, tzoff;
  double   blk_time;
  double   pc2ist[TimeParams];
  double   pc2blk[SeqParams ];
  double   gps_val[MaxGPS], blk_val[MaxBLK];
  unsigned gps_seq[MaxGPS], blk_seq[MaxBLK];
/*TimeType gps_int[MaxGPS], blk_int[MaxBLK]; */
  int      gps_ind, blk_ind;
  RecType  rec[MaxRecs+1];
  int      rec_ind;
  double   RecStartTime[MaxRecs+1];
  int marker_offsets[3][5];
  //BeamHeaderType BeamHdr;
} GlobalInfoType;

/*
   One Single ShmKey is used to shmget ShmSize data space.
   This space is divided then for each rack.
*/
enum { ShmKey=1034, PageSize=4096,
       ShmDataOff = ((sizeof(GlobalInfoType)+ExtraBuf)/PageSize + 1)*PageSize,
       ShmDataSize= (((MaxRecs+1)*RecSize+ExtraBuf)/PageSize + 1)*PageSize,
       ShmSize    = ShmDataOff+ShmDataSize
     };

# define FFT_CYCLE CHANNEL*2
# define BLK_INTEG 8192
# define BASE_CLK 400.0e6

# endif

/*These are flag values used for diffrent conditions in the gps_func  */
typedef enum {fresh_gps=1<<0, FindExpGPS=1<<1, InitGPS=1<<2, OnlyValid=1<<3, ValidGPSFound=1<<4, LastGPSValid=1<<5} GPSflagVals;
GPSflagVals GPSflag;
//int GPSflag =  InitGPS; // this initialization is must


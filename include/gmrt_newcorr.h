#ifndef __NEWCORR_H__
#define __NEWCORR_H__
#include <time.h>
//#include <gmrt_delay_fns.h>

#ifdef __cplusplus
extern "C" {
#endif

/* corr.h : Changed on 5 Feb, 1997.
   Fundamental Requirement of structures declared here is that
   they should have same sizes and alignment on all machines.
   DOS is exempted.

   April 1999:  Extensively revised for new version, filename
                changed to newcorr.h
*/


enum { USB_130, USB_175, LSB_130, LSB_175, MAX_BANDS };
enum { RRLL, RRRL, RR__, MACMODES } ;
enum { DAS_CARD0, DAS_CARD1, DAS_CARDS,BEDAS_CARDS, GPUDAS_CARDS };   /* FishCamp Data Acquisition Cards jnc 15/Apr/05 */
enum { IndianPolar, UsbPolar, LsbPolar, UsbCopy, LsbCopy,    /* New Mode Names */
       AllU130, AllU175,AllL130, AllL175, 
       arar_arar, alal_alal, brbr_brbr, blbl_blbl,
       aral_brbl, aral_alar, brbl_blbr, arbr_albl,          /* Classic Mode Names */
       DpcMuxVals} ;
enum { NAMELEN=32, DATELEN=32 };
enum { MAX_ANTS=32, MAX_SAMPS=64, MAX_FFTS=MAX_SAMPS, MAC_CARDS=33,
       MAX_BASE=DAS_CARDS*32*MAC_CARDS, MAX_CHANS=32768, POLS=2  
     };
    /* be sure that  MAX_FFTS and MAX_SAMPS are both even numbers ! */

enum { MAX_PROJECTS=500, MAX_SCANS=100 } ; 
enum { MAX_ARRAYS=1 };
enum { MAX_BEAMS=4 };
enum { MAX_GATES=16 };
enum { LittleEndian=1, BigEndian=0 };
enum { TransitObs = 32768 } ;

enum { TimeSize=sizeof(double), WtSize = 2*sizeof(float), 
       ActiveScans=8, DataFlagSize=ActiveScans*sizeof(int)} ;


typedef struct interfaces
{
  char iface[30];
}  Interface;

typedef struct
{ char  name[4];
  unsigned char samp_id[MAX_BANDS] ;
  double bx, by, bz;  /* metres */
  double d0[MAX_BANDS], p0[MAX_BANDS];
          /*  
              samp_id is the local reference for sampler;
              corr->sampler[samp_id].dpc gives the absolute
              sampler channel number 

              scan_id is dynamic and is only locally revelant
              to a specific program, pointing to an index
              in a scan table which the program may be maintaining.
              Others have global significance for all programs

          */
} AntennaParType;

typedef struct
{ int    macs, channels, pols, sta, cntrl, statime, iabeam, pabeam1, pabeam2;
    float  iabeam_res, pabeam1_res, pabeam2_res, f_step, clock; /* Hz  */
      unsigned char dpcmux,clksel,fftmode,macmode ;  /* replaces old dummy int */
} CorrParType ;

typedef struct
{ int   antmask, samplers, baselines, channels, lta, gsb_maxchan, gsb_fstop;
  short bandmask, mode, gsb_stokes ;
  //short chan_num[MAX_CHANS] ;  /* %3% Maximum Buffer Allocated */
  unsigned short chan_num[MAX_CHANS] ;  /* %3% Maximum Buffer Allocated */ //to accomodate 64k channel
  double mjd_ref, t_unit, gsb_acq_bw, gsb_final_bw; /* (mjd_ref*t_unit), (timestamp*t_unit)  in sec */
  double dut1;       /* dut1 Assumed from IERS Bulletin D (sec) JNC06Feb09*/
} DasParType ;

typedef struct
{ char object[NAMELEN];
  struct { float i,q,u,v ; } flux ;
  double mjd0 /* fractional mjd, to which ra, dec refer  */ ;
        /*
           mjd0 refers to the epoch of ra_app,dec_app.
           Note that the timestamp in data is wrt to the global
           reference time contained in daspar->mjd_ref
        */
  double ra_app,dec_app,ra_mean,dec_mean,dra,ddec ; /* rad, rad/s */
  double freq[2], first_lo[2],bb_lo[2];   /* Hz */
  double rest_freq[2], lsrvel[2] ;  /* Hz, km/s  */
  double ch_width ;  /* Hz */
  int id, net_sign[MAX_BANDS], mode , dum1;
  unsigned int antmask; /* antennas to fringe stop */
  unsigned short bandmask, dum2; 
  short calcode, qual ;
} SourceParType ;

typedef struct
{ char   object[NAMELEN], date_obs[DATELEN];
  char   observer[NAMELEN], project[NAMELEN], code[8] ;
  double ra_date, dec_date, mjd_ref, dra, ddec, period;
  int    antmask, bandmask, flag, seq , fold_flg;
         /* we use (daspar.antmask & scanpar.antmask) */
  float  integ, f_step , dm;
  int    rf[2], first_lo[2], bb_lo[2];  /*  Hz  */
  float  BW;
  int ref_ch, i_side_band;
} ScanParType;

typedef struct
{ char code[8], observer[NAMELEN], title[NAMELEN] ;
  unsigned int antmask ;    /* antennas to record */
  unsigned short bandmask,seq;
} ProjectType ;

typedef struct
{ int status ;
  float t ;  /* program dependent meaning ! */
  ProjectType proj ;
  SourceParType source ;
} ScanInfoType ;


typedef struct { unsigned char ant_id, band, fft_id, dpc; } SamplerType;

typedef struct
{ SamplerType samp[2] ;
  unsigned char card,chip,pol,word_incr ;
     /*  e.g., RRLL pol=1? word_incr=2 represents RR component ;
               RRLL pol=0?  word_incr=2 represents LL  component ;
               RRRL pol=0? word_incr=2 represents RL component ;
               RR__ pol=0? , word_incr=1
     */
} BaseParType ;         /*   try this structure instead of BaseType */

typedef struct
{ unsigned char endian,dummy[7];
  char          version [NAMELEN   ];  /* should begin with string "CORR" */
  char          bandname[MAX_BANDS ][8];
  AntennaParType  antenna [MAX_ANTS  ];      /* Ant pos, freq & other config */
  SamplerType   sampler [MAX_SAMPS ];      /* ant, band vs. ffts           */
  BaseParType   baseline[MAX_BASE  ];      /* Pair of ant, band            */
  CorrParType   corrpar;                   /* Max. enabled mac_params      */
  DasParType    daspar;                    /* Actually useful params       */
} CorrType;


typedef struct { unsigned int ext_model,idle,stop,userflag ; } AntennaFlagType ;

typedef struct { float phase, dp, delay, dd /* sec/sec */ ; } ModelParType ;
typedef struct { float phase, dp, delay, dslope; } DataParType;
           /* units:  phase in radian, dp in radian/sec,
                      delay in sec, dslope in radians/channel
           */

typedef struct
{ double t0 ;  /* seconds wrt corr->daspar.mjd_ref */
  int ant_id, band ;
  ModelParType par ;
} ExtModelType ;

typedef struct
{
  int      BitPackFlag;            // 0/1/2 no Folding, Fixed Period, Polycos.
  int      DataBitFormat;          // 16/8/4/2 bits.
  int      DataLSBBits;          // 16/8/4/2 bits.
  int      DataBitWinSize;          // 16/8/4/2 bits.
} BitPackType;

typedef struct
{
  int      FoldingFlag;            // 0/1/2 no Folding, Fixed Period, Polycos.
  double   FoldingPeriod;
} FoldingType;

typedef struct
{
  int      DedispersionFlag;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
  double      dD_RF;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
  double      dD_DM;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
  double      dD_BW;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
  int      dD_RefChan;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
  int      dD_iSB;       // 0/1/2 no de-Dispersion, incoherent, coherent dispersion
} dDType;

typedef struct
{
  int      GattingFlag;                 // 0/1 realtime Gating ON/OFF
  int      NoOfGates;              // Number of Gates. MAX 16
  int      GtPeriod;               // Period for Gating.
  int      GtNoOfBins;             // Total no. of Bins Across whole period.
  int      GtStartBinNo[MAX_GATES];// Start bin no. for each Gate.
  int      GtStopBinNo[MAX_GATES]; // Stop bin no. for each Gate.
} GattingType;

typedef struct
{
  int      FilterFlag;              // 0/1 RFI filter OFF/ON
  int      FftLength;              // 0/1 RFI filter OFF/ON
  int      NBands;              // 0/1 RFI filter OFF/ON
  int      FilterDataType;              // 0/1 RFI filter OFF/ON
} RfiFilterType;

typedef struct
{
  int      BeamHostID;                         // Beam host ID/Rank No.
  char     BeamHostName[128];                   // Beam host ID/Rank No.
  int      PreTimeInt;                         // No of. integration No. off FFTs
  double   SampInterval;                       // Sampling interval time Pre+Post intefration.
  int      GAC_maskP1[MAX_BEAMS];             // GAC MASK pol1
  int      GAC_maskP2[MAX_BEAMS];             // GAC MASK pol2
  int      GAC_maskFp[MAX_BEAMS];             // GAC MASK pol2
  int      GAC_vlt_maskP1[MAX_BEAMS];             // GAC MASK pol1
  int      GAC_vlt_maskP2[MAX_BEAMS];             // GAC MASK pol2
  int      PostFreqInt[MAX_BEAMS];            // Post freq/chan integration process_psr pipeline.
  int      PostTimeInt[MAX_BEAMS];            // Post time integration process_psr pipeline.
  double   PrePeriod[MAX_BEAMS];              // precessed period, currently floating from ia/pa_scan.hdr
  float    DM[MAX_BEAMS];                     // DM, currently floating from ia/pa_scan.hdr
  int      BeamType[MAX_BEAMS];               // IA/PA/Voltage
  int      NStokes[MAX_BEAMS];                // TI/FULL STOKES/voltage R,L or R&L
  char     TargetCalType[MAX_BEAMS][32];      // Target pulsar or flux/phase cal.
  BitPackType BitPackParams;
  dDType dDParams;
  FoldingType FoldingParams;
  RfiFilterType RfiFilterParams;              // 0/1 RFI filter OFF/ON
//  GattingType GattingParams[MAX_BEAMS];              // 0/1 RFI filter OFF/ON
  GattingType GattingParams;              // 0/1 RFI filter OFF/ON
} BeamGenType; // general Beam header related.

typedef struct
{ short int SyncCmd;
  ProjectType Project[MAX_PROJECTS] ;
  ScanInfoType ScanTab[MAX_SCANS];
  CorrType corr;
  ScanParType   scanpar[MAX_ARRAYS];
  BeamGenType  BeamGenHdr;
} BeamHeaderType; // Full DAS + Beam header related.

# define AntennaTypeSize     sizeof(AntennaType)
# define MacFftTypeSize      sizeof(MacFftType)
# define SamplerTypeSize     sizeof(SamplerType)
# define DataParTypeSize     sizeof(DataParType)
# define CorrTypeSize        sizeof(CorrType)
# define CorrSize            sizeof(CorrType)
# define Corr2Size           16192   /* size of 128 channel, 60 samp CorrType */

typedef struct
{ int in0,in1,out0,out1 ;} IndexType ;

typedef struct 
{ double bxcd,bycd,bzsd,fixed,phase, bb_lo,freq, f_step ; 
} GeomType ;
# define DAS_H_KEY_GPTOOL 2017
# define DAS_D_KEY_GPTOOL 2018
# define DAS_H_KEY_GPTOOL_INLINE 3017
# define DAS_D_KEY_GPTOOL_INLINE 3018
# define DAS_H_KEY 1030
# define DAS_D_KEY 1031
# define DAS_H0_KEY 1032
# define DAS_D0_KEY 1033
//# define DAS_BUFSIZE 545259520 // 520 MB for GUP corr 32 ant, 4 stokes 4096 chan 4 buffs
# define DAS_BUFSIZE 2147483648 // 2GB 
# define DAS_HDRSIZE  200000

typedef struct
{ int s0,s1, card;  /* card points to relevant delay/FFT card */
   /* 
      The two relevant samplers are given by 
      daspar.samp_num[s0] and daspar.samp_num[s1]
   */
  int delay, p0, pstep0,p1,pstep1, fstc ;  /* is int ok? */
  /* Do not plant delay difference between two streams here;
     the difference must be handled as static difference
     during initialisation
  */
  float p2fstc, fstc_step ; /* is float fine : sirothia 13oct */
} FftDelayParType ;

typedef struct
{ double clock, t_update  ;
  double pc_time ;
  double t_off; /* JNC 16Dec98*/
  double delay_step, fstc_scale, nco_tick_time ;
  int cycle, seq, cards ;
            /*cycle = STA cycles between model update */
  unsigned char dpcmux,clksel,fftmode,macmode ;
  ModelParType par[MAX_SAMPS] ;
  FftDelayParType fdpar[MAX_SAMPS/2] ;
} ModelInfoType ;

typedef struct
{ int active, status, scan, scan_off;
  CorrType corr ;
  ModelInfoType model ;
  BeamHeaderType BeamHeader;
  char buf[DAS_HDRSIZE];
} DasHdrType ;

enum { BufMarked=1, BufReady=1<<1,  Rack0Bad=1<<2,Rack1Bad=1<<3,Rack2Bad=1<<4,
       Rack3Bad=1<<5,Rack4Bad=1<<6,Rack5Bad=1<<7, BufFinish=1<<8,
       MaxDataBuf=512 
     };

enum { MAX_EVENTS=50000 } ;
typedef struct
{ float t ;
  unsigned char type, cmd ;
  unsigned short seq ;
  int flag_num, scan_num ;  /* indexes on AntennaFlag and ScanInfo */
} EventLogType ;

typedef struct
{ int t_off, wt_off, par_off, data_off, data_words ;
  int par_words, wordsize ; //caution :  change from short to int
} RecOffsetType ;

typedef struct
{ int off ;
  BaseParType base ;
  char name[12] ;
} MacWordType ;

typedef struct
{ unsigned char seq, scan ;
  int status,recl, seqnum ;
  RecOffsetType off ;
  MacWordType *mac ;
  char *buf ;
} RecBufType ;
  
typedef struct
{ int active,status;
  unsigned short events,flags,starts,stops ;
  CorrType corr ;
  AntennaFlagType flag[MAX_EVENTS][MAX_BANDS] ;
  EventLogType event[MAX_EVENTS] ;
  ScanInfoType scaninfo[MAX_SCANS] ;
  RecOffsetType offset ;
} DataInfoType ;

typedef struct
{ int flag,rec,seqnum ;
  unsigned short flag_seq, newstate ;
} DataTabType;

typedef struct
{ int flag, blocksize, maxblocks, cur_block, first_block, cur_rec;
  DataTabType dtab[MaxDataBuf];
  char buf[DAS_BUFSIZE] ;
} DataBufType ;

#define CHANNEL 2048
#define FFTLEN (2*CHANNEL)
#define NCHAN 64
#define CORRLEN (NCHAN*FFTLEN)
#define NUM_ANT 1     // No of acquation node          // %2% New variable for dual pol
#define NUM_ACQ 1     // No of acquation node          // %2% New variable for dual pol

#define BandWidth 400000000 //change this acording to decimation	~Commented By Sandeep
//#define BandWidth 200000000 //change this acording to decimation	~Added By Sandeep

#define BLOCKTIME 0.67108864
//#define BLOCKTIME (8/32.0)
//#define SAMP_CLK 0.0000000025
//#define SAMP_CLK 0.000000005
#define SAMP_CLK 0.00000000125

/* commented out global variables AJ

static float phase_ti[NUM_ACQ*NCHAN*FFTLEN/2];
static float dphase_t0[NUM_ACQ*NCHAN*FFTLEN/2];


CorrType corr;
DasParType daspar;
ModelParType mpar[MAX_SAMPS];  // changed single struct to array for all antennas
SourceParType source;
ModelInfoType model ;  // Extra Added GSB %%

CorrType *Corr;    //Defined in parallerl_correlator.h  'corr'
ModelInfoType *Model ;
DasHdrType  *dHdr ;
DataBufType *dBuf ;
*/


double mjd_cal(struct tm *t);
double lmst(double mjd);
void calModelPar(double tm, CorrType *corr, ModelParType *mpar, SourceParType *source, int *antmask);

#ifdef __cplusplus
}
#endif

#endif

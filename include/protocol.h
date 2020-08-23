/* protocol.h */
# ifndef _DAS_PROT_
# define _DAS_PROT_
/***************
   October 1996 : New features/revisions in AcqSrv,CorrSrv & related commands;
                      still retains some obsolete definitions

   96oct27      : Included MsgBufType definition;
***************/

enum { BigEnd=0, LittleEnd=1 };   /* definition MUST AGREE with corr.h */
typedef enum { AnyNode, CorrHost, PulsarHost, InpNode, CmdNode,
               AcqNode, CorrNode, DataLogNode, AllNodes, Nodes 
             } NodeType;
typedef enum { AckMsg, DAS, DASIA, DASPA, NodeSrv, InfoCmd, CliReq, AcqSrv, 
	        CorrSrv, DASPMT, Services
             } ServiceType;
typedef enum { Fopen, Fclose, Fread, Fwrite, Fgets, Fputs, Fops } FopType;
typedef enum { Endian=1, Request=2, Reply=4, Success=8, Failure=16,
               Complete=32, Incomplete=64
             } FlagType;
typedef enum { Init, Finish, Start, Stop, Suspend, Resume, Monitor, Gate,
               UserCmds, NoCmd
             } UserCmdType;

typedef enum { DAS_INIT, DAS_FINISH, DAS_START, DAS_STOP, DAS_SUSPEND,
               DAS_RESUME, DAS_MON,DAS_GATE, DasCmds, STARTPROJ,STOPPROJ, ADDPROJ,
	       DELETEPROJ,DAS_UNINIT = DasCmds
             } DasCmdType; /* This also specifies DAS state */

/**** For sockcmd ***/
/**
typedef enum { DAS_INIT, DAS_FINISH, STARTPROJ,STOPPROJ,
               DAS_SUSPEND, DAS_RESUME, DAS_MON, ADDPROJ,DELETEPROJ,
               D0INC, P0INC, USE,IDLE,REALLOC, SET_FLAG,CLEAR_FLAG,
               DasCmds, DAS_UNINIT = DasCmds
             } DasCmdType;*/ /* This also specifies DAS state for sockcmd*/

typedef enum { MSG_PRINT, CliReqs } CliReqType;
typedef enum { ACQ_DATA, Acqcmds } AcqCmdType ;

typedef enum { SetDly, ResetDpc, SetFringe, InitFft, Fstc, CorrCmds 
             } CorrCmdType ;

typedef unsigned char  UNC;
typedef unsigned short UNS;
typedef short         SHRT;
# ifdef __MSDOS__
typedef unsigned long  UNL;
# else
typedef unsigned int   UNL;
# endif
typedef struct ProtTag
{ UNC  origin;   /* MSG Originating station */
  UNC  dest;     /* Destination station */
  UNC  type;     /* The type of req, eg. Sh_com, RPC, FileIO, DAS, etc. */
  UNC  cmd;      /* Actual service name or shell command. */
  UNC  arg[4];   /* Used as a parameter of any type, */
                 /* that can be coded in 4 bytes. */
  UNC  id;       /* Request no, from the origin, wraps at 2^8 */
  UNC  flag;     /* Flags of queries or replies */
  UNS  usrflag;  /* Any short val or set of flags for user */
  UNL  len;      /* Length of the packet that follows */
} ProtocolType;

# define ProtSz  sizeof(ProtocolType)

enum { Host = CorrHost };
//DasCmdType DasState = DAS_UNINIT;
/*
void ConsProt(int origin, int dest, int type, int cmd, int id,
     int flag, short usrflag, UNL len, ProtocolType *p)
*/
# define ConsProt(org, dst, cmdtype, cmdval, seq, flg, usrflg, size, p) \
  if (p)                          \
  { (p)->origin = (UNC) (org);    \
    (p)->dest   = (UNC) (dst);    \
    (p)->type   = (UNC) (cmdtype);\
    (p)->cmd    = (UNC) (cmdval); \
    (p)->id     = (UNC) (seq);    \
    (p)->flag   = (UNC) (flg);    \
    (p)->usrflag= (UNS) (usrflg); \
    (p)->len    = (UNL) (size);   \
  }

# define SwapShort(x) (((x) / 256) | (((x) & 255) << 8))
# define SwapLong(x) ((((x) & 255) << 24) | (((x) & (255 << 8)) << 8) | \
                     (((x) & (255 << 16)) >> 8) | (((x) & (255 << 24)) >> 24))
# define SwapArg(x) {UNC *p=x,t=*p;*p=p[3];p[3]=t;t=p[1];p[1]=p[2];p[2]=t;}

# define ProtMaxBuf  16384  /* Bytes */
# define ProtBufSize 1024   /* Bytes */

enum { InpMsgQ=105, CmdMsgQ=101, MsgPerm=0766 } ;
enum { MsgBufSz=ProtBufSize-sizeof(UNL), MsgSz=MsgBufSz-ProtSz };
enum { SockCmdAcq30=1,Acq30SockCmd=2};
/**
typedef struct
{ long msglevel;
  ProtocolType prot;
  char buf[MsgSz];
} MsgBufType;

char *ServiceName[Services] =
{ "AckMsg", "DAS","DASIA","DASPA","NodeSrv", "InfoCmd", "CliReq", "AcqSrv", "CorrSrv","DASPMT" };
char *NodeName[Nodes] =
{ "AnyNode","CorrHost","PulsarHost","InpNode","CmdNode","AcqNode","CorrNode",
  "DataLogNode", "AllNodes"
};
char *StateName[DasCmds+1] =
{ "Initialized","Finished","Acq_started","Acq_stopped","Suspended","Resumed", 
  "", "UnInitialized"
};
char *UserCmdName[UserCmds] =
{"Init","Finish","Start","Stop","Suspend","Resume","Monitor","Gate" };

char *CorrCmdName[CorrCmds]  =
{ "SetDelay","ResetDpc", "SetFringe", "InitFft", "Fstc" };

char *DasCmdName[2*(DasCmds+1)] =
{"Init","Finish","StartProj","StopProj", "Suspend","Resume","Monitor",
 "AddProj","DelProj","D0inc", "P0inc", "UseAnt","IdleAnt",
 "ReallocAnt","FlagAnt","ClearAnt" };
**/
# endif

#include "gmrt_newcorr.h"
#include "protocol.h"
#include"externalLibraries.h"
#include "frb_shm.h"
#ifndef CLASS_CORRELATOR
#define CLASS_CORRELATOR
class Correlator
{	
	public:
		void initializeReadSHM();
		void initializeReadSHM_FRB(char fileSHM);
		int initializeWriteSHM();
		int initializeWriteSHM_FRB(char fileSHM);
		void writeToSHM_FRB(unsigned char* rawData);
		void writeToSHM(unsigned short int* rawData);
		void readFromSHM_FRB(unsigned char* rawData);
		void readFromSHM(unsigned short int* rawData);
		void writeToSHM(short int* rawData,char* header);
		void copyHeaderInfo();		
		Correlator(int _nchan,float _sampling);
		Correlator(DasHdrType *_dataHdrRead ,DataBufType *_dataBufferRead);
	private:
		static DasHdrType*	dataHdrWrite;
		static DataBufType*	dataBufferWrite;
		static DataHeader*	dataHdrWriteFRB;
		static DataBuffer*	dataBufferWriteFRB;
		static DataTabType*	dataTabWrite;

		static int		recNumWrite;	

		static DasHdrType*	dataHdrRead;
		static DataBufType*	dataBufferRead;

		static DataHeader*	dataHdrReadFRB;
		static DataBuffer*	dataBufferReadFRB;


		static DataTabType*	dataTabRead;
		static int		recNumRead;	
		static long int		currentReadBlock;
		int			DataOff;
		char			debug;
		int			nchan;
		float			sampling;
};
#endif

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import SymLogNorm
from matplotlib.backends.backend_pdf import PdfPages
import matplotlib.ticker
import os.path


def plotAll():
	gptoolinfile=np.loadtxt("./gptool.in",comments=["#","--"],usecols=[0],dtype="string")

	data=np.loadtxt("./intensity_summary.gpt")
	nblocks=np.shape(data)[0]
	nchan=np.shape(data)[1]-1
	
	plt.xlim((0,np.shape(data)[0]))
	plt.ylim((0,np.shape(data)[1]))

	flagPercents=getFlagPercents(nchan,nblocks)
	with PdfPages('summary.gpt.pdf') as pdf:
		fig, ax = plt.subplots(figsize=(10,5.27))
		ax.set_title("%s sec averaged normalized intensity in each channel"%gptoolinfile[17],y=1.1)
		plotIntensity(fig,ax,data,gptoolinfile,log=True)
		pdf.savefig()

		fig, ax = plt.subplots(figsize=(10,5.27))
		ax.set_title("Flagging summary. Total flagged: %.1f  %% "%np.mean(flagPercents),y=1.1)
		plotIntensity(fig,ax,flagPercents,gptoolinfile,log=False)
		pdf.savefig()

		fig, ax = plt.subplots(figsize=(10,5.27))
		ax.set_title("Appropiately scaled folded profiles"%np.mean(flagPercents),y=1.1)
		plotProfile(ax)
		pdf.savefig()
def getFlagPercents(nchan,nblocks):
	if(os.path.isfile("flag_stats.gpt")):
		timeflags=np.loadtxt("flag_stats.gpt")
		return timeflags.reshape((nblocks,nchan))
		
	else:
		return np.zeros((nblocks,nchan))


'''
def getFlagPercents(blockEdges,nchan,nblocks):
	if(os.path.isfile("timeflag.gpt")):
		timeflags=np.fromfile("timeflag.gpt",dtype=np.uint8)
		blockAvg=np.zeros((nblocks,nchan))
		blockEdges=np.cumsum(blockEdges).astype("int64")
		blockAvg[0]=np.mean(timeflags[:blockEdges[0]])
		for i in range(1,nblocks):
			blockAvg[i,:]=np.mean(timeflags[blockEdges[i-1]:blockEdges[i]])
	if(os.path.isfile("chanflag.gpt")):
		freqflags=np.fromfile("chanflag.gpt",dtype=np.uint8).reshape((nblocks,nchan)).astype("float64")
	
	
	
	if(os.path.isfile("chanflag.gpt") and os.path.isfile("timeflag.gpt")):
		freqflags[freqflags<0.9]=blockAvg[freqflags<0.9]
	elif(os.path.isfile("timeflag.gpt")):
		freqflags=blockAvg
	else:
		freqflags=np.zeros((nblocks,nchan))
	return 100*(freqflags)
'''
def plotIntensity(fig,ax0,data,gptoolinfile,log=True):

	nblocks=np.shape(data)[0]
	nchan=np.shape(data)[1]-1
	if(log):
		im=ax0.imshow((data[:,1:].T),extent=[0,nblocks,0,nchan],
                                              vmin=0.9, vmax=1.1,aspect=0.8*nblocks/float(nchan),origin='lower')
	else:
		im=ax0.imshow(data[:,1:].T,extent=[0,nblocks,0,nchan],vmin=0, vmax=np.max(data[:,1:]),aspect=0.8*nblocks/float(nchan),origin='lower')
	fig.colorbar(im,  pad=0.2)
	if(int(gptoolinfile[5])==-1):
		
		lowfreq=float(gptoolinfile[3])+float(gptoolinfile[4])
		highfreq=float(gptoolinfile[3])
		f = lambda x : highfreq-x*float(gptoolinfile[4])/nchan
	else:
		lowfreq=float(gptoolinfile[3])
		highfreq=float(gptoolinfile[3])+float(gptoolinfile[4])
		f = lambda x : lowfreq+x*float(gptoolinfile[4])/nchan
	
	ax0.set_xticks(np.arange(0,nblocks+1,int(nblocks/10)))
	ax0.set_yticks(np.arange(0,nchan+1,int(nchan/8)))
	ax0.set_yticklabels(np.arange(0,nchan+1,int(nchan/8)))
	ax0.set_ylabel("channel #")
	ax0.set_xlabel("block #")
	
	
	

	ax1=ax0.twinx()
	ax1.set_yticks(np.arange(0,nchan+1,int(nchan/8)))
	ax1.set_yticklabels(np.arange(lowfreq,highfreq+int(gptoolinfile[5]),int(gptoolinfile[5])*int(float(gptoolinfile[4])/8)))
	ax1.set_ylabel("frequency (MHz)")

	ax2=ax1.twiny()
	ax2.set_xticks(np.arange(0,nblocks+1,int(nblocks/10)))
	ax2.set_xticklabels(np.arange(0,(nblocks+1)*float(gptoolinfile[17]),int(nblocks*float(gptoolinfile[17])/10)))
	ax2.set_xlabel("time (sec)")

def plotProfile(ax):
	
	filteredProfile=np.loadtxt("profile_filtered.gpt")
	unfilteredProfile=np.loadtxt("profile_unfiltered.gpt")
	filteredNoise=np.take(filteredProfile[:,1],np.arange(np.argmax(filteredProfile)+len(filteredProfile)/4,np.argmax(filteredProfile)+3*len(filteredProfile)/4),mode='wrap')
	unfilteredNoise=np.take(unfilteredProfile[:,1],np.arange(np.argmax(filteredProfile)+len(filteredProfile)/4,np.argmax(filteredProfile)+3*len(filteredProfile)/4),mode='wrap')
	filteredProfile[:,1]=(filteredProfile[:,1]-np.mean(filteredNoise))
	filteredProfile[:,1]/=np.max(filteredProfile[:,1])
	unfilteredProfile[:,1]=(unfilteredProfile[:,1]-np.mean(unfilteredNoise))
	unfilteredProfile[:,1]/=np.max(unfilteredProfile[:,1])
	ax.plot(unfilteredProfile[:,1],label="unfiltered")
	ax.plot(filteredProfile[:,1],label="filtered")
	ax.set_xlabel("bin #")
	ax.set_ylabel("intensity variation w.r.t mean")
	ax.legend()
	
plotAll()

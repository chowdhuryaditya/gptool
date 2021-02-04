CUR_DIR=$(shell pwd)
#########################
PGPLOT_INCL=#-I/pathtopgplot/src/pgplot
PGPLOT_LIB=#-L/pathtopgplot/usr/src/pgplot
OPENMP_INCL=
OPENMP_LIB=
ADDITIONAL_CFLAGS=
ADDITIONAL_FCFLAGS=
#########################

CC=g++
FC=gfortran

CFLAGS_DEBUG= -g -w


CFLAGS=-fPIC -c $(CFLAGS_DEBUG)  -I$(CUR_DIR)/include/  $(OPENMP_INCL) $(PGPLOT_INCL) $(ADDITIONAL_CFLAGS) 

FCFLAGS=-L$(LD_LIBRARY_PATH) -fPIC -L$(CUR_DIR)/lib/ -lgptool $(PGPLOT_LIB) -lcpgplot -lpgplot   $(OPENMP_LIB) -fopenmp -L$(ADDITIONAL_FCFLAGS) -lgcc -lm -lc -lstdc++ -lX11 -lgfortran

all: libraries
	$(CC) $(CFLAGS) gptool.cpp -fopenmp
	$(FC) -o gptool gptool.o  $(FCFLAGS) 
	make -C fileToSHM CFLAGS=' $(PGPLOT_INCL)'
	rm -rf *.o

libraries:
	make -C lib CC="$(CC)" CFLAGS=' $(PGPLOT_INCL) $(CFLAGS)'



clean:
	make -C lib clean
	make -C fileToSHM clean
	rm -rf gptool

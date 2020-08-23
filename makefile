CUR_DIR=$(shell pwd)
#########################
PGPLOT_INCL= #-I/Data/astrosoft/pgplot
PGPLOT_LIB= #-L/Data/astrosoft/pgplot
OPENMP_INCL= #-I/path/to/dir/with/omp.h
OPENMP_LIB= #-L/path/to/dir/with/libomp.so
ADDITIONAL_CFLAGS=
ADDITIONAL_FCFLAGS=
#########################

CC=g++
FC=gfortran

CFLAGS_DEBUG= -g -w


CFLAGS=-fPIC -c $(CFLAGS_DEBUG)  -I$(CUR_DIR)/include/  $(OPENMP_INCL) $(PGPLOT_INCL) $(ADDITIONAL_CFLAGS) 

FCFLAGS=-L$(LD_LIBRARY_PATH) -fPIC -L$(CUR_DIR)/lib/ -lgptool $(PGPLOT_LIB) -lcpgplot -lpgplot $(OPENMP_LIB) -fopenmp -L$(ADDITIONAL_FCFLAGS) -lgcc -lm -lc -lstdc++ -lX11 -lgfortran

all: libraries
	$(CC) $(CFLAGS) gptool.cpp -fopenmp
	$(FC) -o gptool gptool.o  $(FCFLAGS) 
	rm -rf *.o

libraries:
	make -C lib CC="$(CC)" CFLAGS='$(CFLAGS)'


clean:
	make -C lib clean
	rm -rf gptool

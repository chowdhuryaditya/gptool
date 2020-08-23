LIB_OBJ = libinformation.o libbasicanalysis.o libplot.o libadvancedanalysis.o librfifiltering.o libshm.o libaquiredata.o 


all: $(LIB_OBJ)
	ar rc libgptool.a $^
	@echo "Done compiling libraries"
%.o : 
	$(CC) $(@:.o=.cpp) -o $@ $(CFLAGS) 
	
clean:
	rm -rf *.o
	rm -rf *.a

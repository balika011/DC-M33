PSPSDK=$(shell psp-config --pspsdk-path)
LoadCoreForKernel_OBJS = LoadCoreForKernel_0000.o LoadCoreForKernel_0001.o

%.o: LoadCoreForKernel.S
	psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(LoadCoreForKernel_OBJS)
	psp-ar cru libloadcore_kernel.a $(LoadCoreForKernel_OBJS)
	psp-ranlib libloadcore_kernel.a

clean:
	rm -f *.o
	rm -f *.a

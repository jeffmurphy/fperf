# Build Notes
#
# FLAG			REQUIRES
# -DLINUX		-rdynamic
# -DTHREAD_SAFE		-D_REENTRANT -lpthread 
# -DDEMANGE_GNU_CXX	-liberty

    CXX = g++
    CCC = $(CXX)
     CC = gcc
     AS = /usr/bin/as
     OS = -DDEMANGLE -DLINUX -rdynamic #-DSOLARIS
#-DDEMANGLE_GNU_CXX 
 CFLAGS = -g -I. -DDO_STACK_TRACE \
	  $(OS) -DTHREAD_SAFE -D_REENTRANT # -DDEBUG
 CXXFLAGS = $(CFLAGS)
   LIBS = -L. -lfperf -lpthread -ldl -liberty

FPERFSRC = setup.c exit.c lock.c start.c end.c fptime.c list.c
FPERFOBJ = $(FPERFSRC:%.c=%.o)
FPERFHDR = $(FPERFSRC:%.c=%.h)
FPERFASM = $(FPERFSRC:%.c=%.s)

all:	libfperf.a t1 t2 t3 t4 t4 t5

t1:	t1.o libfperf.a
	$(CC) $(CFLAGS) -o t1 t1.o $(LIBS)

t2:	t2.o libfperf.a
	$(CC) $(CFLAGS) -o t2 t2.o $(LIBS)

t3:	t3.o libfperf.a
	$(CXX) $(CFLAGS) -o t3 t3.o $(LIBS)

t4:	t4.o libfperf.a
	$(CC) $(CFLAGS) -o t4 t4.o $(LIBS)

t5:	t5.o libfperf.a
	$(CC) $(CFLAGS) -o t5 t5.o $(LIBS)

libfperf.a:	$(FPERFOBJ) $(FPERFHDR)
	@echo "Building libfperf.a .."
	ar rv libfperf.a $(FPERFOBJ)

lock.o:	lock.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S lock.c -o lock.s
	@$(AS) -Qy -o lock.o lock.s
	@rm -f lock.s

setup.o:	setup.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S setup.c -o setup.s
	@$(AS) -Qy -o setup.o setup.s
	@rm -f setup.s

exit.o:	exit.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S exit.c -o exit.s
	@$(AS) -Qy -o exit.o exit.s
	@rm -f exit.s

start.o:	start.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S start.c -o start.s
	@$(AS) -Qy -o start.o start.s
	@rm -f start.s

end.o:	end.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S end.c -o end.s
	@$(AS) -Qy -o end.o end.s
	@rm -f end.s

fptime.o:	fptime.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S fptime.c -o fptime.s
	@$(AS) -Qy -o fptime.o fptime.s
	@rm -f fptime.s

list.o:	list.c $(FPERFHDR)
	$(CC) $(CFLAGS) -S list.c -o list.s
	@$(AS) -Qy -o list.o list.s
	@rm -f list.s

clean:
	rm -f *.o *.s *.S_orig *~ *.a t? core fperf.dat.*




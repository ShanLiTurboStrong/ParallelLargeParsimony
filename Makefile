CC=g++ -m64
CFLAGS=-g -O3 -Wall -std=c++11 -Iobjs/
ISPC=ispc

OMP=-fopenmp -DOMP

ISPCFLAGS=-O2 --target=avx1-i32x8 --arch=x86-64

APP_NAME=parsimony-omp-ispc
OBJDIR=objs

SRCDIR=src

LDFLAGS= -lm

CFILES_SEQ = src/crun-seq.cpp
CFILES_PAR = src/crun-omp.cpp	
HFILES_SEQ = src/util.h src/SmallParsimony.hpp src/LargeParsimony.hpp
HFILES_PAR = src/util.h src/LargeParsimony-omp.hpp


default: crun-seq $(APP_NAME)

.PHONY: dirs clean

dirs: 
	mkdir -p $(OBJDIR)/

clean:
	rm -rf $(OBJDIR) *.pyc *~ $(APP_NAME) *.dSYM *.tgz crun-seq crun-omp

OBJS=$(OBJDIR)/crun-omp.o $(OBJDIR)/parsimony_ispc.o

$(APP_NAME): dirs $(OBJS)
	$(CC) $(CFLAGS) $(OMP) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/crun-omp.o: $(CFILES_PAR) $(HFILES_PAR) $(OBJDIR)/parsimony_ispc.h
	$(CC) $< $(CFLAGS) $(OMP) -c -o $@

$(OBJDIR)/%_ispc.h $(OBJDIR)/%_ispc.o: $(SRCDIR)/%.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h

crun-seq: $(CFILES_SEQ) $(HFILES_SEQ) 
	$(CC) $(CFLAGS) -o crun-seq $(CFILES_SEQ) $(LDFLAGS)

# crun-omp: $(CFILES_PAR) $(HFILES_PAR)
# 	$(CC) $(CFLAGS) $(OMP) -o crun-omp $(CFILES_PAR) $(LDFLAGS)





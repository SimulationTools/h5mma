UNAME := $(shell uname)

# Linux specific paths
ifeq ($(UNAME), Linux)
MLINKDIR := /cluster/MATHEMATICA/7.0.1/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions
HDF5DIR  := /cluster/hdf5/1.8.1
endif

# Mac OSX specific paths
ifeq ($(UNAME), Darwin)
MLINKDIR := /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/CompilerAdditions
HDF5DIR  := /usr/local/hdf5-1.8.5
endif

INCS = -I${MLINKDIR} -I${HDF5DIR}/include
LIBS = -L${MLINKDIR} -L${HDF5DIR}/lib

MPREP = ${MLINKDIR}/mprep
MCC   = ${MLINKDIR}/mcc

PKGFILES = MacOSX-x86-64 Linux-x86-64 h5mma.m h5mma.nb phi.0.xy.h5 Kernel

all : h5mma

h5mma : h5mmatm.cc h5mma.cc h5wrapper.cc
	${MCC} -n -Wall $(INCS) h5mma.cc h5mmatm.cc h5wrapper.cc $(LIBS) -lhdf5 -xo h5mma
	cp -R h5mma/* ./
	rm -r h5mma

h5mma.zip : 
	zip -r h5mma.zip ${PKGFILES}
	
h5mma.tar.gz : 
	tar czvf h5mma.tar.gz ${PKGFILES}
	
h5mma.tar.bz2 : 
	tar cjvf h5mma.tar.bz2 ${PKGFILES}

h5mmatm.cc : h5mma.tm
	${MPREP} $? -o $@

.PHONY : clean
clean :
	-rm -rf h5mmatm.cc MacOSX-x86-64 Linux-x86-64 h5mma.zip h5mma.tar.gz h5mma.tar.bz2 *.o h5mma

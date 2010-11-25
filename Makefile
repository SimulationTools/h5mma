UNAME := $(shell uname)

# Linux specific paths (for damiana)
ifeq ($(UNAME), Linux)
MLINKDIR   := /cluster/MATHEMATICA/7.0.1/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions
HDF5DIR    := /cluster/hdf5/1.8.4-patch1
INSTALLDIR := ${HOME}/.Mathematica/Applications/h5mma
EXEDIR     = Linux-x86-64
endif

# Mac OSX specific paths
ifeq ($(UNAME), Darwin)
MLINKDIR   := /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/CompilerAdditions
HDF5DIR    := /usr/local/hdf5-1.8.5-patch1
INSTALLDIR := ${HOME}/Library/Mathematica/Applications/h5mma
EXEDIR     = MacOSX-x86-64
endif

INCLUDES = -I${MLINKDIR} -I${HDF5DIR}/include
LDFLAGS  = -Wl,-rpath,${MLINKDIR} -Wl,-rpath,${HDF5DIR}/lib -L${HDF5DIR}/lib
CFLAGS   = -n -Wall -Wno-write-strings -O3 
MPREP = ${MLINKDIR}/mprep
MCC   = ${MLINKDIR}/mcc

PKGFILES = ${EXEDIR} h5mma.m Kernel

all : h5mma

h5mma : h5mmatm.cc h5mma.cc h5wrapper.cc h5wrapper.h
	${MCC} $(CFLAGS) $(INCLUDES) h5mma.cc h5mmatm.cc h5wrapper.cc $(LDFLAGS) -lhdf5 -xo h5mma
	@cp -R h5mma/* ./
	@rm -r h5mma

h5mma.zip : 
	@zip -r h5mma.zip ${PKGFILES}
	
h5mma.tar.gz : 
	@tar -czf h5mma.tar.gz ${PKGFILES}
	
h5mma.tar.bz2 : 
	@tar -cjf h5mma.tar.bz2 ${PKGFILES}

install : h5mma.tar.gz
	@echo "Installing in " ${INSTALLDIR}
	@mkdir -p ${INSTALLDIR}
	@tar zxf h5mma.tar.gz -C ${INSTALLDIR}

h5mmatm.cc : h5mma.tm
	${MPREP} $? -o $@

.PHONY : clean
clean :
	@rm -rf h5mmatm.cc MacOSX-x86-64 Linux-x86-64 h5mma.zip h5mma.tar.gz h5mma.tar.bz2 *.o h5mma

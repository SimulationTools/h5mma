
MLINKDIR = /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit
SYS = MacOSX
CADDSDIR = ${MLINKDIR}/CompilerAdditions

INCDIR = ${CADDSDIR}
LIBDIR = ${CADDSDIR}

H5DIR = /usr/local/hdf5-1.8.5

MPREP = ${CADDSDIR}/mprep
MCC = ${CADDSDIR}/mcc

PKGFILES = MacOSX-x86-64 h5mma.m h5mma.nb phi.0.xy.h5 Kernel

all : h5mma

h5mma : h5mmatm.cc h5mma.cc
	${MCC} -n -Wall -I$(H5DIR)/include -I$(CADDSDIR) h5mma.cc h5mmatm.cc -L$(H5DIR)/lib -lhdf5_cpp -lhdf5 -xo h5mma
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
	-rm -rf h5mmatm.cc MacOSX-x86-64 h5mma.zip h5mma.tar.gz h5mma.tar.bz2 *.o

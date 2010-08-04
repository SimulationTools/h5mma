
MLINKDIR = /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit
SYS = MacOSX
CADDSDIR = ${MLINKDIR}/CompilerAdditions

INCDIR = ${CADDSDIR}
LIBDIR = ${CADDSDIR}

H5DIR = /Users/ian/Software/hdf5-1.8.1

MPREP = ${CADDSDIR}/mprep

all : h5mma

h5mma : h5mmatm.o h5mma.o
	${CXX} -I${INCDIR} -I$(H5DIR)/include h5mmatm.o h5mma.o -L${LIBDIR} -L$(H5DIR)/lib -lMLi3 -lhdf5_cpp -lhdf5 -o $@

.cc.o :
	${CXX} -c -I${INCDIR} -I$(H5DIR)/include $<

h5mmatm.cc : h5mma.tm
	${MPREP} $? -o $@

.PHONY : clean
clean :
	-rm -f h5mma h5mmatm.o h5mma.o h5mmatm.cc

# mcc = /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/CompilerAdditions/mcc

# #addtwo.tm.c:	addtwo.tm
# #	$(mprep) addtwo.tm  -o addtwo.tm.c

# addtwo:	addtwo.c
# 	$(mcc) addtwo.tm addtwo.c -o addtwo

# h5mma:	h5mma.cc
# 	$(mcc) h5mma.tm h5mma.cc -o h5mma -I/Users/ian/Software/hdf5-1.8.1/include -L/Users/ian/Software/hdf5-1.8.1/lib -lhdf5_cpp -lhdf5

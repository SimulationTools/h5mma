
UNAME := $(shell uname)
HOSTNAME := $(shell hostname)

# Override any of the below paths in a make.defs file
-include make.defs

# Damiana specific paths
ifeq ($(HOSTNAME), login-damiana)
MLINKDIR   ?= /cluster/MATHEMATICA/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions
HDF5DIR    ?= /cluster/hdf5/1.8.4-patch1
endif

ifeq ($(UNAME), Linux)
# Linux specific paths
INSTALLDIR ?= ${HOME}/.Mathematica/Applications/h5mma
EXEDIR     = Linux-x86-64
else ifeq ($(UNAME), Darwin)
  # Mac OSX specific paths
  MLINKDIR   ?= /Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/CompilerAdditions
  INSTALLDIR ?= ${HOME}/Library/Mathematica/Applications/h5mma
  EXEDIR     = MacOSX-x86-64
  ifneq ($(wildcard /opt/local/lib/libhdf5.dylib),)
    # MacPorts
    HDF5DIR    ?= /opt/local
  else ifneq ($(wildcard /sw/lib/libhdf5.dylib),)
    # Fink
    HDF5DIR    ?= /sw
endif
endif

INCLUDES = -I${MLINKDIR} -I${HDF5DIR}/include
LDFLAGS  = -Wl,-rpath,${MLINKDIR} -Wl,-rpath,${HDF5DIR}/lib -L${HDF5DIR}/lib
CFLAGS   = -n -Wall -Wno-write-strings -O3 
MPREP = ${MLINKDIR}/mprep
MCC   = ${MLINKDIR}/mcc

PKGFILES = ${EXEDIR} h5mma.m Kernel doc COPYING COPYING.LESSER COPYING.HDF5 README

all : h5mma

h5mma : h5mmatm.cc h5mma.cc h5wrapper.cc h5wrapper.h
	@if [ ! -d $(HDF5DIR) ]; then echo "HDF5 not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@if [ ! -d $(MLINKDIR) ]; then echo "MathLink not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	${MCC} $(CFLAGS) $(INCLUDES) h5mma.cc h5mmatm.cc h5wrapper.cc $(LDFLAGS) -lhdf5 -xo h5mma
	@cp -R h5mma/* ./
	@rm -r h5mma

h5mma-osx-hdf5static : h5mmatm.cc h5mma.cc h5wrapper.cc h5wrapper.h
	@if [ ! -d $(HDF5DIR) ]; then echo "HDF5 not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@if [ ! -d $(MLINKDIR) ]; then echo "MathLink not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@rm -rf MacOSX-x86-64
	@mkdir MacOSX-x86-64
	g++ $(CFLAGS) $(INCLUDES) -c h5wrapper.cc
	g++ $(CFLAGS) $(INCLUDES) -c h5mma.cc
	g++ $(CFLAGS) $(INCLUDES) -c h5mmatm.cc
	g++ h5mma.o h5mmatm.o h5wrapper.o -F$(MLINKDIR) -framework mathlink $(HDF5DIR)/lib/libhdf5.a $(HDF5DIR)/lib/libsz.a $(HDF5DIR)/lib/libz.a -o MacOSX-x86-64/h5mma
	install_name_tool -change @executable_path/../Frameworks/mathlink.framework/Versions/3.16/mathlink $(MLINKDIR)/mathlink.framework/mathlink MacOSX-x86-64/h5mma

h5mma-osx-hdf5mmastatic : h5mmatm.cc h5mma.cc h5wrapper.cc h5wrapper.h
	@if [ ! -d $(HDF5DIR) ]; then echo "HDF5 not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@if [ ! -d $(MLINKDIR) ]; then echo "MathLink not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@rm -rf MacOSX-x86-64
	@mkdir MacOSX-x86-64
	g++ $(CFLAGS) $(INCLUDES) -c h5wrapper.cc
	g++ $(CFLAGS) $(INCLUDES) -c h5mma.cc
	g++ $(CFLAGS) $(INCLUDES) -c h5mmatm.cc
	g++ h5mma.o h5mmatm.o h5wrapper.o $(MLINKDIR)/libMLi3.a $(HDF5DIR)/lib/libhdf5.a $(HDF5DIR)/lib/libsz.a $(HDF5DIR)/lib/libz.a -framework CoreFoundation -framework Foundation -o MacOSX-x86-64/h5mma

h5mma.zip : 
	@zip -r h5mma.zip ${PKGFILES}

h5mma.tar.gz : 
	@tar -czf h5mma.tar.gz ${PKGFILES}

h5mma.tar.bz2 : 
	@tar -cjf h5mma.tar.bz2 ${PKGFILES}

h5mma.dmg : h5mma-osx-hdf5static
	@rm -f h5mma.sparseimage h5mma.dmg
	@hdiutil convert empty.dmg -format UDSP -o h5mma
	@hdiutil mount h5mma.sparseimage
	@cp -R ${PKGFILES} /Volumes/h5mma/h5mma
	@hdiutil eject /Volumes/h5mma
	@hdiutil convert h5mma.sparseimage -format UDBZ -o h5mma.dmg

install : h5mma.tar.gz
	@echo "Installing in " ${INSTALLDIR}
	@mkdir -p ${INSTALLDIR}
	@tar zxf h5mma.tar.gz -C ${INSTALLDIR}

h5mmatm.cc : h5mma.tm
	${MPREP} $? -o $@

.PHONY : clean
clean :
	@rm -rf h5mmatm.cc MacOSX-x86-64 Linux-x86-64 h5mma.zip h5mma.tar.gz h5mma.tar.bz2 *.o h5mma h5mma.dmg h5mma.sparseimage

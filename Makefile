
UNAME := $(shell uname)
ARCH := $(shell uname -m)
HOSTNAME := $(shell hostname)

# Override any of the below paths in a make.defs file
-include make.defs

CXX ?= g++

ifeq ($(UNAME), Linux)
  # Linux specific paths
  ifeq ($(ARCH), x86_64)
    EXEDIR     = Linux-x86-64
    MATHLIBS   = -L${MLINKDIR} -lML64i4 -lrt
  else
  ifeq ($(ARCH), i686)
    EXEDIR     = Linux
    MATHLIBS   = -L${MLINKDIR} -lML32i4 -lrt
  endif
  endif
else
ifeq ($(UNAME), Darwin)
  # Mac OSX specific paths
  MLINKDIR   ?= /Applications/Mathematica.app/Contents/SystemFiles/Links/MathLink/DeveloperKit/MacOSX-x86-64/CompilerAdditions
  EXEDIR     = MacOSX-x86-64
  MATHLIBS   = $(MLINKDIR)/libMLi4.25.a # or "-F$(MLINKDIR) -framework mathlink" for dynamic linking
  ifneq ($(wildcard /opt/local/lib/libhdf5.dylib),)
    # MacPorts
    HDF5DIR    ?= /opt/local
  else
  ifneq ($(wildcard /sw/lib/libhdf5.dylib),)
    # Fink
    HDF5DIR    ?= /sw
  else
  ifneq ($(wildcard /usr/local/lib/libhdf5.dylib),)
    # Homebrew
    HDF5DIR    ?= /usr/local
  endif
  endif
  endif
endif
endif

INCLUDES = -I${MLINKDIR} -I${HDF5DIR}/include
CFLAGS   = -Wall -Wno-write-strings -O3
ifeq ($(UNAME), Darwin)
	CFLAGS += -mmacosx-version-min=10.7
endif
MPREP = ${MLINKDIR}/mprep
MCC   = ${MLINKDIR}/mcc

PKGFILES = MacOSX-x86-64/h5mma Linux-x86-64/h5mma Linux/h5mma Windows-x86-64/h5mma Windows-x86-64/*.dll Windows/h5mma Windows/*.dll h5mma.m Kernel doc COPYING COPYING.LESSER COPYING.HDF5 COPYING.MATHLINK README BUILD_ID

all : h5mma

h5mma : h5mmatm.cc h5mma.cc h5wrapper.cc h5wrapper.h BUILD_ID
	@echo "Compiling h5mma"
	@if [ ! -d "$(HDF5DIR)" ]; then echo "HDF5 not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@if [ ! -d "$(MLINKDIR)" ]; then echo "MathLink not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@rm -rf $(EXEDIR)
	@mkdir $(EXEDIR)
	@$(CXX) $(CFLAGS) $(INCLUDES) -c h5wrapper.cc
	@$(CXX) $(CFLAGS) $(INCLUDES) -c h5mma.cc
	@$(CXX) $(CFLAGS) $(INCLUDES) -c h5mmatm.cc
ifeq ($(UNAME), Darwin)
	@$(CXX) h5mma.o h5mmatm.o h5wrapper.o $(HDF5DIR)/lib/libhdf5.a $(HDF5DIR)/lib/libsz.a $(MATHLIBS) -lz -framework CoreFoundation -framework Foundation -o $(EXEDIR)/h5mma -mmacosx-version-min=10.7
else
ifeq ($(UNAME), Linux)
	@$(CXX) -static -pthread h5mma.o h5mmatm.o h5wrapper.o -L${HDF5DIR}/lib -lhdf5 -L${SZIPDIR}/lib -lsz $(MATHLIBS) -lz -ldl -luuid -o $(EXEDIR)/h5mma
endif
endif

h5mmatm.cc : h5mma.tm
	@if [ ! -d "$(MLINKDIR)" ]; then echo "MathLink not found - create or check make.defs file"; echo "See make.defs.example file for reference"; exit 1; fi
	@${MPREP} $? -o $@

tarball: ${PKGFILES}
	mkdir h5mma
	rsync -aR ${PKGFILES} h5mma/
	tar -czf h5mma.tar.gz h5mma
	rm -rf h5mma

zip: ${PKGFILES}
	mkdir h5mma
	rsync -aR ${PKGFILES} h5mma/
	zip -r h5mma.zip h5mma
	rm -rf h5mma

BUILD_ID :
	@date +"%B %d, %Y" > BUILD_ID
	@echo "Build id:" `cat BUILD_ID`

GIT_REVISION :
	@git rev-parse HEAD > GIT_REVISION
	@echo "Git revision:" `cat GIT_REVISION`

.PHONY : clean
clean :
	@rm -rf h5mmatm.cc MacOSX-x86-64 Linux-x86-64 Linux Windows-x86-64 Windows h5mma.zip h5mma.tar.gz *.o h5mma BUILD_ID GIT_REVISION

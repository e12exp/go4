#general make file for any Go4 module
ifndef GO4SYS
$(error GO4SYS is not set. Did you do a "source go4login new?")
endif

include $(GO4SYS)/Makefile.config

# for compile example under cygwin, 
# relative path to GO4SYS directory should be specified
ifdef GO4_WIN32
  GO4SYS = ..
endif   

# comment to switch on optimization
DOOPTIMIZATION = false

# to add some more definitions, which can be used in #ifdef inside user code,
# just uncomment and edit following line
#DEFINITIONS += -D__USERDEF__

# if there are include files in other directories,
# just uncomment and edit following file
#INCLUDES += -I/home/usr/special/includes

# if some special libraries must be linked to the analysis
# uncomment and edit following line
#LIBS_FULLSET += -L/home/usr/special/lib -lspecial

all: build

CalifaProc.o:	CalifaConfig.h CalifaProc.cxx

#FOO:=$(shell copySampleConfig.sh)

include Module.mk

test:
	@echo $(MODULE_NAME)

#GCCDIR=/home/pklenze/software/gcc-4.9.1/
GCCDIR=/usr
CXXFLAGS:=$(CXXFLAGS) -std=c++0x -g -I $(PWD) -Wall
LDFLAGS:=$(LDFLAGS) -L $(GCCDIR)/lib64
CXX=$(GCCDIR)/bin/g++
LD_LIBRARY_PATH:=$(LD_LIBRARY_PATH):$(GCCDIR)/lib
build: all-$(MODULE_NAME)

clean-obj: clean-obj-$(MODULE_NAME)
	@echo "Clean objects in $(MODULE_NAME)"

clean: clean-$(MODULE_NAME)
	@echo "Clean everything in $(MODULE_NAME)"
	rm -f *~

ifdef DOMAP
ifeq ($(wildcard $(GO4MAP)),)
map:
	@echo "Map for Go4 classes not generated. Call make map in $(GO4SYS) directory"
else
map: map-$(MODULE_NAME)
	@echo "Map for $(MODULE_NAME) done"
endif
else
map:
	@echo "Map for $(MODULE_NAME) can not be generated with ROOT $(shell root-config --version)"
endif

ifdef DOPACKAGE
package:
	@tar chf $(MODULE_NAME).tar Module.mk
	@tar rhf $(MODULE_NAME).tar $(subst $(CURDIR),.,$(DISTRFILES))
	@tar rhf $(MODULE_NAME).tar Makefile --ignore-failed-read
	@gzip -f $(MODULE_NAME).tar
	@echo "$(MODULE_NAME).tar.gz done"
endif

include $(GO4SYS)/Makefile.rules

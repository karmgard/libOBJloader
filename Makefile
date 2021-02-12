#
SHELL=/bin/bash

# Make sure the .dependencies file exists, otherwise the include at the bottom will choke
$(shell touch .dependencies)

LIBSRC=model.cpp
LIBHDR=model.h vertex.h face.h material.h object.h group.h gl.h 
LIBOBJ=$(subst .cpp,.o,${LIBSRC})
LIBBIN=libobjloader.so

GCC_VERSION=`g++ -dumpversion`
ARCH=x86_64
OS=linux

INCLUDEPATHS= -I. -I./include \
	-I/usr/include/c++/${GCC_VERSION} \
	-I/usr/include/c++/${GCC_VERSION}/${ARCH}-linux-gnu \
	-I/usr/lib/gcc/${ARCH}-linux-gnu/${GCC_VERSION}/include \
	-I/usr/include/${ARCH}-linux-gnu \
	-I/usr/include/${ARCH}-linux-gnu/c++/${GCC_VERSION} \
	-I/usr/include/ImageMagick \
	-I/usr/local/include

LIBSEARCH=-L./ -L/usr/local/lib
LIBRARIES=-lglut -lGL -lGLU -lm
DEBUG=0

ifeq (${DEBUG},1)
  CPUOPT=-g3 -Wall -Wunused -pg -fno-strict-aliasing -finline-functions -std=c++0x -fopenmp
  LIBS=${LIBSEARCH} ${LIBRARIES} -pg -lm
else
  CPUOPT=-Wall -Wunused -mhard-float -fno-strict-aliasing -finline-functions -std=c++0x -fopenmp
  LIBS=${LIBSEARCH} ${LIBRARIES}
endif

ifneq (${OS},darwin)
  override CPUOPT+= $(patsubst %,%,-Wno-unused-but-set-variable)
  override CPUOPT+= $(patsubst %,%,-std=gnu++0x)
else
  override CPUOPT += $(patsubst %,%,-D__DARWIN__)
endif

CC=g++ $(CPUOPT) $(INCLUDEPATHS) 
LINK=g++ -o $(BIN) $(OBJ) $(LIBS)

all:	lib

install:
	@cp ${LIBBIN} ${HOME}/lib/
	@cp ${LIBHDR} ${HOME}/include

.cpp.o:
	@echo ">>>>>>>>>>>> Compiling $< -> $@ <<<<<<<<<<<<<"
	$(CC) -c $< -o $@

lib:	${LIBOBJ} ${LIBSRC}
	@echo ">>>>>>>>>>>> Making Library <<<<<<<<<<<<<"
	$(CC) -fPIC $(CPUOPT) -c ${LIBSRC}
	$(CC) -shared -o ${LIBBIN} ${LIBOBJ}

.PHONY: clean tidy force depend dep backup

clean:
	rm -f $(LIBOBJ) $(LIBBIN) *~ *.bak .*.bak gmon.out example example2 *.o

tidy:
	rm -f $(LIBOBJ) $(LIBBIN)

force:	tidy all

backup:
	@tar -zcf $(LIBBIN).tar.gz $(LIBSRC) $(LIBHDR) Makefile .dependencies 

depend dep:
ifneq (${OS},darwin)
	makedepend  $(INCLUDEPATHS) $(SRC) -f .dependencies;
else
	makedepend -D__DARWIN__ $(INCLUDEPATHS) $(SRC) -f .dependencies;
endif

include .dependencies
# DO NOT DELETE

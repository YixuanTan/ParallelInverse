PETSC_DIR=/home/smartcoder/Documents/Programs/PETSC/petsc-2.3.3-p16

PETSC_ARCH=linux-gnu-cxx-debug

MPI_PATH=/home/smartcoder/Documents/Programs/mpich-install/bin
#MPI_PATH=/usr/local/mpich2/latest/bin

#CXX=/usr/bin/mpicxx
CXX=/home/smartcoder/Documents/Programs/mpich-install/bin/mpicxx
#CXX=/usr/local/mpich2/latest/bin/mpicxx

#If there are any .hpp files
HS = mycalls.hpp share.hpp

STUFF = -std=c++11 -DMPICH_IGNORE_CXX_SEEK -I. -I/usr/include/ -I/usr/local/include\
	-I${PETSC_DIR}/include\
	-I${PETSC_DIR}/bmake/${PETSC_ARCH}\
	-I${MPI_PATH}/include\
	-I/usr/local/lib
	#-I/usr/include/X11

LDFLAGS =\
	-L/usr/lib/libblas\
    -L$(PETSC_DIR)/lib/$(PETSC_ARCH)\
    -L${MPI_PATH}/lib\
    -L${MPI_PATH}/lib64\
    -lpetscdm\
    -lpetscksp\
    -lpetscmat\
    -lpetscvec\
    -lblas\
    -llapack\
    -lpthread\
    -lpetsc

LDLIBS = -lglib -lm 

LDLIBS += -framework IOK

include ${PETSC_DIR}/bmake/common/base

mycalls.o: share.hpp mycalls.hpp mycalls.cpp
	${CXX} ${STUFF} -g -c mycalls.cpp

main.o: main.cpp ${HS}
	${CXX} ${STUFF} -g -c main.cpp

radiation: main.o mycalls.o
	${CXX} -Dradiation ${STUFF} -DMPI_VERSION -o driver main.o mycalls.o ${LDFLAGS}

driver: main.o mycalls.o
	${CXX} ${STUFF} -DMPI_VERSION -o driver main.o mycalls.o ${LDFLAGS}

clean: 
	\rm *.o results.m

include ${PETSC_DIR}/bmake/common/test
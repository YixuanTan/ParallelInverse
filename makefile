PETSC_DIR=/users/tany3/petsc-2.3.3-p16

PETSC_ARCH=linux-gnu-cxx-debug

MPI_PATH=/usr/local/mpich3/3.1.2-thread-multiple/bin
#MPI_PATH=/usr/local/mpich2/latest/bin

#CXX=mpicxx
CXX=/usr/local/mpich3/3.1.2-thread-multiple/mpicxx
#CXX=/usr/local/mpich2/latest/bin/mpicxx

#If there are any .hpp files
HS = mycalls.hpp share.hpp

STUFF = -DMPICH_IGNORE_CXX_SEEK -I. -I/usr/include/ -I/usr/local/include\
	-I${PETSC_DIR}/include\
	-I${PETSC_DIR}/bmake/${PETSC_ARCH}\
	-I${MPI_PATH}/include \
	-I/usr/include/X11

LDFLAGS =\
	-L$(PETSC_DIR)/lib/$(PETSC_ARCH)\
	-L${MPI_PATH}/lib\
	-L${MPI_PATH}/lib64\
	-lpetscdm\
	-lpetscksp\
	-lpetscmat\
	-lpetscvec\
	-llapack\
	-lpthread\
	-lpetsc

LDLIBS = -lglib -lm 


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














ALL: NLDriver
#PETSC_DIR     = /import/users/pyled/PETSC/petsc-2.3.3-p16                                                                                                                                     
PETSC_DIR       = /users/tany3/petsc-2.3.3-p16
p
# Include directory                                                                                                                                                                            

INCDIR        = -I../trunk/include -I${PETSC_DIR}/include -I${PETSC_DIR}/externalpackages/fblaslapack/linux-gnu-cxx-opt

# Library information                                                                                                                                                                          

LIBDIR        = -L../trunk/lib -L${PETSC_DIR}/externalpackages/fblaslapack/linux-gnu-cxx-opt

FEMLIBS       = -lFemLib -lfblas -lflapack

# Compiler Parameters                                                                                                                                                                          

DEFINE            = -DHAVE_FORTRAN_LAPACK -DMPICH_IGNORE_CXX_SEEK -DHAVE_BLASLAPACK -DF_NEEDS_UNDSC -DPETSC_2_3_3

CPPFLAGS      = ${DEFINE} -Wall -fmessage-length=0 ${INCDIR} ${LIBDIR} `pkg-config --cflags --libs libmds`
LINKFLAGS     = -fmessage-length=0 ${INCDIR} ${LIBDIR}

include ${PETSC_DIR}/bmake/common/base

# Targets                                                                                                                                                                                      

NLDriver: Model.o Load.o LAPACKWrappers.o NLDriver.o Output.o Checkpoint.o DebugUtil.o ElementParams.o TemperatureField.o chkopts
        ${CLINKER} ${LINKFLAGS} -o NLDriver LAPACKWrappers.o Model.o Load.o NLDriver.o Output.o Checkpoint.o DebugUtil.o ElementParams.o TemperatureField.o ${FEMLIBS} ${PETSC_LIB} `pkg-confi\
g --cflags --libs libmds`

purge:
        rm -f *.o NLDriver








# Makefile
# GNU Makefile for Voronoi tessellation and sparse phase-field grain growth
# Questions/comments to kellet@rpi.edu (Trevor Keller)

# includes
incdir = include
algodir = algorithms

# IBM XL compiler
BG_XL = /bgsys/drivers/ppcfloor/comm/xl
BG_INC = -I$(BG_PATH)/include
BG_LIB = -L$(BG_PATH)/lib

# compilers/flags
compiler = g++ -O3 -Wall
pcompiler = mpic++ -O3 -std=c++0x -Wall
flags = -I$(incdir) -I$(algodir) -I$(algodir)/topology

# RPI CCI AMOS compilers/flags
#qcompiler = mpic++ -g -qarch=qp -qtune=qp -qflag=w -qstrict -qreport
qcompiler = mpic++ -O5 -qarch=qp -qtune=qp -qflag=w -qstrict -qprefetch=aggressive -qsimd=auto -qhot=fastmath -qinline=level=10
#qflags = $(CFLAGS) $(BG_INC) $(BG_LIB) $(LDFLAGS) $(flags)

# ONLY uncomment the following if <module load experimental/zlib> FAILS.
qflags = $(BG_INC) $(BG_LIB) $(flags) -I/bgsys/apps/CCNI/zlib/zlib-1.2.7/include -L/bgsys/apps/CCNI/zlib/zlib-1.2.7/lib

# dependencies
core = $(incdir)/MMSP.utility.hpp \
       $(incdir)/MMSP.grid.hpp \
       $(incdir)/MMSP.sparse.hpp

# the program
graingrowth.out: main.cpp graingrowth.cpp tessellate.hpp $(core)
	$(compiler) -DPHASEFIELD $(flags) $< -o graingrowth.out -lz -pthread

mc: main.cpp graingrowth_MC.cpp tessellate.hpp $(core)
	$(compiler) $(flags) $< -o graingrowth.out -lz -pthread

parallel: main.cpp graingrowth.cpp tessellate.hpp $(core)
	$(pcompiler) -DPHASEFIELD $(flags) -include mpi.h $< -o parallel_GG.out -lz

parallelmc: main.cpp graingrowth_MC.cpp tessellate.hpp $(core)
	$(pcompiler) $(flags) -include mpi.h $< -o parallel_MC.out -lz

bgq: main.cpp graingrowth.cpp tessellate.hpp $(core)
	$(qcompiler) $(qflags) -DBGQ -DSILENT -DPHASEFIELD -DRAW $< -o q_GG.out

bgqmc: main.cpp graingrowth_MC.cpp tessellate.hpp $(core)
	$(qcompiler) $(qflags) -DBGQ -DSILENT -DRAW $< -o q_MC.out

wrongendian: wrongendian.cpp
	$(compiler) $< -o $@.out -lz -pthread

mmsp2vtk: TKmmsp2vti.cpp $(core)
	$(compiler) $(flags) $< -o $@ -lz

mmsp2vtkRecolor: mmsp2vtkRecolor.cpp $(core)
	$(compiler) $(flags) $< -o $@ -lz

mmsp2vtikeeporder: mmsp2vti_keeporder.cpp $(core)
	$(compiler) $(flags) $< -o $@ -lz

mmsp2vti200orient: mmsp2vti_200orient.cpp $(core)
	$(compiler) $(flags) $< -o $@ -lz

clean:
	rm -rf graingrowth.out parallel_GG.out parallel_MC.out q_GG.out q_MC.out wrongendian.out

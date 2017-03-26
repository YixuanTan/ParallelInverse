/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Header file for Petsc Functions to execute all Petsc maintenance
 %% behind the scenes.
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
#define _PETSC
#include "share.hpp"
/*%%%%%%%%%%%%%%%%%%%%%%%%%%% Function prototypes %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
void Petsc_Init(int argc,char **args,char *help);
void Petsc_End();
void Petsc_Solve(PETSC_STRUCT *obj);
void Vec_Create(PETSC_STRUCT *obj, PetscInt m);
void Mat_Create(PETSC_STRUCT *obj, PetscInt m, PetscInt n, PetscInt num_of_heaters);
void Mat_Preallocation(PETSC_STRUCT *obj, PetscInt d_nnz[], PetscInt o_nnz[]);
void Petsc_Assem_Vectors(PETSC_STRUCT *obj);
void Petsc_Assem_Matrices(PETSC_STRUCT *obj);
void Petsc_Destroy(PETSC_STRUCT *obj);
void Petsc_View(PETSC_STRUCT obj, PetscViewer viewer, char* Sname);
void Create_Inverse(PETSC_STRUCT *obj, PetscInt num_of_equations, PetscInt num_of_heaters, PetscInt num_of_surfacenodes);
void Assem_Inverse(PETSC_STRUCT *obj);
void Petsc_Solve_Inverse(PETSC_STRUCT *obj);
void Petsc_Destroy_Inverse(PETSC_STRUCT *obj);

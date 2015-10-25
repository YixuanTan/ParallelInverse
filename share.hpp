#include "petscmat.h"
#include "petscksp.h"
typedef Mat PETSC_MAT;
typedef Vec PETSC_VEC;
typedef struct
{
    PETSC_MAT Amat; /*linear system matrix*/
    KSP ksp; /*linear solver context*/
    PC pc; /*preconditioner context*/
    PETSC_VEC rhs; /*petsc rhs vector*/
    PETSC_VEC sol; /*petsc solution vector*/
    
    PETSC_MAT stiffness_matrix;
    PETSC_VEC current_temperature_field_local;
    
    PETSC_MAT S_matrix;
    PETSC_MAT R_matrix;
    PETSC_MAT RS_matrix;
    PETSC_MAT RSRS_regularized_matrix;
    
    KSP ksp_inverse; /*linear solver context*/
    PC pc_inverse; /*preconditioner context*/
    PETSC_VEC rhs_inverse; /*petsc rhs vector*/
    PETSC_VEC sol_inverse; /*petsc solution vector*/
   
}PETSC_STRUCT;


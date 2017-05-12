/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% PETSc behind the scenes maintenance functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
#include "mycalls.hpp"
#include "petscmat.h"
#include <petscksp.h>
#include <iostream>
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to initialize Petsc
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
void Petsc_Init(int argc, char **args,char *help)
{
    PetscErrorCode ierr;
    PetscInt n;
    PetscMPIInt size;
    PetscInitialize(&argc,&args,(char *)0,help);
    ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); //CHKERRQ(ierr);
    ierr = PetscOptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL); //CHKERRQ(ierr);
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to finalize Petsc
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_End()
{
    PetscErrorCode ierr;
    ierr = PetscFinalize(); //CHKERRQ(ierr);
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to create the rhs and soln vectors
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Vec_Create(PETSC_STRUCT *obj, PetscInt m)
{
    PetscErrorCode ierr;
    ierr = VecCreate(PETSC_COMM_WORLD, &obj->rhs); //CHKERRQ(ierr);
    ierr = VecSetSizes(obj->rhs, PETSC_DECIDE, m); //CHKERRQ(ierr);
    ierr = VecSetFromOptions(obj->rhs); //CHKERRQ(ierr);
    ierr = VecDuplicate(obj->rhs, &obj->sol); //CHKERRQ(ierr);
    
   ierr = VecDuplicate(obj->rhs, &obj->current_temperature_field_local);
    
    ierr = VecSetOption(obj->current_temperature_field_local, VEC_IGNORE_OFF_PROC_ENTRIES);
    
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to create the system matrix
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Mat_Create(PETSC_STRUCT *obj, PetscInt m, PetscInt n, PetscInt num_of_heaters)
{
    PetscErrorCode ierr;
    ierr = MatCreate(PETSC_COMM_WORLD, &obj->Amat);
    ierr = MatSetSizes(obj->Amat,PETSC_DECIDE,PETSC_DECIDE,m,n);
    ierr = MatSetType(obj->Amat, MATMPIAIJ);

    ierr = MatCreate(PETSC_COMM_WORLD, &obj->stiffness_matrix);
    ierr = MatSetSizes(obj->stiffness_matrix,PETSC_DECIDE,PETSC_DECIDE,m,n);
    ierr = MatSetType(obj->stiffness_matrix, MATMPIAIJ);
    // d_nz <= 9  o_nz <=8 (at least one at the diagonal)
//    ierr = MatMPIAIJSetPreallocation(obj->stiffness_matrix, 9, d_nnz, 8, o_nnz);

    // heat_matrix is added for the inverse analysis purpose
    ierr = MatCreate(PETSC_COMM_WORLD, &obj->heat_matrix);
    ierr = MatSetSizes(obj->heat_matrix,PETSC_DECIDE,PETSC_DECIDE,m,num_of_heaters);
    ierr = MatSetFromOptions(obj->heat_matrix);

    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to create d_nnz and o_nnz
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Mat_Preallocation(PETSC_STRUCT *obj, PetscInt d_nnz[], PetscInt o_nnz[])
{
    PetscErrorCode ierr;
    // d_nz <= 9  o_nz <=8 (at least one at the diagonal)
    ierr = MatMPIAIJSetPreallocation(obj->Amat, 9, d_nnz, 8, o_nnz);
    
    // d_nz <= 9  o_nz <=8 (at least one at the diagonal)
    ierr = MatMPIAIJSetPreallocation(obj->stiffness_matrix, 9, d_nnz, 8, o_nnz);
    
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to solve a linear system using KSP
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_Solve(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = KSPCreate(PETSC_COMM_WORLD,&obj->ksp); //CHKERRQ(ierr);
    
    // DIFFERENT_NONZERO_PATTERN or SAME_NONZERO_PATTERN does not change the solution. TESTED.
    ierr = KSPSetOperators(obj->ksp,obj->Amat,obj->Amat, SAME_NONZERO_PATTERN); //CHKERRQ(ierr);
    ierr = KSPGetPC(obj->ksp,&obj->pc); //CHKERRQ(ierr);
    ierr = PCSetType(obj->pc,PCBJACOBI);
    ierr = KSPSetTolerances(obj->ksp,1.e-7,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT); //CHKERRQ(ierr);
    ierr = KSPSetFromOptions(obj->ksp); //CHKERRQ(ierr);
    ierr = KSPSolve(obj->ksp,obj->rhs,obj->sol); //CHKERRQ(ierr);
    ierr = VecAssemblyBegin(obj->sol); //CHKERRQ(ierr);
    ierr = VecAssemblyEnd(obj->sol); //CHKERRQ(ierr);
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to do final assembly of matrices
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_Assem_Matrices(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = MatAssemblyBegin(obj->Amat, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    ierr = MatAssemblyEnd(obj->Amat, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    
    ierr = MatAssemblyBegin(obj->stiffness_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    ierr = MatAssemblyEnd(obj->stiffness_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    
    ierr = MatAssemblyBegin(obj->heat_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    ierr = MatAssemblyEnd(obj->heat_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);

    //Indicate same nonzero structure of successive linear system matrices
    ierr = MatSetOption(obj->Amat, MAT_NO_NEW_NONZERO_LOCATIONS);
    ierr = MatSetOption(obj->stiffness_matrix, MAT_NO_NEW_NONZERO_LOCATIONS);
    ierr = MatSetOption(obj->heat_matrix, MAT_NO_NEW_NONZERO_LOCATIONS);

    ierr = MatSetOption(obj->Amat, MAT_SYMMETRIC);
    ierr = MatSetOption(obj->stiffness_matrix, MAT_SYMMETRIC);

    return;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to do final assembly of vectors
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_Assem_Vectors(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = VecAssemblyBegin(obj->rhs); //CHKERRQ(ierr);
    ierr = VecAssemblyEnd(obj->rhs); //CHKERRQ(ierr);
    
    ierr = VecAssemblyBegin(obj->current_temperature_field_local); //CHKERRQ(ierr);
    ierr = VecAssemblyEnd(obj->current_temperature_field_local); //CHKERRQ(ierr);

    return;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to Destroy the matrix and vectors that have been created
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_Destroy(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = VecDestroy(obj->rhs); //CHKERRQ(ierr);
    ierr = VecDestroy(obj->sol); //CHKERRQ(ierr);
    ierr = MatDestroy(obj->Amat); //CHKERRQ(ierr);
    ierr = KSPDestroy(obj->ksp); //CHKERRQ(ierr);

    ierr = VecDestroy(obj->current_temperature_field_local);
    ierr = MatDestroy(obj->stiffness_matrix); //CHKERRQ(ierr);
    ierr = MatDestroy(obj->heat_matrix); //CHKERRQ(ierr);

    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to View the matrix and vectors that have been created in an m-file
 %% Note: Assumes all final assemblies of matrices and vectors have been performed
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_View(PETSC_STRUCT obj, PetscViewer viewer, char* Sname)
{
    PetscErrorCode ierr;
    ierr = PetscViewerASCIIOpen(PETSC_COMM_WORLD, Sname, &viewer); //CHKERRQ(ierr);
    ierr = PetscViewerPushFormat(viewer,PETSC_VIEWER_ASCII_MATLAB); //CHKERRQ(ierr);
    ierr = PetscObjectSetName((PetscObject)obj.S_matrix,"S_matrix"); //CHKERRQ(ierr);
    //    ierr = PetscObjectSetName((PetscObject)obj.rhs,"rhs"); //CHKERRQ(ierr);
    //ierr = PetscObjectSetName((PetscObject)obj.sol,"sol"); //CHKERRQ(ierr);
    ierr = MatView(obj.S_matrix,viewer); //CHKERRQ(ierr);
    //ierr = VecView(obj.rhs, viewer); //CHKERRQ(ierr);
    //ierr = VecView(obj.sol, viewer); //CHKERRQ(ierr);
    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to create the vectors and matries in inverse analysis
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Create_Inverse(PETSC_STRUCT *obj, PetscInt num_of_equations, PetscInt num_of_heaters, PetscInt num_of_surfacenodes)
{
    PetscErrorCode ierr;
    ierr = MatCreate(PETSC_COMM_WORLD, &obj->S_matrix);
    ierr = MatSetSizes(obj->S_matrix,PETSC_DECIDE,PETSC_DECIDE,num_of_equations,num_of_heaters);
    ierr = MatSetFromOptions(obj->S_matrix);
    
    ierr = MatCreate(PETSC_COMM_WORLD, &obj->R_matrix);
    ierr = MatSetSizes(obj->R_matrix,PETSC_DECIDE,PETSC_DECIDE,num_of_surfacenodes,num_of_equations);
    ierr = MatSetFromOptions(obj->R_matrix);
    
    ierr = MatCreate(PETSC_COMM_WORLD, &obj->RSRS_regularized_matrix);
    ierr = MatSetSizes(obj->RSRS_regularized_matrix,PETSC_DECIDE,PETSC_DECIDE,num_of_surfacenodes,num_of_heaters);
    ierr = MatSetFromOptions(obj->RSRS_regularized_matrix);
    
    ierr = VecCreate(PETSC_COMM_WORLD, &obj->rhs_inverse); //CHKERRQ(ierr);
    ierr = VecSetSizes(obj->rhs_inverse, PETSC_DECIDE, num_of_heaters); //CHKERRQ(ierr);
    ierr = VecSetFromOptions(obj->rhs_inverse); //CHKERRQ(ierr);
    ierr = VecDuplicate(obj->rhs_inverse, &obj->sol_inverse); //CHKERRQ(ierr);

    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to assemble the vectors and matrices in inverse analysis
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Assem_Inverse(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = MatAssemblyBegin(obj->S_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    ierr = MatAssemblyEnd(obj->S_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    
    ierr = MatAssemblyBegin(obj->RSRS_regularized_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);
    ierr = MatAssemblyEnd(obj->RSRS_regularized_matrix, MAT_FINAL_ASSEMBLY); //CHKERRQ(ierr);

    //Indicate same nonzero structure of successive linear system matrices
    ierr = MatSetOption(obj->S_matrix, MAT_NO_NEW_NONZERO_LOCATIONS);
    ierr = MatSetOption(obj->RSRS_regularized_matrix, MAT_NO_NEW_NONZERO_LOCATIONS);

    ierr = VecAssemblyBegin(obj->rhs_inverse); //CHKERRQ(ierr);
    ierr = VecAssemblyEnd(obj->rhs_inverse); //CHKERRQ(ierr);

    return;
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 %% Function to solve the inverse problem equation system
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void Petsc_Solve_Inverse(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = KSPCreate(PETSC_COMM_WORLD,&obj->ksp_inverse); //CHKERRQ(ierr);
    
    ierr = KSPSetOperators(obj->ksp_inverse, obj->RSRS_regularized_matrix, obj->RSRS_regularized_matrix, DIFFERENT_NONZERO_PATTERN); //CHKERRQ(ierr);
    ierr = KSPGetPC(obj->ksp_inverse,&obj->pc_inverse); //CHKERRQ(ierr);
    ierr = PCSetType(obj->pc_inverse,PCJACOBI);
    ierr = KSPSetTolerances(obj->ksp_inverse,1.e-7,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT); //CHKERRQ(ierr);
    ierr = KSPSetFromOptions(obj->ksp_inverse); //CHKERRQ(ierr);
    ierr = KSPSolve(obj->ksp_inverse,obj->rhs_inverse,obj->sol_inverse); //CHKERRQ(ierr);
    ierr = VecAssemblyBegin(obj->sol_inverse); //CHKERRQ(ierr);
    ierr = VecAssemblyEnd(obj->sol_inverse); //CHKERRQ(ierr);
    return;
}

void Petsc_Destroy_Inverse(PETSC_STRUCT *obj)
{
    PetscErrorCode ierr;
    ierr = VecDestroy(obj->rhs_inverse); //CHKERRQ(ierr);
    ierr = VecDestroy(obj->sol_inverse); //CHKERRQ(ierr);
    ierr = KSPDestroy(obj->ksp_inverse); //CHKERRQ(ierr);
    ierr = MatDestroy(obj->R_matrix); //CHKERRQ(ierr);
    ierr = MatDestroy(obj->S_matrix); //CHKERRQ(ierr);
    ierr = MatDestroy(obj->RSRS_regularized_matrix); //CHKERRQ(ierr);
    return;
}


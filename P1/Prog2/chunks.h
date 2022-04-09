/**
 *  \file chunks.h (interface file)
 *
 *  \brief Problem name: Compute Matrix Determinant.
 *
 *  In this file the functions to save/get the information of each matrix are implemented.
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the main thread:
 *     \li putMatrix
 *     \li endMatrix.
 *  Definition of the operations carried out by the worker threads:
 *     \li getMatrix
 *
 *  \author Diogo Filipe Amaral Carvalho - 92969 - April 2022
 *  \author Rafael Ferreira Baptista - 93367 - April 2022
 */

#ifndef CHUNKS_H
#define CHUNKS_H

/**
 *  \brief Store a struct to inform that there are no more matrices to be processed.
 *
 *  Operation carried out by the main thread.
 *
 */
extern void endMatrix();


/**
 *  \brief Store a matrix in the data transfer region.
 *
 *  Operation carried out by the main thread.
 *
 *  \param buffer pointer to the start of the chunk
 *  \param order_of_matrix number of bytes of the chunk
 *  \param matrix_id file identifier
 */
extern void putMatrix (double * buffer, int order_of_matrix, int matrix_id);

/**
 *  \brief Get a chunk from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId consumer identification
 *
 *  \return value
 */
extern struct MatrixInfo getMatrix (unsigned int workerId);

/** \brief struct to store the information of one matrix */
extern struct MatrixInfo {
   int matrix_id;        /* matrix identifier */  
   int order_of_matrix;    /* Order of the matrix */
   double * matrix_pointer;  /* Pointer to the start of the matrix */
} MatrixInfo;


#endif /* CHUNKS_H */

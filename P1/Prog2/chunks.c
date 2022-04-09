/**
 *  \file chunks.c (implementation file)
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "probConst.h"

/** \brief struct to store the information of one matrix */
struct MatrixInfo {
   int matrix_id;        /* matrix identifier */  
   int order_of_matrix;    /* Order of the matrix */
   double * matrix_pointer;  /* Pointer to the start of the matrix */
};

/** \brief main producer thread return status */
extern int statusProd;

/** \brief consumer threads return status array */
extern int *statusWorkers;

/** \brief storage region for matrixes struct */
static struct MatrixInfo mem[K];

/** \brief insertion pointer */
static unsigned int ii;

/** \brief retrieval pointer */
static unsigned int ri;

/** \brief flag signaling the data transfer region is full */
static bool full;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;;

/** \brief producers synchronization point when the data transfer region is full */
static pthread_cond_t fifoFull;

/** \brief consumers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

/**
 *  \brief Initialization of the data transfer region.
 *
 *  Internal monitor operation.
 */
static void initialization (void)
{
                                                                                   /* initialize FIFO in empty state */
  ii = ri = 0;                                        /* FIFO insertion and retrieval pointers set to the same value */
  full = false;                                                                                  /* FIFO is not full */

  pthread_cond_init (&fifoFull, NULL);                                      /* initialize main synchronization point */
  pthread_cond_init (&fifoEmpty, NULL);                                  /* initialize workers synchronization point */
}


/**
 *  \brief Store a struct in fifo to inform that there are no more matrices to be processed.
 *
 *  Operation carried out by the main thread.
 *
 */
void endMatrix() {
  if ((statusProd = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusProd;                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }

  while (full)                                                           /* wait if the data transfer region is full */
  { if ((statusProd = pthread_cond_wait (&fifoFull, &accessCR)) != 0)
       { errno = statusProd;                                                          /* save error in errno */
         perror ("error on waiting in fifoFull");
         statusProd = EXIT_FAILURE;
         pthread_exit (&statusProd);
       }
  }

  mem[ii].matrix_id = -1;                                                                   /* store values in the FIFO */
  mem[ii].order_of_matrix = -1;
  mem[ii].matrix_pointer =  NULL;
  ii = (ii + 1) % K;
  full = (ii == ri);

  if ((statusProd = pthread_cond_signal (&fifoEmpty)) != 0)       /* let a worker know that a value has been stored */
     { errno = statusProd;                                                             /* save error in errno */
       perror ("error on signaling in fifoEmpty");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }

  if ((statusProd = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusProd;                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }
}


/**
 *  \brief Store a matrix in the data transfer region.
 *
 *  Operation carried out by the main thread.
 *
 *  \param matrix_pointer pointer to the start of the matrix
 *  \param order_of_matrix number of bytes of the chunk
 *  \param matrix_id file identifier
 */
void putMatrix (double * matrix_pointer, int order_of_matrix, int matrix_id)
{
  if ((statusProd = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusProd;                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while (full)                                                           /* wait if the data transfer region is full */
  { if ((statusProd = pthread_cond_wait (&fifoFull, &accessCR)) != 0)
       { errno = statusProd;                                                          /* save error in errno */
         perror ("error on waiting in fifoFull");
         statusProd = EXIT_FAILURE;
         pthread_exit (&statusProd);
       }
  }

  mem[ii].matrix_id = matrix_id;                                                              /* store values in the FIFO */
  mem[ii].order_of_matrix = order_of_matrix;
  mem[ii].matrix_pointer =  matrix_pointer;
  ii = (ii + 1) % K;
  full = (ii == ri);

  if ((statusProd = pthread_cond_signal (&fifoEmpty)) != 0)       /* let a worker know that a value has been stored */
     { errno = statusProd;                                                             /* save error in errno */
       perror ("error on signaling in fifoEmpty");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }

  if ((statusProd = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusProd;                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusProd = EXIT_FAILURE;
       pthread_exit (&statusProd);
     }
}

/**
 *  \brief Get a matrix from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId consumer identification
 *
 *  \return value
 */
struct MatrixInfo getMatrix (unsigned int workerId)
{
  struct MatrixInfo matrixinfo;                                                                       /* retrieved value */

  if ((statusWorkers[workerId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusWorkers[workerId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusWorkers[workerId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[workerId]);
     }
  pthread_once (&init, initialization);                                                /* internal data initialization */

  while ((ii == ri) && !full)                                             /* wait if the data transfer region is empty */
  { if ((statusWorkers[workerId] = pthread_cond_wait (&fifoEmpty, &accessCR)) != 0)
       { errno = statusWorkers[workerId];                                                          /* save error in errno */
         perror ("error on waiting in fifoEmpty");
         statusWorkers[workerId] = EXIT_FAILURE;
         pthread_exit (&statusWorkers[workerId]);
       }
  }

  matrixinfo = mem[ri];                                                              /* retrieve a  value from the FIFO */
  ri = (ri + 1) % K;
  full = false;

  if ((statusWorkers[workerId] = pthread_cond_signal (&fifoFull)) != 0)       /* let a producer know that a value has been
                                                                                                            retrieved */
     { errno = statusWorkers[workerId];                                                             /* save error in errno */
       perror ("error on signaling in fifoFull");
       statusWorkers[workerId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[workerId]);
     }

  if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)                                   /* exit monitor */
     { errno = statusWorkers[workerId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusWorkers[workerId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[workerId]);
     }

  return matrixinfo;
}

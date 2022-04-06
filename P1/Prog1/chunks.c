/**
 *  \file chunks.c (implementation file)
 *
 *  \brief Problem name: Count Words.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the producers / consumers:
 *     \li putChunk
 *     \li getChunk
 *     \li endChunk.
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

/** \brief struct to store the information of one chunk*/
struct ChunkInfo {
   int fileId;        /* file identifier */  
   int chunk_size;    /* Number of bytes of the chunk */
   unsigned char * file_pointer;  /* Pointer to the start of the chunk */
};

/** \brief main producer thread return status */
extern int statusProd;

/** \brief consumer threads return status array */
extern int statusWorkers[N];

/** \brief storage region for chunks */
static struct ChunkInfo mem[K];

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
 *  \brief Store a struct in fifo to inform that there are no more chunks to be processed.
 *
 *  Operation carried out by the main thread.
 *
 */
void endChunk() {
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

  mem[ii].fileId = -1;                                                                   /* store values in the FIFO */
  mem[ii].chunk_size = -1;
  mem[ii].file_pointer =  NULL;
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
 *  \brief Store a chunk in the data transfer region.
 *
 *  Operation carried out by the main thread.
 *
 *  \param file_pointer pointer to the start of the chunk
 *  \param chunk_size number of bytes of the chunk
 *  \param file_id file identifier
 */
void putChunk (unsigned char * buffer, unsigned int chunk_size, unsigned int file_id)
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

  mem[ii].fileId = file_id;                                                              /* store values in the FIFO */
  mem[ii].chunk_size = chunk_size;
  mem[ii].file_pointer =  buffer;
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
 *  \brief Get a chunk from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId consumer identification
 *
 *  \return value
 */
struct ChunkInfo getChunk (unsigned int workerId)
{
  struct ChunkInfo chunkinfo;                                                                       /* retrieved value */

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

  chunkinfo = mem[ri];                                                              /* retrieve a  value from the FIFO */
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

  return chunkinfo;
}

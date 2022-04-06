/**
 *  \file chunks.h (interface file)
 *
 *  \brief Problem name: Producers / Consumers.
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

#ifndef CHUNKS_H
#define CHUNKS_H

/**
 *  \brief Store a struct to inform that there are no more chunks to be processed.
 *
 *  Operation carried out by the main thread.
 *
 */
extern void endChunk();


/**
 *  \brief Store a chunk in the data transfer region.
 *
 *  Operation carried out by the main thread.
 *
 *  \param file_pointer pointer to the start of the chunk
 *  \param chunk_size number of bytes of the chunk
 *  \param file_id file identifier
 */
extern void putChunk (unsigned char * buffer, unsigned int chunk_size, unsigned int file_id);

/**
 *  \brief Get a chunk from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId consumer identification
 *
 *  \return value
 */
extern struct ChunkInfo getChunk (unsigned int workerId);

/** \brief struct to store the information of one chunk*/
extern struct ChunkInfo {
   int fileId;        /* file identifier */  
   int chunk_size;    /* Number of bytes of the chunk */
   unsigned char * file_pointer;  /* Pointer to the start of the chunk */
} ChunkInfo;

#endif /* CHUNKS_H */

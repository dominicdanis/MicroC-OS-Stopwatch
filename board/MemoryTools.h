/* MemoryTools.c is a module containing a function to compute checksums, configure CRC module and
 * get CRC for a given range of addresses
 * Author: Dominic Danis Last Edit: 10/22/2021
 * */
#include "MCUType.h"

#ifndef MEMORYTOOLS_H_
#define MEMORYTOOLS_H_

/* INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);
 *  Description: Computes the check sum for a specified block of memory
 *  Arguments: INT8U *startaddr - pointer to the address to begin the checksum
 *             INT8U *endaddr   - pointer to the address to end the checksum
 *  Return value: Returns an INT16U containing the checksum value of the specific block
 * */
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);


/* void MemCRCConfig()
 * Description: Configures the CRC module with our given seed, polynomial, size, complement and transpose read/writes
 * Arguments: none
 * Returns: none
 * */
void MemCRCConfig(void);

INT16U MemCRCGet(INT8U *startaddr, INT8U *endaddr);



#endif

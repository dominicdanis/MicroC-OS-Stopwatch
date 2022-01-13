/* MemoryTools.c is a module containing a function to compute checksums, configure CRC module and
 * get CRC for a given range of addresses
 * Author: Dominic Danis Last Edit: 10/22/2021
 * */

#include "MemoryTools.h"
#include "MCUType.h"


//Defines for CRC Config
#define CRC_POLYNOMIAL 0x8005
#define SEED 0x0000

/* INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr);
 *  Description: Computes the check sum for a specified block of memory
 *  Arguments: INT8U *startaddr - pointer to the address to begin the checksum
 *             INT8U *endaddr   - pointer to the address to end the checksum
 *  Return value: Returns an INT16U containing the checksum value of the specific block
 * */
INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr){
    INT16U sum = 0;
    while(startaddr<endaddr){
        sum = sum + (INT16U)*startaddr;                 //Accumulate values at each address
        startaddr++;
    }
    sum = sum + (INT16U)*endaddr;                       //Avoid terminal count bug
    return sum;
}

/* void MemCRCConfig()
 * Description: Configures the CRC module with our given seed, polynomial, size, complement and transpose read/writes
 * Arguments: none
 * Returns: none
 * */

void MemCRCConfig(void){
    //Hopefully all is enabled correctly
    SIM->SCGC6 |= SIM_SCGC6_CRC(1);
    CRC0->CTRL = CRC0->CTRL | CRC_CTRL_TCRC(0);         //Enable 16 bit CRC mode
    CRC0->CTRL = CRC0->CTRL | CRC_CTRL_TOTR(2) | CRC_CTRL_TOT(2) | CRC_CTRL_FXOR(1) ;      //program transpose and complement options
    CRC0->GPOLY_ACCESS16BIT.GPOLYL = CRC_GPOLY_LOW(CRC_POLYNOMIAL);
    CRC0->CTRL = CRC0->CTRL | CRC_CTRL_WAS(1);
    CRC0->ACCESS16BIT.DATAL = CRC_DATAL_DATAL(SEED);
    CRC0->CTRL = CRC0->CTRL & (0xFDFFFFFF);

}

//States for CRC writing
typedef enum{HU,HL,LU,LL}WRITE_DEST;


/* INT16U MemCRCGet(INT8U *startaddr, INT8U *endaddr)
 * Description: Runs CRC test for a given address on the CRC module
 * Arguments: INT8U *startaddr - pointer to the address to begin the CRC
 *            INT8U *endaddr   - pointer to the address to end the CRC
 * Return:    Returns the CRC16 an INT16U
 * */

INT16U MemCRCGet(INT8U *startaddr, INT8U *endaddr){
    WRITE_DEST current_state = HU;                     //write to HU first
    while(startaddr<endaddr){

        switch(current_state){                         //write to different registers depending on state
            case HU:
                CRC0->ACCESS8BIT.DATAHU = *startaddr;
                startaddr++;
                current_state = HL;
                break;
            case HL:
                CRC0->ACCESS8BIT.DATAHL = *startaddr;
                startaddr++;
                current_state = LU;
                break;
            case LU:
                CRC0->ACCESS8BIT.DATALU = *startaddr;
                startaddr++;
                current_state = LL;
                break;
            case LL:
                CRC0->ACCESS8BIT.DATALL = *startaddr;
                startaddr++;
                current_state = HU;
                break;
            default:
                current_state = HU;
                break;
        }
    }
    switch(current_state){                             //Avoid terminal count bug
        case HU:
            CRC0->ACCESS8BIT.DATAHU = *startaddr;
            break;
        case HL:
            CRC0->ACCESS8BIT.DATAHL = *startaddr;
            break;
        case LU:
            CRC0->ACCESS8BIT.DATALU = *startaddr;
            break;
        case LL:
            CRC0->ACCESS8BIT.DATALL = *startaddr;
            break;
        default:
            break;
    }
    return CRC0->ACCESS16BIT.DATAL;
}

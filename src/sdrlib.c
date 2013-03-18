/**
 * This is intended as a small and simple
 * SDR library with as low code complexity, 
 * and as few dependencies as possible.  
 * 
 * The goal is to make code readability, maintainability
 * and portability as high as possible.   
 */

#include <stdio.h>
#include <stdlib.h>
#include <sdrlib.h>


/**
 * Create a new SdrLib instance.
 * @return a new SdrLib instance
 */  
SdrLib *sdrCreate()
{
    SdrLib *inst = (SdrLib *)malloc(sizeof(SdrLib));
    if (!inst)
        return inst;
    return inst;
}

/**
 * Delete an SdrLib instance, stopping
 * any processing and freeing any resources.
 * @param sdrlib and SDRLib instance.
 */   
void sdrDelete(SdrLib *sdrlib)
{
    if (!sdrlib)
        return;
    //TODO: clean up here
    free(sdrlib);
}


#ifndef _SDRLIB_H_
#define _SDRLIB_H_


/**
 * This is intended as a small and simple
 * SDR library with as low code complexity, 
 * and as few dependencies as possible.  
 * 
 * The goal is to make code readability, maintainability
 * and portability as high as possible.   
 */
 
typedef struct
{
    int x;
} SdrLib;

/**
 * Create a new SdrLib instance.
 * @return a new SdrLib instance
 */  
SdrLib *sdrCreate();

/**
 * Delete an SdrLib instance, stopping
 * any processing and freeing any resources.
 * @param sdrlib and SDRLib instance.
 */   
void sdrDelete(SdrLib *sdrlib);







#endif /* _SDRLIB_H_ */


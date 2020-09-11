#include "FreeRTOS.h"
#include "lwmem/lwmem.h"

void vPortFree( void *pv )
{
    lwmem_free(pv);
}

void *pvPortMalloc( size_t xWantedSize )
{
    return lwmem_malloc( xWantedSize );
}
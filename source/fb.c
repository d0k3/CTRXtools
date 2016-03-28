#include <inttypes.h>

void __attribute__ ((naked)) a11Entry()
{
    __asm__ ("ldr r0,=0x1FFF4D80\n\t mov sp, r0");
    regSet();
}

void regSet()
{
    volatile uint32_t *entry = (uint32_t *)0x1FFFFFF8;

    *((volatile uint32_t*)0x10400468) = FB_TOP_LEFT;
    *((volatile uint32_t*)0x1040046c) = FB_TOP_LEFT;
    *((volatile uint32_t*)0x10400494) = FB_TOP_RIGHT;
    *((volatile uint32_t*)0x10400498) = FB_TOP_RIGHT;
   
    *((volatile uint32_t*)0x10400568) = FB_BOTTOM;
    *((volatile uint32_t*)0x1040056c) = FB_BOTTOM;

    //once we set the registers we just want to loop, since we're done with a11
    *entry = 0;

    while(!*entry);

    ((void (*)())*entry)();
}
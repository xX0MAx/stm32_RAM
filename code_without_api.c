void _start() {
    volatile unsigned int* GPIOC_BSRR = (volatile unsigned int*)0x40011010;
    volatile unsigned int* GPIOC_CRH = (volatile unsigned int*)0x40011004;
    
    *GPIOC_CRH = (*GPIOC_CRH & ~(0xF << 20)) | (0x1 << 20);
    
    int counter = 0;

    while(counter < 10) {
        *GPIOC_BSRR = (1 << 13);
        for(volatile int i = 0; i < 1000000; i++);
        *GPIOC_BSRR = (1 << (13 + 16));
        for(volatile int i = 0; i < 1000000; i++);
        counter++;
    }
}
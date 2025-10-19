#include <stdbool.h>

#define API_ADDRESS 0x200004E8

typedef struct {
    void (*println)(const char*);
    void (*print_int)(int);
    void (*delay)(unsigned int);
    void (*digitalWrite)(bool);
} RAM_API;

void _start() {
    RAM_API* api = (RAM_API*)API_ADDRESS;

    int counter = 0;
    bool stateLed = 1;

    while(counter != 10){
        api->digitalWrite(stateLed);
        api->delay(500);
        api->print_int(counter);
        stateLed = !stateLed;
        counter++;
    }

    api->println("RAM API work");
}
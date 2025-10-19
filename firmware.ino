#include <SD.h>
#include <SPI.h>

#define SD_PIN PA4
#define CODE_BUFFER_SIZE 512 //Значение можно менять, в зависимости от веса кода, главное выравнивание учитывать

//Тут начало секции для API, если она не нужна просто вырежьте её
struct RAM_API {
    void (*println)(const char*);
    void (*print_int)(int);
    void (*delay)(uint32_t);
    void (*digitalWrite)(bool);
};

RAM_API api __attribute__((aligned(4))) __attribute__((section(".ram_api"))) = {
    .println = [](const char* str) { Serial1.println(str); },
    .print_int = [](int value) { Serial1.println(value); },
    .delay = [](uint32_t ms) { delay(ms); },
    .digitalWrite = [](bool state) { digitalWrite(PC13, state); }
};
//Тут конец, кстати если кол-во методов будет нечётное, то адруино может прописать, что есть проблемы с выравниванием, но их нет, тут уже всё продумано) 

uint8_t codeBuffer[CODE_BUFFER_SIZE] __attribute__((aligned(4))) __attribute__((section(".ram_code")));

typedef void (*ram_function)(void);

class RAMLoader {
public:
    bool initializeSD() {
        Serial1.println("Initializing SD card...");
        delay(100);
        return SD.begin(SD_PIN);
    }

    bool loadBinToRAM(const char* filename) {
        File file = SD.open(filename);
        if (!file) {
            Serial1.print("File not found: ");
            Serial1.println(filename);
            return false;
        }
        
        uint32_t fileSize = file.size();
        if (fileSize > CODE_BUFFER_SIZE) {
            Serial1.println("File too big");
            file.close();
            return false;
        }
        
        memset(codeBuffer, 0, CODE_BUFFER_SIZE);
        
        uint32_t bytesRead = file.read(codeBuffer, fileSize);
        float percentUsed = (file.size() * 100.0) / CODE_BUFFER_SIZE;
        file.close();

        Serial1.print("Loaded ");
        Serial1.print(bytesRead);
        Serial1.print(" bytes to RAM(");
        Serial1.print(percentUsed);
        Serial1.print("% from ");
        Serial1.print(CODE_BUFFER_SIZE);
        Serial1.println(" bytes)");
        Serial1.print("Buffer address: 0x");
        Serial1.println((uint32_t)codeBuffer, HEX);

        //Ну и тут чуть чуть API
        Serial1.print("API address: 0x");
        Serial1.println((uint32_t)&api, HEX);
        //Просто для отображения, куда API сохранилось, что бы для кода на SD указать адрес
        
        return bytesRead > 0;
    }

    bool executeFromRAM() {
        if (!codeBuffer) {
            Serial1.println("No code loaded");
            return false;
        }
        
        Serial1.println("Executing from RAM...");
        
        ram_function func = (ram_function)((uint32_t)codeBuffer | 1);
        
        Serial1.print("Jumping to: 0x");
        Serial1.println((uint32_t)func, HEX);

        asm volatile ("dsb" ::: "memory");
        asm volatile ("isb" ::: "memory");
        
        func();
        
        Serial1.println("Returned from RAM code");
        return true;
    }
};

RAMLoader ramLoader;

void setup() {
    Serial1.begin(9600);
    delay(5000);
    
    pinMode(PC13, OUTPUT);
    
    if (ramLoader.initializeSD()) {
        if (ramLoader.loadBinToRAM("example_name.bin")) { //Укажите имя вашего файла
            ramLoader.executeFromRAM();
        }
    }
    
    digitalWrite(PC13, HIGH);

    Serial1.println("Back to main loop");
}

void loop() {
    digitalWrite(PC13, !digitalRead(PC13));
    delay(2000);
}
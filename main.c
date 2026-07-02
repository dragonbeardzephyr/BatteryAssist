#include <pspuser.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <psppower.h>
#include <pspsyscon.h>


PSP_MODULE_INFO("BatteryAssist", 0x1000, 0, 1);
PSP_NO_CREATE_MAIN_THREAD();


enum powerState {
    On,
    OnLow,
    OnChargingFull,
    OnChargingMedium,
    OnChargingLow
};

enum powerState currentState = On;


int blinkState = 0;


volatile int running = 0;


#define MS_LED 0
#define WLAN_LED 1
#define POWER_LED 2
#define BLUETOOTH_LED 3 // Only on the PSP GO

#define LED_ON 1
#define LED_OFF 0



SceUID thid = -1; // Thread ID for the main thread

void loadConfig() {} //Use this for custom configs later



void setState(void) {

    int life = scePowerGetBatteryLifePercent();

    if (scePowerIsBatteryCharging()) {
        if (life <=5 ) {
            currentState = OnChargingLow;
        } else if (life < 99) {
            currentState = OnChargingMedium;
        } else {
            currentState = OnChargingFull;
        }

    } else {
        if (life <= 5) {
            currentState = OnLow;
        } else {
            currentState = On;
        }
    }
}



void flashPowerLED() {
    for (int i = 0; i < 5; i++) {
        sceSysconCtrlLED(POWER_LED, LED_OFF);
        sceKernelDelayThread(50000); // 50 ms
        sceSysconCtrlLED(POWER_LED, LED_ON);
        sceKernelDelayThread(50000); // 50 ms
    }
}



void response(enum powerState state) {
    switch (state) {
        case On:
            // Normal operation, no LED changes needed
            break;
        case OnLow:
            flashPowerLED(); // Bllink the power LED to indicate low battery
            break;
         case OnChargingFull:
            //sceSysconCtrlLED(POWER_LED, blinkState ? LED_ON : LED_OFF);
            break;
        case OnChargingMedium:
            //sceSysconCtrlLED(POWER_LED, blinkState ? LED_ON : LED_OFF);
            break;
        case OnChargingLow:
            // 
            break;
    }
} 



int main_thread(SceSize args, void *argp) {

    while ( sceKernelFindModuleByName("sceKernelLibrary") == NULL ) {
        sceKernelDelayThread(1000);
    }
    
    sceKernelDelayThread(5000000); // 5 seconds

    while (running) {

        setState();
        response(currentState);

        sceKernelDelayThread(500000); // 0.5 seconds
    }
    return 0;
}



int module_start(SceSize args, void *argp) {
    running = 1;

    SceUID fd = sceIoOpen("ms0:/seplugins/battery_log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, "Plugin Started\n", 15);
        sceIoClose(fd);
    }

    thid = sceKernelCreateThread("BatteryAssistThread", main_thread, 0x3F, 0x1000, 0, NULL);
    if(thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }

    return 0;
}

int module_stop(SceSize args, void *argp) {
    running = 0;

    if (thid >= 0) {
        sceKernelWaitThreadEnd(thid, NULL);
        sceKernelDeleteThread(thid);
    }

    SceUID fd = sceIoOpen("ms0:/seplugins/battery_log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, "Plugin Stopped\n", 15);
        sceIoClose(fd);
    }

    return 0;
}






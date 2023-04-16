#include "mbed.h"
using namespace std::chrono_literals;
static BufferedSerial pc_uart(USBTX, USBRX); //define USB UART port to PC

#define waveformLength (128)
#define lookUpTableDelay (10)
#define bufferLength (32)
#define modulatingFreq (10)

AnalogOut Aout(PA_4);

InterruptIn keyboard0(D2);
InterruptIn keyboard1(D3);
InterruptIn keyboard2(D4);

InterruptIn button(BUTTON1);
DigitalOut modulated(LED1);

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
int idC = 0;
int idE = 0;
int idG = 0;

float waveform[waveformLength]= { //128 samples of a sine waveform
    0.500, 0.525, 0.549, 0.574, 0.598, 0.622, 0.646, 0.670, 0.693, 0.715, 0.737,
    0.759, 0.780, 0.800, 0.819, 0.838, 0.856, 0.873, 0.889, 0.904, 0.918, 0.931,
    0.943, 0.954, 0.964, 0.972, 0.980, 0.986, 0.991, 0.995, 0.998, 1.000, 1.000,
    0.999, 0.997, 0.994, 0.989, 0.983, 0.976, 0.968, 0.959, 0.949, 0.937, 0.925,
    0.911, 0.896, 0.881, 0.864, 0.847, 0.829, 0.810, 0.790, 0.769, 0.748, 0.726,
    0.704, 0.681, 0.658, 0.634, 0.610, 0.586, 0.562, 0.537, 0.512, 0.488, 0.463,
    0.438, 0.414, 0.390, 0.366, 0.342, 0.319, 0.296, 0.274, 0.252, 0.231, 0.210,
    0.190, 0.171, 0.153, 0.136, 0.119, 0.104, 0.089, 0.075, 0.063, 0.051, 0.041,
    0.032, 0.024, 0.017, 0.011, 0.006, 0.003, 0.001, 0.000, 0.000, 0.002, 0.005,
    0.009, 0.014, 0.020, 0.028, 0.036, 0.046, 0.057, 0.069, 0.082, 0.096, 0.111,
    0.127, 0.144, 0.162, 0.181, 0.200, 0.220, 0.241, 0.263, 0.285, 0.307, 0.330,
    0.354, 0.378, 0.402, 0.426, 0.451, 0.475, 0.500};

void playNote(int freq, int duration, bool modulated) {
    int i = duration;
    int j = waveformLength;
    int waitTime = (1000000 / waveformLength / freq - lookUpTableDelay) << 0;
    int modulateCount1 = 0;
    int modulateCount2 = 0;
    if (modulated){
        printf("Play notes %d, modulated.\n", freq);
        while (i--) {
            j = waveformLength;
            modulateCount1++;
            if (modulateCount1 % (freq / modulatingFreq))
                modulateCount2 = (modulateCount2 + 1) % waveformLength;
            while (j--) {
                Aout = waveform[j] * waveform[modulateCount2];
                wait_us(waitTime);
            }
        }
        modulateCount1 = 0;
    }else {
        printf("Play notes %d\n", freq);
        while (i--) {
            j = waveformLength;
            while (j--) {
                Aout = waveform[j];
                wait_us(waitTime);
            }
        }
    }
    
}

void modulateNotes(void) {modulated = !modulated;}

void playNoteC(void) { idC = queue.call_every(1ms, playNote, 131, 500, modulated); }
void playNoteE(void) { idE = queue.call_every(1ms, playNote, 165, 500, modulated); }
void playNoteG(void) { idG = queue.call_every(1ms, playNote, 196, 500, modulated); }

void stopPlayNoteC(void) { queue.cancel(idC); }
void stopPlayNoteE(void) { queue.cancel(idE); }
void stopPlayNoteG(void) { queue.cancel(idG); }

int main(void) {
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    keyboard0.fall(queue.event(playNoteC));
    keyboard1.fall(queue.event(playNoteE));
    keyboard2.fall(queue.event(playNoteG));
    keyboard0.rise(queue.event(stopPlayNoteC));
    keyboard1.rise(queue.event(stopPlayNoteE));
    keyboard2.rise(queue.event(stopPlayNoteG));
    button.rise(queue.event(modulateNotes));
}
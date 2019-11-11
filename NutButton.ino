#include <avr/pgmspace.h>

#define PIN LED_BUILTIN

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PIN, OUTPUT);
}

// because our bit-level sampling rate matters for noise, we use direct HW access for speed
#if 1  // fast version using direct hardware access
#define PIN_PORT (( (PIN)<8 ? PORTD : PORTB   )) // hoping the compiler folds that a compile-time
#define PIN_BIT  (( (PIN)<8 ? (PIN) : (PIN-8) ))
#define PIN_MASK (1 << (PIN_BIT))
#define getPortVal() (PIN_PORT & ~(PIN_MASK))    // read out old value at start, outside of loop
#define writeBit(bit, oldPortVal) ((PIN_PORT = (oldPortVal) | ((PIN_MASK) & -bit)))
#else  // slow version calling digitalWrite()
#define getPortVal() (0) // not used
#define writeBit(bit, oldPortVal) (digitalWrite(PIN, (bit)))
#endif

// plays an audio sample through a single-bit port using sigma-delta coding
void playSampleBuffer(const char* data/*in PROGMEM*/, unsigned int length, int sampleRate) {
  // Because interrupts cause audio artifacts, but micros() does not work with noInterrupt(),
  // we must time this manually. The following variable is the length of one loop iteration,
  // in tenth of microseconds (100 ns). It has been manually tuned (approximately).
  const long tenthMicrosPerLoop = 36;
  const long tenthMicrosPerPeriod = 10000000 / sampleRate; // sample period in tenth of micro seconds
  noInterrupts();
  long tenthMicros = 0;  // how many tenth of microseconds have elapsed within this sample period?
  char residual = 0;     // current aggregate quantization error
  const unsigned char oldPortVal = getPortVal();
  for (unsigned int t = 0; t < length; tenthMicros += tenthMicrosPerLoop) {
    // note: the loop body was written without conditionals in order to ensure constant timing

    // produce one bit
    int sampleValue = (char)pgm_read_byte(data + t); // sample value to output (stored in EEPROM=PROGMEM)
    sampleValue += residual;                  // incorporate error from last sample (note: we need 9 bits here, hence 'int')
    char outBit = (sampleValue >= 0);         // quantize to 1 bit
    writeBit(outBit, oldPortVal);             // send to digital port
    char outValue = (254 & -outBit) - 127;    // convert bit to actual value (127 or -127) (note: avoid multiplication or conditional)
    residual = (char)sampleValue - outValue;  // compute quantization error of this step (note: guaranteed to fit into 8 bits)

    // advance the time index if the time duration of a sample period has elapsed
    int advance = (tenthMicros > tenthMicrosPerPeriod);
    tenthMicros -= tenthMicrosPerPeriod & -advance;
    t += advance;
  }
}

// the audio waveform, in 8 kHz signed 8-bit
// This was synthesized with the TTS built into Minecraft :)
PROGMEM const char audioData[] = { // PROGMEM -> reside in EEPROM
  128-128, 129-128, 128-128, 129-128, 127-128, 129-128, 129-128, 127-128,
  129-128, 128-128, 128-128, 127-128, 128-128, 129-128, 127-128, 129-128,
  128-128, 129-128, 128-128, 128-128, 128-128, 128-128, 128-128, 128-128,
  129-128, 126-128, 129-128, 127-128, 128-128, 127-128, 128-128, 128-128,
  128-128, 128-128, 128-128, 129-128, 128-128, 129-128, 129-128, 129-128,
  129-128, 129-128, 128-128, 128-128, 128-128, 129-128, 127-128, 128-128,
  127-128, 127-128, 127-128, 127-128, 127-128, 126-128, 126-128, 127-128,
  126-128, 127-128, 127-128, 127-128, 127-128, 128-128, 128-128, 129-128,
  128-128, 131-128, 129-128, 131-128, 130-128, 131-128, 129-128, 130-128,
  130-128, 129-128, 130-128, 130-128, 128-128, 129-128, 127-128, 127-128,
  127-128, 126-128, 126-128, 125-128, 127-128, 126-128, 125-128, 126-128,
  127-128, 126-128, 126-128, 127-128, 128-128, 127-128, 127-128, 130-128,
  128-128, 130-128, 128-128, 129-128, 136-128, 128-128, 125-128, 127-128,
  130-128, 128-128, 130-128, 136-128, 135-128, 130-128, 129-128, 122-128,
  120-128, 122-128, 124-128, 127-128, 134-128, 135-128, 135-128, 134-128,
  131-128, 125-128, 125-128, 126-128, 122-128, 121-128, 121-128, 118-128,
  118-128, 116-128, 117-128, 117-128, 121-128, 127-128, 130-128, 136-128,
  140-128, 144-128, 146-128, 149-128, 150-128, 149-128, 151-128, 148-128,
  144-128, 140-128, 133-128, 127-128, 120-128, 112-128, 110-128, 103-128,
  102-128, 98-128, 98-128, 97-128, 100-128, 102-128, 106-128, 113-128,
  116-128, 124-128, 129-128, 137-128, 143-128, 148-128, 155-128, 158-128,
  163-128, 163-128, 164-128, 163-128, 159-128, 155-128, 151-128, 145-128,
  139-128, 132-128, 126-128, 120-128, 114-128, 107-128, 102-128, 98-128,
  97-128, 95-128, 93-128, 97-128, 98-128, 100-128, 104-128, 110-128,
  113-128, 120-128, 125-128, 130-128, 134-128, 140-128, 144-128, 147-128,
  148-128, 156-128, 156-128, 148-128, 148-128, 150-128, 148-128, 145-128,
  147-128, 148-128, 140-128, 134-128, 126-128, 118-128, 113-128, 115-128,
  115-128, 120-128, 123-128, 123-128, 120-128, 121-128, 115-128, 111-128,
  114-128, 115-128, 113-128, 114-128, 117-128, 115-128, 117-128, 117-128,
  120-128, 123-128, 131-128, 135-128, 143-128, 148-128, 154-128, 157-128,
  160-128, 161-128, 159-128, 158-128, 158-128, 151-128, 145-128, 137-128,
  128-128, 118-128, 108-128, 101-128, 93-128, 89-128, 85-128, 82-128,
  83-128, 85-128, 86-128, 93-128, 99-128, 107-128, 115-128, 123-128,
  135-128, 144-128, 153-128, 160-128, 170-128, 175-128, 177-128, 177-128,
  178-128, 175-128, 172-128, 165-128, 159-128, 151-128, 142-128, 133-128,
  123-128, 114-128, 106-128, 100-128, 92-128, 90-128, 86-128, 85-128,
  86-128, 86-128, 90-128, 92-128, 98-128, 105-128, 110-128, 120-128,
  123-128, 132-128, 138-128, 144-128, 147-128, 151-128, 157-128, 167-128,
  161-128, 153-128, 157-128, 161-128, 155-128, 154-128, 156-128, 149-128,
  140-128, 131-128, 120-128, 110-128, 109-128, 110-128, 108-128, 113-128,
  115-128, 113-128, 111-128, 108-128, 103-128, 105-128, 109-128, 108-128,
  111-128, 118-128, 119-128, 121-128, 124-128, 126-128, 128-128, 133-128,
  141-128, 146-128, 154-128, 162-128, 162-128, 163-128, 166-128, 161-128,
  158-128, 158-128, 152-128, 145-128, 138-128, 128-128, 119-128, 107-128,
  98-128, 88-128, 84-128, 79-128, 78-128, 77-128, 80-128, 83-128,
  89-128, 95-128, 103-128, 109-128, 121-128, 133-128, 141-128, 152-128,
  163-128, 170-128, 177-128, 183-128, 181-128, 184-128, 181-128, 178-128,
  172-128, 167-128, 156-128, 148-128, 140-128, 128-128, 119-128, 108-128,
  99-128, 91-128, 85-128, 81-128, 78-128, 77-128, 80-128, 83-128,
  87-128, 93-128, 97-128, 107-128, 115-128, 124-128, 132-128, 140-128,
  149-128, 155-128, 159-128, 159-128, 165-128, 178-128, 162-128, 150-128,
  157-128, 160-128, 155-128, 155-128, 156-128, 141-128, 133-128, 124-128,
  105-128, 100-128, 103-128, 105-128, 108-128, 115-128, 114-128, 112-128,
  113-128, 109-128, 101-128, 107-128, 110-128, 107-128, 109-128, 113-128,
  115-128, 117-128, 125-128, 122-128, 129-128, 139-128, 143-128, 153-128,
  163-128, 169-128, 174-128, 178-128, 179-128, 177-128, 173-128, 168-128,
  159-128, 150-128, 136-128, 122-128, 111-128, 97-128, 86-128, 76-128,
  68-128, 60-128, 59-128, 59-128, 62-128, 68-128, 77-128, 85-128,
  97-128, 107-128, 120-128, 132-128, 146-128, 160-128, 174-128, 184-128,
  190-128, 196-128, 199-128, 197-128, 195-128, 189-128, 183-128, 176-128,
  168-128, 159-128, 146-128, 134-128, 123-128, 111-128, 104-128, 90-128,
  86-128, 78-128, 76-128, 72-128, 75-128, 73-128, 78-128, 80-128,
  87-128, 92-128, 101-128, 107-128, 116-128, 122-128, 130-128, 135-128,
  143-128, 144-128, 171-128, 162-128, 147-128, 152-128, 160-128, 173-128,
  178-128, 183-128, 160-128, 160-128, 155-128, 141-128, 132-128, 122-128,
  120-128, 126-128, 132-128, 135-128, 126-128, 131-128, 120-128, 118-128,
  111-128, 96-128, 83-128, 69-128, 68-128, 65-128, 79-128, 82-128,
  91-128, 106-128, 126-128, 147-128, 167-128, 182-128, 190-128, 200-128,
  207-128, 200-128, 196-128, 180-128, 166-128, 149-128, 131-128, 109-128,
  90-128, 75-128, 63-128, 58-128, 58-128, 53-128, 63-128, 68-128,
  85-128, 99-128, 113-128, 128-128, 135-128, 153-128, 156-128, 171-128,
  166-128, 174-128, 170-128, 180-128, 178-128, 177-128, 171-128, 167-128,
  160-128, 155-128, 143-128, 135-128, 122-128, 116-128, 107-128, 102-128,
  97-128, 92-128, 90-128, 92-128, 94-128, 96-128, 94-128, 101-128,
  101-128, 115-128, 110-128, 123-128, 120-128, 121-128, 132-128, 127-128,
  142-128, 130-128, 146-128, 138-128, 147-128, 142-128, 187-128, 156-128,
  121-128, 135-128, 145-128, 171-128, 187-128, 175-128, 135-128, 145-128,
  124-128, 119-128, 107-128, 100-128, 87-128, 121-128, 148-128, 136-128,
  154-128, 134-128, 123-128, 113-128, 112-128, 73-128, 47-128, 29-128,
  19-128, 56-128, 81-128, 105-128, 122-128, 155-128, 186-128, 224-128,
  244-128, 236-128, 228-128, 213-128, 202-128, 184-128, 153-128, 121-128,
  86-128, 82-128, 70-128, 62-128, 55-128, 47-128, 64-128, 79-128,
  102-128, 104-128, 113-128, 119-128, 130-128, 149-128, 150-128, 141-128,
  139-128, 137-128, 145-128, 142-128, 147-128, 129-128, 151-128, 151-128,
  168-128, 170-128, 162-128, 156-128, 154-128, 152-128, 141-128, 127-128,
  113-128, 98-128, 103-128, 103-128, 97-128, 104-128, 95-128, 114-128,
  112-128, 130-128, 116-128, 125-128, 119-128, 125-128, 128-128, 122-128,
  115-128, 115-128, 114-128, 126-128, 122-128, 131-128, 126-128, 122-128,
  150-128, 190-128, 116-128, 123-128, 125-128, 150-128, 187-128, 209-128,
  164-128, 128-128, 124-128, 101-128, 101-128, 117-128, 88-128, 113-128,
  145-128, 167-128, 171-128, 171-128, 133-128, 95-128, 102-128, 68-128,
  54-128, 38-128, 19-128, 51-128, 103-128, 154-128, 173-128, 193-128,
  193-128, 207-128, 230-128, 213-128, 185-128, 153-128, 120-128, 120-128,
  115-128, 111-128, 85-128, 90-128, 85-128, 100-128, 114-128, 110-128,
  97-128, 106-128, 101-128, 118-128, 120-128, 113-128, 105-128, 118-128,
  131-128, 141-128, 144-128, 133-128, 129-128, 141-128, 147-128, 146-128,
  155-128, 146-128, 158-128, 164-128, 177-128, 160-128, 155-128, 136-128,
  128-128, 119-128, 110-128, 101-128, 99-128, 99-128, 108-128, 115-128,
  126-128, 123-128, 126-128, 122-128, 120-128, 122-128, 115-128, 114-128,
  105-128, 104-128, 108-128, 113-128, 119-128, 124-128, 131-128, 136-128,
  144-128, 135-128, 158-128, 192-128, 116-128, 111-128, 117-128, 143-128,
  184-128, 208-128, 168-128, 127-128, 113-128, 92-128, 96-128, 117-128,
  102-128, 120-128, 150-128, 167-128, 187-128, 178-128, 147-128, 100-128,
  86-128, 61-128, 45-128, 37-128, 22-128, 46-128, 103-128, 157-128,
  190-128, 210-128, 197-128, 193-128, 202-128, 192-128, 177-128, 146-128,
  114-128, 97-128, 113-128, 120-128, 121-128, 123-128, 102-128, 105-128,
  105-128, 112-128, 106-128, 98-128, 89-128, 87-128, 103-128, 111-128,
  116-128, 132-128, 124-128, 143-128, 144-128, 152-128, 144-128, 138-128,
  135-128, 129-128, 146-128, 148-128, 162-128, 164-128, 165-128, 161-128,
  161-128, 147-128, 139-128, 118-128, 108-128, 100-128, 101-128, 111-128,
  116-128, 119-128, 125-128, 129-128, 131-128, 128-128, 117-128, 108-128,
  97-128, 101-128, 101-128, 101-128, 100-128, 107-128, 117-128, 126-128,
  145-128, 138-128, 149-128, 142-128, 133-128, 188-128, 154-128, 107-128,
  109-128, 113-128, 155-128, 202-128, 211-128, 155-128, 128-128, 88-128,
  73-128, 110-128, 117-128, 131-128, 143-128, 156-128, 168-128, 184-128,
  179-128, 131-128, 90-128, 55-128, 27-128, 38-128, 41-128, 53-128,
  87-128, 130-128, 170-128, 205-128, 210-128, 196-128, 178-128, 167-128,
  153-128, 149-128, 135-128, 113-128, 115-128, 119-128, 135-128, 143-128,
  141-128, 122-128, 103-128, 95-128, 85-128, 91-128, 93-128, 88-128,
  90-128, 96-128, 103-128, 126-128, 142-128, 148-128, 149-128, 148-128,
  140-128, 139-128, 141-128, 131-128, 133-128, 143-128, 149-128, 166-128,
  179-128, 175-128, 164-128, 154-128, 136-128, 124-128, 113-128, 104-128,
  100-128, 103-128, 113-128, 122-128, 131-128, 135-128, 126-128, 123-128,
  104-128, 100-128, 92-128, 94-128, 92-128, 92-128, 106-128, 115-128,
  131-128, 144-128, 149-128, 147-128, 147-128, 147-128, 144-128, 139-128,
  138-128, 168-128, 142-128, 103-128, 105-128, 112-128, 157-128, 206-128,
  215-128, 165-128, 133-128, 72-128, 57-128, 88-128, 111-128, 133-128,
  155-128, 156-128, 163-128, 176-128, 170-128, 126-128, 82-128, 38-128,
  12-128, 30-128, 51-128, 75-128, 114-128, 159-128, 187-128, 213-128,
  214-128, 194-128, 168-128, 157-128, 137-128, 137-128, 129-128, 124-128,
  123-128, 137-128, 144-128, 148-128, 151-128, 125-128, 106-128, 82-128,
  70-128, 70-128, 77-128, 82-128, 85-128, 95-128, 112-128, 126-128,
  158-128, 159-128, 161-128, 147-128, 133-128, 129-128, 129-128, 137-128,
  133-128, 149-128, 152-128, 168-128, 180-128, 176-128, 161-128, 151-128,
  121-128, 118-128, 106-128, 108-128, 107-128, 113-128, 118-128, 124-128,
  136-128, 129-128, 126-128, 114-128, 101-128, 95-128, 94-128, 98-128,
  100-128, 107-128, 116-128, 125-128, 136-128, 146-128, 141-128, 153-128,
  134-128, 143-128, 124-128, 127-128, 131-128, 125-128, 134-128, 180-128,
  159-128, 116-128, 119-128, 118-128, 159-128, 206-128, 203-128, 145-128,
  101-128, 52-128, 41-128, 85-128, 117-128, 140-128, 164-128, 168-128,
  168-128, 177-128, 163-128, 115-128, 79-128, 40-128, 29-128, 41-128,
  63-128, 82-128, 120-128, 160-128, 195-128, 209-128, 214-128, 180-128,
  164-128, 145-128, 141-128, 144-128, 134-128, 134-128, 120-128, 131-128,
  135-128, 143-128, 134-128, 116-128, 87-128, 78-128, 66-128, 75-128,
  88-128, 97-128, 109-128, 118-128, 129-128, 141-128, 155-128, 152-128,
  148-128, 137-128, 123-128, 120-128, 125-128, 130-128, 145-128, 155-128,
  160-128, 172-128, 171-128, 174-128, 153-128, 145-128, 120-128, 108-128,
  108-128, 102-128, 111-128, 113-128, 122-128, 123-128, 131-128, 130-128,
  126-128, 120-128, 113-128, 97-128, 104-128, 103-128, 110-128, 115-128,
  121-128, 123-128, 126-128, 139-128, 134-128, 136-128, 136-128, 124-128,
  125-128, 126-128, 134-128, 138-128, 147-128, 148-128, 141-128, 144-128,
  182-128, 156-128, 120-128, 108-128, 101-128, 130-128, 176-128, 187-128,
  146-128, 110-128, 66-128, 51-128, 83-128, 123-128, 135-128, 161-128,
  156-128, 152-128, 160-128, 157-128, 121-128, 91-128, 68-128, 42-128,
  61-128, 77-128, 90-128, 128-128, 158-128, 189-128, 202-128, 208-128,
  179-128, 162-128, 149-128, 135-128, 134-128, 125-128, 115-128, 116-128,
  122-128, 132-128, 138-128, 140-128, 121-128, 102-128, 88-128, 81-128,
  87-128, 88-128, 96-128, 95-128, 109-128, 125-128, 136-128, 159-128,
  154-128, 149-128, 140-128, 129-128, 130-128, 123-128, 139-128, 132-128,
  148-128, 157-128, 161-128, 171-128, 167-128, 156-128, 145-128, 129-128,
  120-128, 108-128, 105-128, 104-128, 100-128, 113-128, 116-128, 132-128,
  135-128, 136-128, 128-128, 120-128, 114-128, 108-128, 112-128, 109-128,
  108-128, 107-128, 111-128, 119-128, 129-128, 137-128, 134-128, 138-128,
  130-128, 133-128, 130-128, 141-128, 140-128, 138-128, 139-128, 143-128,
  140-128, 147-128, 141-128, 177-128, 149-128, 102-128, 98-128, 99-128,
  138-128, 177-128, 186-128, 136-128, 89-128, 62-128, 52-128, 88-128,
  118-128, 131-128, 144-128, 161-128, 170-128, 180-128, 177-128, 133-128,
  87-128, 64-128, 58-128, 60-128, 85-128, 89-128, 111-128, 150-128,
  182-128, 203-128, 209-128, 187-128, 158-128, 148-128, 140-128, 132-128,
  125-128, 120-128, 105-128, 121-128, 129-128, 139-128, 136-128, 118-128,
  95-128, 74-128, 82-128, 84-128, 97-128, 107-128, 106-128, 117-128,
  131-128, 148-128, 158-128, 161-128, 146-128, 131-128, 125-128, 122-128,
  124-128, 129-128, 128-128, 139-128, 150-128, 167-128, 174-128, 175-128,
  156-128, 143-128, 129-128, 119-128, 115-128, 102-128, 105-128, 104-128,
  116-128, 126-128, 132-128, 129-128, 129-128, 118-128, 118-128, 113-128,
  105-128, 104-128, 105-128, 111-128, 116-128, 129-128, 125-128, 132-128,
  132-128, 137-128, 140-128, 142-128, 136-128, 131-128, 131-128, 136-128,
  136-128, 144-128, 144-128, 135-128, 138-128, 137-128, 144-128, 129-128,
  147-128, 152-128, 102-128, 93-128, 90-128, 118-128, 156-128, 185-128,
  158-128, 116-128, 90-128, 73-128, 79-128, 111-128, 117-128, 133-128,
  158-128, 181-128, 189-128, 185-128, 152-128, 97-128, 74-128, 64-128,
  61-128, 70-128, 72-128, 89-128, 127-128, 181-128, 208-128, 210-128,
  192-128, 158-128, 148-128, 147-128, 144-128, 123-128, 107-128, 101-128,
  104-128, 126-128, 138-128, 129-128, 123-128, 103-128, 98-128, 98-128,
  104-128, 104-128, 98-128, 109-128, 104-128, 119-128, 133-128, 140-128,
  145-128, 148-128, 145-128, 139-128, 134-128, 131-128, 129-128, 132-128,
  142-128, 149-128, 161-128, 163-128, 164-128, 155-128, 146-128, 134-128,
  116-128, 113-128, 101-128, 103-128, 111-128, 117-128, 127-128, 126-128,
  131-128, 127-128, 130-128, 122-128, 113-128, 113-128, 103-128, 109-128,
  109-128, 118-128, 118-128, 125-128, 130-128, 133-128, 143-128, 138-128,
  136-128, 131-128, 129-128, 127-128, 136-128, 130-128, 131-128, 130-128,
  135-128, 140-128, 148-128, 151-128, 149-128, 145-128, 135-128, 116-128,
  131-128, 148-128, 102-128, 89-128, 96-128, 117-128, 146-128, 179-128,
  153-128, 116-128, 107-128, 92-128, 91-128, 108-128, 111-128, 118-128,
  152-128, 174-128, 175-128, 177-128, 155-128, 117-128, 109-128, 95-128,
  79-128, 59-128, 58-128, 70-128, 107-128, 156-128, 168-128, 178-128,
  182-128, 186-128, 191-128, 189-128, 166-128, 130-128, 119-128, 109-128,
  113-128, 114-128, 107-128, 100-128, 104-128, 114-128, 110-128, 107-128,
  99-128, 87-128, 95-128, 106-128, 113-128, 116-128, 125-128, 135-128,
  147-128, 160-128, 154-128, 145-128, 140-128, 137-128, 141-128, 140-128,
  139-128, 137-128, 146-128, 155-128, 161-128, 155-128, 143-128, 134-128,
  125-128, 127-128, 114-128, 112-128, 102-128, 107-128, 113-128, 122-128,
  123-128, 122-128, 117-128, 118-128, 115-128, 121-128, 115-128, 115-128,
  113-128, 118-128, 121-128, 125-128, 129-128, 128-128, 133-128, 134-128,
  134-128, 133-128, 133-128, 132-128, 137-128, 134-128, 141-128, 138-128,
  142-128, 142-128, 145-128, 144-128, 139-128, 137-128, 138-128, 126-128,
  123-128, 145-128, 121-128, 83-128, 97-128, 118-128, 135-128, 154-128,
  142-128, 112-128, 111-128, 118-128, 106-128, 107-128, 107-128, 107-128,
  141-128, 171-128, 172-128, 163-128, 161-128, 142-128, 128-128, 118-128,
  86-128, 60-128, 60-128, 70-128, 95-128, 125-128, 143-128, 149-128,
  173-128, 192-128, 193-128, 191-128, 170-128, 151-128, 142-128, 133-128,
  119-128, 105-128, 96-128, 92-128, 99-128, 109-128, 100-128, 99-128,
  104-128, 111-128, 116-128, 122-128, 114-128, 115-128, 123-128, 133-128,
  140-128, 142-128, 137-128, 131-128, 142-128, 138-128, 140-128, 137-128,
  140-128, 145-128, 153-128, 157-128, 151-128, 151-128, 146-128, 143-128,
  134-128, 122-128, 106-128, 106-128, 106-128, 111-128, 113-128, 114-128,
  117-128, 121-128, 133-128, 129-128, 130-128, 124-128, 122-128, 123-128,
  118-128, 118-128, 110-128, 117-128, 117-128, 121-128, 128-128, 128-128,
  133-128, 133-128, 136-128, 133-128, 135-128, 133-128, 136-128, 140-128,
  137-128, 136-128, 138-128, 144-128, 141-128, 137-128, 125-128, 144-128,
  125-128, 110-128, 116-128, 128-128, 136-128, 137-128, 134-128, 119-128,
  125-128, 125-128, 110-128, 104-128, 105-128, 113-128, 131-128, 147-128,
  142-128, 143-128, 151-128, 147-128, 140-128, 130-128, 108-128, 94-128,
  91-128, 88-128, 94-128, 107-128, 116-128, 132-128, 151-128, 165-128,
  168-128, 177-128, 175-128, 170-128, 161-128, 148-128, 130-128, 116-128,
  113-128, 100-128, 98-128, 91-128, 86-128, 92-128, 101-128, 109-128,
  113-128, 122-128, 124-128, 128-128, 138-128, 140-128, 140-128, 141-128,
  137-128, 137-128, 139-128, 134-128, 132-128, 129-128, 130-128, 135-128,
  143-128, 148-128, 151-128, 151-128, 150-128, 147-128, 141-128, 134-128,
  124-128, 114-128, 110-128, 106-128, 107-128, 109-128, 112-128, 113-128,
  119-128, 121-128, 126-128, 132-128, 130-128, 130-128, 128-128, 122-128,
  119-128, 119-128, 119-128, 120-128, 124-128, 127-128, 129-128, 137-128,
  136-128, 138-128, 138-128, 136-128, 136-128, 138-128, 136-128, 130-128,
  131-128, 132-128, 133-128, 134-128, 139-128, 135-128, 126-128, 125-128,
  128-128, 132-128, 129-128, 126-128, 123-128, 120-128, 121-128, 116-128,
  111-128, 112-128, 117-128, 124-128, 129-128, 130-128, 134-128, 139-128,
  144-128, 143-128, 139-128, 130-128, 125-128, 121-128, 117-128, 113-128,
  110-128, 112-128, 115-128, 122-128, 127-128, 134-128, 142-128, 152-128,
  155-128, 156-128, 151-128, 144-128, 139-128, 132-128, 127-128, 118-128,
  111-128, 108-128, 104-128, 102-128, 104-128, 106-128, 113-128, 115-128,
  122-128, 125-128, 131-128, 137-128, 140-128, 141-128, 141-128, 138-128,
  136-128, 135-128, 131-128, 131-128, 129-128, 134-128, 136-128, 137-128,
  137-128, 138-128, 141-128, 139-128, 137-128, 132-128, 128-128, 126-128,
  122-128, 120-128, 116-128, 115-128, 114-128, 116-128, 117-128, 119-128,
  120-128, 124-128, 128-128, 130-128, 130-128, 128-128, 127-128, 129-128,
  125-128, 127-128, 126-128, 124-128, 127-128, 127-128, 130-128, 134-128,
  134-128, 136-128, 136-128, 136-128, 132-128, 133-128, 131-128, 130-128,
  129-128, 131-128, 130-128, 126-128, 129-128, 129-128, 128-128, 125-128,
  124-128, 123-128, 125-128, 127-128, 123-128, 122-128, 123-128, 127-128,
  129-128, 131-128, 128-128, 129-128, 133-128, 130-128, 127-128, 127-128,
  124-128, 125-128, 126-128, 127-128, 126-128, 124-128, 125-128, 128-128,
  128-128, 129-128, 127-128, 129-128, 135-128, 138-128, 138-128, 139-128,
  136-128, 139-128, 134-128, 131-128, 128-128, 123-128, 118-128, 118-128,
  115-128, 113-128, 112-128, 113-128, 116-128, 118-128, 121-128, 126-128,
  130-128, 135-128, 138-128, 136-128, 138-128, 137-128, 139-128, 138-128,
  136-128, 132-128, 129-128, 129-128, 129-128, 130-128, 130-128, 133-128,
  131-128, 132-128, 130-128, 128-128, 130-128, 128-128, 125-128, 126-128,
  122-128, 120-128, 119-128, 120-128, 119-128, 118-128, 118-128, 122-128,
  127-128, 128-128, 129-128, 132-128, 133-128, 136-128, 133-128, 129-128,
  123-128, 125-128, 125-128, 127-128, 127-128, 129-128, 128-128, 133-128,
  136-128, 137-128, 137-128, 139-128, 135-128, 136-128, 132-128, 131-128,
  129-128, 127-128, 127-128, 125-128, 123-128, 119-128, 118-128, 117-128,
  120-128, 117-128, 117-128, 117-128, 119-128, 123-128, 128-128, 132-128,
  132-128, 135-128, 136-128, 134-128, 134-128, 138-128, 135-128, 136-128,
  136-128, 136-128, 137-128, 140-128, 135-128, 134-128, 129-128, 126-128,
  124-128, 123-128, 120-128, 117-128, 115-128, 119-128, 122-128, 123-128,
  125-128, 124-128, 123-128, 122-128, 121-128, 124-128, 124-128, 120-128,
  121-128, 125-128, 128-128, 129-128, 132-128, 131-128, 131-128, 135-128,
  137-128, 138-128, 141-128, 137-128, 137-128, 135-128, 138-128, 137-128,
  135-128, 132-128, 130-128, 126-128, 128-128, 126-128, 126-128, 124-128,
  122-128, 125-128, 124-128, 124-128, 124-128, 122-128, 122-128, 123-128,
  122-128, 120-128, 119-128, 120-128, 119-128, 122-128, 125-128, 128-128,
  131-128, 132-128, 135-128, 137-128, 137-128, 136-128, 134-128, 133-128,
  129-128, 129-128, 129-128, 127-128, 126-128, 127-128, 128-128, 129-128,
  129-128, 130-128, 130-128, 131-128, 129-128, 129-128, 126-128, 124-128,
  122-128, 123-128, 125-128, 125-128, 129-128, 129-128, 130-128, 132-128,
  133-128, 133-128, 131-128, 129-128, 127-128, 125-128, 127-128, 124-128,
  126-128, 123-128, 125-128, 127-128, 126-128, 128-128, 126-128, 128-128,
  128-128, 130-128, 131-128, 132-128, 134-128, 130-128, 128-128, 129-128,
  130-128, 128-128, 127-128, 127-128, 126-128, 126-128, 128-128, 129-128,
  126-128, 129-128, 126-128, 126-128, 126-128, 128-128, 128-128, 127-128,
  127-128, 126-128, 128-128, 129-128, 127-128, 130-128, 129-128, 130-128,
  128-128, 129-128, 131-128, 129-128, 130-128, 130-128, 130-128, 129-128,
  128-128, 128-128, 128-128, 127-128, 127-128, 127-128, 129-128, 127-128,
  129-128, 127-128, 126-128, 127-128, 127-128, 128-128, 128-128, 128-128,
  125-128, 126-128, 127-128, 128-128, 126-128, 126-128, 126-128, 127-128,
  127-128, 127-128, 130-128, 129-128, 133-128, 129-128, 128-128, 129-128,
  127-128, 130-128, 127-128, 129-128, 129-128, 129-128, 129-128, 130-128,
  130-128, 129-128, 129-128, 128-128, 126-128, 124-128, 127-128, 127-128,
  127-128, 126-128, 128-128, 127-128, 131-128, 129-128, 131-128, 130-128,
  130-128, 127-128, 128-128, 127-128, 127-128, 128-128, 127-128, 125-128,
  128-128, 129-128, 130-128, 128-128, 126-128, 128-128, 126-128, 129-128,
  127-128, 127-128, 130-128, 127-128, 129-128, 130-128, 129-128, 129-128,
  126-128, 126-128, 129-128, 126-128, 127-128, 125-128, 128-128, 125-128,
  130-128, 126-128, 131-128, 130-128, 130-128, 126-128, 128-128, 125-128,
  128-128, 127-128, 129-128, 126-128, 131-128, 131-128, 132-128, 132-128,
  132-128, 131-128, 129-128, 126-128, 127-128, 124-128, 127-128, 127-128,
  126-128, 126-128, 127-128, 129-128, 129-128, 130-128, 129-128, 128-128,
  126-128, 126-128, 127-128, 127-128, 127-128, 126-128, 129-128, 129-128,
  131-128, 129-128, 128-128, 127-128, 127-128, 127-128, 128-128, 126-128,
  125-128, 126-128, 128-128, 130-128, 127-128, 131-128, 126-128, 130-128,
  126-128, 130-128, 127-128, 128-128, 128-128, 129-128, 130-128, 127-128,
  130-128, 128-128, 128-128, 128-128, 128-128, 127-128, 128-128, 128-128,
  131-128, 128-128, 131-128, 125-128, 130-128, 128-128, 129-128, 128-128,
  125-128, 126-128, 126-128, 127-128, 128-128, 127-128, 129-128, 129-128,
  127-128, 127-128, 128-128, 131-128, 126-128, 127-128, 128-128, 128-128,
  128-128, 130-128, 129-128, 128-128, 129-128, 128-128, 128-128, 126-128,
  128-128, 127-128, 126-128, 126-128, 127-128, 128-128, 129-128, 129-128,
  129-128, 128-128, 128-128, 129-128, 129-128, 127-128, 127-128, 129-128,
  131-128, 130-128, 129-128, 128-128, 127-128, 129-128, 126-128, 128-128,
  127-128, 128-128, 128-128, 128-128, 127-128, 129-128, 130-128, 128-128,
  126-128, 129-128, 127-128, 127-128, 129-128, 126-128, 129-128, 127-128,
  127-128, 128-128, 125-128, 128-128, 129-128, 129-128, 130-128, 129-128,
  131-128, 130-128, 129-128, 129-128, 128-128, 128-128, 126-128, 128-128,
  126-128, 127-128, 127-128, 128-128, 129-128, 126-128, 129-128, 128-128,
  129-128, 127-128, 129-128, 128-128, 129-128, 127-128, 128-128, 127-128,
  128-128, 126-128, 127-128, 127-128, 126-128, 127-128, 129-128, 129-128,
  129-128, 130-128, 129-128, 131-128, 129-128, 131-128, 129-128, 129-128,
  127-128, 126-128, 127-128, 128-128, 128-128, 127-128, 128-128, 128-128,
  129-128, 129-128, 129-128, 128-128, 128-128, 128-128, 126-128, 126-128,
  126-128, 128-128, 127-128, 128-128, 126-128, 127-128, 129-128, 128-128,
  128-128, 128-128, 128-128, 129-128, 128-128, 129-128, 129-128, 129-128,
  127-128, 129-128, 128-128, 129-128, 130-128, 128-128, 128-128, 130-128,
  129-128, 128-128, 128-128, 127-128, 127-128, 126-128, 127-128, 127-128,
  129-128, 127-128, 126-128, 127-128, 128-128, 128-128, 128-128, 128-128,
  127-128, 130-128, 129-128, 130-128, 128-128, 129-128, 129-128, 129-128,
  128-128, 128-128, 130-128, 128-128, 129-128, 129-128, 128-128, 127-128,
  127-128, 127-128, 127-128, 126-128, 128-128, 126-128, 128-128, 126-128,
  128-128, 129-128, 129-128, 129-128, 128-128, 129-128, 130-128, 129-128,
  129-128, 128-128, 127-128, 127-128, 127-128, 127-128, 126-128, 127-128,
  128-128, 130-128, 127-128, 128-128, 129-128, 128-128, 129-128, 129-128,
  128-128, 129-128, 128-128, 129-128, 129-128, 127-128, 127-128, 129-128,
  127-128, 128-128, 126-128, 129-128, 129-128, 129-128, 129-128, 127-128,
  128-128, 130-128, 128-128, 127-128, 128-128, 128-128, 127-128, 129-128,
  128-128, 126-128, 130-128, 127-128, 127-128, 128-128, 127-128, 127-128,
  128-128, 126-128, 127-128, 128-128, 129-128, 129-128, 128-128, 128-128,
  129-128, 129-128, 129-128, 128-128, 127-128, 126-128, 129-128, 130-128,
  128-128, 129-128, 127-128, 129-128, 128-128, 128-128, 127-128, 128-128,
  127-128, 127-128, 128-128, 129-128, 127-128, 128-128, 126-128, 129-128,
  128-128, 128-128, 128-128, 128-128, 129-128, 129-128, 128-128, 129-128,
  129-128, 127-128, 128-128, 127-128, 127-128, 125-128, 128-128, 127-128,
  128-128, 128-128, 128-128, 127-128, 129-128, 127-128, 128-128, 128-128,
  129-128, 128-128, 129-128, 128-128, 129-128, 130-128, 128-128, 129-128,
  127-128, 127-128, 127-128, 126-128, 127-128, 126-128, 127-128, 128-128,
  129-128, 128-128, 128-128, 129-128, 128-128, 129-128, 130-128, 129-128,
  130-128, 128-128, 128-128, 128-128, 129-128, 127-128, 128-128, 127-128,
  128-128, 127-128, 128-128, 127-128, 127-128, 128-128, 126-128, 129-128,
  128-128, 128-128, 129-128, 129-128, 128-128, 129-128, 128-128, 129-128,
  128-128, 127-128, 127-128, 127-128, 128-128, 129-128, 128-128, 128-128,
  129-128, 127-128, 128-128, 127-128, 128-128, 126-128, 129-128, 127-128,
  129-128, 130-128, 129-128, 130-128, 128-128, 129-128, 127-128, 128-128,
  127-128, 126-128, 127-128, 127-128, 126-128, 127-128, 126-128, 128-128,
  127-128, 127-128, 126-128, 129-128, 128-128, 130-128, 129-128, 129-128,
  130-128, 129-128, 129-128, 130-128, 131-128, 129-128, 128-128, 127-128,
  128-128, 128-128, 128-128, 127-128, 127-128, 125-128, 128-128, 127-128,
  128-128, 128-128, 127-128, 128-128, 128-128, 128-128, 128-128, 128-128,
  127-128, 127-128, 129-128, 128-128, 128-128, 128-128, 127-128, 130-128,
  128-128, 127-128, 129-128, 126-128, 128-128, 125-128, 129-128, 128-128,
  128-128, 128-128, 129-128, 128-128, 130-128, 129-128, 127-128, 130-128,
  128-128, 130-128, 128-128, 128-128, 127-128, 128-128, 127-128, 128-128,
  128-128, 128-128, 127-128, 129-128, 129-128, 127-128, 128-128, 129-128,
  129-128, 127-128, 128-128, 128-128, 128-128, 127-128, 128-128, 128-128,
  128-128, 128-128, 127-128, 130-128, 128-128, 129-128, 128-128, 130-128,
  128-128, 128-128, 129-128, 127-128, 128-128, 128-128, 127-128, 128-128,
  128-128, 128-128, 129-128, 128-128, 127-128, 130-128, 129-128, 128-128,
  128-128, 127-128, 128-128, 128-128, 128-128, 127-128, 127-128, 127-128,
  129-128, 129-128, 128-128, 127-128, 129-128, 128-128, 129-128, 129-128,
  129-128, 129-128, 129-128, 129-128, 129-128, 129-128, 127-128, 129-128,
  127-128, 128-128, 127-128, 128-128, 128-128, 128-128, 128-128, 128-128,
  128-128, 127-128, 129-128, 128-128, 127-128, 127-128, 128-128, 128-128,
  128-128, 129-128, 129-128, 128-128, 128-128, 128-128, 128-128, 128-128,
  128-128, 127-128, 128-128, 127-128, 128-128, 128-128, 128-128, 128-128,
  128-128, 128-128, 129-128, 127-128, 130-128, 128-128, 128-128, 129-128,
  128-128, 129-128, 127-128, 128-128, 129-128, 126-128, 128-128, 127-128,
  127-128, 129-128, 127-128, 129-128, 127-128, 128-128, 128-128, 128-128,
  128-128, 128-128, 128-128, 129-128, 127-128, 129-128, 128-128, 129-128,
  128-128, 128-128, 129-128, 128-128, 129-128, 129-128, 128-128, 128-128,
  129-128, 128-128, 128-128, 127-128, 128-128, 126-128, 128-128, 127-128,
  129-128, 128-128, 127-128, 128-128, 128-128, 128-128, 129-128, 128-128,
  127-128, 128-128, 127-128, 128-128, 128-128, 127-128, 128-128, 127-128,
  128-128, 128-128, 128-128, 130-128, 128-128, 129-128, 128-128, 129-128,
  127-128, 128-128, 128-128, 127-128, 127-128, 127-128, 128-128, 127-128,
  129-128, 128-128, 129-128, 126-128, 129-128, 129-128, 127-128, 127-128,
  128-128, 128-128, 127-128, 127-128, 127-128, 127-128, 128-128, 129-128,
  127-128, 129-128, 128-128, 128-128, 129-128, 129-128, 128-128, 128-128,
  129-128, 128-128, 127-128, 128-128, 127-128, 129-128, 128-128, 127-128,
  129-128, 128-128, 128-128, 128-128, 129-128, 128-128, 129-128, 128-128,
  128-128, 129-128, 127-128, 127-128, 128-128, 127-128, 128-128, 130-128,
  127-128, 128-128, 129-128, 128-128, 128-128, 128-128, 128-128, 128-128,
  127-128, 127-128, 127-128, 127-128, 128-128, 129-128, 128-128, 128-128,
  128-128, 128-128, 129-128, 129-128, 128-128, 127-128, 129-128, 129-128,
  127-128, 128-128, 128-128, 127-128, 127-128, 128-128, 127-128, 127-128,
  126-128, 127-128, 128-128, 127-128, 127-128, 129-128, 129-128, 127-128,
  129-128, 128-128, 128-128, 128-128, 129-128, 127-128, 129-128, 129-128,
  128-128, 128-128, 130-128, 128-128, 128-128, 128-128, 129-128, 127-128,
  129-128, 127-128, 127-128, 129-128, 127-128, 129-128, 128-128, 128-128,
  128-128, 128-128, 128-128, 127-128, 129-128, 126-128, 128-128, 127-128,
  127-128, 128-128, 127-128, 128-128, 128-128, 128-128, 129-128, 128-128,
  128-128, 128-128, 129-128, 128-128, 129-128, 129-128, 128-128, 128-128,
  128-128, 128-128, 128-128, 129-128, 127-128, 127-128, 128-128, 128-128,
  127-128, 128-128, 128-128, 128-128, 127-128, 127-128, 128-128, 128-128,
  127-128, 128-128, 128-128, 127-128, 128-128, 129-128, 127-128, 129-128,
  129-128, 129-128, 128-128, 129-128, 128-128, 128-128, 127-128, 129-128,
  129-128, 126-128, 129-128, 128-128, 128-128, 127-128, 129-128, 128-128,
  128-128, 128-128, 128-128, 128-128, 129-128, 129-128, 128-128, 128-128,
  128-128, 128-128, 128-128, 126-128, 127-128, 128-128, 128-128, 127-128,
  129-128, 127-128, 127-128, 129-128, 128-128, 129-128, 127-128, 129-128,
  128-128, 128-128, 129-128, 128-128, 130-128, 129-128, 128-128, 128-128,
  128-128, 128-128, 129-128, 127-128, 129-128, 127-128, 128-128, 127-128,
  129-128, 127-128, 128-128, 127-128, 129-128, 128-128, 129-128, 128-128,
  128-128, 129-128, 128-128, 128-128, 130-128, 129-128, 126-128, 128-128,
  128-128, 128-128, 128-128, 128-128, 128-128, 129-128, 127-128, 128-128,
  129-128, 130-128, 127-128, 128-128, 129-128, 128-128, 129-128, 128-128,
  129-128, 128-128, 128-128, 126-128, 129-128, 128-128, 127-128, 127-128,
  127-128, 127-128, 128-128, 127-128, 128-128, 128-128, 127-128, 129-128,
  128-128, 128-128, 129-128, 128-128, 129-128, 128-128, 128-128, 129-128,
  128-128, 129-128, 127-128, 128-128, 128-128, 129-128, 127-128, 129-128
};
unsigned int audioLength = sizeof(audioData);
const int audioSampleRate = 8000;

// the loop function runs over and over again forever
void loop() {
  playSampleBuffer(audioData, audioLength, audioSampleRate);
  for(;;); // loop forever, since we want to play the sample only once
}

#include "daisysp.h"
#include "daisy_seed.h"
//#include <algorithm>
//#include <iostream>

using namespace daisysp;
using namespace daisy;
using daisy::Encoder;

//constants
const int numHarmonics = 5;
//objects 
static DaisySeed              hw;
static HarmonicOscillator<numHarmonics> harm;
Encoder encoders[5];
//globals
float amplitudes[numHarmonics];
float encoderValues[numHarmonics];
//function protos
void handleEncoders();
void SetAmplitudeValues();

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    handleEncoders();
    SetAmplitudeValues();
    harm.SetAmplitudes(amplitudes);

    for(size_t i = 0; i < size; i += 2)
    {
    out[i] = out[i + 1] = harm.Process();
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    //init harmonic oscillator
    harm.Init(sample_rate);
    harm.SetFirstHarmIdx(1);
    harm.SetFreq(440.0f);

    // initialize encoders 
    encoders[0].Init(hw.GetPin(19), hw.GetPin(20), hw.GetPin(27));
    encoders[1].Init(hw.GetPin(21), hw.GetPin(22), hw.GetPin(28));
    encoders[2].Init(hw.GetPin(23), hw.GetPin(24), hw.GetPin(29));
    encoders[3].Init(hw.GetPin(25), hw.GetPin(26), hw.GetPin(30));
    encoders[4].Init(hw.GetPin(3), hw.GetPin(2), hw.GetPin(4));

    hw.StartLog(true);

    // start callback
    hw.StartAudio(AudioCallback);
    while(1)
    {
 
    //hw.DelayMs(1000);
    for(int i= 0; i<numHarmonics;i++)
    {
    hw.PrintLine("Frequency:\t%d", encoderValues[i]);
    }
    }
}

// function defs
void handleEncoders()
{
    float totalSum = 0.0f;

    for (int i = 0; i < numHarmonics; i++)
    {
        encoders[i].Debounce();
        int encoderState = encoders[i].Increment(); // will return +1 when CW and -1 when CCW

        switch(encoderState)
        {
            case 1: encoderValues[i] += 0.01f; break; // Add a small value when turned CW
            case -1: encoderValues[i] -= 0.01f; break; // Subtract a small value when turned CCW
            default: break;
        }

        if(encoders[i].RisingEdge())
        {
            encoderValues[i] = 0.0f; // reset value to zero when clicked 
        }

        // Ensure values are within bounds
       // encoderValues[i] = std::max(0.0f, std::min(1.0f, encoderValues[i]));

        // Update the total sum
        totalSum += encoderValues[i];
    }

    // Normalize if the total sum exceeds 1
    if (totalSum > 1.0f)
    {
        for (int i = 0; i < numHarmonics; i++)
        {
            encoderValues[i] /= totalSum;
        }
    }
}

void SetAmplitudeValues()
    {
        for (int i = 0; i < numHarmonics; i++)
        {
            amplitudes[i] = encoderValues[i];
        }
    }

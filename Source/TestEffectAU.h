//
//  TestEffectAU.h
//  TestEffectAU
//
//  Created by Chris Nash on 03/10/2014.
//  Copyright (c) 2014 UWE. All rights reserved.
//

#pragma once

#include "TestEffectAUBase.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Wet Dry Mix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Float32 WetDry(Float32 fDry, Float32 fWet, Float32 fMixer)
    {
        
        fWet *= 1 - fMixer;
        fDry *= fMixer;
        
        
        fMixer = (fWet*0.5) + (fDry*0.5);
        fMixer = fMixer * 2;
        
        
        return fMixer;
    };
    





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//FILTER
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct MyFilter
{
    //Initialise function
    void initialise (Float32 coeff)
    {
        CoA = coeff;
        CoB = (1 - coeff);
        PrevSamp = 0;
    }
    
    //Process function
    Float32 process (Float32 input)
    {
        
        float fOut = (CoA * input) + (CoB * PrevSamp);
        PrevSamp = fOut;
        return fOut;
    }
    Float32 CoA, CoB;
    Float32 PrevSamp;
    
};




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Modulation (LFO)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct Mod
{
    //Declare local variables and constants for sinewave modulation
    const Float32 fTwoPI = 2 * M_PI;
    Float32 fRectify, fSine, fPhasePos;
    
    
    //Initialise function
    void initialise ()
    {
        //Set variables to 0
        fPhasePos = 0;
    }
    
    
    //Process function
    Float32 process (Float32 fFreqParam, Float32 fDepthParam, Float32 sampleRate, Float32 fOffset)
    {
        
    //Scale and offset variable fFreq
    fFreqParam = ((15 * fFreqParam) + 1);
        
    Float32 fPhaseInc = (fTwoPI * fFreqParam) / sampleRate;
    
    //Step through sinewave for each sample
    fPhasePos = (fPhasePos + fPhaseInc);
    
    //Test Phase pos if it is out of range then reet to 0
    if(fPhasePos > fTwoPI)  fPhasePos = fPhasePos - fTwoPI;
    
    
    //Add modulation rate and depth to Sinewave using given slider (offset if required)
    fSine = (sin(fPhasePos + fOffset) * fDepthParam);
    
    //Rectify sinewave to give a value between 0 and 1
    return fRectify = (0.5f * (fSine + 1.0f));
    }

    
};




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//DELAY
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct Delay
{
    //Shared variables for Delay function
    Float32 fSR;
    SInt32 iBufferSize, iBufferReadPos, iBufferWritePos;
    Float32 *pfCircularBuffer;
    struct MyFilter filter;
    
    //Initialise function
    SInt32 initialise (Float32 GetSampleRate)
    {
        //Initialise variables and assign them values as required
        fSR = GetSampleRate;
        iBufferSize = (SInt32)(2.0*fSR)+1;
        pfCircularBuffer = new Float32[iBufferSize];
        iBufferWritePos = 0;
        
        //Step through buffer and set every value to 0
        for(int iPos = 0; iPos < iBufferSize; iPos++)
            pfCircularBuffer[iPos] = 0;
        
        return iBufferSize;
    }
    
    //Write function
    void write (Float32 fInput)
    {
        //SORT BUFFER OUT
        iBufferWritePos++;
        
        // ensure buffer is within limits
        if( iBufferWritePos == iBufferSize ) iBufferWritePos = 0;
        
        // actually write to the buffer
        pfCircularBuffer[iBufferWritePos] = fInput;
        
    }
    
    //Read function
    Float32 read (Float32 fDelayTime)
    {
        //Calculate Delay Offset and store it in read position variable
        iBufferReadPos = iBufferWritePos - (fDelayTime);
        
        // Check if read position is below 0 if it is add buffer size to read the position from the end of the buffer (remembering that the buffer is circular this would still put the read position in the correct place)
        if ( iBufferReadPos < 0) iBufferReadPos = iBufferReadPos + iBufferSize;
        
        //Set output to be read position
        Float32 fOutput = pfCircularBuffer[iBufferReadPos];
        
        return fOutput;
    }
};




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TestEffectAU : public TestEffectAUBase
{
public:
	TestEffectAU(AudioUnit component) : TestEffectAUBase(component) {}
	
    // for preparing the creation and destruction of your plugin (e.g. allocating/freeing memory)
    OSStatus Initialize();
    void Cleanup();
		
	// callbacks use signed int values as sometimes can be dummy or user values (negative)
	void PresetLoaded(SInt32 iPresetNum, char *sPresetName);
	void OptionChanged(SInt32 iOptionMenu, SInt32 iItem);
	void ButtonPressed(SInt32 iButton);
	
	// tail time describes how long it takes for nominal-level signal to decay to silence at the
	// end of input
 	bool SupportsTail() { return true; }
	Float64 GetTailTime() { return 0; }
		
private:
    OSStatus ProcessCore(const Float32 *pfInBuffer0, const Float32 *pfInBuffer1,
						 Float32 *pfOutBuffer0, Float32 *pfOutBuffer1, UInt32 iInStride,
						 UInt32 iOutStride, UInt32 inFramesToProcess );
    
    //Declare all structures and variables to be used in TestEffect.cpp
    //Structures
    struct Delay delay[20];
    MyFilter smoothDelayTime;
    Mod modulation[2];
    
    //Booleans
    bool iTapTempo;
    bool iCount;
    
    //Floats
    Float32 fDelayTimeA,
            fSR,
            fDelayQuantize,
            fSmoothing,
            fMod1,
            fMod,
            fCentreLPF,
            fLPostFeedback,
            fRPostFeedback,
            fLFeedback,
            fRFeedback,
            fRFeedbackLPF,
            fLFeedbackLPF,
            fDelayModL,
            fDelayModR;
    
    //Intergrs
    SInt32  pluginMode,
            iBufferSize,
            filterMode,
            delayInSamples,
            iDelayBPM;
    };

//
//  TestEffectAU.cpp
//  TestEffectAU
//
//  Created by Chris Nash on 01/10/2014.
//  Copyright (c) 2014 UWE. All rights reserved.
//

#include "TestEffectAU.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AUDIOCOMPONENT_ENTRY(AUBaseFactory, TestEffectAU)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize and Cleanup are only called on creation and destruction

OSStatus TestEffectAU::Initialize()
{
    
    OSStatus res = TestEffectAUBase::Initialize(); // do basic setup
    
    // put your own additional initialisations here (e.g. allocating memory)
    
    
    // Initialise all variables to 0
    delayInSamples = 0;
    iCount = false;
    pluginMode = 0;
    filterMode = 0;
    fSmoothing = 0;
    fLPostFeedback = 0;
    fRPostFeedback = 0;
    fLFeedback = 0;
    fRFeedback = 0;
    fRFeedbackLPF = 0;
    fLFeedbackLPF = 0;
    fMod = 0;
    fMod1 = 0;
    fCentreLPF = 0;
    iTapTempo = 0;
    fDelayQuantize = 1;
    fDelayModL = 0;
    fDelayModR = 0;

    //Initialise variables in delay array
    for(int iDelayArray = 0; iDelayArray < 20; iDelayArray++)
    {
        delay[iDelayArray].initialise(GetSampleRate());
    }
    
    //Initialise variables in modulation delay
    for(int iModArray = 0; iModArray < 2; iModArray++)
    {
        modulation[iModArray].initialise();
    }

    // Declare and setup some filters to be used in the Centre delay feedback loop
    pFilters[kFilter0] -> FilterConfig(kLPF, 1000, 0);
    pFilters[kFilter1] -> FilterConfig(kLPF, 1000, 0);
    
    //Set buffersize to be used in process core
    fSR = GetSampleRate();
    iBufferSize = (SInt32)(2.0*fSR)+1;
    
    //This derives us an inital BPM of 120BPM regardless of system sample rate
    fDelayTimeA = ((GetSampleRate() * 60)/120);
    iDelayBPM = fDelayTimeA;
    
    // Initilise smoothing function
    smoothDelayTime.initialise(0.00005);
    
    return res;
}

void TestEffectAU::Cleanup()
{
    // put your own additional clean up code here (e.g. free memory)
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAU::PresetLoaded(SInt32 iPresetNum, char *sPresetName)
{
    // a preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using GetParameter and use them to set state variables in the plugin
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAU::OptionChanged(SInt32 iOptionMenu, SInt32 iItem)
{
    // the option menu, with index iOptionMenu, has been changed to the entry, iItem
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAU::ButtonPressed(SInt32 iButton)
{
    
    //Check if Kbutton4 has been pressed
    if(iButton == kButton4)
    {
        //turn on indicator
        pfIndicatorValues[kButton4] = true;
        //if the button is pressed and iCount is true store sample value in fDelayBPM, also se iCount to false
        if ( iCount ) {
            iCount = false;
            iDelayBPM = delayInSamples;
        }
        //if the button is pressed and iCount is false set delay in samples to 0 and set iCount to 0
        if ( !iCount ) {
            delayInSamples = 0;
            iCount = true;
        }
    }
    
    //Check to see if the LCR Delay button has been pressed
    if(iButton == kButton0)
    {
        pluginMode = 0;
        
        pfIndicatorValues[kButton0] = true;
        pfIndicatorValues[kButton1] = false;
        pfIndicatorValues[kButton2] = false;
    }
    
    //Check to see if the Ping Pong button has been pressed
    if(iButton == kButton1)
    {
        pluginMode = 1;
        
        pfIndicatorValues[kButton0] = false;
        pfIndicatorValues[kButton1] = true;
        pfIndicatorValues[kButton2] = false;
    }
    
    //Check to see if the Mod Delay button has been pressed
    if(iButton == kButton2)
    {
        pluginMode = 2;
    
        pfIndicatorValues[kButton0] = false;
        pfIndicatorValues[kButton1] = false;
        pfIndicatorValues[kButton2] = true;
    }
    
    
    //Check to see if LPF button has been pressed
    if(iButton == kButton3 && !pfIndicatorValues[kButton3])
    {
        filterMode = 1;
        pfIndicatorValues[kButton3] = true;
        
    }
    // check if it's pressed again, if so turn indiczator off and reset filter mode
    else if(iButton == kButton3 && pfIndicatorValues[kButton3])
    {
            
        filterMode = 0;
        pfIndicatorValues[kButton3] = false;
        
    };
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAU::ProcessCore(const Float32 *pfInBuffer0,
                                   const Float32 *pfInBuffer1,
                                   Float32 *pfOutBuffer0,
                                   Float32 *pfOutBuffer1,
                                   UInt32 iInStride,
                                   UInt32 iOutStride,
                                   UInt32 inFramesToProcess)
{
    // Declare Variables to be used within process core (these are local variables)
    Float32 fIn0, fIn1, fMix,fMixFBack, fFeedbackGainA, fDelayL, fDelayR, fCentreFeedback, fOut0, fOut1, fBeatDivision, fModDepth, fModFreq, fWetDry;
    
    // Setup and scale parameters to be used inside procees core
    fFeedbackGainA = GetParameter(kValue0) * 0.95;
    fBeatDivision = GetParameter(kValue1);
    fModDepth = GetParameter(kValue2);
    fModFreq = GetParameter(kValue3);
    fWetDry = (GetParameter(kValue4) * 1.5) - 0.5;
    
    // loop while there are still sample frame to process
    while ( inFramesToProcess-- )
    {
        //advance delayinsamples counter
        if( iCount ) delayInSamples++;
        
        // ensure delay in samples is within buffer limits if its outside reset to 0
        if(delayInSamples == iBufferSize) delayInSamples = 0;
        
        //this is our tap tempo test
        if(fDelayTimeA != iDelayBPM)
            fDelayTimeA = iDelayBPM;
        
        //Set our delay time relevent to the delay time slider
        fDelayQuantize = fDelayTimeA / fBeatDivision;
        
        // Smooth Delay Time Slider to prevent audio artifacts
        fSmoothing = smoothDelayTime.process(fDelayQuantize);
        
        // Now i'm going to build a switch case to decide how the state of the pluginMode activates different plugin settings
        switch ( pluginMode ) {

//-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`
                
            // LCR delay
            case 0:
                
                    // read left and right input samples into local variables
                    fIn0 = *pfInBuffer0;
                    fIn1 = *pfInBuffer1;
                    // Create Mono mix to be used in Centre section of LCR Delay
                    fMix = (0.5 * fIn0) + (0.5 * fIn1);
                    
                    // Insert feedback loop into process
                    fMixFBack = fMix + fCentreLPF;
                    
                    // Send write position to delay structure to be written to how ever many delays we want
                    delay[0].write(*pfInBuffer0);
                    delay[1].write(*pfInBuffer1);
                    delay[2].write(fMixFBack);
            
                    // Send delay read position to centre feedback network
                    fCentreFeedback = delay[2].read(fSmoothing) * fFeedbackGainA;
                    
                    //Test if LPF switch is on if it is run signal from the Centre Feedback line through it, if not bypass filter
                    if(filterMode == 1)
                    {
                        fCentreLPF = pFilters[kFilter0] -> Filter(fCentreFeedback);
                    }
                    else{fCentreLPF = fCentreFeedback;}
                    
                    // Sum centre signal with left and right signal
                    fDelayL = (delay[0].read(fSmoothing) * 0.5) + (fCentreLPF * 0.5);
                    fDelayR = (delay[1].read(fSmoothing) * 0.5) + (fCentreLPF * 0.5);
                    
                    // sum signal lines together and route to stereo outputs
                    fOut0 = WetDry(fIn0, fDelayL, fWetDry);
                    fOut1 = WetDry(fIn1, fDelayR, fWetDry);
                    
                    break;
                
//-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`
                
            // Ping Pong delay
            case 1:
                
                    // read left and right input samples into local variables
                    fIn0 = *pfInBuffer0;
                    fIn1 = *pfInBuffer1;
                    
                    //Mix feedback network with input
                    fLFeedback = fIn1 + fRFeedbackLPF;
                    fRFeedback = fIn0 + fLFeedbackLPF;
                
                    //write position to buffer
                    delay[3].write(fRFeedback);
                    delay[4].write(fLFeedback);
                
                    //Set read position and multiply with feedback gain slider
                    fLPostFeedback = delay[3].read(fSmoothing) * fFeedbackGainA;
                    fRPostFeedback = delay[4].read(fSmoothing) * fFeedbackGainA;
                
                    // check filter mode and run through LPF filter if desired
                    if(filterMode == 1)
                    {
                        fRFeedbackLPF = pFilters[kFilter1] -> Filter(fLPostFeedback);
                        fLFeedbackLPF = pFilters[kFilter0] -> Filter(fRPostFeedback);
                    }
                    else{fRFeedbackLPF = fLPostFeedback;
                         fLFeedbackLPF = fRPostFeedback;}
                
                    //Mix wet and dry signals together, set these to wet/dry slider
                    fOut0 = WetDry(fIn0, fRFeedback, fWetDry);
                    fOut1 = WetDry(fIn1, fLFeedback, fWetDry);
                
                    break;

//-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-`-
                
            //Stereo Chorus/Delay
            case 2:
                
                    // read left and right input samples into local variables
                    fIn0 = *pfInBuffer0;
                    fIn1 = *pfInBuffer1;
                
                    //build Stereo Cross Modulated Delay
                    
                    //Mix feedback and input values then se to variables
                    fLFeedback = fIn0 + fRFeedbackLPF;
                    fRFeedback = fIn1 + fLFeedbackLPF;
                
                    //Write values to delay structure
                    delay[5].write(fLFeedback);
                    delay[6].write(fRFeedback);
                    
                    //Set delay values to left and right variables
                    fDelayModL = fSmoothing;
                    fDelayModR = fSmoothing;
                
                    //Set modulation to local variables
                    fMod = modulation[0].process(fModFreq, fModDepth, GetSampleRate(), 0);
                    fMod1 = modulation[1].process(fModFreq, fModDepth, GetSampleRate(), M_PI_2);
                
                    //Modulate delay time with modulation variable
                    Float32 fModDelayTime = fMod * fDelayModL;
                    Float32 fModDelayTime1 = fMod1 * fDelayModR;

                    //send delay time to read position and multiply by feedback gain slider
                    fLPostFeedback = delay[5].read(fModDelayTime * fModDepth) * fFeedbackGainA;
                    fRPostFeedback = delay[6].read(fModDelayTime1 * fModDepth) * fFeedbackGainA;
                
                    // check filter mode and run through LPF filter if desired
                    if(filterMode == 1)
                    {
                        fRFeedbackLPF = pFilters[kFilter1] -> Filter(fLPostFeedback);
                        fLFeedbackLPF = pFilters[kFilter0] -> Filter(fRPostFeedback);
                    }
                    else{fRFeedbackLPF = fLPostFeedback;
                        fLFeedbackLPF = fRPostFeedback;}
                
                    //Mix wet and dry signals together, set these to wet/dry slider
                    fOut0 = WetDry(fIn0, fLFeedback, fWetDry);
                    fOut1 = WetDry(fIn1, fRFeedback, fWetDry);
                    
                break;
                
        }
        
        // write processed local variables to left and right output samples
        *pfOutBuffer0 = fOut0;
        *pfOutBuffer1 = fOut1;
        
        // move pointers along input and output buffers ready for next frame
        pfInBuffer0 += iInStride;
        pfInBuffer1 += iInStride;
        pfOutBuffer0 += iOutStride;
        pfOutBuffer1 += iOutStride;
    }
    
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

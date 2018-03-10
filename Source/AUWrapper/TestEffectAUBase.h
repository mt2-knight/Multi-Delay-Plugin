/*
 *	File: TestEffectAU.h
 */

#ifndef __TestEffectAUBase_h__
#define __TestEffectAUBase_h__

#include "AUEffectBase.h"
#include "TestEffectAUVersion.h"
#include "TestEffectAU_common.h"
#include "SimpleFilter.h"
#include <AudioToolbox/AudioUnitUtilities.h> // for listeners

#if AU_DEBUG_DISPATCHER
#include "AUDebugDispatcher.h"
#endif

class TestEffectAUBase : public AUEffectBase
{
public:
	TestEffectAUBase(AudioUnit component);
    
    virtual UInt32 SupportedNumChannels(const AUChannelInfo** outInfo);
	
	virtual OSStatus Initialize();
    virtual void Cleanup();
	
	virtual OSStatus GetPresets (CFArrayRef *outData) const;
	
	virtual OSStatus NewFactoryPresetSet (const AUPreset &inNewFactoryPreset);
	
	virtual	OSStatus GetParameterValueStrings(AudioUnitScope inScope, 
													 AudioUnitParameterID inParameterID,
													 CFArrayRef *outStrings);
    
	virtual	OSStatus GetParameterInfo(AudioUnitScope inScope,
											 AudioUnitParameterID inParameterID,
											 AudioUnitParameterInfo &outParameterInfo);
    
	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
											AudioUnitElement inElement, UInt32 &outDataSize,
											Boolean	&outWritable);
	
	virtual OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
										AudioUnitElement inElement, void *outData);
	
	virtual OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
										AudioUnitElement inElement, const void *inData, UInt32 inDataSize);
	
    virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags,
										const AudioBufferList &inBuffer, AudioBufferList &outBuffer,
										UInt32 inFramesToProcess);
	
	OSStatus SetParameterRefreshView(AudioUnitParameterID iParamID, AudioUnitParameterValue fValue);
	
	// callbacks use signed int values as sometimes can be dummy or user values (negative) 
	virtual void PresetLoaded(SInt32 iPresetNum, char *sPresetName);
	virtual void OptionChanged(SInt32 iOptionMenu, SInt32 iItem);
    virtual void ButtonPressed(SInt32 iButton);
	
	// tail time describes how long it takes for nominal-level signal to decay to silence at the
	// end of input
 	virtual	bool SupportsTail() { return true; }
	virtual Float64 GetTailTime() { return 0; }
	
	/*! @method Version */
	virtual OSStatus Version() { return kTestEffectAUVersion; }
	
protected:
	// put your private class member variables here
	
	// filters array
	SimpleFilter *pFilters[kNumberOfFilters];
	
	// array for transfering values from main effect process to graphical indicators
	Float32 pfIndicatorValues[kNumberOfIndicators];
	// storage for last button pressed (signed int to allow dummy values)
	SInt32 iLastButton;
	
	AUPreset pPresetsArray[kNumberOfPresets];
	
	OSStatus AddParameterListener(AudioUnitParameterID paramID);
	OSStatus AddPropertyEventListener(AudioUnitPropertyID propertyID, void *inRefCon);
	
	OSStatus ProcessInterleavedStereoInput(const AudioBuffer& obInBuffer, AudioBuffer& obOutBuffer,
										   UInt32 inFramesToProcess,
										   AudioUnitRenderActionFlags& ioActionFlags);
	
    OSStatus ProcessDeInterleavedStereoInput(const AudioBufferList& obInBuffers, 
											 AudioBufferList& obOutBuffers,
											 UInt32 inFramesToProcess,
											 AudioUnitRenderActionFlags& ioActionFlags);
	
    virtual OSStatus ProcessCore(const Float32 *pfInBuffer0, const Float32 *pfInBuffer1,
						 Float32 *pfOutBuffer0, Float32 *pfOutBuffer1, UInt32 iInStride,
                         UInt32 iOutStride, UInt32 inFramesToProcess );
};

#endif

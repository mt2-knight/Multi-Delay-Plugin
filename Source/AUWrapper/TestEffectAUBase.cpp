/*
 *	File: TestEffectAU.cpp
 */

#include "TestEffectAUBase.h"

// Use a single AUChannelInfo format (2 -> 2 channels) to make code simpler
// Note that ProcessBufferLists uses this info.
// If changed this, would need to process buffers in a different manner (currently assumes 2 -> 2)
static AUChannelInfo kChannelFormat = {2, 2};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AUDIOCOMPONENT_ENTRY(AUBaseFactory, TestEffectAUBase)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TestEffectAUBase::TestEffectAUBase(AudioUnit component)	: AUEffectBase(component)
{
	// NB: default behaviour of AUEffectBase constructor is to set inProcessesInPlace = true
	UInt16 iCount;
	
	// Basic initialisation in superclass
	CreateElements();
	// Use STL vector for faster indexed access where parameters are sequentially numbered
	Globals()->UseIndexedParameters(kNumberOfParameters);
	
	// set default values; params are always from zero
	for(iCount = 0; iCount < kNumberOfParameters; iCount++)
	{
		SetParameter(iCount, kDefaultValues[iCount]);
    }
	
	// populate presets array
	for(iCount = 0; iCount < kNumberOfPresets; iCount++)
	{
		CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault,
													(kPresetNames[iCount] ? kPresetNames[iCount] : DUMMYSTRING), 
													kCFStringEncodingASCII);
		if(str) {
			pPresetsArray[iCount].presetNumber = iCount;
			pPresetsArray[iCount].presetName = str;
		}
	}
	
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UInt32 TestEffectAUBase::SupportedNumChannels (const AUChannelInfo** outInfo)
{
	// override AUBase: describe I/O capabilities
	// deliberately reduced to a single format to reduce code complexity
    if ( outInfo != NULL )
        *outInfo = &kChannelFormat;
    return 1; // return *quantity* of AUChannelInfo configs provided
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void ParameterListenerCB(void *inRefCon,
						 void *inObject,
						 const AudioUnitParameter *inParameter,
						 AudioUnitParameterValue inValue)
{
	if(inValue == KDUMMYVAL) return; // ignore
	
	TestEffectAUBase *mainclass = (TestEffectAUBase *)inObject;
	SInt32 iVal = (SInt32)inValue;
	
	if(inParameter && mainclass) {
		AudioUnitParameterID iParam = inParameter->mParameterID;
		if (iParam < kNumberOfOptionMenus)
		{
			if(iVal >= kMinValues[iParam] && iVal <= kMaxValues[iParam])
				mainclass->OptionChanged((SInt32)iParam, iVal);
		}
	}
}

OSStatus TestEffectAUBase::AddParameterListener(AudioUnitParameterID paramID)
{
    OSStatus result;
    AudioUnitParameter auParameter;
    AUParameterListenerRef auListener;
	
    result = AUListenerCreate(ParameterListenerCB, NULL, NULL, NULL, 0, &auListener);
	
	if(result == noErr)
	{
		auParameter.mAudioUnit = mComponentInstance;
		auParameter.mParameterID = paramID;
		auParameter.mScope = kAudioUnitScope_Global;
		auParameter.mElement = 0;
		
		result = AUListenerAddParameter(auListener, this, &auParameter);
	}
    return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void PropertyListenerCB(void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inEventHostTime,
						AudioUnitParameterValue inValue)
{
	// inValue only valid for parameter changes
	// SetProperty occurs before this callback
	
	TestEffectAUBase *mainclass = (TestEffectAUBase *)inObject;
	
	if(inEvent && mainclass)
	{
		switch(inEvent->mArgument.mProperty.mPropertyID)
		{
			case kButtonsPropertyID:
			{
				SInt32 iVal = *(SInt32 *)inRefCon;
				if(iVal >= kButton0 && iVal < kNumberOfButtons)
					mainclass->ButtonPressed(iVal);
				break;
			}
			case kAudioUnitProperty_PresentPreset:
			{
				// SetProperty not called for this one it seems, so cannot user data cannot be update/read 
				// in the same way as for the buttons?  No particular advantage in updating a member variable
				// (mLastPreset) with this information
				AUPreset auPreset;
				UInt32 iSize = sizeof(auPreset);
				OSStatus result;
				
				result = AudioUnitGetProperty(mainclass->GetComponentInstance(), kAudioUnitProperty_PresentPreset,
											  kAudioUnitScope_Global, 0, &auPreset, &iSize);
				if(result == noErr)
				{
					static char sName[1024];
					sName[0] = '\0';
					Boolean ret = CFStringGetCString(auPreset.presetName, sName, 1024, kCFStringEncodingASCII);
					if(ret) {
						mainclass->PresetLoaded(auPreset.presetNumber, sName);
					}
					CFRelease(auPreset.presetName); // as required by Audio Unit Properties Reference
				}
				break;
			}
		}
	}
}

OSStatus TestEffectAUBase::AddPropertyEventListener(AudioUnitPropertyID propertyID, void *inRefCon)
{	
    OSStatus result;
    AudioUnitEvent auPropertyEvent;
    AudioUnitProperty auProperty; 
    AUEventListenerRef auListener;
	
    auProperty.mAudioUnit = mComponentInstance;
    auProperty.mPropertyID = propertyID;
    auProperty.mScope = kAudioUnitScope_Global;
    auProperty.mElement = 0;
	
    auPropertyEvent.mEventType = kAudioUnitEvent_PropertyChange;
    auPropertyEvent.mArgument.mProperty = auProperty;
	
    result = AUEventListenerCreate(PropertyListenerCB, inRefCon, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode,
								   PROPERTY_UPDATE_TIME, PROPERTY_GRANULARITY_TIME, &auListener);
    if (noErr == result) 
        AUEventListenerAddEventType(auListener, this, &auPropertyEvent);
	
    return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize and Cleanup are only called on creation and destruction

OSStatus TestEffectAUBase::Initialize()
{
	OSStatus res = AUEffectBase::Initialize(); // must do normal setup (p57 of Audio Unit Programming Guide 2007)
	SInt32 iPos;
	
	// create filters
	for(iPos = 0; iPos < kNumberOfFilters; iPos++)
	{
		pFilters[iPos] = new SimpleFilter(GetSampleRate());
	}
	
	// set indicators to zero
	for(iPos = 0; iPos < kNumberOfIndicators; iPos++)
	{
		pfIndicatorValues[iPos] = 0;
	}
	iLastButton = KDUMMYVAL;
	
	// setup callbacks
	for(iPos = kOption0; iPos < kNumberOfOptionMenus; iPos++) {
		AddParameterListener(iPos);
	}
	AddPropertyEventListener(kButtonsPropertyID, &iLastButton);
	AddPropertyEventListener(kAudioUnitProperty_PresentPreset, NULL);
	
	// set default preset as current if exists
	if(kDefaultPreset >= 0 && kDefaultPreset < kNumberOfPresets)
	{
		NewFactoryPresetSet(pPresetsArray[kDefaultPreset]);
	}
	
	// put your initialisations here
	
    return res;
}

void TestEffectAUBase::Cleanup(){
    AUEffectBase::Cleanup();
    
    SInt32 iPos;
    for(iPos = 0; iPos < kNumberOfFilters; iPos++)
        delete pFilters[iPos];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::GetPresets(CFArrayRef *outData) const
{
	UInt16 iCount;
	
	if(outData == NULL) return noErr;
	
	CFMutableArrayRef parray=CFArrayCreateMutable(NULL, kNumberOfPresets, NULL);
	
	for(iCount = 0; iCount < kNumberOfPresets; iCount++)
	{	
		CFArrayAppendValue(parray, &pPresetsArray[iCount]); 
	} 
	*outData = (CFArrayRef)parray;
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::NewFactoryPresetSet(const AUPreset &inNewFactoryPreset)
{ 
	SInt32 chosenPreset = inNewFactoryPreset.presetNumber;
	UInt16 iCount;
	
	if(chosenPreset >= 0 && chosenPreset < kNumberOfPresets)
	{
		for(iCount = 0; iCount < kNumberOfParameters; iCount++)
		{
			if(kPresetValues[chosenPreset][iCount] != KDUMMYVAL)
			{
				SetParameter(iCount, kPresetValues[chosenPreset][iCount]);
			}
		}
		
		SetAFactoryPresetAsCurrent(pPresetsArray[chosenPreset]);
		
		// returning noErr triggers update via AUBase::PropertyChanged which refreshes all the parameters 
		// (whether changed by SetParameter or not) in the interface and other listeners
		// The only way of avoiding non-changed parameters being updated is to return something other than
		// noErr and manually call AUParameterListenerNotify.  However, that will not affect how user
		// presets (as opposed to factory presets) are recalled.
		return noErr;
	}
	return kAudioUnitErr_InvalidProperty;
} 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::GetParameterValueStrings(AudioUnitScope inScope, 
												AudioUnitParameterID inParameterID,
												CFArrayRef *outStrings)
{
	if ((inScope == kAudioUnitScope_Global) && (inParameterID < kNumberOfOptionMenus))
	{
		// This method applies only to the options popups
		
		// When this method gets called by the AUBase::DispatchGetPropertyInfo method, which 
		// provides a null value for the outStrings parameter, just return without error.
		if (outStrings == NULL) return noErr;
		
		CFMutableArrayRef strarr = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
		SInt16 iCnt;
		
		for (iCnt = kMinValues[inParameterID]; iCnt <= kMaxValues[inParameterID]; iCnt++)
		{
			CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault,
														(kItemNames[inParameterID][iCnt] ? kItemNames[inParameterID][iCnt] : DUMMYSTRING),
														kCFStringEncodingASCII);
			if(strarr && str)
				CFArrayAppendValue(strarr, str);
		}
		*outStrings = strarr;
		return noErr;
    }
    return kAudioUnitErr_InvalidParameter;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::GetParameterInfo(AudioUnitScope inScope,
										AudioUnitParameterID inParameterID,
										AudioUnitParameterInfo &outParameterInfo )
{
	OSStatus result = noErr;
	
	// indicate parameter type to host application; these flags affect the appearance (or whether it is hidden) 
	// in the Generic view
	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;
	
	if (inScope == kAudioUnitScope_Global)
	{
		if(inParameterID < kNumberOfParameters)
		{
			// params always indexed from zero
			CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault,
														(kParameterNames[inParameterID] ? kParameterNames[inParameterID] : DUMMYSTRING), 
														kCFStringEncodingASCII);
			AUBase::FillInParameterName (outParameterInfo, str, false);
			
			if(inParameterID < kNumberOfOptionMenus)
			{
				// choose an "indexed" parameter type to indicate that should be an integer control 
				// or popup in the generic view
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			} else {
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
			}
			outParameterInfo.minValue = kMinValues[inParameterID];
			outParameterInfo.maxValue = kMaxValues[inParameterID];
			outParameterInfo.defaultValue = kDefaultValues[inParameterID];
		} else {
			result = kAudioUnitErr_InvalidParameter;
		}
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }
	
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::GetPropertyInfo (AudioUnitPropertyID inID,
										AudioUnitScope inScope,
										AudioUnitElement inElement,
										UInt32 &outDataSize,
										Boolean &outWritable)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
			case kAudioUnitProperty_CocoaUI:
				outDataSize = sizeof (AudioUnitCocoaViewInfo);
				outWritable = false;
				return noErr;
			case kIndicatorsPropertyID:
				// Indicator display values being read at intervals by view class
				outDataSize = kNumberOfIndicators * sizeof(Float32 *);
				outWritable = false;
				return noErr;
			case kButtonsPropertyID:
				// Buttons in display which can generate events in this class
				outDataSize = sizeof(SInt32); // last button pressed
				outWritable = true;
				return noErr;
			// kAudioUnitProperty_PresentPreset is in AUBase
		}
	}	
	
	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::GetProperty(AudioUnitPropertyID inID,
								   AudioUnitScope inScope,
								   AudioUnitElement inElement,
								   void *outData)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("uk.ac.uwe.audiounit.TestEffectAU") );
				
				if (bundle == NULL) return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
															 CFSTR("TestEffectAU_UWEGUIViewFactory"), 
															 CFSTR("bundle"), 
															 NULL);
                
                if (bundleURL == NULL) return fnfErr;
				
				AudioUnitCocoaViewInfo cocoaInfo;
				cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
				cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "TestEffectAU_UWEGUIViewFactory", 
																		   kCFStringEncodingUTF8);
				
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
				
				return noErr;
			}
			case kIndicatorsPropertyID:
				// Indicator display values being read at intervals by view class
				*((Float32 **)(outData)) = pfIndicatorValues;
				return noErr;
			case kButtonsPropertyID:
				// Buttons in display which can generate events in this class
				*((SInt32 *)(outData)) = iLastButton;
				return noErr;
			// kAudioUnitProperty_PresentPreset is in AUBase
		}
	}
	
	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::SetProperty(AudioUnitPropertyID inID,
								   AudioUnitScope 		inScope,
								   AudioUnitElement 	inElement,
								   const void *			inData,
								   UInt32 				inDataSize)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID)
		{
			case kButtonsPropertyID:
				iLastButton = *((SInt32 *)inData);
				return noErr;
			// kAudioUnitProperty_PresentPreset is in AUBase
		}
	}
	return AUEffectBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags,
										  const AudioBufferList & inBuffer,
										  AudioBufferList &outBuffer,
										  UInt32 inFramesToProcess)
{
	// Override version in AUEffectBase such that can process multiple channels at the same 
	// time rather than one at once
    UInt32 uiInBuffers = inBuffer.mNumberBuffers;
    UInt32 uiOutBuffers = outBuffer.mNumberBuffers;
    
	ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    
    if ( uiInBuffers == 1 )
    {
        if ( uiOutBuffers == 1 )
        {
            // interleaved input, interleaved output
            if (inBuffer.mBuffers[0].mNumberChannels != (UInt16)kChannelFormat.inChannels)
                return kAudioUnitErr_FormatNotSupported;
            if (outBuffer.mBuffers[0].mNumberChannels != (UInt16)kChannelFormat.outChannels)
                return kAudioUnitErr_FormatNotSupported;
            return ProcessInterleavedStereoInput(inBuffer.mBuffers[0], 
												 outBuffer.mBuffers[0], 
												 inFramesToProcess, ioActionFlags);
        }
        // interleaved input, deinterleaved output
		return kAudioUnitErr_FormatNotSupported;
    }
    
    // deinterleaved input
    if (uiInBuffers != (UInt16)kChannelFormat.inChannels || uiOutBuffers != (UInt16)kChannelFormat.outChannels)
        return kAudioUnitErr_FormatNotSupported;
    
    return ProcessDeInterleavedStereoInput(inBuffer, 
										   outBuffer, 
										   inFramesToProcess, ioActionFlags);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::ProcessInterleavedStereoInput(const AudioBuffer& obInBuffer, 
													 AudioBuffer& obOutBuffer,
													 UInt32 inFramesToProcess,
													 AudioUnitRenderActionFlags& ioActionFlags )
{
    ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
	
	// channels interleaved in the same buffers, so use pointer offset and stride of 2
    const Float32 *pfInBuffer0 = (const Float32 *)obInBuffer.mData;
    const Float32 *pfInBuffer1 = pfInBuffer0+1;
    Float32 *pfOutBuffer0 = (Float32 *)obOutBuffer.mData;
    Float32 *pfOutBuffer1 = pfOutBuffer0+1;
    
    return ProcessCore( pfInBuffer0, pfInBuffer1, pfOutBuffer0, pfOutBuffer1,
					   2, 2, inFramesToProcess );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::ProcessDeInterleavedStereoInput(const AudioBufferList& obInBuffers, 
													   AudioBufferList& obOutBuffers,
													   UInt32 inFramesToProcess,
													   AudioUnitRenderActionFlags& ioActionFlags)
{
    ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
	
	// channels in separate buffers, so stride of 1
    const AudioBuffer& obInBuffer0 = obInBuffers.mBuffers[0];
    const AudioBuffer& obInBuffer1 = obInBuffers.mBuffers[1];
    AudioBuffer& obOutBuffer0 = obOutBuffers.mBuffers[0];
    AudioBuffer& obOutBuffer1 = obOutBuffers.mBuffers[1];
	
    const Float32 *pfInBuffer0 = (const Float32 *)obInBuffer0.mData;
    const Float32 *pfInBuffer1 = (const Float32 *)obInBuffer1.mData;
    Float32 *pfOutBuffer0 = (Float32 *)obOutBuffer0.mData;
    Float32 *pfOutBuffer1 = (Float32 *)obOutBuffer1.mData;
    
    return ProcessCore( pfInBuffer0, pfInBuffer1, pfOutBuffer0, pfOutBuffer1,
					   1, 1, inFramesToProcess );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::SetParameterRefreshView(AudioUnitParameterID iParamID, AudioUnitParameterValue fValue)
{
	static AudioUnitParameter auParam;
	
	SetParameter(iParamID, fValue);
	
	auParam.mAudioUnit = GetComponentInstance();
	auParam.mScope = kAudioUnitScope_Global;
	auParam.mParameterID = iParamID;
	return AUParameterListenerNotify(NULL, NULL, &auParam);
}       

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAUBase::PresetLoaded(SInt32 iPresetNum, char *sPresetName)
{
	// a preset has been loaded, so can perform setup, such as retrieving parameter values 
	// with GetParameter and using them to set state variables in the plugin

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAUBase::OptionChanged(SInt32 iOptionMenu, SInt32 iItem)
{
	// option menu iOptionMenu has been changed to iItem

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestEffectAUBase::ButtonPressed(SInt32 iButton)
{
	// button iButton has been pressed

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus TestEffectAUBase::ProcessCore(const Float32 *pfInBuffer0,
								   const Float32 *pfInBuffer1,
								   Float32 *pfOutBuffer0,
								   Float32 *pfOutBuffer1,
								   UInt32 iInStride,
								   UInt32 iOutStride,
								   UInt32 inFramesToProcess)
{
	//Float32 fGain = GetParameter(kValue0);
	Float32 fIn0, fIn1;
	
	// loop until latest slice of data frames is processed
    while ( inFramesToProcess-- )
    {
		// read left and right input samples
        fIn0 = *pfInBuffer0;
        fIn1 = *pfInBuffer1;
		
		// write left and right output samples
		*pfOutBuffer0 = fIn0;
		*pfOutBuffer1 = fIn1;
		
		// move pointers along input and output buffers ready for next frame
        pfInBuffer0 += iInStride;
        pfInBuffer1 += iInStride;
        pfOutBuffer0 += iOutStride;
        pfOutBuffer1 += iOutStride;
    }
	
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

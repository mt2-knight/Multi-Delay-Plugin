/*
 *	File: TestEffectAU_UWEGUIView.h
 */

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

@interface TestEffectAU_UWEGUIView : NSView
{	
	NSMutableArray *uiOptionArray, *uiSliderArray, *uiValArray, *uiButtonIndicatorArray, *uiMeterIndicatorArray;
	NSWindow *uiDocWindow;
	
    // Other Members
    AudioUnit 				mAU;
    AUParameterListenerRef	mParameterListener;
	Float32					*pfIndicatorValues;
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)_synchronizeUIWithParameterValues;
- (void)_addListeners;
- (void)_removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue;

@end

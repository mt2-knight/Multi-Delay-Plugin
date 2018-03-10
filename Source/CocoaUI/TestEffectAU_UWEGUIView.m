/*
 *	File: TestEffectAU_UWEGUIView.m
 */

#import "TestEffectAU_UWEGUIView.h"
#import "NSCustomIRCell.h"
#import "NSCustomLICell.h"
#include "TestEffectAU_common.h"

AudioUnitParameter auparamarray[kNumberOfParameters];

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____
void ParameterListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue)
{
	TestEffectAU_UWEGUIView *SELF = (TestEffectAU_UWEGUIView *)inRefCon;
    
    [SELF _parameterListener:inObject parameter:inParameter value:inValue];
}

@implementation TestEffectAU_UWEGUIView

#pragma mark ____ INIT and DEALLOC ____
+ (void)initialize
{
	int idx;
    if ( self == [TestEffectAU_UWEGUIView class] ) {
		// only initialize once (see NSObject docs)
		
		for(idx = 0; idx < kNumberOfParameters; idx++) {
			auparamarray[idx].mAudioUnit = 0;
			auparamarray[idx].mParameterID = idx;
			auparamarray[idx].mScope = kAudioUnitScope_Global;
			auparamarray[idx].mElement = 0;
		}
	}
}

- (void)dealloc
{
    [self _removeListeners];
    [super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
/******************************************************************************/
/* execution order is initWithFrame, then awakeFromNib, then setAU */
/******************************************************************************/

- (void)setAU:(AudioUnit)inAU
{
	// remove previous listeners
	if (mAU) [self _removeListeners];
	mAU = inAU;
    
	// add new listeners
	[self _addListeners];
	
	// initial setup
	[self _synchronizeUIWithParameterValues];
	
	if(mAU) {
		UInt32 datasize = kNumberOfIndicators * sizeof(Float32 *);
		ComponentResult gotIndicatorPtr = AudioUnitGetProperty(mAU, kIndicatorsPropertyID, kAudioUnitScope_Global, 
															   1, &pfIndicatorValues, &datasize);
		
		// detach a thread to handle the meter updates; running a timer in this thread does not work 
		// properly because when a user clicks and holds a slider it stops the callback being called
		if(gotIndicatorPtr == noErr) {
			[NSThread detachNewThreadSelector:@selector(animateIndicators:) toTarget:self withObject:nil];
		}
	}
}

- (void)animateIndicators:(id)anObject
{
	// indicator update thread loop function
    
	// create new autorelease pool for the thread
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	Float32 floatValue;
	int idx;
	
    // animate
    while (YES) {
		if(kButtonsVisible)
		{
			for(idx = kButton0; idx < kNumberOfButtons; idx++)
			{
				floatValue = pfIndicatorValues[idx];
				if(floatValue > 0.5) {
					[[uiButtonIndicatorArray objectAtIndex:idx] setState:NSOnState];
				} else {
					[[uiButtonIndicatorArray objectAtIndex:idx] setState:NSOffState];
				}
			}
		}
		
		if(kMetersVisible)
		{
			for(idx = kMeterLeft; idx <= kMeterRight; idx++)
			{
				floatValue = pfIndicatorValues[idx];
				if(floatValue < 0) floatValue = 0;
				if(floatValue > 1) floatValue = 1;
				[[uiMeterIndicatorArray objectAtIndex:(idx - kMeterLeft)] setFloatValue:floatValue];
			}
		}			
		[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:INDICATOR_UPDATE_TIME]];
    }
	
    // release the autorelease pool
    [pool release];
}

#define EDGEGAP 12
#define COLGAP 8
#define BUTTONGAP 7
#define SLIDERWIDTH 225
#define SROWSPACING 23
#define LABELSPACE 10
#define LFNTHEIGHT 9
#define VALWIDTH 60
#define METERWIDTH 15
#define DOCWINBUTTON 30
#define DOCWINWIDTH 300

- (void)awakeFromNib
{
	// NIB has been loaded and initialised, so can now manually set characteristics
	
	int idx, cnt, col;
	NSString *str;
	int visibleoptions = 0;
	int visiblesliders = 0;

	// change font point size such that ascender height matches required constant
	CGFloat lfsize = [NSFont labelFontSize];
	//	NSFont *lfnt = [NSFont boldSystemFontOfSize:lfsize];
	NSFont *lfnt = [NSFont fontWithName:@"Helvetica-Bold" size:lfsize];
	CGFloat newlfsize = lfsize * LFNTHEIGHT / [lfnt ascender];
	NSFont* newlfnt = [NSFont fontWithDescriptor:[lfnt fontDescriptor] size:newlfsize];
	CGFloat newlfntheight = [newlfnt boundingRectForFont].size.height;
	//	printf("---> %s %f\n", [[fnt displayName] cStringUsingEncoding:NSASCIIStringEncoding],[NSFont labelFontSize]);
	NSColor *labeltextcol = [NSColor whiteColor];
	NSColor *valbackcol = [NSColor colorWithCalibratedRed:0.9 green:0.9 blue:0.9 alpha:1.0];
	
	NSDictionary *fontattr = [NSDictionary dictionaryWithObject:newlfnt forKey:NSFontAttributeName];
	NSSize maxlabelsize = NSMakeSize(0,0);
	
	for(idx = 0; idx < kNumberOfParameters; idx++)
	{
		if(kParameterVisible[idx]) {
			if(idx < kNumberOfOptionMenus)
				visibleoptions++;
			else
				visiblesliders++;
			
			// find maximum label width
			str = [NSString stringWithCString:(kParameterNames[idx] ? kParameterNames[idx] : DUMMYSTRING) 
									 encoding:NSASCIIStringEncoding];

			NSSize txtsize = [str sizeWithAttributes:fontattr];
			if(txtsize.width > maxlabelsize.width)
				maxlabelsize.width = txtsize.width;
			if(txtsize.height > maxlabelsize.height)
				maxlabelsize.height = txtsize.height;
		}
	}
	maxlabelsize.width += 20; // small fudge required to get text to fit
	
	// vertical; 0.5 edgegap, docwinbutton, 0.5 edgegap, options, edgegap, sliders, edgegap, indicators, buttons, edgegap
	// horizontal for sliders; edgegap, label, colgap, slider, colgap, valuebox, edgegap
	// horizontal for meters; (main width), colgap, meter, colgap, meter, edgegap
	// horizontal for buttons; edgegap, button, buttongap, button, ... , edgegap
	
	int topheight = EDGEGAP + DOCWINBUTTON;
	int totalheight = topheight;
	if(visibleoptions) totalheight += (visibleoptions * SROWSPACING) + EDGEGAP;
	if(visiblesliders) totalheight += (visiblesliders * SROWSPACING) + EDGEGAP;
	if(kButtonsVisible) totalheight += (2 * SROWSPACING) + EDGEGAP;

	int metersheight = (visiblesliders + visibleoptions) * SROWSPACING;
	if(visiblesliders && visibleoptions) metersheight += EDGEGAP;	

	int labelpartswidth = EDGEGAP + maxlabelsize.width + COLGAP;
	int mainwidth = labelpartswidth + SLIDERWIDTH + COLGAP + VALWIDTH + EDGEGAP;
	int optionmenuwidth = SLIDERWIDTH + COLGAP + VALWIDTH;
	int totalwidth = mainwidth;
	if(kMetersVisible) totalwidth += (2*COLGAP) + (2*METERWIDTH) + EDGEGAP;
	
	float buttonwidth = (totalwidth - (2*EDGEGAP) - (BUTTONGAP * (kNumberOfButtons - 1))) / (1.0 * kNumberOfButtons);
	
	NSSize framesize = {totalwidth, totalheight};
	
	// adjust frame to fit
	[self setFrameSize:framesize];
	
	// switch off resize
	[self setAutoresizingMask:NSViewNotSizable];
	
	// background image
	//	NSLog(@"supported types:%@", [NSImage imageFileTypes]);
	NSBundle *resource = [NSBundle bundleForClass:[self class]];
	NSString *backgroundPath = [resource pathForResource: @"background" ofType: @"png"];
	NSImage *bkimg = [[NSImage alloc] initWithContentsOfFile:backgroundPath];
	NSImageView *bkgnd = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, totalwidth, totalheight)];
	[bkgnd setImageScaling:NSScaleNone
     ]; // do not scale, to prevent image distortion 
	[bkgnd setImage:bkimg];
	[self addSubview:bkgnd];
	
	// ************************************
	// labels, option popups, sliders, values
	// ************************************
	uiOptionArray = [[NSMutableArray alloc] initWithCapacity:visibleoptions];
	uiSliderArray = [[NSMutableArray alloc] initWithCapacity:visiblesliders];
	uiValArray = [[NSMutableArray alloc] initWithCapacity:visiblesliders];
	float vpos = totalheight - topheight;
	int firstslider = true;
	
	for (idx = 0; idx < kNumberOfParameters; idx++)
	{
		if(!kParameterVisible[idx]) continue;
		
		vpos -= SROWSPACING;
		
		// NSMakeRect; x, y, w, h
		if(idx < kNumberOfOptionMenus) {
			NSPopUpButton *menu = [[NSPopUpButton alloc]
								   initWithFrame:NSMakeRect(labelpartswidth, vpos, optionmenuwidth, SROWSPACING)];
			//[uiOptionPopup removeAllItems];
			for (cnt = kMinValues[idx]; cnt <= kMaxValues[idx]; cnt++)
			{
				str = [NSString stringWithCString:(kItemNames[idx - kOption0][cnt] ? kItemNames[idx - kOption0][cnt] : DUMMYSTRING) encoding:NSASCIIStringEncoding];
				[menu addItemWithTitle:str];
			}
			[menu setTag:idx]; // to identify later
			[menu setFont:newlfnt];
			[menu setTarget:self]; 
			[menu setAction:@selector(iaOptionChanged:)];
			[self addSubview:menu];
			[uiOptionArray addObject:menu];
			[menu release];
		} else {
			if(firstslider && visibleoptions > 0)
			{
				firstslider = false;
				vpos -= EDGEGAP;
			}
			
			NSSlider *slider = [[NSSlider alloc] 
								initWithFrame:NSMakeRect(labelpartswidth, vpos, SLIDERWIDTH, SROWSPACING)];
			NSTextField *val = [[NSTextField alloc]
								initWithFrame:NSMakeRect(labelpartswidth + SLIDERWIDTH + COLGAP, vpos + (0.1 * newlfntheight), VALWIDTH, newlfntheight)];
			
			[slider setContinuous:YES];
			[slider setMinValue:kMinValues[idx]];
			[slider setMaxValue:kMaxValues[idx]];
			[slider setTag:idx]; // to identify later
			//[[slider cell] setControlSize:NSSmallControlSize];
			[slider setTarget:self]; 
			[slider setAction:@selector(iaSVChanged:)];
			[self addSubview:slider];
			[uiSliderArray addObject:slider];
			[slider release];
			
			[val setAlignment:NSCenterTextAlignment];
			//	[val setBezelStyle:NSTextFieldRoundedBezel];
			[val setFont:newlfnt];
			[val setBackgroundColor:valbackcol];
			[val setTag:idx]; // to identify later
			[val setTarget:self]; 
			[val setAction:@selector(iaSVChanged:)];
			[self addSubview:val];
			[uiValArray addObject:val];
			[val release];
		}

		// do label afterwards such that can vary vpos above
		NSTextField *label = [[NSTextField alloc] 
							  initWithFrame:NSMakeRect(EDGEGAP, vpos, maxlabelsize.width, newlfntheight)];
		str = [NSString stringWithCString:(kParameterNames[idx] ? kParameterNames[idx] : DUMMYSTRING) encoding:NSASCIIStringEncoding];
		[label setStringValue:str];
		[label setAlignment:NSCenterTextAlignment];
		[label setEditable:NO];
		[label setSelectable:NO];
		[label setDrawsBackground:NO];
		[label setBordered:NO];
		[label setFont:newlfnt];
		[label setTextColor:labeltextcol];
		[[label cell] setWraps:NO];
		[self addSubview:label];
		[label release];
	}
	
	// ************************************
	// help button which opens a documentation window
	// ************************************
	NSButton *helpbutton = [[NSButton alloc] initWithFrame:NSMakeRect(totalwidth - (0.5 * EDGEGAP+DOCWINBUTTON), 
																	  totalheight - (0.5 * EDGEGAP+DOCWINBUTTON), 
																	  DOCWINBUTTON, DOCWINBUTTON)];
	[helpbutton setBezelStyle:NSHelpButtonBezelStyle];
	[helpbutton setTitle:@""];
	[helpbutton setEnabled:YES];
	[helpbutton setTarget:self]; 
	[helpbutton setAction:@selector(iaHelpButtonPressed:)];
	[self addSubview:helpbutton];
	[helpbutton release];
	
	uiDocWindow = [[NSWindow alloc]
				   initWithContentRect:NSMakeRect(0, 0, DOCWINWIDTH, totalheight)
				   styleMask:NSTitledWindowMask | NSClosableWindowMask
				   backing:NSBackingStoreBuffered defer:NO];
	[uiDocWindow setReleasedWhenClosed:NO];
	[uiDocWindow setLevel:NSFloatingWindowLevel];
	str = [NSString stringWithCString:kDocumentationTitle encoding:NSASCIIStringEncoding];
	[uiDocWindow setTitle:str];
	[uiDocWindow orderOut: nil];
	
	NSScrollView *docview = [[NSScrollView alloc] initWithFrame:[[uiDocWindow contentView] frame]];
	[docview setBorderType:NSNoBorder];
	[docview setHasVerticalScroller:YES];
	[docview setHasHorizontalScroller:NO];
	[docview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	
	NSSize docviewsize = [docview contentSize]; // taking account of scrollbar size
	NSTextView *doctext = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, docviewsize.width, docviewsize.height)];
	[doctext setMinSize:NSMakeSize(0.0, docviewsize.height)];
	[doctext setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
	[doctext setVerticallyResizable:YES];
	[doctext setHorizontallyResizable:NO];
	[doctext setAutoresizingMask:NSViewWidthSizable];
	[[doctext textContainer] setContainerSize:NSMakeSize(docviewsize.width, FLT_MAX)];
	[[doctext textContainer] setWidthTracksTextView:YES];
	str = [NSString stringWithCString:kDocumentationText encoding:NSASCIIStringEncoding];
	[doctext insertText:str];
	[doctext setEditable:NO];
	[docview setDocumentView:doctext];
	[doctext release];
	[uiDocWindow setContentView:docview];
	[docview release];
	
	// ************************************
	// buttons, indicators
	// ************************************
	
	if(kButtonsVisible)
	{
		uiButtonIndicatorArray = [[NSMutableArray alloc] initWithCapacity:kNumberOfButtons];
		
		for (idx = kButton0, col = 0; idx < kNumberOfButtons; idx++, col++)
		{
			float hpos1 = EDGEGAP + (col * (buttonwidth + BUTTONGAP)); // button
			float hpos2 = hpos1 + (buttonwidth * 0.15); // indicator
			
			NSCustomIRCell *indicatorcell = [[NSCustomIRCell alloc] init];
            
			NSButton *indicator = [[NSButton alloc] initWithFrame:NSMakeRect(hpos2, EDGEGAP + 1.5 * SROWSPACING, 0.7 * buttonwidth, 0.5 * SROWSPACING)];
			// these attributes give correct operation with state and if disabled for user input etc.
			[indicator setCell:indicatorcell];
			[indicator setButtonType:NSPushOnPushOffButton];
			[indicator setBezelStyle:NSSmallSquareBezelStyle]; // scalable style
			[indicator setTitle:nil];
			[indicator setEnabled:NO];
			[self addSubview:indicator];
			[uiButtonIndicatorArray addObject:indicator];
			[indicator release];
			// Do NOT release the indicatorcolor and indicatorfilter
			
			NSButton *pushbutton = [[NSButton alloc] initWithFrame:NSMakeRect(hpos1, EDGEGAP, buttonwidth, SROWSPACING)];
			//[pushbutton setButtonType:NSMomentaryLightButton];
			// NSRoundedBezelStyle does not seem to be full width?  Use this one instead:
			[pushbutton setBezelStyle:NSRoundRectBezelStyle];
			str = [NSString stringWithCString:(kButtonNames[col] ? kButtonNames[col] : DUMMYSTRING) encoding:NSASCIIStringEncoding];
			[pushbutton setTitle:str];
			[pushbutton setTag:idx]; // to identify later
			[pushbutton setEnabled:YES];
			[pushbutton setFont:newlfnt];
            
            NSColor *color = [NSColor whiteColor];
            NSMutableAttributedString *colorTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[pushbutton attributedTitle]];
            NSRange titleRange = NSMakeRange(0, [colorTitle length]);
            [colorTitle addAttribute:NSForegroundColorAttributeName value:color range:titleRange];
            [pushbutton setAttributedTitle:colorTitle];
            
            
//            [pushbutton setFont:<#(NSFont * _Nullable)#>]
            
            [pushbutton setTarget:self];
			[pushbutton setAction:@selector(iaButtonPressed:)];
			[self addSubview:pushbutton];
			[pushbutton release];
		}
	}
	// ************************************
	// level meters
	// ************************************
	
	if(kMetersVisible)
	{
		uiMeterIndicatorArray = [[NSMutableArray alloc] initWithCapacity:2];
		for(idx = 0; idx < 2; idx++)
		{
			NSLevelIndicator *meter = [[NSLevelIndicator alloc]
									   initWithFrame:NSMakeRect(mainwidth+COLGAP+(idx*(COLGAP+METERWIDTH)), 
																totalheight-topheight-metersheight,
																METERWIDTH, metersheight)];
			
			NSCustomLICell *metercell = [[NSCustomLICell alloc] init];
			[meter setCell:metercell];
			[[meter cell] setLevelIndicatorStyle:NSContinuousCapacityLevelIndicatorStyle];		
			[meter setEnabled:NO];  // no user input (clicking)
			[meter setMinValue:0.0];
			[meter setMaxValue:1.0];
			// NSLevelIndicator is horizontal by default, so have to do a rotation to make a vertical meter
			[meter setBoundsRotation:90.0]; // make vertical
			[self addSubview:meter];
			[uiMeterIndicatorArray addObject:meter];
			[meter release];
		}	
	}
}	 

/*
 - (id)initWithFrame:(NSRect)frame
 {
 self = [super initWithFrame:frame];
 
 printf("----> initWithFrame\n");
 return self;
 }
 */

#pragma mark ____ INTERFACE ACTIONS ____

- (void)iaOptionChanged:(NSPopUpButton *)sender
{
	// this will only happen if the item is visible
	int param = [sender tag];

	NSEnumerator *enu = [uiOptionArray objectEnumerator];
	id obj;
	while(obj = [enu nextObject]) {
		if([obj tag] == param) {
			int selitem = [obj indexOfSelectedItem];
			// popup menu changed
			NSAssert(AUParameterSet(mParameterListener, sender, &auparamarray[param], (Float32)selitem, 0) == noErr,
					 @"[TestEffectAU_UWEGUIView iaOptionChanged:] AUParameterSet()");
			break;
		}
	}	
}

- (void)iaSVChanged:(id)sender
{
	// this will only happen if the item is visible
	float floatValue = [sender floatValue]; // get current value
	float beforeValue = floatValue;
	int param = [sender tag]; // get tag which identifies parameter number
	BOOL isslider = YES;
	
	if([sender class] == [NSTextField class]) {
		// textfield not slider
		isslider = NO;
	}
	
	// must do range checking before AUParameterSet but after found parameter index
	if(floatValue < kMinValues[param]) floatValue = kMinValues[param];
	if(floatValue > kMaxValues[param]) floatValue = kMaxValues[param];		
	
	NSAssert(AUParameterSet(mParameterListener, sender, &auparamarray[param], (Float32)floatValue, 0) == noErr,
			 @"[TestEffectAU_UWEGUIView iaSVChanged:] AUParameterSet()");
	
	if (!isslider) {
		NSEnumerator *enu = [uiSliderArray objectEnumerator];
		id obj;
		while(obj = [enu nextObject]) {
			if([obj tag] == param) {
				[obj setFloatValue:floatValue];
				break;
			}
		}
	}
	
	if (isslider || (!isslider && beforeValue != floatValue)) {
		// changed the slider; or textfield and value was out of range (so need to update text 
		// display again to force back into range)
		NSString *fmtstr = [NSString stringWithFormat:@"%1.4f", floatValue];
		NSEnumerator *enu = [uiValArray objectEnumerator];
		id obj;
		while(obj = [enu nextObject]) {
			if([obj tag] == param) {
				[obj setStringValue:fmtstr];
				break;
			}
		}
	}
}

- (void)iaButtonPressed:(NSButton *)sender
{
	// this will only happen if the item is visible
	SInt32 bnum  = [sender tag];
	
	NSAssert(AudioUnitSetProperty(mAU, kButtonsPropertyID, kAudioUnitScope_Global, 0, &bnum, sizeof(SInt32)) == noErr,
			 @"[TestEffectAU_UWEGUIView iaButtonPressed:] AudioUnitSetProperty()");
}

- (void)iaHelpButtonPressed:(NSButton *)sender
{
	// help button pressed
	if([uiDocWindow isVisible] == YES)
	{
		// close window if already visible
		[uiDocWindow performClose:sender];
	} else {
		// show documentation window just to the side of the main window
		NSRect mainrect = [[self window] frame];
		NSRect inforect = NSMakeRect(mainrect.origin.x + mainrect.size.width + 5, 
									 mainrect.origin.y,
									 DOCWINWIDTH, mainrect.size.height);	
		[uiDocWindow setFrame:inforect display:YES];
		[uiDocWindow makeKeyAndOrderFront:sender];
	}		
}

#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)_addListeners
{
	NSAssert ( AUListenerCreate( ParameterListenerDispatcher, self, 
								CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, PARAMETER_UPDATE_TIME,
								&mParameterListener	) == noErr, @"[TestEffectAU_UWEGUIView _addListeners] AUListenerCreate()");
	
    int idx;
    for (idx = 0; idx < kNumberOfParameters; ++idx) {
        auparamarray[idx].mAudioUnit = mAU;
        NSAssert (	AUListenerAddParameter (mParameterListener, NULL, &auparamarray[idx]) == noErr,
				  @"[TestEffectAU_UWEGUIView _addListeners] AUListenerAddParameter()");
    }
}

- (void)_removeListeners
{
    int idx;
    for (idx = 0; idx < kNumberOfParameters; ++idx) {
        NSAssert (	AUListenerRemoveParameter(mParameterListener, NULL, &auparamarray[idx]) == noErr,
				  @"[TestEffectAU_UWEGUIView _removeListeners] AUListenerRemoveParameter()");
    }
    
	NSAssert (	AUListenerDispose(mParameterListener) == noErr,
			  @"[TestEffectAU_UWEGUIView _removeListeners] AUListenerDispose()");
}

- (void)_synchronizeUIWithParameterValues
{
	Float32 value;
    int idx;
	
    for (idx = 0; idx < kNumberOfParameters; ++idx) {
        // only has global parameters
		NSAssert (	AudioUnitGetParameter(mAU, auparamarray[idx].mParameterID, kAudioUnitScope_Global, 0, &value) == noErr,
				  @"[TestEffectAU_UWEGUIView synchronizeUIWithParameterValues] (x.1)");
		NSAssert (	AUParameterSet (mParameterListener, self, &auparamarray[idx], value, 0) == noErr,
				  @"[TestEffectAU_UWEGUIView synchronizeUIWithParameterValues] (x.2)");
		NSAssert (	AUParameterListenerNotify (mParameterListener, self, &auparamarray[idx]) == noErr,
				  @"[TestEffectAU_UWEGUIView synchronizeUIWithParameterValues] (x.3)");
	}
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)_parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue
{
	// called via SetParameter from main AU class
	NSEnumerator *enu;
	id obj;
	
  	int param = inParameter->mParameterID;
	
	if(param < 0 || param >= kNumberOfParameters)
		return;
	if(!kParameterVisible[param])
		return;
	
	if(inValue < kMinValues[param])
		inValue = kMinValues[param];
	if(inValue > kMaxValues[param])
		inValue = kMaxValues[param];
	
	if(param >= kOption0 && param < kNumberOfOptionMenus) {
		int val = (int)inValue;

		enu = [uiOptionArray objectEnumerator];
		while(obj = [enu nextObject]) {
			if([obj tag] == param) {
				[obj selectItemAtIndex:val];
				break;
			}
		}
	}
	
	if(param >= kValue0 && param < kNumberOfParameters) {
		NSString *fmtstr = [NSString stringWithFormat:@"%1.4f", inValue];
		
		enu = [uiValArray objectEnumerator];
		while(obj = [enu nextObject]) {
			if([obj tag] == param) {
				[obj setStringValue:fmtstr];
				break;
			}
		}
		enu = [uiSliderArray objectEnumerator];
		while(obj = [enu nextObject]) {
			if([obj tag] == param) {
				[obj setFloatValue:inValue];
				break;
			}
		}
	}	
}

@end

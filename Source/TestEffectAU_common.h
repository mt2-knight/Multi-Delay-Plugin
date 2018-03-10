#ifndef __TestEffectAU_common_h__
#define __TestEffectAU_common_h__

// Much of this information is used twice; once to setup the generic AU view and again for the 
// graphical (Cocoa) view.

// Note: KDUMMYVAL cannot be used as a genuine parameter value
#define KDUMMYVAL -1000
#define DUMMYSTRING "?????"
#define GCCNOWARN __attribute__((unused))

enum {
	kOption0 = 0, kOption1, kValue0, kValue1, kValue2, kValue3, kValue4, kValue5, kValue6, 
	kValue7, kValue8, kValue9, kValue10, kValue11, kNumberOfParameters
};

enum { kNumberOfOptionMenus = kValue0 };

static const char* __attribute__((unused)) kParameterNames[kNumberOfParameters] = {
	"Option 0", "Option 1", 
	"Decay", "Delay Time", "Mod Depth", "Mod Freq",
	"Wet/Dry", "Value 5", "Value 6", "Value 7", 
	"Value 8", "Value 9", "Value 10", "Value 11"};

static const Boolean kParameterVisible[kNumberOfParameters] = 
{false, false, true, true, true, true, true, false, false, false, false, false, false, false};

// ====================================================================================
// Popup menu (option) items

enum {
	kItem0 = 0, kItem1, kItem2, kItem3, kMaxItems
};

static const char *kItemNames[kNumberOfOptionMenus][kMaxItems] = {
	{"Item 0", "Item 1", "Item 2"},
	{"Item 0", "Item 1", "Item 2", "Item 3"},
};
// REMEMBER to update the kMaxValues array if the number of items change
// (not very elegant, but cannot initialise an array with a value computed with sizeof at compile time)

// ====================================================================================
// Ranges and defaults

static const Float32 kMinValues[kNumberOfParameters] = {kItem0, kItem0, 0, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const Float32 kMaxValues[kNumberOfParameters] = {kItem2, kItem3, 1, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const Float32 kDefaultValues[kNumberOfParameters] = {kItem0, kItem0, 0.5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// parameter defaults will only apply if have not saved a different configuration in the host
// (e.g. in AU Lab .trak file settings will often override these)

// ====================================================================================
// Buttons

enum {
	kButton0 = 0, kButton1, kButton2, kButton3, kButton4, kNumberOfButtons
};

static const char GCCNOWARN *kButtonNames[kNumberOfButtons] = {
	"LCR", "Ping Pong", "Mod Delay", "LPF", "Tap"};
static const Boolean kButtonsVisible = true; // including the indicators

// ====================================================================================
// Filters

enum { kFilter0 = 0, kFilter1, kFilter2, kFilter3, kFilter4, kFilter5, kNumberOfFilters };

// ====================================================================================
// Documentation window

static const char GCCNOWARN *kDocumentationTitle = "Information";
static const char GCCNOWARN *kDocumentationText = 
"Multi Function Digital Delay.\n\nDesigned for use as a stereo plugin,\n\nWith simple controls all relative to the function they perform this plugin maybe used to easily and effective design a multitude of delay effects with little knowledge of advance delay functions.\n\n"
"Controls:\n\nDecay - Controls how long the delayed effect lasts\n\nDelay Time - Set the length of delay (Longest = Left, Shortest = Right)\n\nMod Depth - Controls the depth of modulation (This control is only active with Mod Delay mode selected)\n\nMod Freq - Controls the frequency of modulation scale is from 1Hz-15Hz (This control is only active with Mod Delay mode selected)\n\nWet/Dry - Controls Wet/Dry Mix (Left = Wet, Right = Dry)\n\n\nPlugin Modes can be selected on the buttons at the bottom, selecting between a Left-Center-Right Delay, a Ping Pong Delay and a Modulated Delay, the following 2 buttons are used to enable the Low Pass Filter and tap in the Tap Tempo";

// ====================================================================================
// Factory presets

enum{ 
	kPreset0 = 0, kPreset1 = 1, kPreset2 = 2, kPreset3 = 3, kPreset4 = 4, kPreset5 = 5, kPreset6 = 6, kPreset7 = 7, kPreset8 = 8, kPreset9 = 9, kPreset10 = 10, kPreset11 = 11, kPreset12 = 12, kPreset13 = 13,  kPreset14 = 14, kPreset15 = 15, kPreset16 = 16, kPreset17 = 17, kPreset18 = 18, kNumberOfPresets
};

static const char GCCNOWARN *kPresetNames[kNumberOfPresets] = {"1 Beat Delay", "1 Beat Dotted Delay", "1 Beat Triplet Delay","1/2th Delay","1/2th Dotted Delay", "1/2th Triplet Delay","1/4 Beat Delay","1/4 Dotted Delay","1/4 Triplet Delay","1/8th Delay","1/8th Dotted Delay","1/8th Triplet Delay","1/16th Delay","Call and Response","'Vox' Silky Gargle (Mod|LPF)","Thick Chorus (Mod|LPF)","Wobbly Flange (Mod|LPF)","'Drums' Squelch... Splash","Davey-Wavey", };

// use KDUMMYVAL if something is supposed to be ignored

static const Float32 kPresetValues[kNumberOfPresets][kNumberOfParameters] = {
{kItem0, kItem0, 0.5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 0.6667, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 0.7519, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 2.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 2.6667, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 2.9940, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 4.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 5.3334, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 5.9880, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 8.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 10.6667, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 12.0482, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.3344, 0.5, 0, 0, 0.75, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.75, 16, 0.1, 1.0, 0.75, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.55, 16, 0.2, 0.2, 0, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.8, 16, 0.1, 0.75, 0.5, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 1.0, 0.2, 0.5, 0.5, 0, 0, 0, 0, 0, 0, 0},
{kItem0, kItem0, 0.5, 2.0, 1.0, 0.0, 0.5, 0, 0, 0, 0, 0, 0, 0}
};
static const int kDefaultPreset = KDUMMYVAL;

// ====================================================================================
// Notification Interval for parameter listener
#define PARAMETER_UPDATE_TIME 0.050

// Notification Interval for property event listener
#define PROPERTY_UPDATE_TIME 0.005
// Multiple events are ignored below granularity limit
#define PROPERTY_GRANULARITY_TIME 0.001

// constant to describe indicator update rate in seconds, 
// such that can update values at suitable speed etc.
#define INDICATOR_UPDATE_TIME 0.020

// ====================================================================================
// indicators above buttons first, then meters
enum
{
	kMeterLeft = kNumberOfButtons,
	kMeterRight,
	kNumberOfIndicators
};

static const Boolean kMetersVisible = false;

#define kIndicatorsPropertyID 64000
#define kButtonsPropertyID 64001

#endif

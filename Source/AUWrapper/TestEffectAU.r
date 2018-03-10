/*
 *	File: TestEffectAU.r
 */
#include <AudioUnit/AudioUnit.r>

#include "TestEffectAUVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_TestEffectAU				1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TestEffectAU~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_TestEffectAU
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	TestEffectAU_COMP_SUBTYPE
#define COMP_MANUF		TestEffectAU_COMP_MANF	

#define VERSION			kTestEffectAUVersion
#define NAME			"UWE: TestEffectAU"
#define DESCRIPTION		"UWE TestEffectAU Audio Unit"
#define ENTRY_POINT		"TestEffectAUEntry"

#include "AUResources.r"

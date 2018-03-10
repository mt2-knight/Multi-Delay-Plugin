/*
 *	File: TestEffectAU_UWEGUIViewFactory.h
 */

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>

@class TestEffectAU_UWEGUIView;


@interface TestEffectAU_UWEGUIViewFactory : NSObject <AUCocoaUIBase>
{
    IBOutlet TestEffectAU_UWEGUIView *	uiFreshlyLoadedView;
}

- (NSString *) description;	// string description of the view

@end
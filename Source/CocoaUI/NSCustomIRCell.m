/*
 *  NSCustomIRCell.m
 */

#import "NSCustomIRCell.h"

/* Subclass of NSButtonCell such that I can have a custom colour scheme for a simple indicator object.
 Responds to the "state".  Tried creating raw NSCell and NSControl, but easier to adapt the button. */
 
@implementation NSCustomIRCell

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	// colours are red, green, blue, alpha
	CGFloat colarray[2][3][4] = {
		{{0.85, 0.85, 0.85, 0.5}, {0.85, 0.85, 0.85, 0.5}, {0.85, 0.85, 0.85, 0.5}},
		{{0.0, 0.6, 0.0, 1.0}, {0.0, 0.85, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}}};
	//	{{0.7, 0.7, 0.7, 1.0}, {0.85, 0.85, 0.85, 1.0}, {0.95, 0.95, 0.95, 1.0}},

	NSBezierPath *indpath = [NSBezierPath bezierPathWithRect:cellFrame];
	NSPoint radpoint = {0, 0};
	NSGradient *gradient;
	NSColor *colours[3];
	int state = 0;
	int pos;
	
	if([self state] == NSOnState) {
		state = 1;
	}
	for(pos = 0; pos < 3; pos++) {
		colours[pos] = [NSColor colorWithDeviceRed:colarray[state][pos][0] green:colarray[state][pos][1] 
											 blue:colarray[state][pos][2] alpha:colarray[state][pos][3]];
	}
    gradient = [[[NSGradient alloc]
				 initWithColorsAndLocations:colours[2], (CGFloat)0.0,
				 colours[1], (CGFloat)0.4,
				 colours[0], (CGFloat)1.0,
				 nil] autorelease];
    [gradient drawInBezierPath:indpath relativeCenterPosition:radpoint];
	// Do NOT release barpath or colourX
}

@end

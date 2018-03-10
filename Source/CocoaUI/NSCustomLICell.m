/*
*  NSCustomLICell.m
*/

#import "NSCustomLICell.h"

/* Subclass to replace NSLevelIndicatorCell such that I can have a custom colour scheme.
 Responds to the "floatValue".  Tried using QuartzCore framework with CIHueAdjust and 
 CIGammaAdjust, but switching on the animation layer seems to stop the update of the 
 level indicators in the timer callback functioning */

@implementation NSCustomLICell

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	// colours are red, green, blue, alpha
	CGFloat colarray[6][4] = {
		{0.6, 0.6, 0.6, 0.5}, {0.8, 0.8, 0.8, 0.5}, {0.9, 0.9, 0.9, 0.5},
		{0.0, 0.6, 0.0, 1.0}, {0.0, 0.85, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}};
	
	float min = [self minValue];
	float max = [self maxValue];
	float fracval = ([self floatValue] - min) / (max - min);
	NSRect barrect = cellFrame;
	NSBezierPath *barpath;
	NSGradient *gradient;
	NSColor *colours[6];
	int pos;
	
	for(pos = 0; pos < 6; pos++) {
		colours[pos] = [NSColor colorWithDeviceRed:colarray[pos][0] green:colarray[pos][1] 
											  blue:colarray[pos][2] alpha:colarray[pos][3]];
	}
	
	// background bar
	barpath = [NSBezierPath bezierPathWithRect:barrect];	
    gradient = [[[NSGradient alloc]
				 initWithColorsAndLocations:colours[0], (CGFloat)0.0,
				 colours[1], (CGFloat)0.2,
				 colours[2], (CGFloat)0.4,
				 colours[2], (CGFloat)0.6,
				 colours[1], (CGFloat)0.8,
				 colours[0], (CGFloat)1.0,
				 nil] autorelease];
    [gradient drawInBezierPath:barpath angle:90.0];
	
	// foreground bar (changing width because this is rotated 90 degrees from standard level indicator)
	barrect.size.width = fracval * barrect.size.width; // scale bar appropriately
	
	barpath = [NSBezierPath bezierPathWithRect:barrect];	
    gradient = [[[NSGradient alloc]
				 initWithColorsAndLocations:colours[3], (CGFloat)0.0,
				 colours[4], (CGFloat)0.2,
				 colours[5], (CGFloat)0.4,
				 colours[5], (CGFloat)0.6,
				 colours[4], (CGFloat)0.8,
				 colours[3], (CGFloat)1.0,
				 nil] autorelease];
    [gradient drawInBezierPath:barpath angle:90.0];
	
	// Do NOT release barpath or colourX
}

@end

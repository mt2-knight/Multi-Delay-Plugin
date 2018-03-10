/*
 *  SimpleFilter.h
 */

#ifndef __SimpleFilter_h__
#define __SimpleFilter_h__

#include <CoreServices/CoreServices.h>
#include <math.h>

enum { kLPF = 0, kHPF, kBPF, kBRF, kNumberOfFilterTypes };

class SimpleFilter
{
public:
	SimpleFilter(Float32 samplerate);
	void FilterConfig(UInt16 type, Float32 fc, Float32 bw);
	Float32 Filter(Float32 sval);
private:
	Float32 fSampleRate;
	UInt16 iType;
	Float32 fFilta[3], fFiltb[3], fFiltvalx[3], fFiltvaly[3];
};

#endif

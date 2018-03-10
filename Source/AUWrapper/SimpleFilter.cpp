/*
 *  SimpleFilter.cpp
 */

// NB: second-order filter designs from Zolzer "DAFX - Digital Audio Effects" p42-43 and p34

#include "SimpleFilter.h"

SimpleFilter::SimpleFilter(Float32 samplerate)
{
	UInt16 icnt;

	iType = 100; // dummy
	fSampleRate = samplerate;
	for(icnt = 0; icnt < 3; icnt++) {
		fFilta[icnt] = fFiltb[icnt] = fFiltvalx[icnt] = fFiltvaly[icnt] = 0;
	}
}

void SimpleFilter::FilterConfig(UInt16 type, Float32 fc, Float32 bw)
{
	// type = kLPF/kHPF/kBPF/kBRF, fc = cutoff/centre frequency, bw = bandwidth
	// (bw ignored for kLPF and kHPF)
	
	// basic data checking, just to be on the safe side and catch some of the more
	// obvious divide by zeros and other rubbish (type cannot be -ve as UInt)
	if((fSampleRate <= 0) || (type >= kNumberOfFilterTypes))
		return;
	
	// if possible, better to fix out of range values than fail silently
	if(fc < 20) fc = 20; // value of 20 produces less clicks than allowing all the way to 0 
	if(bw < 0) bw = 0;	
	
	// Check for values which cause instability and make corrections (rather than
	// failing completely by returning)
	// In particular, tan(PI/2) tends to infinity, so must be avoided:
	// --> fc must be < 0.5 Fs
	// --> bw must be < 0.25 Fs
	// keep them slightly under just to be on the safe side
	if(fc > 0.49 * fSampleRate) {
		fc = 0.49 * fSampleRate;
	}
	if(bw > 0.24 * fSampleRate) {
		bw = 0.24 * fSampleRate;
	}
	
	iType = type;
	
	if(iType == kLPF || iType == kHPF) {
		Float32 fOmega = M_PI * (fc/fSampleRate);
		Float32 fKval = tan(fOmega);
		Float32 fKvalsq = fKval * fKval;
		Float32 fRootTwo = sqrt(2.0);
		Float32 ffrac = 1.0 / (1.0 + fRootTwo * fKval + fKvalsq);
		
		if(type == kLPF) {
			fFiltb[0] =	 fKvalsq * ffrac;
			fFiltb[1] =  2.0 * fKvalsq * ffrac;
			fFiltb[2] =  fKvalsq * ffrac;
		} else if(type == kHPF) {
			fFiltb[0] =	 ffrac;
			fFiltb[1] =  -2.0 * ffrac;
			fFiltb[2] =  ffrac;
		}
		fFilta[0] =  0.0;
		fFilta[1] =  2.0 * (fKvalsq - 1.0) * ffrac;
		fFilta[2] =  (1.0 - fRootTwo * fKval + fKvalsq) * ffrac;
	}
	else if(iType == kBPF || iType == kBRF) {
		Float32 fOmegaA = M_PI * (fc/fSampleRate);
		Float32 fOmegaB = M_PI * (bw/fSampleRate);
        Float32 fCval = (tan(fOmegaB) - 1) / (tan(fOmegaB) + 1);
		Float32 fDval = -1.0 * cos(2.0 * fOmegaA);
		
		fFiltb[0] =	 -1.0 * fCval;
		fFiltb[1] =  fDval * (1.0 - fCval);
		fFiltb[2] =  1.0;
		fFilta[0] =  0.0;
		fFilta[1] =  -1.0 * fDval * (1.0 - fCval);
		fFilta[2] =  fCval;
	}
}

Float32 SimpleFilter::Filter(Float32 sval)
{
	fFiltvalx[2] = fFiltvalx[1];
	fFiltvalx[1] = fFiltvalx[0];
	fFiltvaly[2] = fFiltvaly[1];
	fFiltvaly[1] = fFiltvaly[0];

	if(iType == kLPF || iType == kHPF) {
		fFiltvalx[0] = sval
			- fFiltvalx[1] * fFilta[1]
			- fFiltvalx[2] * fFilta[2];
		
		fFiltvaly[0] = fFiltvalx[0] * fFiltb[0]
			+ fFiltvalx[1] * fFiltb[1]
			+ fFiltvalx[2] * fFiltb[2];
		return fFiltvaly[0];
	}
	else if(iType == kBPF || iType == kBRF) {
		fFiltvalx[0] = sval;
		
		fFiltvaly[0] = fFiltvalx[0] * fFiltb[0]
			+ fFiltvalx[1] * fFiltb[1]
			+ fFiltvalx[2] * fFiltb[2]
			+ fFiltvaly[1] * fFilta[1]
			+ fFiltvaly[2] * fFilta[2];
		
		if(iType == kBPF) {
			return 0.5 * (fFiltvalx[0] - fFiltvaly[0]);
		} else {
			return 0.5 * (fFiltvalx[0] + fFiltvaly[0]);			
		}
	}
	return 0;
}

/*
void SimpleFilter::Filter(Float32 *sval)
{
	*sval = Filter(*sval);
}
*/

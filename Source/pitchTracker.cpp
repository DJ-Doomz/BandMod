#include "pitchTracker.h"
#include <math.h>

// uses code from the following source:
/*
 *  mdaTrackerProcessor.cpp
 *  mda-vst3
 *
 *  Created by Arne Scheffler on 6/14/08.
 *
 *  mda VST Plug-ins
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

pitchTracker::pitchTracker()
{
	b1 = 0; b2 = 0; tmp = 0; e = 0; s = 0; n = 0; m = 0; mn = 0; tmp2 = 0; bo = 0;  ddp = 0.9;
	dn = 0;
	// threshold
	t = (float)pow(10.0, 3.0 * 0.1 - 3.8);

	// filter freq initialization stuff?
	float j, k, r = 0.999f;

	j = r * r - 1;
	k = (float)(2.f - 2.f * r * r * cos(0.647f * 50 / 48000));
	o = (float)((sqrt(k * k - 4.f * j * j) - k) / (2.f * j));
	i = (1.f - o) * (1.f - o);

	// m is max
	m = (int)(48000 / pow(10.0f, (float)(1.6f + 2.2f * 0.9)));
	mn = 48000 / 15.0; //lower limit

	// init pitch
	dp = 100.0 / 48000.0;
}

float pitchTracker::getPitch(float x)
{
	b1 = o * b1 + i * x;
	b2 = o * b2 + b1; //low-pass filter
	if (b2 > t) //if >thresh
	{
		if (s < 1) //and was <thresh
		{
			if (n < mn) //not long ago
			{
				tmp2 = b2 / (b2 - bo); //update period
				tmp = twopi / (n + dn - tmp2);
				dp = dp + ddp * (tmp - dp);
				dn = tmp2;
			}
			n = 0; //restart period measurement
		}
		s = 1;
	}
	else
	{
		if (n > m) s = 0; //now <thresh 
	}
	n++;
	bo = b2;

	return dp;
}


void pitchTracker::setMin(float newMin)
{
	mn = newMin;
}

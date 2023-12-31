#ifndef ANALYSER_H
#define ANALYSER_H

#include "defs.h"

/*
 * An Analyser is an object which wants to process an entire song to
 * calculate some kind of metadata about it. This could be bpm, the
 * summary, key or something else crazy. This is to help consolidate the
 * many different threads currently processing the whole track in Mixxx on load.
 *   -- Adam
 */

#include "trackinfoobject.h"

class Analyser {

public:
    virtual void initialise(TrackPointer tio, int sampleRate, int totalSamples) {
        Q_UNUSED(tio); Q_UNUSED(sampleRate); Q_UNUSED(totalSamples); // silence warnings
    }
    virtual void process(const CSAMPLE* pIn, const int iLen) = 0;
    virtual void finalise(TrackPointer tio) { Q_UNUSED(tio); }
    virtual ~Analyser() {}
};

#endif

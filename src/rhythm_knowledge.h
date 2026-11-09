/* Original generation rules, MIT; derived GMD tables have separate CC BY 4.0 attribution.
 * No raw pattern bank or neural runtime. Toussaint 2005 motivates cyclic spacing; weights are design choices. */
#pragma once
#define GUIDED_TRIES 64
#define ENSEMBLE_TRIES 32
#define ENSEMBLE_TICKS 128
static const unsigned char guided_strength_steps[3]={2,4,16};
static const unsigned char guided_strength_hits[3]={1,3,16};
static const unsigned char guided_strength_rotate[3]={2,5,31};
/* Roles are temporary rhythmic densities/divisions, not named drum instruments. */
static const unsigned char ensemble_hits[8]={4,3,3,2,2,2,1,1};
static const unsigned char ensemble_division[8]={1,1,1,2,2,1,2,2};
static const unsigned char ensemble_anchor[8]={1,0,0,0,0,0,0,0};

#ifndef PICN_GROOVE_ENABLED
#define PICN_GROOVE_ENABLED 1
#endif
#if PICN_GROOVE_ENABLED
#include "groove_tables.h"
#endif

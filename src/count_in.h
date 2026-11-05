/* Host count-in is indistinguishable from playback on MPC. Opt-in guard,
 * matched to MPC's setting: 0 Off, 1 Record, 2 Record + Play.
 * No allocation, I/O, locks, or polling. Times are quarter-note beats. */
typedef struct {int running,waiting;double previous,elapsed;} CountIn;
static void count_in_reset(CountIn *c){memset(c,0,sizeof *c);}
static int count_in_allow(CountIn *c,int mode,int playing,int recording,double ppq,double block){
 if(!playing||!isfinite(ppq)||ppq<0||!isfinite(block)||block<=0){count_in_reset(c);return 0;}
 if(!c->running){c->running=1;c->waiting=mode==2||(mode==1&&recording);c->elapsed=0;c->previous=ppq;}
 if(!mode)c->waiting=0;
 if(c->waiting){
  /* MPC rewinds at the end of pre-roll. Ignore normal PPQ rounding. */
  if(ppq<c->previous-fmax(1./960.,block*2.)||c->elapsed>=8.)c->waiting=0;
  else {c->elapsed+=block;c->previous=ppq;return 0;}
 }
 c->previous=ppq;return 1;
}

/* Offline regression: no ALSA device opened; outgoing notes are captured. */
#define snd_seq_get_queue_status countin_queue_status
#define snd_seq_queue_status_get_real_time countin_queue_time
#define main original_mod_test_main
#include "mod_engine_test.c"
#undef main
#undef snd_seq_get_queue_status
#undef snd_seq_queue_status_get_real_time
static snd_seq_real_time_t clock_time={10,0};
int countin_queue_status(snd_seq_t *s,int q,snd_seq_queue_status_t *status){(void)s;(void)q;(void)status;return 0;}
const snd_seq_real_time_t *countin_queue_time(const snd_seq_queue_status_t *s){(void)s;return &clock_time;}
static void guard_tests(void){
 CountIn c={0};double d=.01;
 assert(count_in_allow(&c,0,1,1,1.43,d)); /* unchanged default */
 count_in_reset(&c);assert(count_in_allow(&c,1,1,0,9.25,d)); /* Play parked mid-bar */
 count_in_reset(&c);assert(!count_in_allow(&c,1,1,1,1.43,d));
 assert(!count_in_allow(&c,1,1,1,5.42,d));assert(count_in_allow(&c,1,1,1,0,d));
 assert(count_in_allow(&c,1,1,1,4,d));assert(count_in_allow(&c,1,1,1,0,d)); /* loop after start */
 assert(!count_in_allow(&c,1,0,0,0,d));assert(!count_in_allow(&c,1,1,1,1.43,d)); /* rearm */
 count_in_reset(&c);assert(!count_in_allow(&c,2,1,0,7,d));
 assert(!count_in_allow(&c,2,1,0,6.9999,d)); /* no false release on rounding */
 assert(count_in_allow(&c,2,1,0,4,d)); /* non-zero rewind */
 count_in_reset(&c);int blocks=0;while(!count_in_allow(&c,2,1,0,blocks*.125,.125)){assert(++blocks<=65);}
 assert(blocks==64); /* mismatched setting: bounded two 4/4 bars */
 assert(count_in_allow(&c,2,1,0,20,d)); /* timeout not rearmed until Stop */
 assert(!count_in_allow(&c,2,1,0,NAN,d));assert(!c.running);
 count_in_reset(&c);assert(!count_in_allow(&c,2,1,0,0,d));assert(count_in_allow(&c,0,1,0,.1,d));
 puts("PASS Off/Record/Record+Play, parked Play, rewind, loop, restart, jitter, invalid time and timeout");
}
static void integration(void){
 Engine *e=fresh();ns=removed=0;for(int i=1;i<LANES;i++)e->lane[i].f[12]=1;
 snd_seq_queue_status_malloc(&e->queue_status);put(e,"can_redo",1);
 put(e,"mod_pad",1);put(e,"mod0_on",1);put(e,"mod0_dest",7);put(e,"mod0_wave",4);put(e,"mod_mix",-100);
 picn_engine_context(e,0,0,0x7fce,48000);assert(e->host_recording);
 song_pos(e,1.43,126,1,480);song_pos(e,5.42,126,1,480);assert(ns==0&&!e->playing);
 song_pos(e,0,126,1,480);assert(ns==2&&e->playing); /* exactly one On and Off */
 assert(sent[0].type==SND_SEQ_EVENT_NOTEON&&sent[1].type==SND_SEQ_EVENT_NOTEOFF);
 song_pos(e,0,126,0,480);ns=0;song_pos(e,1.43,126,1,480);assert(ns==0);
 put(e,"can_redo",0);song_pos(e,2,126,1,480);assert(ns==2); /* cancel waiting */
 snd_seq_queue_status_free(e->queue_status);e->queue_status=NULL;done(e);
 puts("PASS recorded host flags, silence during pre-roll, exact restart notes and Off override");
}
static void states(void){
 Engine *e=fresh(),*t=fresh();char b[32768],original[32768];assert(write_evolution(e,original,sizeof original)>0);
 put(t,"can_redo",2);assert(read_evolution(t,original)&&t->count_in_mode==0);
 put(e,"can_redo",2);assert(write_evolution(e,b,sizeof b)>0&&strstr(b,"COUNTIN 2 END"));
 assert(read_evolution(t,b)&&t->count_in_mode==2&&!t->count_in.running);
 char *p=strstr(b,"COUNTIN 2");p[8]='3';assert(!read_evolution(t,b)&&t->count_in_mode==2);
 put(e,"can_redo",0);assert(write_evolution(e,b,sizeof b)>0&&!strcmp(b,original));
 char value[32];put(e,"can_redo",1);assert(get_param(e,"can_redo",value,sizeof value)>0&&!strcmp(value,"1"));
 done(e);done(t);puts("PASS project round-trip, old-state default Off, atomic invalid-state rejection, unchanged Off state");
}
int main(void){guard_tests();integration();states();puts("ALL COUNT-IN TESTS PASSED");return 0;}

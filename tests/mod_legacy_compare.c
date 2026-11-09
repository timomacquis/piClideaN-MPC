/* Byte-level event comparison against the preserved 1.2.5 engine sources. */
#include <assert.h>
#define snd_seq_event_output_direct captured_output
#define snd_seq_remove_events captured_remove
#ifdef LEGACY_MOD
#include "../Validation/Mod-palette-baseline/src/engine.c"
#else
#include "../src/engine.c"
#endif
#undef snd_seq_event_output_direct
#undef snd_seq_remove_events
static uint64_t hash=1469598103934665603ull;static unsigned count;
int captured_output(snd_seq_t *s,snd_seq_event_t *v){(void)s;const unsigned char *b=(const unsigned char*)v;for(size_t n=0;n<sizeof *v;n++){hash^=b[n];hash*=1099511628211ull;}count++;return 0;}
int captured_remove(snd_seq_t *s,snd_seq_remove_events_t *r){(void)s;(void)r;return 0;}
int main(void){
 Engine *e=calloc(1,sizeof *e);assert(e);e->sample_rate=48000;e->mode=1;e->state_valid=1;e->random=123;
 for(int i=0;i<LANES;i++){init_lane(&e->lane[i],i);e->active_note[i]=-1;e->reset_at[i]=-1;}mod_init(e);
 e->seq=(snd_seq_t*)1;e->queue=1;e->port=1;snd_seq_remove_events_malloc(&e->rem);
 for(int wave=0;wave<8;wave++)for(int rate=0;rate<2;rate++)for(int mix=0;mix<3;mix++){
  panic_owned(e);mod_init(e);e->mod_pad[0]=0;e->mod[0].mix=(mix-1)*80;
  for(int j=0;j<2;j++){ModOsc *o=&e->mod[0].osc[j];o->f[0]=1;o->f[1]=wave;o->f[2]=rate;o->f[8]=j?1:7;o->f[5]=80;o->f[7]=j*90;}mod_coefficients(e,0);
  for(int block=0;block<2500;block++){mod_advance(e,48,120);int budget=EVENTS;assert(span(e,block*.002,(block+1)*.002,120,1+block*.001,INFINITY,&budget));}
  assert(!e->errors);
 }
 panic_owned(e);printf("events=%u hash=%016llx\n",count,(unsigned long long)hash);snd_seq_remove_events_free(e->rem);free(e);return 0;
}

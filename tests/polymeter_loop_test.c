/* Regression through the actual transport callback, with MIDI captured offline. */
#define snd_seq_get_queue_status loop_queue_status
#define snd_seq_queue_status_get_real_time loop_queue_time
#define main original_mod_test_main
#include "mod_engine_test.c"
#undef main
#undef snd_seq_get_queue_status
#undef snd_seq_queue_status_get_real_time
static snd_seq_real_time_t loop_time={10,0};
int loop_queue_status(snd_seq_t *s,int q,snd_seq_queue_status_t *status){(void)s;(void)q;(void)status;return 0;}
const snd_seq_real_time_t *loop_queue_time(const snd_seq_queue_status_t *s){(void)s;return &loop_time;}

static Engine *setup(double begin,double end,int loop){
 Engine *e=fresh();snd_seq_queue_status_malloc(&e->queue_status);
 for(int i=0;i<LANES;i++){
  Lane *l=&e->lane[i];l->f[0]=(int[]){10,7,12,16}[i%4];l->f[1]=l->f[0];l->f[2]=0;l->f[3]=1;l->f[5]=0;l->f[6]=100;pattern(l);
  for(int j=0;j<32;j++)l->values[j][1]=30+j;
  if(i>=4)l->f[12]=1;
 }
 picn_engine_context(e,begin,end,loop?((1<<2)|(1<<12)):0,48000);
 return e;
}
static void finish(Engine *e){snd_seq_queue_status_free(e->queue_status);e->queue_status=NULL;done(e);}
static snd_seq_event_t reference[16384];
static int with_lfo;
static void compare_loops(double begin,double length,int playmode,int rotate,int ratchet,int hold){
 const int blocks=4800;const double delta=.01;int nref;long heads[LANES];
 Engine *plain=setup(begin,begin+length,0),*loop=setup(begin,begin+length,1);
 Engine *both[]={plain,loop};
 for(int n=0;n<2;n++)for(int i=0;i<LANES;i++){
  Lane *l=&both[n]->lane[i];l->playmode=playmode;l->auto_rotate=rotate;l->shift=1;l->every=1;l->hold=hold;
  if(ratchet)for(int j=0;j<32;j++)l->values[j][3]=3;
 }
 if(with_lfo)for(int n=0;n<2;n++){
  Engine *e=both[n];put(e,"mod_pad",1);put(e,"mod0_on",1);put(e,"mod0_dest",with_lfo==1?1:7);put(e,"mod0_wave",4);put(e,"mod0_depth",100);put(e,"mod_mix",-100);
 }
 ns=removed=0;
 for(int block=0;block<blocks;block++)song_pos(plain,begin+block*delta,120,1,240);
 assert(!plain->errors);nref=ns;memcpy(reference,sent,ns*sizeof *sent);memcpy(heads,plain->head,sizeof heads);
 ns=removed=0;
 for(int block=0;block<blocks;block++){song_pos(loop,begin+fmod(block*delta,length),120,1,240);if(loop->errors){fprintf(stderr,"block=%d begin=%g len=%g mode=%d rotate=%d ratchet=%d hold=%d offset=%g ppq=%g errors=%u\n",block,begin,length,playmode,rotate,ratchet,hold,loop->lane_beat_offset,loop->expected_ppq,loop->errors);break;}}
 assert(!loop->errors);assert(ns==nref);if(with_lfo!=2&&!hold)assert(!removed);
 for(int n=0;n<ns;n++){
  assert(sent[n].type==reference[n].type);assert(sent[n].data.note.note==reference[n].data.note.note);
  assert(sent[n].data.note.velocity==reference[n].data.note.velocity);
  assert(sent[n].time.time.tv_sec==reference[n].time.time.tv_sec);
  assert(abs((int)sent[n].time.time.tv_nsec-(int)reference[n].time.time.tv_nsec)<=2);
 }
 assert(!memcmp(heads,loop->head,sizeof heads));
 finish(plain);finish(loop);
}
static void transport_resets(void){
 Engine *e=setup(0,16,1);ns=removed=0;
 for(int n=0;n<1700;n++)song_pos(e,fmod(n*.01,16),120,1,240);
 assert(e->lane_beat_offset>15.9);
 put(e,"reset_phase",1);double boundary=ceil((e->expected_ppq+e->lane_beat_offset-1e-8)/.25)*.25;
 assert(fabs(e->reset_at[0]-boundary)<1e-8);
 for(int n=1700;n<1726;n++)song_pos(e,fmod(n*.01,16),120,1,240);
 assert(e->reset_at[0]<0&&fabs(e->lane[0].origin-boundary)<1e-8);
 int cancellations=removed;song_pos(e,5,120,1,240);assert(removed>cancellations&&fabs(e->lane_beat_offset)<1e-8);
 song_pos(e,5,120,0,240);assert(!e->valid&&!e->playing);
 song_pos(e,0,120,1,240);assert(fabs(e->lane_beat_offset)<1e-8);
 assert(!e->errors);finish(e);
 puts("PASS explicit RESET PHASE uses continuous clock; seek and Stop/Play discard loop offset");
}
int main(void){
 transport_resets();
 compare_loops(0,16,0,0,0,0);
 compare_loops(4,16,0,0,0,0);
 compare_loops(0,3,0,0,1,0);
 for(int mode=0;mode<5;mode++)compare_loops(0,16,mode,1,0,0);
 compare_loops(0,16,0,0,0,1);
 with_lfo=1;compare_loops(0,16,0,0,1,0);
 with_lfo=2;compare_loops(0,16,0,0,0,0);
 puts("PASS actual song_pos: 10/7/12/16-step lanes, 64-step/nonzero/short loops, five traversals, rotation, ratchets, HOLD, MIDI timestamps and playheads match uninterrupted playback");
 return 0;
}

/* Regression: actual wrapper, fake ALSA fixture, no device. */
#define picn_state_write snapshot_format_original
#define main old_mod_test_main
#include "mod_engine_test.c"
#undef main
#undef picn_state_write
static pthread_mutex_t *observed_lock;
static int unlocked,concurrency_phase;
static void *try_control(void *unused){(void)unused;int rc=pthread_mutex_trylock(observed_lock);unlocked=rc==0;if(!rc)pthread_mutex_unlock(observed_lock);return NULL;}
static void assert_available(void){if(concurrency_phase)return;pthread_t t;unlocked=0;assert(!pthread_create(&t,NULL,try_control,NULL));pthread_join(t,NULL);assert(unlocked);}
int picn_state_write(const void *snapshot,char *b,int size){assert_available();return snapshot_format_original(snapshot,b,size);}
#include "../third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c"
static int burst,total,retired,release_test;static unsigned char seen[NPARAMS];
static intptr_t host(AEffect *a,int32_t op,int32_t i,intptr_t v,void *p,float f){
 (void)v;(void)p;
 if(op==audioMasterAutomate||op==audioMasterUpdateDisplay){observed_lock=&((wrap_t*)a->object)->lock;assert_available();}
 if(op==audioMasterAutomate){if(release_test)assert(f==0);burst++;total++;seen[i]=1;
  if(!strcmp(PARAMS[i].key,"line_select")||!strcmp(PARAMS[i].key,"line")){retired++;a->setParameter(a,i,f);}
  if(!strcmp(PARAMS[i].key,"line_select16"))a->setParameter(a,i,f);
 }return 0;
}
static void *offline_create(const char *dir){(void)dir;return fresh();}
static void offline_destroy(void *e){done(e);}
static void blocks(AEffect *a,int n){for(int j=0;j<n;j++){burst=0;housekeeping(a,128);assert(burst<=PICN_NOTIFICATION_BUDGET);}}
static void *controls(void *ptr){AEffect *a=ptr;int q=-1;for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,"l0_velocity"))q=i;assert(q>=0);for(int j=0;j<800;j++){a->setParameter(a,q,(j%128)/127.f);a->getParameter(a,q);char text[PARAM_TEXT_MAX];a->dispatcher(a,effGetParamDisplay,q,0,text,0);}return NULL;}
int main(void){
 mpc_engine_t offline=*mpc_engine();offline.create=offline_create;offline.destroy=offline_destroy;g_api=&offline;
 AEffect *a=VSTPluginMain(host);assert(a);wrap_t *w=a->object;Engine *e=w->dsp;observed_lock=&w->lock;
 blocks(a,700);
 /* Releases must stay literal zero even when the engine readback is nonzero. */
 for(int i=0;i<40;i++)w->holdFrames[i]=1;release_test=1;
 for(int j=0;j<5;j++){burst=0;housekeeping(a,128);assert(burst==8);}
 release_test=0;for(int i=0;i<40;i++)assert(!w->holdFrames[i]);
 puts("PASS 40 explicit momentary zero releases drain in five bounded blocks");
 retired=total=0;memset(seen,0,sizeof seen);
 for(int program=1;program<NUM_PROGRAMS;program++){
  a->dispatcher(a,effSetProgram,0,program,NULL,0);blocks(a,700);assert(!retired);
 }
 eng_set(w,"line_select16","12");eng_set(w,"l15_velocity","73");
 char before[32768],after[32768];assert(eng_get(w,"state",before,sizeof before)>0);
 void *ptr=NULL;int n=a->dispatcher(a,effGetChunk,0,0,&ptr,0);assert(n>0&&n<32768);char saved[32768];memcpy(saved,ptr,n);
 eng_set(w,"line_select16","0");assert(a->dispatcher(a,effSetChunk,0,n,saved,0));
 blocks(a,700);assert(e->selected==12&&!retired);assert(eng_get(w,"state",after,sizeof after)>0);assert(!strcmp(before,after));
 for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,"l15_velocity"))assert(seen[i]);
 puts("PASS preset/project recalls: <=8 callbacks/block, canonical updates drain, no retired-selector echo, A13 and full state retained; host callbacks and snapshot formatting outside engine mutex");
 PicnSavedState snap;capture_saved_state(e,&snap);eng_set(w,"l0_velocity","23");assert(write_saved_state(&snap,after,sizeof after)>0&&!strcmp(before,after));
 puts("PASS immutable snapshot remains exact after the live state changes; no state format change");
 midi_problem(e);char status[128];get_param(e,"status",status,sizeof status);assert(strstr(status,"CHECK MIDI"));e->midi_error_until=diagnostic_time()-1;get_param(e,"status",status,sizeof status);assert(!strstr(status,"CHECK MIDI"));assert(e->errors);e->release_pending=1;get_param(e,"status",status,sizeof status);assert(strstr(status,"CHECK MIDI"));e->release_pending=0;
 puts("PASS MIDI warning expires after recovery, historical errors retained, pending Note Off remains visible");

 for(int lane=0;lane<LANES;lane++){
  for(int step=0;step<32;step++){int values[]={1,127,100,8,150};memcpy(e->lane[lane].values[step],values,sizeof values);e->lane[lane].gates[step]=95;}
  variation_capture(&e->lane[lane],&e->alternate[lane]);e->variation_bank[lane]=1;
 }e->variation_ready=65535;
 assert(eng_get(w,"state",before,sizeof before)>0);observed_lock=&w->lock;
 n=a->dispatcher(a,effGetChunk,0,0,&ptr,0);assert(n>17000&&n<32768);memcpy(saved,ptr,n);
 assert(a->dispatcher(a,effSetChunk,0,n,saved,0));assert(eng_get(w,"state",after,sizeof after)>0&&!strcmp(before,after));
 printf("PASS dense 16-lane PICN8 state, all step locks and A/B banks: %d bytes within 32768-byte chunk\n",n);
 assert(str_to_norm(&PARAMS[525],"999")==1.f);
 concurrency_phase=1;pthread_t ui;assert(!pthread_create(&ui,NULL,controls,a));blocks(a,800);pthread_join(ui,NULL);
 a->dispatcher(a,effClose,0,0,NULL,0);puts("PASS concurrent UI parameter/display operations and housekeeping");return 0;
}

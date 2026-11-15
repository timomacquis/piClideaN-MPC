/* Incoming musical events never mutate editor state or duplicate sequencer output. */
#define main old_mod_test_main
#include "mod_engine_test.c"
#undef main
#include "../third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c"
static void *offline_create(const char *dir){(void)dir;return fresh();}
static void offline_destroy(void *ptr){done(ptr);}
static intptr_t host(AEffect *e,int32_t op,int32_t i,intptr_t v,void *p,float f){(void)e;(void)op;(void)i;(void)v;(void)p;(void)f;return 0;}
int main(void){
 mpc_engine_t offline=*mpc_engine();offline.create=offline_create;offline.destroy=offline_destroy;g_api=&offline;
 AEffect *a=VSTPluginMain(host);assert(a);wrap_t *w=a->object;Engine *e=w->dsp;
 eng_set(w,"line_select16","12");eng_set(w,"selected_step","7");ns=removed=0;
 for(int view=0;view<2;view++){
  eng_set(w,"edit_view",view?"1":"0");char before[32768],after[32768];assert(eng_get(w,"state",before,sizeof before)>0);unsigned revision=e->revision;
  for(int playing=0;playing<2;playing++){e->playing=playing;
   for(int flags=0;flags<2;flags++)for(int ch=0;ch<16;ch++)for(int note=0;note<128;note++){
    VstMidiEvent m={0};m.type=1;m.byteSize=sizeof m;m.flags=flags;m.midiData[0]=0x90|ch;m.midiData[1]=note;m.midiData[2]=100;
    VstEvents events={0};events.numEvents=1;events.events[0]=(VstEvent*)&m;assert(a->dispatcher(a,effProcessEvents,0,0,&events,0));
    m.midiData[0]=0x80|ch;m.midiData[2]=0;assert(a->dispatcher(a,effProcessEvents,0,0,&events,0));
   }
  }
  assert(e->selected==12&&e->step==7&&e->revision==revision);assert(eng_get(w,"state",after,sizeof after)>0&&!strcmp(before,after));
 }
 assert(ns==0&&removed==0);e->playing=0;
 eng_set(w,"line_select16","4");eng_set(w,"selected_step","3");assert(e->selected==4&&e->step==3);
 a->dispatcher(a,effClose,0,0,NULL,0);
 puts("PASS live/clip input on all channels and pitches leaves LINE/EDIT, lane, step and musical state unchanged, stopped or playing; no MIDI output; screen selection remains functional");return 0;
}

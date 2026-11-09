/* Actual wrapper + actual state parser; fake creation avoids ALSA/device access. */
#define main old_mod_test_main
#include "mod_engine_test.c"
#undef main
#include "../third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c"
static intptr_t host(AEffect *e,int32_t op,int32_t i,intptr_t v,void *p,float f){(void)e;(void)op;(void)i;(void)v;(void)p;(void)f;return 0;}
static void *offline_create(const char *dir){(void)dir;return fresh();}
static void offline_destroy(void *p){done(p);}
static void state(AEffect *a,char *buffer){assert(eng_get(a->object,"state",buffer,32768)>0);}
static void alias_checks(AEffect *a){
 Engine *e=((wrap_t*)a->object)->dsp;eng_set(a->object,"line_select16","4");eng_set(a->object,"edit_view","0");
 Lane untouched=e->lane[0];int fields[]={4,6,5,7,0,1,2,3},expected[]={127,100,50,95,32,32,31,4};
 for(int q=0;q<8;q++){char key[32];snprintf(key,sizeof key,"q%d",q);int index=-1;for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,key))index=i;assert(index>=0);set_param_now(a,index,1);assert(e->lane[4].f[fields[q]]==expected[q]);}
 assert(!memcmp(&untouched,&e->lane[0],sizeof untouched));
 eng_set(a->object,"l4_velocity","64");eng_set(a->object,"l4_prob","23");eng_set(a->object,"l4_humanize","2");eng_set(a->object,"selected_step","7");eng_set(a->object,"edit_view","1");
 for(int q=0;q<8;q++){char key[32];snprintf(key,sizeof key,"q%d",q);for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,key)){set_param_now(a,i,.25f);set_param_now(a,i,1);break;}}
 assert(e->lane[4].f[4]==64&&e->lane[4].f[6]==23&&e->lane[4].f[5]==2);
 assert(e->lane[4].gates[7]==95&&e->lane[4].values[7][1]==127&&e->lane[4].values[7][2]==100&&e->lane[4].values[7][3]==8&&e->lane[4].values[7][4]==150);
 assert(!memcmp(&untouched,&e->lane[0],sizeof untouched));
 puts("PASS actual wrapper Q-Link aliases: LINE affects selected lane only; EDIT protects LINE references and edits selected-step GATE/velocity/probability/ratchet/decay");
}
int main(void){
 mpc_engine_t offline=*mpc_engine();offline.create=offline_create;offline.destroy=offline_destroy;g_api=&offline;
 AEffect *a=VSTPluginMain(host),*b=VSTPluginMain(host);assert(a&&b&&a->numPrograms==7&&a->numParams==4096);
 char current[32768],restored[32768],name[64];state(a,current);assert(!strcmp(current,PICN_FACTORY[0].state));
 for(int i=0;i<PICN_FACTORY_COUNT;i++){
  assert(a->dispatcher(a,effGetProgramNameIndexed,i,0,name,0));assert(!strcmp(name,PICN_FACTORY[i].name));
  a->dispatcher(a,effSetProgram,0,i,NULL,0);assert(a->dispatcher(a,effGetProgram,0,0,NULL,0)==i);
  state(a,current);assert(!strcmp(current,PICN_FACTORY[i].state));
  Engine *e=((wrap_t*)a->object)->dsp;if(i==0)for(int lane=0;lane<LANES;lane++)assert(e->lane[lane].f[1]==0&&!e->lane[lane].hold);for(int s=0;s<MOD_SLOTS;s++)for(int j=0;j<2;j++)assert(!e->mod[s].osc[j].f[0]);
 }
 alias_checks(a);
 assert(!a->dispatcher(a,effGetProgramNameIndexed,-1,0,name,0));assert(!a->dispatcher(a,effGetProgramNameIndexed,PICN_FACTORY_COUNT,0,name,0));
 a->dispatcher(a,effSetProgram,0,3,NULL,0);eng_set(a->object,"l0_velocity","73");state(a,current);assert(strcmp(current,PICN_FACTORY[3].state));
 a->dispatcher(a,effSetProgram,0,3,NULL,0);state(a,restored);assert(!strcmp(current,restored));
 void *chunk=NULL;intptr_t size=a->dispatcher(a,effGetChunk,0,0,&chunk,0);assert(size>0&&size<32768);char *saved=malloc(size);memcpy(saved,chunk,size);assert(!strncmp(saved,"PCPROG1 3\n",10));
 assert(b->dispatcher(b,effSetChunk,0,size,saved,0));assert(b->dispatcher(b,effGetProgram,0,0,NULL,0)==3);state(b,restored);assert(!strcmp(current,restored));free(saved);
 const char *bad="PCPROG1 6\nPICN7 broken";assert(!b->dispatcher(b,effSetChunk,0,strlen(bad)+1,(void*)bad,0));assert(b->dispatcher(b,effGetProgram,0,0,NULL,0)==3);state(b,restored);assert(!strcmp(current,restored));
 assert(!b->dispatcher(b,effSetChunk,0,0,(void*)current,0));char nul[]="PCPROG1 1\nPICN7\0bad";assert(!b->dispatcher(b,effSetChunk,0,sizeof nul,nul,0));
 const char *raw=PICN_FACTORY[4].state;assert(b->dispatcher(b,effSetChunk,0,strlen(raw)+1,(void*)raw,0));assert(b->dispatcher(b,effGetProgram,0,0,NULL,0)==4);
 char unavailable[32768];snprintf(unavailable,sizeof unavailable,"PCPROG1 999\n%s",raw);assert(b->dispatcher(b,effSetChunk,0,strlen(unavailable)+1,unavailable,0));state(b,restored);assert(!strcmp(raw,restored));assert(b->dispatcher(b,effGetProgram,0,0,NULL,0)==0);
 a->dispatcher(a,effSetProgram,0,1,NULL,0);state(b,restored);assert(!strcmp(raw,restored));
 a->dispatcher(a,effSetProgram,0,-1,NULL,0);assert(a->dispatcher(a,effGetProgram,0,0,NULL,0)==1);
 Engine *musical=((wrap_t*)a->object)->dsp;variation_copy(musical);musical->lane[0].gates[3]=77;state(a,current);assert(!strncmp(current,"PICN8",5));chunk=NULL;size=a->dispatcher(a,effGetChunk,0,0,&chunk,0);assert(size>0&&size<32768);saved=malloc(size);memcpy(saved,chunk,size);assert(b->dispatcher(b,effSetChunk,0,size,saved,0));state(b,restored);assert(!strcmp(current,restored));free(saved);
 a->dispatcher(a,effClose,0,0,NULL,0);b->dispatcher(b,effClose,0,0,NULL,0);
 puts("PASS 7 native programs, explicit INIT, complete recalls and LFO OFF, 4096 stable handles, edited project/index round-trip, same-program echo, raw legacy states, malformed rejection, removed-index fallback and independent instances");
 return 0;
}

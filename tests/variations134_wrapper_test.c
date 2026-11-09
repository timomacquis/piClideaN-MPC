#define main previous_test_main
#include "mod_engine_test.c"
#undef main
#include "../third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c"
static int rebuilds,automations,max_burst;
static intptr_t host_callback(AEffect *e,int32_t op,int32_t i,intptr_t v,void *p,float f){(void)e;(void)i;(void)v;(void)p;(void)f;if(op==audioMasterUpdateDisplay)rebuilds++;if(op==audioMasterAutomate)automations++;return 0;}
static void step_ui(wrap_t *w){int before=automations;housekeeping(&w->fx,128);int burst=automations-before;if(burst>max_burst)max_burst=burst;}
static int index_of(const char *key){for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,key))return i;assert(0);return -1;}
int main(void){Engine *e=fresh();variation_reset(e);wrap_t *w=calloc(1,sizeof *w);w->dsp=e;w->master=host_callback;w->fx.object=w;pthread_mutex_init(&w->lock,NULL);g_api=mpc_engine();memset(w->last_on,-1,sizeof w->last_on);for(int i=0;i<NPARAMS;i++)w->last_pos[i]=-1;changed_all(e);visual_snapshot(e,0,1);for(int i=0;i<700;i++)step_ui(w);rebuilds=automations=max_burst=0;
 assert(PARAMS[index_of("undo")].momentary&&!PARAMS[index_of("redo")].nopts&&!PARAMS[index_of("rhythm_choice")].nopts);
 set_param_now(&w->fx,index_of("undo"),1);set_param_now(&w->fx,index_of("undo"),0);assert(e->dialog==7&&!e->rand_mode);
 for(int i=0;i<40;i++){set_param_now(&w->fx,index_of("rhythm_choice"),(i%3)*.5f);set_param_now(&w->fx,index_of("redo"),i%2);set_param_now(&w->fx,index_of("reset_phase"),1);visual_snapshot(e,2048,0);for(int j=0;j<30;j++)step_ui(w);}
 for(int j=0;j<300;j++)step_ui(w);assert(!rebuilds&&max_burst<=10);printf("PASS VARIATIONS native wrapper: rebuilds=%d, peak callbacks=%d, 40 A/B-strength-ALTER operations; device touch validation pending\n",rebuilds,max_burst);pthread_mutex_destroy(&w->lock);free(w);done(e);return 0;}

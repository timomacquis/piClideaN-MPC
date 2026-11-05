/* Actual MIDI capture, no device access. */
#define main previous_mod_main
#include "mod_engine_test.c"
#undef main
static double event_time(int i){return sent[i].time.time.tv_sec+sent[i].time.time.tv_nsec/1e9;}
static void gate_duration(void){
 Engine *e=fresh();for(int i=1;i<16;i++)e->lane[i].f[12]=1;Lane *l=&e->lane[0];l->f[0]=4;l->f[1]=4;pattern(l);put(e,"edit_view",1);put(e,"step_gate",80);assert(l->f[7]==50&&l->gates[0]==80&&personalized(l,0));
 int budget=EVENTS;ns=0;assert(span(e,0,.25,120,1,INFINITY,&budget));assert(ns==2&&sent[0].type==SND_SEQ_EVENT_NOTEON&&sent[1].type==SND_SEQ_EVENT_NOTEOFF);assert(fabs(event_time(1)-event_time(0)-.1)<1e-8);
 put(e,"step_ratchet",2);ns=0;budget=EVENTS;assert(span(e,0,.25,120,2,INFINITY,&budget));assert(ns==4);assert(fabs(event_time(1)-event_time(0)-.05)<1e-8);assert(fabs(event_time(3)-event_time(2)-.05)<1e-8);
 l->hold=1;ns=0;budget=EVENTS;assert(span(e,0,.25,120,3,INFINITY,&budget));assert(sent[ns-1].type==SND_SEQ_EVENT_NOTEON&&l->gates[0]==80);put(e,"l0_hold",0);assert(l->gates[0]==80&&e->release_pending&1);release(e,0);assert(e->active_note[0]<0);
 put(e,"step_gate",50);assert(l->gates[0]==50&&personalized(l,0));put(e,"erase_values",1);assert(l->gates[0]==-1&&!personalized(l,0));put(e,"q3",127);assert(l->gates[0]==95&&l->f[7]==50);put(e,"edit_view",0);put(e,"q3",0);assert(l->f[7]==10&&l->gates[0]==95);put(e,"edit_view",1);int before=l->f[4];put(e,"q0",0);assert(l->f[4]==before);put(e,"step_press1",1);assert(e->step==1&&effective_gate(l,1)==10);done(e);
 puts("PASS GATE: MIDI Note Off timing, ratchet, HOLD priority/release, inherit, erase, contextual Q-Link");
}
static void state_gates(void){Engine *e=fresh(),*t=fresh();char state[32768];for(int i=0;i<16;i++)for(int s=0;s<32;s++)e->lane[i].gates[s]=(signed char)(10+(i*32+s)%86);e->count_in_mode=2;int len=write_evolution(e,state,sizeof state);assert(len>0&&len<32768);assert(read_evolution(t,state));assert(!memcmp(e->lane,t->lane,sizeof e->lane)&&t->count_in_mode==2);char old[32768];strcpy(old,state);strcpy(strstr(old,"GATES "),"COUNTIN 1 END");assert(read_evolution(t,old)&&t->count_in_mode==1);for(int i=0;i<16;i++)for(int s=0;s<32;s++)assert(t->lane[i].gates[s]==-1);Lane before[16];memcpy(before,t->lane,sizeof before);strcpy(strstr(state,"GATES "),"GATES 9 END");assert(!read_evolution(t,state)&&!memcmp(before,t->lane,sizeof before));printf("PASS 512 GATE locks round-trip, old PICN7 inheritance, atomic malformed rejection (%d / 32768 bytes)\n",len);done(e);done(t);}
static void random_density(void){Engine *e=fresh();put(e,"edit_view",0);for(int n=1;n<=32;n++)for(int r=0;r<30;r++){init_lane(&e->lane[0],0);Lane *l=&e->lane[0];l->f[0]=n;l->f[1]=0;l->f[2]=0;pattern(l);put(e,"randomize",1);put(e,"rand_toggle5",1);put(e,"randomize",1);assert(density(l)<=density_limit(n));assert(l->f[0]==n);}
 for(int r=0;r<300;r++){put(e,"open_target",1);put(e,"apply_rhythm",1);for(int i=0;i<16;i++){assert(density(&e->lane[i])<=density_limit(e->lane[i].f[0]));for(int j=0;j<32;j++)assert(e->lane[i].gates[j]==-1);}}
 Lane *l=&e->lane[0];l->f[0]=32;l->f[1]=32;l->f[2]=0;pattern(l);Lane before=*l;put(e,"randomize",1);put(e,"rand_toggle4",1);put(e,"randomize",1);assert(!memcmp(&before,l,sizeof before));
 l->f[0]=16;l->f[1]=12;pattern(l);put(e,"randomize",1);put(e,"rand_toggle4",1);put(e,"randomize",1);assert(l->f[1]==12&&density(l)<=density_limit(l->f[0]));
 l->f[0]=16;l->f[1]=0;pattern(l);for(int s=0;s<16;s++)l->values[s][0]=1;before=*l;put(e,"randomize",1);put(e,"rand_toggle7",1);put(e,"randomize",1);assert(!memcmp(&before,l,sizeof before));done(e);puts("PASS RAND 1-32 steps, 300 Rd ALL, strict 75% density incl overrides, unselected values preserved / impossible selection no-op");}
int main(void){gate_duration();state_gates();random_density();return 0;}

/* Offline authoring of original piClideaN examples. Never opens ALSA. */
#define main original_mod_test_main
#include "mod_engine_test.c"
#undef main
static void blank(Engine *e){
 e->selected=e->step=e->edit_view=0;e->mode=1;e->scale_choice=e->scale_applied_choice=0;e->scale_start=e->scale_applied_start=36;
 for(int i=0;i<LANES;i++){Lane *l=&e->lane[i];l->f[0]=16;l->f[1]=l->f[2]=0;l->f[3]=1;l->f[4]=100;l->f[5]=0;l->f[6]=100;l->f[7]=50;l->f[12]=l->f[13]=0;l->origin=0;pattern(l);}
}
static void lane(Engine *e,int i,int steps,int hits,int rotate,int div){Lane *l=&e->lane[i];l->f[0]=steps;l->f[1]=hits;l->f[2]=rotate;l->f[3]=div;pattern(l);}
int main(void){
 static const char *names[]={"INIT","Straight Four","Broken Grid","Polymeter","Dorian Flow","Slow Orbit","Step Expression"};
 for(int preset=0;preset<7;preset++){
  Engine *e=fresh(),*verify=fresh();blank(e);char state[32768],again[32768];
  if(preset==1){lane(e,0,16,4,0,1);lane(e,1,16,2,4,1);lane(e,2,16,8,0,1);}
  if(preset==2){int hits[]={5,3,7,2};for(int i=0;i<4;i++)lane(e,i,16,hits[i],i,1);}
  if(preset==3){int lengths[]={10,7,12,16},hits[]={3,2,4,5};for(int i=0;i<4;i++)lane(e,i,lengths[i],hits[i],0,1);}
  if(preset==4){int notes[LANES];assert(scale_notes(10,36,notes));e->scale_choice=e->scale_applied_choice=10;for(int i=0;i<LANES;i++)e->lane[i].f[8]=e->lane[i].f[9]=notes[i];for(int i=0;i<8;i++){lane(e,i,16,1,i*2,2);e->lane[i].f[7]=85;}}
  if(preset==5){int lengths[]={5,8,11,16};for(int i=0;i<4;i++){lane(e,i,lengths[i],2,0,3);e->lane[i].auto_rotate=1;e->lane[i].shift=1;e->lane[i].every=2;}}
  if(preset==6){lane(e,0,16,4,0,1);lane(e,1,12,3,1,1);lane(e,2,16,6,0,1);e->lane[0].values[4][1]=65;e->lane[0].gates[8]=85;e->lane[2].values[8][3]=3;e->lane[2].values[8][4]=80;}
  assert(write_evolution(e,state,sizeof state)>0);assert(read_evolution(verify,state));assert(write_evolution(verify,again,sizeof again)>0&&!strcmp(state,again));
  printf("%s\t%s\n",names[preset],state);done(e);done(verify);
 }
 return 0;
}

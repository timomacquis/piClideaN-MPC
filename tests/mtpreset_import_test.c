/* πClideaN 1.4.0 : import d'un motif .mtpreset dans le moteur (navigateur, fil de lecture,
 * choix A-L / paires, application aux lignes 1-8, lecture MIDI, état). Original, MIT. */
#define main old_mod_test_main
#include "mod_engine_test.c"
#undef main
#include <sys/stat.h>
#include <unistd.h>

static void kit(const char *path,const char *steprate){
 FILE *f=fopen(path,"w");assert(f);
 fprintf(f,"MicroTonicPresetV3: {\n\tName = \"T\"\n\tPattern = b\n\tStepRate = %s\n\tSwing = 10\n\tTempo = 120\n\tMutes: { Off, On, Off, Off, Off, Off, Off, Off }\n\tDrumPatches: {\n\t}\n\tPatterns: {\n",steprate);
 for(int k=0;k<12;k++){int length=k==2?24:16;fprintf(f,"\t\t%c: {\n\t\t\tChained = false\n\t\t\tLength = %d\n",'a'+k,length);
  for(int r=0;r<8;r++){char t[33]={0},a[33]={0};for(int s=0;s<32;s++){t[s]=(s%8==(r+k)%8)?'#':'-';a[s]=(s==(r+k)%8)?'#':'-';}
   fprintf(f,"\t\t\t%d: {\n\t\t\t\tAccents = %s\n\t\t\t\tFills = ----------------\n\t\t\t\tTriggers = %s\n\t\t\t}\n",r+1,a,t);}
  fprintf(f,"\t\t}\n");}
 fprintf(f,"\t}\n}\n");fclose(f);
}
static void wait_listing(Engine *e,const char *dir,int count){char key[64];for(int t=0;t<3000;t++){while(mpc_engine_changed(e,key,sizeof key)>0){}if(!strcmp(e->mti_dir,dir)&&e->mti_count==count)return;struct timespec d={0,1000000};nanosleep(&d,NULL);}fprintf(stderr,"listing %s %d (%s %d)\n",dir,count,e->mti_dir,e->mti_count);assert(0);}
static void wait_read(Engine *e,int dialog,const char *status){char key[64];for(int t=0;t<3000;t++){while(mpc_engine_changed(e,key,sizeof key)>0){}if(e->mti_dialog==dialog&&strstr(e->mti_status,status))return;struct timespec d={0,1000000};nanosleep(&d,NULL);}fprintf(stderr,"dialog %d status %s\n",e->mti_dialog,e->mti_status);assert(0);}
static void text(Engine *e,const char *key,char *b){assert(get_param(e,key,b,256)>0);}

int main(void){
 char dir[]="/tmp/picn-mt-XXXXXX";assert(mkdtemp(dir));char sub[512],good[512],bad[512],wav[512],slow[512];
 snprintf(sub,sizeof sub,"%s/Kits",dir);assert(!mkdir(sub,0700));snprintf(good,sizeof good,"%s/Groove (120).mtpreset",dir);kit(good,"1/8");
 snprintf(bad,sizeof bad,"%s/Bad.MTPRESET",dir);kit(bad,"1/12");snprintf(wav,sizeof wav,"%s/hit.wav",dir);FILE *f=fopen(wav,"w");fputs("x",f);fclose(f);
 snprintf(slow,sizeof slow,"%s/Slow.mtpreset",sub);kit(slow,"1/2");
 MTSequence want;assert(mt_load_sequence(good,&want)&&want.active==1&&want.division==2&&want.mutes==2);
 Engine *e=fresh();char b[256];
 /* State the import must keep or replace. */
 e->lane[0].hold=1;e->lane[0].f[7]=70;e->lane[2].values[5][2]=40;e->variation_ready=0xff;e->lane[9].f[0]=7;e->lane[9].values[3][0]=1;Lane keep[8];memcpy(keep,&e->lane[8],sizeof keep);
 int notes[8];for(int i=0;i<8;i++)notes[i]=note(e,i);
 put(e,"choose_bank",1);int other_dialog=e->dialog;
 /* IMPORT opens the files window (and closes another dialog). */
 put(e,"name8",1);assert(e->mti_dialog==1&&(other_dialog==0||e->dialog==0));text(e,"mapping_label",b);assert(!strcmp(b,"1"));
 mti_nav(e,dir);wait_listing(e,dir,3); /* [DIR] Kits, Bad.MTPRESET, Groove (120).mtpreset — no .wav */
 text(e,"name0_display",b);assert(!strcmp(b,"[DIR] Kits"));text(e,"name2_display",b);assert(!strcmp(b,"Groove (120).mtpreset"));text(e,"name3_display",b);assert(!strcmp(b,"—"));
 text(e,"line_label",b);assert(!strcmp(b,dir));text(e,"target_label",b);assert(!strcmp(b,"1 / 1"));
 /* A broken file keeps the files window and says why. */
 put(e,"name1",1);wait_read(e,1,"Unknown StepRate — Bad");
 /* Folders, UP, CLOSE. */
 put(e,"name0",1);wait_listing(e,sub,1);put(e,"name9",1);wait_listing(e,dir,3);
 /* The good file opens the pattern window on the file's current pattern (B). */
 put(e,"name2",1);wait_read(e,2,"Choose a pattern — Groove (120)");assert(e->mti_choice==1);
 text(e,"line_scope",b);assert(!strcmp(b,"A 16  B 16  C 24  D 16  E 16  F 16  G 16  H 16  I 16  J 16  K 16  L 16"));
 text(e,"step_scope",b);assert(!strcmp(b,"B — 16 steps — 1/8 — lines 1-8"));
 put(e,"port_label",13);text(e,"step_scope",b);assert(!strcmp(b,"B+C — 40 steps: more than 32, not available"));
 Lane before[16];memcpy(before,e->lane,sizeof before);put(e,"name15",1);assert(e->mti_dialog==2&&!memcmp(before,e->lane,sizeof before));
 text(e,"step_label",b);assert(!strcmp(b,"This pattern cannot be imported"));
 /* A+B: 32 steps on lines 1-8. */
 put(e,"port_label",12);text(e,"step_scope",b);assert(!strcmp(b,"A+B — 32 steps — 1/8 — lines 1-8"));
 put(e,"name15",1);assert(e->mti_dialog==0);text(e,"step_label",b);assert(!strcmp(b,"Imported: Groove (120) — A+B, 32 steps"));
 MTPattern m;assert(mt_choice(&want,12,&m)&&m.steps==32);
 for(int i=0;i<8;i++){Lane *l=&e->lane[i];
  assert(l->f[0]==32&&l->f[1]==0&&l->f[2]==0&&l->f[3]==2&&l->f[4]==100&&l->f[5]==0&&l->f[6]==100&&l->f[12]==(i==1)&&l->f[13]==0&&note(e,i)==notes[i]);
  for(int k=0;k<32;k++){int hit=(m.trig[i]>>k)&1,acc=(m.acc[i]>>k)&1;assert(l->values[k][0]==hit);assert(l->values[k][1]==(hit&&acc?127:-1));for(int f=2;f<5;f++)assert(l->values[k][f]==-1);}
  assert(!(e->variation_ready&(1u<<i))&&e->head[i]==-1);}
 assert(e->lane[0].hold==1&&e->lane[0].f[7]==70);
 assert(!memcmp(keep,&e->lane[8],sizeof keep)&&e->variation_ready==0);
 /* Bits: line 1 pattern A = hits on steps 1, 9; pattern B (shifted by 16) = steps 18, 26; accents on 1 and 18. */
 assert(m.trig[0]==(0x0101u|0x0202u<<16)&&m.acc[0]==(1u|2u<<16));
 /* What the scheduler reads for each step (span: effective_hit, mod_step_values): hits and velocities 100 / 127. */
 for(int i=0;i<8;i++)for(int k=0;k<32;k++){int hit=(m.trig[i]>>k)&1,acc=(m.acc[i]>>k)&1,mv[6];assert(effective_hit(&e->lane[i],k)==hit);mod_step_values(e,i,k,k,0,120,mv);assert(mv[0]==(hit&&acc?127:100)&&mv[1]==100&&mv[4]==1);}
 /* Saved state, reopened: same lines. */
 char *state=malloc(32768),*again=malloc(32768);assert(get_param(e,"state",state,32768)>0);Engine *t=fresh();set_param(t,"state",state);assert(t->state_valid);
 for(int i=0;i<16;i++){assert(!memcmp(t->lane[i].f,e->lane[i].f,sizeof e->lane[i].f));assert(!memcmp(t->lane[i].values,e->lane[i].values,sizeof e->lane[i].values));}
 assert(get_param(t,"state",again,32768)>0&&!strcmp(state,again));free(state);free(again);done(t);
 /* Host notifications: dialog (option list: screen refresh), texts, entries. */
 put(e,"name8",1);char key[64];int seen=0;for(int k=0;k<200;k++){if(mpc_engine_changed(e,key,sizeof key)<=0)break;if(!strcmp(key,"mapping_label"))seen|=1;if(!strcmp(key,"step_label"))seen|=2;if(!strcmp(key,"name7"))seen|=4;}assert(seen==7);
 wait_listing(e,dir,3); /* reopens in the last folder */
 put(e,"name14",1);assert(e->mti_dialog==0);text(e,"name0_display",b);assert(!strcmp(b,"—"));text(e,"step_label",b);assert(!strcmp(b,"MicroTonic pattern (.mtpreset)"));
 /* Momentary releases and inactive keys never act. */
 memcpy(before,e->lane,sizeof before);put(e,"name15",1);put(e,"name2",1);put(e,"port_label",4);put(e,"name8",0);assert(!memcmp(before,e->lane,sizeof before)&&e->mti_dialog==0);
 /* Teardown with a read in flight. */
 put(e,"name8",1);mti_nav(e,dir);wait_listing(e,dir,3);put(e,"name2",1);mti_stop(e);done(e);
 unlink(good);unlink(bad);unlink(wav);unlink(slow);rmdir(sub);rmdir(dir);
 puts("PASS .mtpreset import: files window (.mtpreset only, folders, UP, CLOSE), broken file kept open, A-L and pairs up to 32 steps, refusal above 32, lines 1-8 replaced (steps, hits, accents 127, division, mutes) and 9-16 kept, note/GATE/HOLD kept, scheduler hits and velocities 100/127, state round-trip, notifications, teardown");
 return 0;
}

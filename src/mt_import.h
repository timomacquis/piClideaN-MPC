/* πClideaN 1.4.0 : import d'un motif MicroTonic (.mtpreset). Original implementation, MIT.
 *
 * Le fil « PiCN Import » (priorité normale, pile 128 Kio, comme « PiCN Rhythm ») liste
 * les dossiers et lit les fichiers : aucun accès disque ni allocation dans les appels du
 * moteur, faits sous le verrou du wrapper. Le moteur reprend les résultats par trylock
 * (song_pos, mpc_engine_changed) et n'attend jamais le fil.
 *
 * Choix : un motif A-L, ou deux motifs consécutifs bout à bout (A+B ... K+L) s'ils
 * tiennent dans 32 pas. Lignes 1-8 = pistes 1-8 : pas, déclenchements par pas (HITS et
 * ROTATE à 0), vélocité 100 et 127 sur les accents, DIVISION = StepRate, mutes du
 * fichier ; comme RDM 8, note, origine et MOD sont gardés, les variantes A/B effacées ;
 * GATE et HOLD de la ligne sont gardés. Lignes 9-16 inchangées.
 *
 * Aucun indice de paramètre ajouté : poignées inactives réutilisées (generate.py). */
#include <dirent.h>
static const char *const mti_buttons[8]={"name8","name9","name10","name11","name12","name13","name14","name15"};
enum {MTI_OPEN,MTI_UP,MTI_SD,MTI_DRIVES,MTI_PREV,MTI_NEXT,MTI_CLOSE,MTI_CONFIRM};
#define MTI_DIALOG "mapping_label"  /* 0 fermé, 1 fichiers, 2 motif */
#define MTI_CHOICE "port_label"     /* 0-11 A-L, 12-22 A+B ... K+L */
#define MTI_FOLDER "line_label"     /* dossier affiché */
#define MTI_PAGE "target_label"     /* page du navigateur */
#define MTI_STATUS "step_label"     /* état de l'import (page CONFIG et fenêtres) */
#define MTI_INFO "step_scope"       /* motif choisi : longueur, division */
#define MTI_LENGTHS "line_scope"    /* longueurs des motifs A-L du fichier */

typedef struct {
 pthread_t thread;pthread_mutex_t lock;atomic_int stop;int started;
 unsigned nav_request,nav_done,read_request,read_done;
 char nav_path[MTI_PATH],read_path[MTI_PATH],directory[MTI_PATH];
 MTIEntry entries[MTI_ENTRIES];int count,overflow,read_ok;MTSequence sequence;
} MTImportWorker;

static int mti_entry_cmp(const void *a,const void *b){const MTIEntry *x=a,*y=b;if(x->directory!=y->directory)return y->directory-x->directory;return strcasecmp(x->name,y->name);}
static void mti_scan(MTImportWorker *w,const char *path,unsigned request){
 MTIEntry *list=calloc(MTI_ENTRIES,sizeof *list);int count=0,overflow=0;DIR *d=list?opendir(path):NULL;
 if(d){struct dirent *e;while((e=readdir(d))){
  if(e->d_name[0]=='.')continue;char full[MTI_PATH];if(snprintf(full,sizeof full,"%s/%s",path,e->d_name)>=(int)sizeof full)continue;
  struct stat st;if(stat(full,&st))continue;int directory=S_ISDIR(st.st_mode);size_t n=strlen(e->d_name);
  if(!directory&&(!S_ISREG(st.st_mode)||n<=9||strcasecmp(e->d_name+n-9,".mtpreset")))continue;
  if(count==MTI_ENTRIES){overflow=1;continue;}snprintf(list[count].name,sizeof list[count].name,"%s",e->d_name);list[count++].directory=directory;}
  closedir(d);qsort(list,(size_t)count,sizeof *list,mti_entry_cmp);}
 pthread_mutex_lock(&w->lock);
 if(w->nav_request==request){if(list)memcpy(w->entries,list,MTI_ENTRIES*sizeof *list);w->count=list?count:0;w->overflow=overflow;snprintf(w->directory,MTI_PATH,"%s",path);w->nav_done=request;}
 pthread_mutex_unlock(&w->lock);free(list);
}
static void mti_read(MTImportWorker *w,const char *path,unsigned request){
 MTSequence *q=malloc(sizeof *q);int ok=q&&mt_load_sequence(path,q);
 pthread_mutex_lock(&w->lock);
 if(w->read_request==request){w->read_ok=ok;if(q)w->sequence=*q;else snprintf(w->sequence.message,MT_MESSAGE,"Out of memory");w->read_done=request;}
 pthread_mutex_unlock(&w->lock);free(q);
}
static void *mti_run(void *ptr){MTImportWorker *w=ptr;prctl(PR_SET_NAME,"PiCN Import",0,0,0);unsigned nav_seen=0,read_seen=0;
 while(!atomic_load(&w->stop)){
  char path[MTI_PATH];pthread_mutex_lock(&w->lock);unsigned nav=w->nav_request,read=w->read_request;pthread_mutex_unlock(&w->lock);
  if(nav!=nav_seen){nav_seen=nav;pthread_mutex_lock(&w->lock);snprintf(path,sizeof path,"%s",w->nav_path);pthread_mutex_unlock(&w->lock);mti_scan(w,path,nav);}
  if(read!=read_seen){read_seen=read;pthread_mutex_lock(&w->lock);snprintf(path,sizeof path,"%s",w->read_path);pthread_mutex_unlock(&w->lock);mti_read(w,path,read);}
  struct timespec delay={0,10000000};nanosleep(&delay,NULL);
 }
 return NULL;
}
static MTImportWorker *mti_worker(Engine *e){
 MTImportWorker *w=e->mti_worker;if(w&&w->started)return w;
 if(!w){w=calloc(1,sizeof *w);if(!w)return NULL;pthread_mutex_init(&w->lock,NULL);e->mti_worker=w;}
 pthread_attr_t attr;int initialized=pthread_attr_init(&attr)==0;struct sched_param priority={0};
 int ok=initialized&&pthread_attr_setstacksize(&attr,128*1024)==0&&pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED)==0&&pthread_attr_setschedpolicy(&attr,SCHED_OTHER)==0&&pthread_attr_setschedparam(&attr,&priority)==0;
 if(ok)ok=pthread_create(&w->thread,&attr,mti_run,w)==0;if(initialized)pthread_attr_destroy(&attr);
 if(!ok)return NULL;w->started=1;return w;
}
static void mti_stop(Engine *e){MTImportWorker *w=e->mti_worker;if(!w)return;atomic_store(&w->stop,1);if(w->started)pthread_join(w->thread,NULL);pthread_mutex_destroy(&w->lock);free(w);e->mti_worker=NULL;}

static void mti_changed(Engine *e){if(!e->mti_ui)e->mti_ui=1;}
static void mti_say(Engine *e,const char *text){snprintf(e->mti_status,sizeof e->mti_status,"%s",text);mti_changed(e);}
static void mti_nav(Engine *e,const char *path){
 MTImportWorker *w=mti_worker(e);if(!w){mti_say(e,"Import unavailable");return;}
 if(strlen(path)>=MTI_PATH||path[0]!='/')return;
 pthread_mutex_lock(&w->lock);snprintf(w->nav_path,MTI_PATH,"%s",path);w->nav_request++;pthread_mutex_unlock(&w->lock);
}
static void mti_choice_name(int c,char *b,int n){if(c<MT_PATTERNS)snprintf(b,n,"%c",'A'+c);else snprintf(b,n,"%c+%c",'A'+c-12,'A'+c-11);}
/* Results of the worker: trylock only, never waits. */
static void mti_poll(Engine *e){
 MTImportWorker *w=e->mti_worker;if(!w||pthread_mutex_trylock(&w->lock))return;
 if(w->nav_done==w->nav_request&&w->nav_done!=e->mti_nav_seen){
  e->mti_nav_seen=w->nav_done;memcpy(e->mti_entries,w->entries,sizeof e->mti_entries);e->mti_count=w->count;e->mti_overflow=w->overflow;e->mti_page=0;
  snprintf(e->mti_dir,sizeof e->mti_dir,"%s",w->directory);mti_changed(e);}
 if(w->read_done==w->read_request&&w->read_done!=e->mti_read_seen){
  e->mti_read_seen=w->read_done;
  if(e->mti_dialog==1){
   if(w->read_ok){e->mti_sequence=w->sequence;MTPattern m;int c=e->mti_sequence.active;if(!mt_choice(&e->mti_sequence,c,&m))for(c=0;c<MT_PATTERNS&&!mt_choice(&e->mti_sequence,c,&m);c++);
    e->mti_choice=c<MT_PATTERNS?c:0;e->mti_dialog=2;snprintf(e->mti_status,sizeof e->mti_status,"Choose a pattern — %s",e->mti_name);}
   else snprintf(e->mti_status,sizeof e->mti_status,"%s — %s",w->sequence.message,e->mti_name);
   mti_changed(e);}
 }
 pthread_mutex_unlock(&w->lock);
}
/* Lines 1-8 receive the chosen pattern, like RDM 8 (guided_apply_eight). */
static void mti_apply(Engine *e,const MTPattern *m){
 rand_clear(e);
 for(int i=0;i<8;i++){Lane *old=&e->lane[i];
  for(int step=0;step<32;step++)for(int f=0;f<5;f++)if(old->values[step][f]>=0||(f<2&&step<m->steps))notify_id(e,LEGACY_CHANGED+14+LANES*4+i*160+step*5+f);
  Lane c;init_lane(&c,i);
  c.f[0]=m->steps;c.f[1]=0;c.f[2]=0;c.f[3]=e->mti_sequence.division;c.f[4]=100;c.f[5]=0;c.f[6]=100;c.f[7]=old->f[7];
  c.f[8]=c.f[9]=note(e,i);c.f[12]=(int)((e->mti_sequence.mutes>>i)&1u);c.f[13]=0;c.hold=old->hold;c.origin=old->origin;
  pattern(&c);
  for(int k=0;k<m->steps;k++){int hit=(int)((m->trig[i]>>k)&1u);c.values[k][0]=hit;if(hit&&((m->acc[i]>>k)&1u))c.values[k][1]=127;}
  e->lane[i]=c;e->variation_ready&=~(1u<<i);e->variation_bank[i]=0;memset(&e->alternate[i],0,sizeof e->alternate[i]);
  e->head[i]=-1;e->ui_offset[i]=0;e->reset_at[i]=-1;variation_changed(e,i);notify_id(e,HOLD_ID+i);
 }
}
static int mti_key(const char *key){for(int i=0;i<8;i++)if(!strcmp(key,mti_buttons[i]))return i;return -1;}
static int mti_set(Engine *e,const char *key,int v){
 int i,b=mti_key(key);
 if(!strcmp(key,MTI_CHOICE)){if(e->mti_dialog==2&&v>=0&&v<MT_CHOICES&&v!=e->mti_choice){e->mti_choice=v;mti_changed(e);}return 1;}
 if(!strcmp(key,MTI_DIALOG)||!strcmp(key,MTI_FOLDER)||!strcmp(key,MTI_PAGE)||!strcmp(key,MTI_STATUS)||!strcmp(key,MTI_INFO)||!strcmp(key,MTI_LENGTHS))return 1; /* read-only */
 if(parse_suffix(key,"name",8,&i)){
  if(v<=0||e->mti_dialog!=1)return 1;int item=e->mti_page*8+i;if(item>=e->mti_count)return 1;
  char path[MTI_PATH];if(snprintf(path,sizeof path,"%s/%s",e->mti_dir,e->mti_entries[item].name)>=(int)sizeof path)return 1;
  if(e->mti_entries[item].directory){mti_nav(e,path);return 1;}
  MTImportWorker *w=mti_worker(e);if(!w){mti_say(e,"Import unavailable");return 1;}
  snprintf(e->mti_name,sizeof e->mti_name,"%s",e->mti_entries[item].name);size_t n=strlen(e->mti_name);if(n>9)e->mti_name[n-9]=0;
  pthread_mutex_lock(&w->lock);snprintf(w->read_path,MTI_PATH,"%s",path);w->read_request++;pthread_mutex_unlock(&w->lock);
  mti_say(e,"Reading...");return 1;
 }
 if(b<0)return 0;
 if(v<=0)return 1; /* momentary release */
 switch(b){
  case MTI_OPEN:rand_clear(e);if(e->dialog){e->dialog=0;changed_dialog(e);}e->mti_dialog=1;mti_nav(e,e->mti_last[0]?e->mti_last:"/media");mti_say(e,"Choose a .mtpreset file");break;
  case MTI_UP:if(e->mti_dialog==1){char path[MTI_PATH];snprintf(path,sizeof path,"%s",e->mti_dir);char *slash=strrchr(path,'/');if(slash==path)path[1]=0;else if(slash)*slash=0;mti_nav(e,path);}break;
  case MTI_SD:case MTI_DRIVES:if(e->mti_dialog==1)mti_nav(e,b==MTI_SD?"/sdcard":"/media");break;
  case MTI_PREV:case MTI_NEXT:if(e->mti_dialog==1){int next=e->mti_page+(b==MTI_NEXT?1:-1);if(next>=0&&next*8<e->mti_count){e->mti_page=next;mti_changed(e);}}break;
  case MTI_CLOSE:if(e->mti_dialog){if(e->mti_dialog==1&&e->mti_dir[0])snprintf(e->mti_last,sizeof e->mti_last,"%s",e->mti_dir);e->mti_dialog=0;if(strncmp(e->mti_status,"Imported",8))mti_say(e,"MicroTonic pattern (.mtpreset)");else mti_changed(e);}break;
  case MTI_CONFIRM:if(e->mti_dialog==2){MTPattern m;char name[8];mti_choice_name(e->mti_choice,name,sizeof name);
   if(!mt_choice(&e->mti_sequence,e->mti_choice,&m)){mti_say(e,"This pattern cannot be imported");break;}
   mti_apply(e,&m);if(e->mti_dir[0])snprintf(e->mti_last,sizeof e->mti_last,"%s",e->mti_dir);e->mti_dialog=0;
   snprintf(e->mti_status,sizeof e->mti_status,"Imported: %s — %s, %d steps",e->mti_name,name,m.steps);mti_changed(e);}
   break;
 }
 return 1;
}
static int mti_get(Engine *e,const char *key,char *b,int size){
 int i;size_t n=strlen(key);const int display=n>8&&!strcmp(key+n-8,"_display");char bare[32];
 if(n>=sizeof bare)return -1;memcpy(bare,key,n+1);if(display)bare[n-8]=0;
 if(parse_suffix(bare,"name",8,&i)){
  if(!display)return snprintf(b,size,"0");int item=e->mti_page*8+i;
  if(e->mti_dialog!=1||item>=e->mti_count)return snprintf(b,size,"—");
  return snprintf(b,size,"%s%s",e->mti_entries[item].directory?"[DIR] ":"",e->mti_entries[item].name);
 }
 if(mti_key(bare)>=0)return display?-1:snprintf(b,size,"0");
 if(!strcmp(bare,MTI_DIALOG))return snprintf(b,size,"%d",e->mti_dialog);
 if(!strcmp(bare,MTI_CHOICE))return snprintf(b,size,"%d",e->mti_choice);
 if(!strcmp(bare,MTI_FOLDER))return snprintf(b,size,"%s",e->mti_dir[0]?e->mti_dir:"...");
 if(!strcmp(bare,MTI_PAGE))return snprintf(b,size,"%d / %d%s",e->mti_page+1,e->mti_count?(e->mti_count+7)/8:1,e->mti_overflow?" (first 256)":"");
 if(!strcmp(bare,MTI_STATUS))return snprintf(b,size,"%s",e->mti_status[0]?e->mti_status:"MicroTonic pattern (.mtpreset)");
 if(!strcmp(bare,MTI_INFO)){MTPattern m;char name[8];mti_choice_name(e->mti_choice,name,sizeof name);static const char *rates[]={"1/32","1/16","1/8","1/4","1/2"};
  if(e->mti_dialog!=2)return snprintf(b,size,"%s","");
  if(mt_choice(&e->mti_sequence,e->mti_choice,&m))return snprintf(b,size,"%s — %d steps — %s — lines 1-8",name,m.steps,rates[e->mti_sequence.division]);
  if(e->mti_choice>=MT_PATTERNS&&e->mti_sequence.pattern[e->mti_choice-12].valid&&e->mti_sequence.pattern[e->mti_choice-11].valid)
   return snprintf(b,size,"%s — %d steps: more than 32, not available",name,e->mti_sequence.pattern[e->mti_choice-12].steps+e->mti_sequence.pattern[e->mti_choice-11].steps);
  return snprintf(b,size,"%s — not available in this file",name);}
 if(!strcmp(bare,MTI_LENGTHS)){int used=0;if(e->mti_dialog!=2)return snprintf(b,size,"%s","");
  for(int k=0;k<MT_PATTERNS&&used<size;k++){const MTPattern *m=&e->mti_sequence.pattern[k];used+=m->valid?snprintf(b+used,size-used,"%s%c %d",k?"  ":"",'A'+k,m->steps):snprintf(b+used,size-used,"%s%c —",k?"  ":"",'A'+k);}
  return used<size?used:size-1;}
 return -1;
}
/* Notification order: dialog first (option list: the wrapper refreshes the screen). */
static int mti_next_key(Engine *e,char *key,int len){
 static const char *keys[]={MTI_DIALOG,MTI_CHOICE,MTI_FOLDER,MTI_PAGE,MTI_STATUS,MTI_INFO,MTI_LENGTHS,"name0","name1","name2","name3","name4","name5","name6","name7"};
 int at=e->mti_ui-1;if(at<0)return 0;if(at>=(int)(sizeof keys/sizeof keys[0])){e->mti_ui=0;return 0;}
 e->mti_ui++;return snprintf(key,len,"%s",keys[at]);
}

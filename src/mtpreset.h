/* MicroTonicPresetV3 reader for πToniC and πClideaN. Original implementation, MIT.
 *
 * Sounds (πToniC): the eight DrumPatches are converted with the rules used for
 * the 150 BEST kits (πToniC scripts/convert_microtonic_private.py), plus the
 * tolerance of the FM-1 importer for options that Patternarium files leave out
 * (Choke, NStereo, velocities, drive, EQ, pan). Define MT_NO_SOUNDS to leave
 * this part out (πClideaN has no sound engine).
 *
 * Sequences (πClideaN): the twelve Patterns a-l (Length, Triggers, Accents of
 * eight rows), StepRate, Mutes and the current Pattern, read like the FM-1
 * importer (tests/mt-preset.mjs of f-ToniC): only the active Length counts,
 * shorter rows are rests, whitespace inside a row is ignored. Fills, Chained,
 * Swing, FillRate and Tempo are syntax-checked, then ignored.
 *
 * Bounded data-only parser, no evaluation: 1 MiB, 20000 tokens, depth 16,
 * 10000 values, 128 fields per group. mt_load*() open and allocate, so they
 * are called from a worker thread, never from the audio thread.
 * Outputs are written only when the whole file is valid. */
#pragma once
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <sys/stat.h>
#ifndef MT_NO_SOUNDS
#include "fields.h"
#endif

#define MT_MAX_BYTES (1024u*1024u)
#define MT_MAX_TOKENS 20000
#define MT_MAX_DEPTH 16
#define MT_MAX_NODES 10000
#define MT_MAX_KEYS 128
#define MT_MESSAGE 96
#define MT_SOUND_FIELDS 24
#define MT_PATTERNS 12

#ifndef MT_NO_SOUNDS
_Static_assert(PT_FIELDS==MT_SOUND_FIELDS,"MicroTonic sound fields");
typedef struct {
 float values[8][PT_FIELDS]; /* both A and B banks receive these */
 int clamped;                /* values brought into πToniC ranges */
 int clap;                   /* NEnvMod = Mod, played as CLAP */
 int outputs;                /* Output other than A, sent to the stereo mix */
 float attenuation_db;       /* common level reduction for positive gains */
 char message[MT_MESSAGE];   /* error text when mt_load/mt_parse return 0 */
} MTKit;

/* Defaults when a field is absent (FM-1 importer 0.9.1); NULL = required. */
static const char *const mt_defaults[PT_FIELDS]={[7]="0",[15]="Off",[16]="0",[17]="1000",[18]="0",[20]="100",[21]="100",[22]="0",[23]="Off"};
#endif
/* Same order as πToniC pt_fields (FREQ ... CHOKE). */
static const char *const mt_sound_keys[MT_SOUND_FIELDS]={"OscFreq","OscWave","OscAtk","OscDcy","ModMode","ModAmt","ModRate","ModVel","Mix","NFilMod","NFilFrq","NFilQ","NEnvAtk","NEnvDcy","NEnvMod","NStereo","DistAmt","EQFreq","EQGain","Level","OscVel","NVel","Pan","Choke"};
enum {MT_ROOT=1,MT_PATCHES,MT_SOUND,MT_PATTERN_LIST,MT_PATTERN,MT_ROW};

typedef struct {int at,len;} MTToken;
typedef struct {
 const char *text;MTToken *tok;int count,i,nodes;
 int field[8][MT_SOUND_FIELDS],output[8],sound[8]; /* token index + 1, 0 = absent */
 int pattern[MT_PATTERNS],length[MT_PATTERNS],row[MT_PATTERNS][8],trig[MT_PATTERNS][8],acc[MT_PATTERNS][8];
 int steprate,active,mutes; /* scalars; mutes = index of its '{' + 1 */
 char message[MT_MESSAGE];
} MTParser;

static int mt_fail(MTParser *p,const char *message){if(!p->message[0])snprintf(p->message,sizeof p->message,"%s",message);return 0;}
static int mt_space(char c){return c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\f'||c=='\v';}
static int mt_punct(char c){return c=='{'||c=='}'||c=='='||c==':'||c==',';}
static int mt_is(const MTParser *p,int i,const char *s){size_t n=strlen(s);return i<p->count&&(size_t)p->tok[i].len==n&&!memcmp(p->text+p->tok[i].at,s,n);}
static int mt_is_punct(const MTParser *p,int i){return i<p->count&&p->tok[i].len==1&&mt_punct(p->text[p->tok[i].at]);}
/* Token content without surrounding quotes (escapes are kept as written). */
static const char *mt_content(const MTParser *p,int i,int *len){const char *s=p->text+p->tok[i].at;int n=p->tok[i].len;if(n>=2&&s[0]=='"'){s++;n-=2;}*len=n;return s;}
static int mt_equal(const MTParser *p,int a,int b){int na,nb;const char *x=mt_content(p,a,&na),*y=mt_content(p,b,&nb);return na==nb&&!memcmp(x,y,(size_t)na);}
static int mt_named(const MTParser *p,int i,const char *s){int n;const char *x=mt_content(p,i,&n);return (size_t)n==strlen(s)&&!memcmp(x,s,(size_t)n);}

static int mt_tokenize(MTParser *p,size_t n){
 const char *t=p->text;size_t at=0;
 if(n>=3&&!memcmp(t,"\xEF\xBB\xBF",3))at=3;
 while(at<n){
  char c=t[at];if(mt_space(c)){at++;continue;}
  if(!c)return mt_fail(p,"Not a text file");
  size_t start=at;
  if(mt_punct(c))at++;
  else if(c=='"'){
   for(at++;;){
    if(at>=n||t[at]=='\r'||t[at]=='\n'||!t[at])return mt_fail(p,"Unclosed text");
    if(t[at]=='"'){at++;break;}
    if(t[at]!='\\'){at++;continue;}
    if(at+1>=n)return mt_fail(p,"Unclosed text");
    char e=t[at+1];
    if(e=='u'){if(at+5>=n)return mt_fail(p,"Invalid text escape");for(int k=2;k<6;k++)if(!isxdigit((unsigned char)t[at+k]))return mt_fail(p,"Invalid text escape");at+=6;}
    else if(e&&strchr("\"\\/bfnrt",e))at+=2;
    else return mt_fail(p,"Invalid text escape");
   }
  }
  else while(at<n&&t[at]&&!mt_space(t[at])&&!mt_punct(t[at])&&t[at]!='"')at++;
  if(p->count>=MT_MAX_TOKENS)return mt_fail(p,"Preset too complex");
  p->tok[p->count].at=(int)start;p->tok[p->count].len=(int)(at-start);p->count++;
 }
 return 1;
}

static int mt_value(MTParser *p,int depth,int role,int sound){
 if(++p->nodes>MT_MAX_NODES||depth>MT_MAX_DEPTH)return mt_fail(p,"Preset too complex");
 if(p->i>=p->count)return mt_fail(p,"Incomplete preset");
 if(!mt_is(p,p->i,"{")){if(mt_is_punct(p,p->i))return mt_fail(p,"Missing value");p->i++;return 1;}
 p->i++;
 const int object=mt_is(p,p->i+1,":")||mt_is(p,p->i+1,"=");
 int keys[MT_MAX_KEYS],nk=0;
 for(;;){
  if(p->i>=p->count)return mt_fail(p,"Incomplete preset");
  if(mt_is(p,p->i,"}"))break;
  if(object){
   const int key=p->i++;
   if(mt_is_punct(p,key)||!(mt_is(p,p->i,":")||mt_is(p,p->i,"=")))return mt_fail(p,"Invalid field");
   p->i++;
   for(int k=0;k<nk;k++)if(mt_equal(p,keys[k],key))return mt_fail(p,"Repeated field");
   if(nk==MT_MAX_KEYS)return mt_fail(p,"Preset too complex");
   keys[nk++]=key;
   int child=0,child_sound=-1,len;const char *name=mt_content(p,key,&len);
   const int group=mt_is(p,p->i,"{"),digit=len==1&&name[0]>='1'&&name[0]<='8'?name[0]-'1':-1;
   if(role==MT_ROOT){
    if(mt_named(p,key,"DrumPatches"))child=MT_PATCHES;
    else if(mt_named(p,key,"Patterns")&&group)child=MT_PATTERN_LIST;
    else if(mt_named(p,key,"Mutes")&&group)p->mutes=p->i+1;
    else if(mt_named(p,key,"StepRate")&&!group)p->steprate=p->i+1;
    else if(mt_named(p,key,"Pattern")&&!group)p->active=p->i+1;
   }
   else if(role==MT_PATCHES&&digit>=0&&group){child=MT_SOUND;child_sound=digit;p->sound[digit]=1;}
   else if(role==MT_SOUND&&!group){
    for(int f=0;f<MT_SOUND_FIELDS;f++)if(mt_named(p,key,mt_sound_keys[f])){p->field[sound][f]=p->i+1;break;}
    if(mt_named(p,key,"Output"))p->output[sound]=p->i+1;
   }
   else if(role==MT_PATTERN_LIST&&len==1&&group&&((name[0]>='a'&&name[0]<='l')||(name[0]>='A'&&name[0]<='L'))){child=MT_PATTERN;child_sound=(name[0]|32)-'a';p->pattern[child_sound]=1;}
   else if(role==MT_PATTERN){
    if(mt_named(p,key,"Length")&&!group)p->length[sound]=p->i+1;
    else if(digit>=0&&group){child=MT_ROW;child_sound=sound*8+digit;p->row[sound][digit]=1;}
   }
   else if(role==MT_ROW&&!group){
    if(mt_named(p,key,"Triggers"))p->trig[sound/8][sound%8]=p->i+1;
    else if(mt_named(p,key,"Accents"))p->acc[sound/8][sound%8]=p->i+1;
   }
   if(!mt_value(p,depth+1,child,child_sound))return 0;
  }
  else if(!mt_value(p,depth+1,0,-1))return 0;
  if(mt_is(p,p->i,","))p->i++;
 }
 p->i++;return 1;
}

/* Plain decimal numbers as written by MicroTonic, plus inf / +inf / -inf. */
static int mt_number(const char *s,int n,double *out){
 char b[64];if(n<=0||n>=(int)sizeof b)return 0;memcpy(b,s,(size_t)n);b[n]=0;
 int i=b[0]=='+'||b[0]=='-';
 if(!strcasecmp(b+i,"inf")){*out=b[0]=='-'?-INFINITY:INFINITY;return 1;}
 int digits=0;while(isdigit((unsigned char)b[i])){i++;digits++;}
 if(b[i]=='.'){i++;while(isdigit((unsigned char)b[i])){i++;digits++;}}
 if(!digits)return 0;
 if(b[i]=='e'||b[i]=='E'){i++;if(b[i]=='+'||b[i]=='-')i++;if(!isdigit((unsigned char)b[i]))return 0;while(isdigit((unsigned char)b[i]))i++;}
 if(b[i])return 0;
 char *end;double x=strtod(b,&end);if(*end||isnan(x))return 0;*out=x;return 1;
}
static int mt_option(const char *s,int n,const char *const *list,int count){for(int i=0;i<count;i++)if((size_t)n==strlen(list[i])&&!memcmp(s,list[i],(size_t)n))return i;return -1;}

/* Tokenize and walk a whole preset; values are captured in p. */
static int mt_scan(MTParser *p,const char *text,size_t n){
 p->text=text;
 if(n>MT_MAX_BYTES)return mt_fail(p,"File too large (1 MB max)");
 if(!(p->tok=malloc(MT_MAX_TOKENS*sizeof *p->tok)))return mt_fail(p,"Out of memory");
 if(!mt_tokenize(p,n))return 0;
 if(!mt_is(p,0,"MicroTonicPresetV3")||!(mt_is(p,1,":")||mt_is(p,1,"=")))return mt_fail(p,"Not a MicroTonic V3 preset");
 p->i=2;if(!mt_value(p,0,MT_ROOT,-1))return 0;
 if(p->i!=p->count)return mt_fail(p,"Unexpected content after preset");
 return 1;
}
/* Worker only: one regular file of at most 1 MiB, NUL-terminated. */
static char *mt_read(const char *path,size_t *size,char message[MT_MESSAGE]){
 FILE *f=fopen(path,"rb");struct stat st;
 if(!f){snprintf(message,MT_MESSAGE,"Missing / unreadable file");return NULL;}
 if(fstat(fileno(f),&st)||!S_ISREG(st.st_mode)){fclose(f);snprintf(message,MT_MESSAGE,"Not a file");return NULL;}
 if(st.st_size>(off_t)MT_MAX_BYTES){fclose(f);snprintf(message,MT_MESSAGE,"File too large (1 MB max)");return NULL;}
 size_t n=(size_t)st.st_size;char *text=malloc(n+1);
 if(!text){fclose(f);snprintf(message,MT_MESSAGE,"Out of memory");return NULL;}
 size_t got=fread(text,1,n,f);fclose(f);
 if(got!=n){free(text);snprintf(message,MT_MESSAGE,"Missing / unreadable file");return NULL;}
 text[n]=0;*size=n;return text;
}
static const char *const mt_offon[2]={"Off","On"};

#ifndef MT_NO_SOUNDS
/* The value text of one field, or its default; NULL when a required field is absent. */
static const char *mt_text(const MTParser *p,int v,int f,int *len){
 int t=p->field[v][f];if(t)return mt_content(p,t-1,len);
 if(!mt_defaults[f])return NULL;
 *len=(int)strlen(mt_defaults[f]);return mt_defaults[f];
}
static int mt_bad(MTParser *p,int v,int f,const char *what){char m[MT_MESSAGE];snprintf(m,sizeof m,"Sound %d: %s %s",v+1,what,mt_sound_keys[f]);return mt_fail(p,m);}

static int mt_convert(MTParser *p,MTKit *kit){
 static const char *const waves[]={"Sine","Triangle","Saw"},*const modes[]={"Decay","Sine","Noise"},*const filters[]={"LP","BP","HP"},*const envs[]={"Exp","Linear","Mod"},*const *offon=mt_offon;
 static const double wave_value[]={0,100./3,200./3};
 double level[8],attenuation=0;
 for(int v=0;v<8;v++){
  char m[MT_MESSAGE];if(!p->sound[v]){snprintf(m,sizeof m,"Sound %d missing",v+1);return mt_fail(p,m);}
  int n;const char *s=mt_text(p,v,19,&n);
  if(!s)return mt_bad(p,v,19,"missing");
  if(!mt_number(s,n,&level[v])||level[v]==INFINITY)return mt_bad(p,v,19,"invalid");
  if(level[v]>attenuation)attenuation=level[v];
 }
 memset(kit,0,sizeof *kit);kit->attenuation_db=(float)attenuation;
 for(int v=0;v<8;v++){
  for(int f=0;f<PT_FIELDS;f++){
   int n,o=0;double x;const char *s=mt_text(p,v,f,&n);
   if(!s)return mt_bad(p,v,f,"missing");
   switch(f){
    case 1:o=mt_option(s,n,waves,3);x=o<0?0:wave_value[o];break;
    case 4:o=mt_option(s,n,modes,3);x=o;break;
    case 9:o=mt_option(s,n,filters,3);x=o;break;
    case 14:o=mt_option(s,n,envs,3);x=o;if(o==2)kit->clap++;break;
    case 15:case 23:o=mt_option(s,n,offon,2);x=o;break;
    case 8:{ /* Mix: "osc/noise" levels, or the oscillator share 0..100 */
     const char *slash=memchr(s,'/',(size_t)n);double a,b;
     if(slash){if(!mt_number(s,(int)(slash-s),&a)||!mt_number(slash+1,n-(int)(slash-s)-1,&b)||!isfinite(a)||!isfinite(b)||a<0||b<0||a+b<=0)return mt_bad(p,v,f,"invalid");x=100*b/(a+b);}
     else{if(!mt_number(s,n,&a))return mt_bad(p,v,f,"invalid");x=100-a;}
     break;}
    case 16:if(!mt_number(s,n,&x))return mt_bad(p,v,f,"invalid");x*=.5;break; /* 0..100 -> DRIVE dB */
    case 19:x=100*pow(10,(level[v]-attenuation)/20);break;
    default:if(!mt_number(s,n,&x))return mt_bad(p,v,f,"invalid");
   }
   if(o<0)return mt_bad(p,v,f,"unknown");
   const PTField *d=&pt_fields[f];double y=x<d->lo?d->lo:x>d->hi?d->hi:x;
   if(y!=x)kit->clamped++;
   /* Same decimal path as the converter and the saved state, so an imported
    * kit is bit-identical to the matching converted .xpl preset. */
   char text[32];snprintf(text,sizeof text,"%.9g",y);kit->values[v][f]=strtof(text,NULL);
  }
  if(p->output[v]){int n;const char *s=mt_content(p,p->output[v]-1,&n);if(!(n==1&&s[0]=='A'))kit->outputs++;}
 }
 return 1;
}

/* Parse a whole .mtpreset text of n bytes. On failure kit is left unchanged
 * except kit->message. */
static int mt_parse(const char *text,size_t n,MTKit *kit){
 MTParser *p=calloc(1,sizeof *p);MTKit *out=calloc(1,sizeof *out);int ok=0;
 if(!p||!out){if(kit)snprintf(kit->message,sizeof kit->message,"Out of memory");free(p);free(out);return 0;}
 ok=mt_scan(p,text,n)&&mt_convert(p,out);
 if(ok){*kit=*out;kit->message[0]=0;}
 else snprintf(kit->message,sizeof kit->message,"%s",p->message[0]?p->message:"Invalid preset");
 free(p->tok);free(p);free(out);return ok;
}
/* Worker only. */
static int mt_load(const char *path,MTKit *kit){size_t n;char *text=mt_read(path,&n,kit->message);if(!text)return 0;int ok=mt_parse(text,n,kit);free(text);return ok;}
#endif

/* ---- Sequences ---- */
#define MT_CHOICES 23 /* 0-11: one pattern A-L; 12-22: A+B ... K+L end to end */
typedef struct {int steps,valid;uint32_t trig[8],acc[8];} MTPattern;
typedef struct {
 MTPattern pattern[MT_PATTERNS];
 int division;          /* StepRate 1/32 1/16 1/8 1/4 1/2 -> 0..4 (default 1/16) */
 int active;            /* current Pattern a-l -> 0..11 (default a) */
 int usable;            /* number of valid patterns */
 unsigned mutes;        /* bit v = row v+1 muted (all Off unless 8 Off/On values) */
 char message[MT_MESSAGE];
} MTSequence;
static const char *const mt_rates[5]={"1/32","1/16","1/8","1/4","1/2"};
/* '#' = set, '-' = rest, whitespace ignored; cells past Length are checked, then ignored. */
static int mt_row_bits(const MTParser *p,int t,int steps,uint32_t *bits){
 *bits=0;if(!t)return 1;int n;const char *s=mt_content(p,t-1,&n);
 for(int i=0,at=0;i<n;i++){char c=s[i];if(mt_space(c))continue;if(c!='#'&&c!='-')return 0;if(c=='#'&&at<steps)*bits|=UINT32_C(1)<<at;at++;}
 return 1;
}
static int mt_sequence(MTParser *p,MTSequence *q){
 memset(q,0,sizeof *q);q->division=1;int n;const char *s;
 if(p->steprate){s=mt_content(p,p->steprate-1,&n);int o=mt_option(s,n,mt_rates,5);if(o<0)return mt_fail(p,"Unknown StepRate");q->division=o;}
 if(p->active){s=mt_content(p,p->active-1,&n);if(n==1&&((s[0]>='a'&&s[0]<='l')||(s[0]>='A'&&s[0]<='L')))q->active=(s[0]|32)-'a';}
 if(p->mutes){unsigned mask=0;int count=0,ok=1;
  for(int i=p->mutes;i<p->count&&!mt_is(p,i,"}");i++){if(mt_is(p,i,","))continue;if(mt_is_punct(p,i)||count==8){ok=0;break;}s=mt_content(p,i,&n);int o=mt_option(s,n,mt_offon,2);if(o<0){ok=0;break;}mask|=(unsigned)o<<count++;}
  if(ok&&count==8)q->mutes=mask;}
 for(int k=0;k<MT_PATTERNS;k++){MTPattern *m=&q->pattern[k];double length;
  if(!p->pattern[k]||!p->length[k])continue;
  s=mt_content(p,p->length[k]-1,&n);if(!mt_number(s,n,&length)||length!=floor(length)||length<1||length>32)continue;
  int ok=1;for(int r=0;r<8&&ok;r++)ok=p->row[k][r]&&mt_row_bits(p,p->trig[k][r],(int)length,&m->trig[r])&&mt_row_bits(p,p->acc[k][r],(int)length,&m->acc[r]);
  if(ok){m->steps=(int)length;m->valid=1;q->usable++;}else memset(m,0,sizeof *m);
 }
 if(!q->usable)return mt_fail(p,"No usable pattern");
 return 1;
}
/* Pattern A-L, or two consecutive patterns end to end when they fit in 32 steps. */
static int mt_choice(const MTSequence *q,int choice,MTPattern *out){
 memset(out,0,sizeof *out);if(choice<0||choice>=MT_CHOICES)return 0;
 if(choice<MT_PATTERNS){if(!q->pattern[choice].valid)return 0;*out=q->pattern[choice];return 1;}
 const MTPattern *a=&q->pattern[choice-12],*b=&q->pattern[choice-11];
 if(!a->valid||!b->valid||a->steps+b->steps>32)return 0;
 out->steps=a->steps+b->steps;out->valid=1;
 for(int r=0;r<8;r++){out->trig[r]=a->trig[r]|b->trig[r]<<a->steps;out->acc[r]=a->acc[r]|b->acc[r]<<a->steps;}
 return 1;
}
static int mt_parse_sequence(const char *text,size_t n,MTSequence *q){
 MTParser *p=calloc(1,sizeof *p);MTSequence *out=calloc(1,sizeof *out);int ok=0;
 if(!p||!out){snprintf(q->message,sizeof q->message,"Out of memory");free(p);free(out);return 0;}
 ok=mt_scan(p,text,n)&&mt_sequence(p,out);
 if(ok){*q=*out;q->message[0]=0;}
 else snprintf(q->message,sizeof q->message,"%s",p->message[0]?p->message:"Invalid preset");
 free(p->tok);free(p);free(out);return ok;
}
/* Worker only. */
static int mt_load_sequence(const char *path,MTSequence *q){size_t n;char *text=mt_read(path,&n,q->message);if(!text)return 0;int ok=mt_parse_sequence(text,n,q);free(text);return ok;}

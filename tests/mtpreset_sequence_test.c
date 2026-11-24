/* πClideaN .mtpreset pattern reader tests. Original implementation, MIT.
 *   mtpreset-sequence-test            unit tests
 *   mtpreset-sequence-test --dump F   one line per value, compared with the
 *                                     FM-1 reader by tests/mtpreset_sequence_compare.mjs */
#define MT_NO_SOUNDS
#include "../src/mtpreset.h"

static int failures;
#define CHECK(c,...) do{if(!(c)){failures++;printf("FAIL line %d: ",__LINE__);printf(__VA_ARGS__);printf("\n");}}while(0)

/* Preset with patterns a..l; lengths[k]<=0 leaves pattern k out. Row text is
 * the same for every row and pattern unless a pattern-specific one is given. */
static char *preset(const char *head,const int lengths[12],const char *trig,const char *acc,int special,const char *special_trig){
 size_t cap=256*1024;char *s=malloc(cap);int n=snprintf(s,cap,"MicroTonicPresetV3: {\n%s\tPatterns: {\n",head);
 for(int k=0;k<12;k++){if(lengths[k]<=0)continue;n+=snprintf(s+n,cap-n,"\t\t%c: {\n\t\t\tChained = false\n\t\t\tLength = %d\n",'a'+k,lengths[k]);
  for(int r=1;r<=8;r++)n+=snprintf(s+n,cap-n,"\t\t\t%d: {\n\t\t\t\tAccents = %s\n\t\t\t\tFills = ----------------\n\t\t\t\tTriggers = %s\n\t\t\t}\n",r,acc,k==special?special_trig:trig);
  n+=snprintf(s+n,cap-n,"\t\t}\n");}
 snprintf(s+n,cap-n,"\t}\n}\n");return s;
}
static int parse(const char *t,MTSequence *q){return mt_parse_sequence(t,strlen(t),q);}

static void unit_tests(void){
 MTSequence q;MTPattern m;char *s;int all16[12];for(int k=0;k<12;k++)all16[k]=16;
 const char *head="\tPattern = c\n\tStepRate = 1/8\n\tMutes: { Off, On, Off, Off, Off, Off, Off, On }\n";
 s=preset(head,all16,"#---#---#---#---","#---------------",-1,"");CHECK(parse(s,&q),"valid refused: %s",q.message);free(s);
 CHECK(q.division==2&&q.active==2&&q.mutes==0x82&&q.usable==12,"root %d %d %x %d",q.division,q.active,q.mutes,q.usable);
 CHECK(q.pattern[0].steps==16&&q.pattern[0].trig[0]==0x1111&&q.pattern[0].acc[0]==1,"bits %x %x",q.pattern[0].trig[0],q.pattern[0].acc[0]);
 /* Pairs: A+B = 32 steps, second pattern shifted by the first length. */
 CHECK(mt_choice(&q,12,&m)&&m.steps==32&&m.trig[3]==0x11111111u&&m.acc[3]==0x10001u,"pair %d %x %x",m.steps,m.trig[3],m.acc[3]);
 CHECK(mt_choice(&q,22,&m)&&m.steps==32,"K+L");CHECK(!mt_choice(&q,23,&m)&&!mt_choice(&q,-1,&m),"range");
 /* Defaults, short rows are rests, long rows cut, whitespace inside quotes. */
 int lens[12]={8,24,32,1,16,16,16,16,16,16,16,16};
 s=preset("",lens,"\"# - #\"","##",-1,"");CHECK(parse(s,&q),"defaults refused: %s",q.message);free(s);
 CHECK(q.division==1&&q.active==0&&q.mutes==0,"defaults");
 CHECK(q.pattern[0].trig[5]==5&&q.pattern[3].trig[5]==1&&q.pattern[0].acc[0]==3&&q.pattern[3].acc[0]==1,"short/space %x %x",q.pattern[0].trig[5],q.pattern[3].trig[5]);
 CHECK(mt_choice(&q,12,&m)&&m.steps==32&&m.trig[0]==(5u|5u<<8),"A+B 8+24");CHECK(!mt_choice(&q,13,&m),"B+C 24+32 refused");
 CHECK(!mt_choice(&q,14,&m),"C+D 32+1 refused");CHECK(mt_choice(&q,2,&m)&&m.steps==32,"C alone 32");
 s=preset("",lens,"################################","-",-1,"");CHECK(parse(s,&q),"bit31: %s",q.message);free(s);
 CHECK(q.pattern[2].trig[7]==0xffffffffu&&q.pattern[0].trig[7]==0xff&&q.pattern[3].trig[7]==1,"bit31 %x",q.pattern[2].trig[7]);
 /* An invalid pattern is unavailable, the others stay usable. */
 s=preset("",all16,"#---","-",4,"#-x-");CHECK(parse(s,&q)&&!q.pattern[4].valid&&q.usable==11,"bad chars %d",q.usable);free(s);
 CHECK(!mt_choice(&q,4,&m)&&!mt_choice(&q,15,&m)&&!mt_choice(&q,16,&m)&&mt_choice(&q,17,&m),"pairs around invalid E");
 int odd[12]={16,0,-1,33,0,16,16,16,16,16,16,16};
 s=preset("",odd,"#---","-",-1,"");CHECK(parse(s,&q)&&q.usable==8&&!q.pattern[1].valid&&!q.pattern[2].valid&&!q.pattern[3].valid,"lengths %d",q.usable);free(s);
 s=preset("",all16,"#---","-",-1,"");char *r=strstr(s,"Length = 16");memcpy(r,"Length = 1.5",12);CHECK(parse(s,&q)&&!q.pattern[0].valid,"fractional length");free(s);
 s=preset("",all16,"#---","-",-1,"");r=strstr(s,"\t\t\t3: {");r[3]='9';CHECK(parse(s,&q)&&!q.pattern[0].valid&&q.usable==11,"missing row");free(s);
 /* Mutes need eight Off/On values, otherwise they are ignored. */
 s=preset("\tMutes: { On, On }\n",all16,"#","-",-1,"");CHECK(parse(s,&q)&&q.mutes==0,"short mutes");free(s);
 s=preset("\tMutes: { On, Off, Off, Off, Off, Off, Off, Maybe }\n",all16,"#","-",-1,"");CHECK(parse(s,&q)&&q.mutes==0,"bad mutes");free(s);
 /* Errors. */
 int none[12]={0};s=preset("",none,"#","-",-1,"");CHECK(!parse(s,&q)&&strstr(q.message,"No usable pattern"),"no pattern: %s",q.message);free(s);
 s=preset("\tStepRate = 1/12\n",all16,"#","-",-1,"");CHECK(!parse(s,&q)&&strstr(q.message,"StepRate"),"steprate: %s",q.message);free(s);
 CHECK(!parse("MicroTonicPresetV3: { Patterns: {",&q)&&strstr(q.message,"Incomplete"),"truncated: %s",q.message);
 CHECK(!parse("hello",&q)&&strstr(q.message,"Not a MicroTonic"),"header");
 printf("%s mtpreset sequence unit tests (%d failure%s)\n",failures?"FAIL":"PASS",failures,failures==1?"":"s");
}

static int dump(const char *path){
 MTSequence q;if(!mt_load_sequence(path,&q)){printf("ERR %s\n",q.message);return 0;}
 printf("division %d\nactive %d\nmutes %u\n",q.division,q.active,q.mutes);
 for(int k=0;k<MT_PATTERNS;k++){const MTPattern *m=&q.pattern[k];printf("pattern %d %d %d",k,m->valid,m->steps);for(int r=0;r<8;r++)printf(" %u/%u",m->trig[r],m->acc[r]);printf("\n");}
 for(int c=MT_PATTERNS;c<MT_CHOICES;c++){MTPattern m;int ok=mt_choice(&q,c,&m);printf("choice %d %d %d",c,ok,m.steps);for(int r=0;r<8;r++)printf(" %u/%u",m.trig[r],m.acc[r]);printf("\n");}
 return 0;
}
int main(int argc,char **argv){if(argc==3&&!strcmp(argv[1],"--dump"))return dump(argv[2]);unit_tests();return failures!=0;}

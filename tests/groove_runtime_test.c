#define main previous_mod_test_main
#include "mod_engine_test.c"
#undef main
#include <time.h>
static void table_preferences(void){Engine *e=fresh();Lane a=e->lane[0];a.f[0]=16;a.f[1]=4;a.f[2]=0;a.f[3]=1;pattern(&a);Lane b=a;b.f[2]=1;pattern(&b);assert(GROOVE_TABLE_BYTES==457);assert(groove_line_score(&a)<groove_line_score(&b));
 /* On a 10-step cycle, rotations preserve cyclic gaps: do not impose a 4/4 phase. */
 a.f[0]=10;a.f[1]=3;a.f[2]=0;pattern(&a);b=a;b.f[2]=1;pattern(&b);assert(groove_line_score(&a)==groove_line_score(&b));
 unsigned ta[4],tb[4];groove_timeline(&a,ta);groove_timeline(&b,tb);for(int t=0;t<128;t++){int unit=1<<a.f[3];assert(!!(ta[t/32]&(1u<<(t%32)))==(t%unit==0&&effective_hit(&a,(t/unit)%a.f[0])));}
 puts("PASS empirical phase preference, normalized gaps without imposed 4/4 on 10-step lane, exact cached event masks, 457-byte tables");done(e);}
static void repeatability(void){Engine*a=fresh(),*b=fresh();variation_reset(a);variation_reset(b);a->random=b->random=13402;
 for(int k=0;k<100;k++){guided_eight_sync(a);guided_eight_sync(b);for(int i=0;i<8;i++)assert(!memcmp(a->lane[i].f,b->lane[i].f,4*sizeof(int)));}
 /* Measure only explicit generation locally; no work added to playback loops. */
 struct timespec x,y;clock_gettime(CLOCK_MONOTONIC,&x);for(int k=0;k<100;k++)guided_eight_sync(a);clock_gettime(CLOCK_MONOTONIC,&y);double ms=((y.tv_sec-x.tv_sec)*1e3+(y.tv_nsec-x.tv_nsec)/1e6)/100;
 printf("PASS seeded repeatability; local PC-only generation timing %.3f ms/request (not Force CPU qualification)\n",ms);done(a);done(b);}
int main(void){table_preferences();repeatability();return 0;}

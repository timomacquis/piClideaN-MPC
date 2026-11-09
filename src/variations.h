/* Bounded, allocation-free variation and ensemble generation. Included by engine.c. */
static void variation_capture(const Lane *l,Variation *v){memcpy(v->geometry,l->f,sizeof v->geometry);memcpy(v->values,l->values,sizeof v->values);memcpy(v->gates,l->gates,sizeof v->gates);}
static void variation_restore(Lane *l,const Variation *v){memcpy(l->f,v->geometry,sizeof v->geometry);memcpy(l->values,v->values,sizeof v->values);memcpy(l->gates,v->gates,sizeof v->gates);pattern(l);}
static void variation_restore_owner(Engine *e,int i,const Variation *v){for(int s=0;s<32;s++)for(int f=0;f<5;f++)if(e->lane[i].values[s][f]!=v->values[s][f])notify_id(e,LEGACY_CHANGED+14+LANES*4+i*160+s*5+f);variation_restore(&e->lane[i],v);}
static void variation_changed(Engine *e,int i){e->q_valid=0;e->mod_latch_valid[i]=0;e->release_pending|=1u<<i;e->step=clampi(e->step,0,e->lane[e->selected].f[0]-1);changed_lane(e,i);e->variation_ui=1;}
static void variation_copy(Engine *e){int i=e->selected;if(!e->variation_bank[i]){variation_capture(&e->lane[i],&e->alternate[i]);e->variation_ready|=1u<<i;e->variation_bank[i]=1;}else if(e->variation_ready&(1u<<i))variation_restore_owner(e,i,&e->alternate[i]);e->variation_status=4;variation_changed(e,i);}
static void variation_select(Engine *e,int bank){int i=e->selected;bank=!!bank;if(bank==e->variation_bank[i])return;if(!(e->variation_ready&(1u<<i))){variation_capture(&e->lane[i],&e->alternate[i]);e->variation_ready|=1u<<i;}
 Variation current;variation_capture(&e->lane[i],&current);variation_restore_owner(e,i,&e->alternate[i]);e->alternate[i]=current;e->variation_bank[i]=bank;e->variation_status=5;variation_changed(e,i);}
static unsigned rhythm_mask(const Lane *l){unsigned mask=0;for(int i=0;i<32;i++)if(effective_hit(l,i*l->f[0]/32))mask|=1u<<i;return mask;}
static int rhythm_count(unsigned x){int n=0;while(x){x&=x-1;n++;}return n;}
static int rhythm_gap(const Lane *l){int max=0,run=0,n=l->f[0];for(int i=0;i<n*2;i++){if(effective_hit(l,i%n))run=0;else if(++run>max)max=run;}return max>n?n:max;}
/* Statistics are soft preferences, never changes to protected values.
 * Cyclic gaps are normalized by length. Metric phase costs apply only to
 * 16/32-step sixteenth-note cycles; no 4/4 phase template on polymetric lanes. */
#if PICN_GROOVE_ENABLED
static int groove_band(const Lane *l){int h=density(l),n=l->f[0];return h*4<=n?0:h*2<=n?1:2;}
static int groove_line_score(const Lane *l){int positions[32],hits=0,n=l->f[0],c=groove_band(l),score=0,phase=0;
 for(int s=0;s<n;s++)if(effective_hit(l,s))positions[hits++]=s;
 if(!hits)return 0;
 for(int k=0;k<hits;k++){int gap=(positions[(k+1)%hits]-positions[k]+n)%n;if(!gap)gap=n;int bin=clampi((gap*16+n/2)/n,1,16);score+=groove_gap_cost[c][bin-1];
 if(l->f[3]==1&&(n==16||n==32))phase+=groove_phase_cost[c][positions[k]%16];}
 return (score+phase)/hits;
}
static void groove_timeline(const Lane *l,unsigned bits[4]){memset(bits,0,4*sizeof *bits);int unit=1<<l->f[3];for(int t=0;t<ENSEMBLE_TICKS;t+=unit)if(effective_hit(l,(t/unit)%l->f[0]))bits[t/32]|=1u<<(t%32);}
static int groove_timeline_band(const unsigned bits[4]){int hits=0;for(int w=0;w<4;w++)hits+=rhythm_count(bits[w]);return hits<=16?0:hits<=32?1:2;}
static int groove_pair_score(const Lane *l,const unsigned current[4],const unsigned previous[8][4],const int previous_band[8],int count){int total=0,used=0,a=0,c=groove_timeline_band(current);for(int w=0;w<4;w++)a+=rhythm_count(current[w]);
 for(int i=0;i<count;i++){int b=0,together=0;for(int w=0;w<4;w++){b+=rhythm_count(previous[i][w]);together+=rhythm_count(current[w]&previous[i][w]);}
 int minimum=a<b?a:b;if(minimum){int bin=clampi((together*16+minimum/2)/minimum,0,16);total+=groove_pair_cost[c][previous_band[i]][bin];used++;}}
 return used?total/used:0;
}
static int groove_density_pick(Engine *e,int index){static const unsigned char rank[8]={1,2,0,3,4,5,6,7};unsigned draw=rng(e)&65535;int h=1;while(h<12&&draw>=groove_density_cdf[rank[index]][h])h++;return h;}
#endif
static int guided_score(const Lane *a,const Lane *b,int strength){unsigned x=rhythm_mask(a),y=rhythm_mask(b);int distance=rhythm_count(x^y);int score=distance*(strength==0?12:strength==1?5:1);
 /* Preserve existing cyclic anchors without imposing a sixteen-step metre. */
 if((x&1)&&!(y&1))score+=12;if((x&(1u<<16))&&!(y&(1u<<16)))score+=6;
 score+=abs(density(b)*a->f[0]-density(a)*b->f[0])*(strength==0?3:1);score+=abs(rhythm_gap(b)-rhythm_gap(a));
 if(!distance&&a->f[3]==b->f[3])score+=200;
#if PICN_GROOVE_ENABLED
 score+=groove_line_score(b); /* bounded empirical preference, below identity penalty */
#endif
 return score;
}
static void guided_alter_sync(Engine *e){int i=e->selected,mask=e->variation_mask&15,strength=clampi(e->variation_strength,0,2);if(!mask){e->variation_status=3;e->variation_ui=1;return;}Lane original=e->lane[i],best;int best_score=INT_MAX,found=0;
 for(int attempt=0;attempt<GUIDED_TRIES;attempt++){Lane candidate=original;int *f=candidate.f;
 if(mask&1){int range=guided_strength_steps[strength];f[0]=clampi(f[0]+(int)(rng(e)%(2*range+1))-range,1,32);}
 if(!(mask&2)&&f[1]>density_limit(f[0]))continue;if(!(mask&4)&&f[2]>=f[0])continue;
 if(mask&2){int range=guided_strength_hits[strength];f[1]=clampi(f[1]+(int)(rng(e)%(2*range+1))-range,0,density_limit(f[0]));}
 if(mask&4){int range=guided_strength_rotate[strength];f[2]=(f[2]+(int)(rng(e)%(2*range+1))-range+64)%f[0];}
 if(mask&8){int range=strength==2?4:1;f[3]=clampi(f[3]+(int)(rng(e)%(2*range+1))-range,0,4);}
 if(!memcmp(f,original.f,4*sizeof(int)))continue;pattern(&candidate);if(density(&candidate)>density_limit(f[0]))continue;
 int score=guided_score(&original,&candidate,strength)+(int)(rng(e)%5);if(score<best_score){best_score=score;best=candidate;found=1;}}
 if(!found){e->variation_status=2;e->variation_ui=1;return;}
 /* Preserve A automatically on the first ALTER and whenever starting from A. */
 if(!e->variation_bank[i]){variation_capture(&original,&e->alternate[i]);e->variation_ready|=1u<<i;e->variation_bank[i]=1;}
 memcpy(e->lane[i].f,best.f,4*sizeof(int));e->lane[i].pattern=best.pattern;e->variation_status=1;variation_changed(e,i);
}
static int ensemble_score(const Lane *l,const unsigned char occupancy[ENSEMBLE_TICKS],int index,const unsigned previous[8][4],const int previous_band[8]){int score=0,events=0,unit=1<<l->f[3];
#if PICN_GROOVE_ENABLED
 unsigned current[4]={0};
#endif
for(int t=0;t<ENSEMBLE_TICKS;t++)if(t%unit==0&&effective_hit(l,(t/unit)%l->f[0])){events++;
#if PICN_GROOVE_ENABLED
 current[t/32]|=1u<<(t%32);score+=occupancy[t]*2; /* allow meaningful shared attacks, discourage saturation */
 if(occupancy[t]>=5)score+=80;
#else
 score+=occupancy[t]*occupancy[t]*14;if(occupancy[t]>=3)score+=200;
#endif
 }
#if PICN_GROOVE_ENABLED
 score+=groove_line_score(l)*2+groove_pair_score(l,current,previous,previous_band,index)*3;
 if(index&&effective_hit(l,0))score+=2;
#else
 if(index&&effective_hit(l,0))score+=14;
#endif
 if(ensemble_anchor[index]&&!effective_hit(l,0))score+=40;
 score+=events*(index>3?2:0);return score;
}
static void guided_apply_eight(Engine *e,const Lane generated[8]){
 rand_clear(e);for(int i=0;i<8;i++){int note_value=note(e,i),mute=e->lane[i].f[12];double origin=e->lane[i].origin;for(int step=0;step<32;step++)for(int f=0;f<5;f++)if(e->lane[i].values[step][f]>=0)notify_id(e,LEGACY_CHANGED+14+LANES*4+i*160+step*5+f);e->lane[i]=generated[i];e->lane[i].f[8]=e->lane[i].f[9]=note_value;e->lane[i].f[12]=mute;e->lane[i].f[4]=100;e->lane[i].f[5]=0;e->lane[i].f[6]=100;e->lane[i].f[7]=50;e->lane[i].hold=0;e->lane[i].origin=origin;e->variation_ready&=~(1u<<i);e->variation_bank[i]=0;memset(&e->alternate[i],0,sizeof e->alternate[i]);e->head[i]=-1;e->ui_offset[i]=0;e->reset_at[i]=-1;variation_changed(e,i);notify_id(e,HOLD_ID+i);}
 for(int s=0;s<MOD_SLOTS;s++)if(e->mod_pad[s]>=0&&e->mod_pad[s]<8){mod_defaults(&e->mod[s]);e->mod_pad[s]=-1;memset(e->mod_phase[s],0,sizeof e->mod_phase[s]);memset(e->mod_walk[s],0,sizeof e->mod_walk[s]);e->mod_chain_valid[s]=0;memset(e->mod_running_cache[s],0,sizeof e->mod_running_cache[s]);e->mod_trace_dirty|=1u<<s;mod_coefficients(e,s);}
 if(e->mod_pad[e->mod_selected]<0)mod_notify_selected(e);e->variation_status=6;e->dialog=0;changed_dialog(e);
}
static void guided_eight_sync(Engine *e){Lane generated[8];unsigned timelines[8][4]={{0}};int bands[8]={0};unsigned char occupancy[ENSEMBLE_TICKS]={0};int common=(rng(e)&1)?16:32;
 for(int i=0;i<8;i++){Lane best;int lowest=INT_MAX;
#if PICN_GROOVE_ENABLED
 int target=groove_density_pick(e,i);
#endif
 for(int attempt=0;attempt<ENSEMBLE_TRIES;attempt++){Lane c;init_lane(&c,i);c.f[0]=common;c.f[3]=ensemble_division[i];if(i>3&&(rng(e)&3)==0)c.f[0]=(rng(e)&1)?12:24;
 
#if PICN_GROOVE_ENABLED
 c.f[1]=clampi((target*c.f[0]+8)/16+(int)(rng(e)%3)-1,1,density_limit(c.f[0]));
#else
 c.f[1]=clampi(ensemble_hits[i]+(int)(rng(e)%3)-1,1,density_limit(c.f[0]));
#endif
c.f[2]=ensemble_anchor[i]?0:rng(e)%c.f[0];pattern(&c);int score=ensemble_score(&c,occupancy,i,timelines,bands)+(int)(rng(e)%7);if(score<lowest){lowest=score;best=c;}}
 generated[i]=best;
#if PICN_GROOVE_ENABLED
 groove_timeline(&best,timelines[i]);bands[i]=groove_timeline_band(timelines[i]);
#endif
 int unit=1<<best.f[3];for(int t=0;t<ENSEMBLE_TICKS;t++)if(t%unit==0&&effective_hit(&best,(t/unit)%best.f[0]))occupancy[t]++;
 }
 guided_apply_eight(e,generated);
}
#include "guidance_worker.h"
static void variation_open(Engine *e){if(e->edit_view)return;rand_clear(e);e->dialog=7;e->variation_status=0;e->variation_ui=1;changed_dialog(e);}
static int variation_set(Engine *e,const char *key,int value){
 if(!strcmp(key,"undo")){if(value>0)variation_open(e);return 1;}
 if(!strcmp(key,"redo")){if(e->dialog==7)variation_select(e,clampi(value,0,1));return 1;}
 if(e->dialog!=7)return 0;
 if(!strcmp(key,"randomize"))return 1;
 if(!strcmp(key,"close")){if(value>0){e->dialog=0;changed_dialog(e);}return 1;}
 if(!strcmp(key,"rhythm_choice")){int next=clampi(value,0,2);if(next!=e->variation_strength){e->variation_strength=next;e->variation_ui=1;}return 1;}
 if(!strcmp(key,"erase_values")){if(value>0)variation_copy(e);return 1;}
 if(!strcmp(key,"reset_phase")){if(value>0)guided_alter(e);return 1;}
 int n;if(parse_suffix(key,"rand_toggle",8,&n)&&n>=4){if(value>0){e->variation_mask^=1u<<(n-4);e->variation_ui=1;}return 1;}
 return 0;
}
static int variation_get(Engine *e,const char *key,char *b,int size){int n;if(!strcmp(key,"undo"))return snprintf(b,size,"0");
 if(!strcmp(key,"redo"))return snprintf(b,size,"%d",e->variation_bank[e->selected]);if(!strcmp(key,"redo_display"))return snprintf(b,size,"%s",e->variation_bank[e->selected]?"B":"A");if(!strcmp(key,"rhythm_choice_display")){static const char *labels[]={"LIGHT","MEDIUM","FREE"};return snprintf(b,size,"%s",labels[clampi(e->variation_strength,0,2)]);}
 if(e->dialog==7){if(!strcmp(key,"can_undo"))return snprintf(b,size,"%d",e->variation_status);if(!strcmp(key,"can_undo_display")){static const char *text[]={"Choose A/B and variation parameters","Variation ready in B","No compatible variation: protected steps / density","Choose at least one parameter","A copied to B","Variant selected","Eight lines generated","Preparing variation..."};return snprintf(b,size,"%s",text[clampi(e->variation_status,0,7)]);}if(!strcmp(key,"rhythm_choice"))return snprintf(b,size,"%d",e->variation_strength);if(parse_suffix(key,"rand_pick",8,&n)&&n>=4)return snprintf(b,size,"%d",!!(e->variation_mask&(1u<<(n-4))));}return -1;
}

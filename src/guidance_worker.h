/* One small mailbox; the owner never waits for candidate search. No Engine
 * state is shared with the worker: its scratch instance has no MIDI port. */
typedef struct {
 pthread_t thread;pthread_mutex_t lock;atomic_int stop;int started;
 unsigned request;atomic_uint complete;int type,selected,mask,strength;unsigned random,ready;unsigned char bank[LANES];Lane lanes[LANES];Variation alternate;
 Engine *scratch;Lane result[8];Variation result_alternate;int result_status,result_bank,result_ready;unsigned result_random;
} GuidanceWorker;
static void *guidance_run(void *ptr){GuidanceWorker *w=ptr;prctl(PR_SET_NAME,"PiCN Rhythm",0,0,0);unsigned seen=0;
 while(!atomic_load(&w->stop)){
 pthread_mutex_lock(&w->lock);unsigned request=w->request;int type=w->type,selected=w->selected;Engine *e=w->scratch;
 if(request!=seen){seen=request;memcpy(e->lane,w->lanes,sizeof w->lanes);e->selected=selected;e->variation_mask=w->mask;e->variation_strength=w->strength;e->variation_ready=w->ready;memcpy(e->variation_bank,w->bank,sizeof w->bank);e->alternate[selected]=w->alternate;e->random=w->random;e->guidance_async=0;e->changes=e->change_read=e->change_write=0;memset(e->queued,0,sizeof e->queued);}
 pthread_mutex_unlock(&w->lock);
 if(request&&request==seen&&request!=w->complete){if(type==8)guided_eight_sync(e);else guided_alter_sync(e);
 pthread_mutex_lock(&w->lock);if(w->request==request){if(type==8)memcpy(w->result,e->lane,sizeof w->result);else w->result[0]=e->lane[selected];w->result_alternate=e->alternate[selected];w->result_bank=e->variation_bank[selected];w->result_ready=!!(e->variation_ready&(1u<<selected));w->result_status=e->variation_status;w->result_random=e->random;w->complete=request;}pthread_mutex_unlock(&w->lock);
 }
 struct timespec delay={0,10000000};nanosleep(&delay,NULL);
 }return NULL;
}
static int guidance_start(Engine *e){GuidanceWorker *w=calloc(1,sizeof *w);if(!w)return 0;w->scratch=calloc(1,sizeof *w->scratch);if(!w->scratch){free(w);return 0;}pthread_mutex_init(&w->lock,NULL);e->guidance_worker=w;return 1;}
static void guidance_stop(Engine *e){GuidanceWorker *w=e->guidance_worker;if(!w)return;atomic_store(&w->stop,1);if(w->started)pthread_join(w->thread,NULL);pthread_mutex_destroy(&w->lock);free(w->scratch);free(w);e->guidance_worker=NULL;}
static void guidance_request(Engine *e,int type){GuidanceWorker *w=e->guidance_worker;if(!w){e->variation_status=2;e->variation_ui=1;return;}
 if(!w->started){pthread_attr_t attr;int initialized=pthread_attr_init(&attr)==0;struct sched_param priority={0};int ok=initialized&&pthread_attr_setstacksize(&attr,128*1024)==0&&pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED)==0&&pthread_attr_setschedpolicy(&attr,SCHED_OTHER)==0&&pthread_attr_setschedparam(&attr,&priority)==0;if(ok)ok=pthread_create(&w->thread,&attr,guidance_run,w)==0;if(initialized)pthread_attr_destroy(&attr);if(!ok){e->variation_status=2;e->variation_ui=1;return;}w->started=1;}
 if(pthread_mutex_trylock(&w->lock)){e->variation_status=2;e->variation_ui=1;return;}
 memcpy(w->lanes,e->lane,sizeof w->lanes);w->selected=e->selected;w->mask=e->variation_mask;w->strength=e->variation_strength;w->ready=e->variation_ready;memcpy(w->bank,e->variation_bank,sizeof w->bank);w->alternate=e->alternate[e->selected];w->random=e->random;w->type=type;w->request++;w->complete=0;e->guidance_sequence=w->request;e->guidance_stamp=e->control_revision;pthread_mutex_unlock(&w->lock);
 if(type==8){e->dialog=0;changed_dialog(e);}e->variation_status=7;e->variation_ui=1;
}
static void guidance_poll(Engine *e){GuidanceWorker *w=e->guidance_worker;if(!w||!e->guidance_sequence||pthread_mutex_trylock(&w->lock))return;
 if(w->complete!=e->guidance_sequence){pthread_mutex_unlock(&w->lock);return;}
 int type=w->type,selected=w->selected,status=w->result_status;Lane result[8];memcpy(result,w->result,sizeof result);Variation alternate=w->result_alternate;int bank=w->result_bank,ready=w->result_ready;unsigned random=w->result_random;
 e->guidance_sequence=0;pthread_mutex_unlock(&w->lock);
 if(e->control_revision!=e->guidance_stamp){e->variation_status=2;e->variation_ui=1;return;}
 e->random=random;if(type==8)guided_apply_eight(e,result);else if(status==1){memcpy(e->lane[selected].f,result[0].f,4*sizeof(int));e->lane[selected].pattern=result[0].pattern;e->alternate[selected]=alternate;e->variation_bank[selected]=bank;if(ready)e->variation_ready|=1u<<selected;variation_changed(e,selected);}
 e->variation_status=status;e->variation_ui=1;e->control_revision++;
}
static void guided_alter(Engine *e){if(e->guidance_async)guidance_request(e,1);else guided_alter_sync(e);}
static void guided_eight(Engine *e){if(e->guidance_async)guidance_request(e,8);else guided_eight_sync(e);}

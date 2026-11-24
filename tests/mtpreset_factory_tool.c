/* πClideaN 1.4.1 : construit les programmes d'usine à partir de fichiers .mtpreset.
 * Pour chaque fichier : état INIT, puis import du motif choisi exactement comme le bouton
 * IMPORT .MTPRESET (mt_load_sequence, mt_choice, mti_apply sur les lignes 1-8), puis état écrit.
 * Usage : mtpreset_factory_tool INIT.state CHOIX fichier.mtpreset...  (CHOIX 12 = A+B)
 * Sortie : une ligne par fichier, "OK <pas> <choix> <état>" ou "ERR <raison>". Original, MIT. */
#define main old_mod_test_main
#include "mod_engine_test.c"
#undef main

int main(int argc,char **argv){
 assert(argc>=3);FILE *f=fopen(argv[1],"r");assert(f);static char init[32768];size_t n=fread(init,1,sizeof init-1,f);fclose(f);init[n]=0;
 while(n&&(init[n-1]=='\n'||init[n-1]=='\r'))init[--n]=0;
 int choice=atoi(argv[2]);static char out[32768];
 for(int a=3;a<argc;a++){
  Engine *e=fresh();set_param(e,"state",init);assert(e->state_valid);
  MTSequence q;if(!mt_load_sequence(argv[a],&q)){printf("ERR %s\n",q.message);done(e);continue;}
  MTPattern m;int used=choice;
  if(!mt_choice(&q,used,&m)){printf("ERR choice %d unavailable\n",choice);done(e);continue;}
  e->mti_sequence=q;mti_apply(e,&m);
  int len=get_param(e,"state",out,sizeof out);assert(len>0&&len<32700&&!strncmp(out,"PICN7 ",6));
  Engine *t=fresh();set_param(t,"state",out);assert(t->state_valid);
  for(int i=0;i<8;i++){assert(t->lane[i].f[0]==m.steps);for(int k=0;k<m.steps;k++)assert(t->lane[i].values[k][0]==(int)((m.trig[i]>>k)&1u));}
  done(t);printf("OK %d %d %s\n",m.steps,used,out);done(e);
 }
 return 0;
}

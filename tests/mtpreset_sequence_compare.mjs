// Compare the C pattern reader (src/mtpreset.h) with the FM-1 reader of f-ToniC.
// node tests/mtpreset_sequence_compare.mjs <mtpreset-sequence-test> <f-ToniC/manager/mt-preset.mjs> <fixture dir>
// Fixtures are private (Patternarium); the test is skipped when they are absent.
import {execFileSync} from 'node:child_process';
import {readFileSync,readdirSync,existsSync,writeFileSync,mkdtempSync,rmSync} from 'node:fs';
import {join,resolve} from 'node:path';
import {pathToFileURL} from 'node:url';
import {tmpdir} from 'node:os';
const [tool,reader,dir]=process.argv.slice(2);
if(!existsSync(reader)||!existsSync(dir)){console.log('SKIP lecteur FM-1 ou fichiers privés absents');process.exit(0);}
const {parseMTPreset}=await import(pathToFileURL(resolve(reader)).href);
const files=readdirSync(dir).filter(f=>f.toLowerCase().endsWith('.mtpreset')).sort();
const tmp=mkdtempSync(join(tmpdir(),'picn-mt-'));let failures=0,checked=0;
function compare(name,text){
 // The FM-1 reader refuses ModRate = inf in sounds; patterns do not depend on it.
 const js=parseMTPreset(text.replace(/=\s*[-+]?inf\b/gi,'= 20000'),name),path=join(tmp,'kit.mtpreset');writeFileSync(path,text);
 const lines=execFileSync(resolve(tool),['--dump',path],{encoding:'utf8'}).trim().split('\n'),c={};
 for(const l of lines){const [key,...rest]=l.split(' ');(c[key]??=[]).push(rest);}
 const want=[];const fail=m=>{failures++;console.log('  FAIL',name,m);};
 if(+c.division[0][0]!==js.division)fail('division');if(+c.active[0][0]!==js.selected)fail('active');if(+c.mutes[0][0]!==js.mutes)fail('mutes');
 js.patterns.forEach((p,k)=>{const row=c.pattern[k];if(row[1]!=='1'||+row[2]!==p.steps)fail(`pattern ${p.letter}`);
  p.lanes.forEach((lane,v)=>{if(row[3+v]!==`${lane.triggers}/${lane.accents}`)fail(`${p.letter}${v+1} ${row[3+v]} / ${lane.triggers}/${lane.accents}`);});});
 // Pairs: concatenation of the FM-1 masks when the total fits in 32 steps.
 for(let k=0;k<11;k++){const a=js.patterns[k],b=js.patterns[k+1],row=c.choice[k],fits=a.steps+b.steps<=32;
  if(row[1]!==(fits?'1':'0'))fail(`pair ${a.letter}+${b.letter} availability`);
  if(fits){if(+row[2]!==a.steps+b.steps)fail(`pair ${a.letter}+${b.letter} length`);
   for(let v=0;v<8;v++){const t=(a.lanes[v].triggers+b.lanes[v].triggers*2**a.steps)>>>0,x=(a.lanes[v].accents+b.lanes[v].accents*2**a.steps)>>>0;if(row[3+v]!==`${t}/${x}`)fail(`pair ${a.letter}+${b.letter} row ${v+1}`);}}}
 checked++;
}
for(const f of files){
 const text=readFileSync(join(dir,f),'utf8');
 compare(f,text);
 // Same file with other lengths (pairs fit or not), as the FM-1 test does.
 for(const length of [1,8,24,32])compare(`${f} Length=${length}`,text.replace(/Length\s*=\s*\d+/g,`Length = ${length}`));
 compare(`${f} StepRate=1/32`,text.replace(/StepRate\s*=\s*\S+/,'StepRate = 1/32'));
}
rmSync(tmp,{recursive:true});
console.log(`${failures?'FAIL':'PASS'} ${checked} lectures de motifs identiques au lecteur FM-1 (12 motifs × 8 lignes, 11 paires, StepRate, motif courant, mutes)`);
process.exit(failures?1:0);

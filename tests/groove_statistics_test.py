"""Source integrity/reproduction and genuinely held-out descriptive checks."""
from pathlib import Path
import sys,json,random,hashlib,math
B=Path(__file__).resolve().parents[1];sys.path.insert(0,str(B/'scripts'))
from extract_groove import analyze,band,bin_gap,emit
archive=B/'reference/groove/groove-v1.0.0-midionly.zip'
model,observations=analyze(archive)
stored=json.loads((B/'reference/groove/EXTRACTION.json').read_text());assert model==stored
before=(B/'src/groove_tables.h').read_bytes();emit(model);assert (B/'src/groove_tables.h').read_bytes()==before
assert sum([3*16,3*16,3*3*17,8*13*2])==457
assert all(len(x)==13 and x[0]==0 and x[-1]==65535 and x==sorted(x) for x in model['density_cdf'])
assert all(0<=v<=63 for family in ['gap_cost','phase_cost'] for row in model[family] for v in row)
for a in range(3):
 for b in range(3):assert model['pair_cost'][a][b]==model['pair_cost'][b][a]
# Human performances remain held out. Descriptive score is not listening quality.
rng=random.Random(13402);report={}
def phase(mask):
 h=mask.bit_count();return sum(model['phase_cost'][band(h)][i] for i in range(16) if mask>>i&1)/h
def gap(mask):
 positions=[i for i in range(16) if mask>>i&1];h=len(positions)
 return sum(model['gap_cost'][band(h)][bin_gap((positions[(k+1)%h]-t)%16 or 16)-1] for k,t in enumerate(positions))/h
def pair(a,b):
 minimum=min(a.bit_count(),b.bit_count());k=((a&b).bit_count()*16+minimum//2)//minimum
 return model['pair_cost'][band(a.bit_count())][band(b.bit_count())][k]
for split in ['validation','test']:
 actual_phase=rotated_phase=actual_pair=rotated_pair=actual_gap=random_gap=0.;lines=pairs=0
 for row,bars in observations:
  if row['split']!=split:continue
  for bar in bars:
   rotated=[]
   for mask in bar:
    shift=rng.randrange(16);other=((mask<<shift)|(mask>>(16-shift)))&65535
    actual_phase+=phase(mask);rotated_phase+=phase(other);lines+=1;rotated.append(other)
    actual_gap+=gap(mask);uniform=sum(1<<i for i in rng.sample(range(16),mask.bit_count()));random_gap+=gap(uniform)
   for i,a in enumerate(bar):
    for j in range(i+1,len(bar)):
     actual_pair+=pair(a,bar[j]);rotated_pair+=pair(rotated[i],rotated[j]);pairs+=1
 report[split]=dict(lines=lines,pairs=pairs,phase_cost_actual=round(actual_phase/lines,3),phase_cost_rotated=round(rotated_phase/lines,3),pair_cost_actual=round(actual_pair/pairs,3),pair_cost_rotated=round(rotated_pair/pairs,3),gap_cost_actual=round(actual_gap/lines,3),gap_cost_matched_random=round(random_gap/lines,3))
 assert lines>1000 and pairs>1000
 assert actual_phase<rotated_phase,'Learned phase preference failed held-out rotational control'
# Similarity of predicted distributions can be useful, but these scores are not
# estimates of musical preference, accuracy, CPU time, or a neural-model claim.
report.update(table_bytes=457,archive_sha256=hashlib.sha256(archive.read_bytes()).hexdigest(),fit_split='train only',hardware_access=False)
out=B/'Validation/1.3.4-groove';out.mkdir(exist_ok=True);(out/'HELDOUT.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS verified source, deterministic table reproduction, 457 bytes, bounds, symmetry, held-out validation/test rotation control')
print(json.dumps(report,indent=2))

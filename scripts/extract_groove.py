"""Reproducible GMD MIDI-only statistics. Run on PC, never imported by plugin.
Requires mido==1.3.3. Derived tables: CC BY 4.0, Google LLC/GMD authors.
Only train/beat/4-4 fits tables. Validation/test are never used for fitting.
"""
from pathlib import Path
import argparse,csv,io,zipfile,json,hashlib,math,collections
import mido
B=Path(__file__).resolve().parents[1]
EXPECTED='651cbc524ffb891be1a3e46d89dc82a1cecb09a57c748c7b45b844c4841dcc1e'
# Official GMD paper mapping; identities discarded after quantization/ranking.
MAP={36:36,38:38,40:38,37:38,48:50,50:50,45:47,47:47,43:43,58:43,46:46,26:46,42:42,22:42,44:42,49:49,55:49,57:49,52:49,51:51,59:51,53:51}
def band(h):return 0 if h<=4 else 1 if h<=8 else 2
def bin_gap(delta,n=16):return max(1,min(16,(delta*16+n//2)//n))
def masks(z,row):
 mid=mido.MidiFile(file=io.BytesIO(z.read('groove/'+row['midi_filename'])))
 if mid.ticks_per_beat<=0 or mid.type==2:raise ValueError('Unsupported MIDI time base')
 ticks=0;notes=[];bad_signature=False
 for msg in mido.merge_tracks(mid.tracks):
  ticks+=msg.time
  if msg.type=='time_signature' and (msg.numerator!=4 or msg.denominator!=4):bad_signature=True
  if msg.type=='note_on' and msg.velocity>0 and msg.note in MAP:
   # Nearest sixteenth on the continuous timeline, not independently per bar.
   q=(ticks*4+mid.ticks_per_beat//2)//mid.ticks_per_beat
   notes.append((q,MAP[msg.note]))
 if bad_signature:return []
 full_bars=ticks//(4*mid.ticks_per_beat);bars=[collections.defaultdict(int) for _ in range(full_bars)]
 for q,p in notes:
  b,s=divmod(q,16)
  if b<full_bars:bars[b][p]|=1<<s
 return [[mask for _,mask in sorted(b.items()) if 0<mask.bit_count()<=12] for b in bars if b]
def costs(values,smooth=.5):
 peak=max(values)+smooth
 return [min(63,round(-8*math.log((v+smooth)/peak))) for v in values]
def analyze(archive):
 assert hashlib.sha256(archive.read_bytes()).hexdigest()==EXPECTED,'GMD ZIP checksum mismatch'
 gaps=[[0.0]*17 for _ in range(3)];phases=[[0.0]*16 for _ in range(3)];pairs=[[[0.0]*17 for _ in range(3)] for _ in range(3)];ranks=[[0.0]*13 for _ in range(8)]
 observations=[];stats=collections.defaultdict(lambda:dict(files=0,bars=0,lines=0));style_counts=collections.Counter()
 with zipfile.ZipFile(archive) as z:
  rows=list(csv.DictReader(io.StringIO(z.read('groove/info.csv').decode())))
  selected=[r for r in rows if r['beat_type']=='beat' and r['time_signature']=='4-4']
  for row in selected:
   bars=[b for b in masks(z,row) if b]
   observations.append((row,bars))
   if row['split']=='train' and bars:style_counts[row['style'].split('/')[0]]+=1
  for row,bars in observations:
   split=row['split'];stats[split]['files']+=1;stats[split]['bars']+=len(bars);stats[split]['lines']+=sum(map(len,bars))
   if split!='train' or not bars:continue
   # Equal style mass, equal performance mass within style, equal bar mass
   # within performance. Long takes do not dominate the extracted preferences.
   weight=1000.0/(len(style_counts)*style_counts[row['style'].split('/')[0]]*len(bars))
   for bar in bars:
    ordered=sorted(bar,key=lambda m:(-m.bit_count(),m))
    for rank in range(8):ranks[rank][ordered[min(rank,len(ordered)-1)].bit_count()]+=weight
    for mask in bar:
     positions=[t for t in range(16) if mask>>t&1];h=len(positions);c=band(h)
     for k,t in enumerate(positions):
      gaps[c][bin_gap((positions[(k+1)%h]-t)%16 or 16)]+=weight/h
      phases[c][t]+=weight/h
    npairs=len(bar)*(len(bar)-1)//2
    for i,a in enumerate(bar):
     for b in bar[i+1:]:
      c,d=sorted((band(a.bit_count()),band(b.bit_count())))
      overlap=((a&b).bit_count()*16+min(a.bit_count(),b.bit_count())//2)//min(a.bit_count(),b.bit_count())
      pairs[c][d][overlap]+=weight/max(1,npairs)
 for c in range(3):
  for d in range(c):pairs[c][d]=pairs[d][c][:]
 # Cumulative density distribution over 1..12 hits. No silent or >75% rows.
 cdf=[]
 for row in ranks:
  values=[v+.5 for v in row[1:]];total=sum(values);running=0;out=[0]
  for v in values:running+=v;out.append(round(running/total*65535))
  out[-1]=65535;cdf.append(out)
 model=dict(schema=1,source='Groove MIDI Dataset v1.0.0 MIDI-only',source_sha256=EXPECTED,license='CC-BY-4.0',fit_split='train',filter='beat, 4-4, full nonempty bars; active lanes with 1..12 sixteenth positions',weighting='equal styles / equal performances per style / equal bars per performance; pairs and gaps normalized per bar/lane',quantization='nearest sixteenth on absolute tick timeline; duplicate same-family attacks merged',expression='velocities and microtiming not exported',statistics=dict(stats),training_styles=dict(sorted(style_counts.items())),gap_cost=[costs(v)[1:] for v in gaps],phase_cost=[costs(v) for v in phases],pair_cost=[[costs([sum(v[j]*(3-abs(j-k)) for j in range(max(0,k-2),min(17,k+3)))/9 for k in range(17)]) for v in row] for row in pairs],density_cdf=cdf)
 return model,observations

def emit(model):
 text=json.dumps(model,indent=2,sort_keys=True)+'\n';destination=B/'reference/groove/EXTRACTION.json';destination.write_text(text)
 header='/* Generated by scripts/extract_groove.py. DO NOT EDIT.\n * Derived from Groove MIDI Dataset v1.0.0, Google LLC and GMD contributors.\n * CC BY 4.0: https://creativecommons.org/licenses/by/4.0/\n * Source ZIP SHA-256: '+EXPECTED+'\n * Changes: quantization, pooling, weighted statistics and compact integer costs.\n * See licenses/GROOVE-ATTRIBUTION.md. No raw performances in runtime.\n */\n#pragma once\n'
 def array(name,ctype,data,dims):
  def format(v):return '{'+','.join(format(x) if isinstance(x,list) else str(x) for x in v)+'}'
  return 'static const '+ctype+' '+name+''.join('['+str(n)+']' for n in dims)+'='+format(data)+';\n'
 header+=array('groove_gap_cost','unsigned char',model['gap_cost'],[3,16])
 header+=array('groove_phase_cost','unsigned char',model['phase_cost'],[3,16])
 header+=array('groove_pair_cost','unsigned char',model['pair_cost'],[3,3,17])
 header+=array('groove_density_cdf','unsigned short',model['density_cdf'],[8,13])
 header+='#define GROOVE_TABLE_BYTES (sizeof groove_gap_cost + sizeof groove_phase_cost + sizeof groove_pair_cost + sizeof groove_density_cdf)\n'
 (B/'src/groove_tables.h').write_text(header)
 return dict(header_sha256=hashlib.sha256(header.encode()).hexdigest(),tables_bytes=3*16+3*16+3*3*17+8*13*2)
def main():
 p=argparse.ArgumentParser();p.add_argument('--archive',type=Path,default=B/'reference/groove/groove-v1.0.0-midionly.zip');args=p.parse_args()
 model,_=analyze(args.archive);result=emit(model);print(json.dumps(dict(**result,statistics=model['statistics'],training_styles=model['training_styles']),indent=2))
if __name__=='__main__':main()

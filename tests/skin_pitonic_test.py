"""Skin-only regression: preserve controls, indices, Q-Links and shared caches."""
from pathlib import Path
import json,hashlib,sys,collections,re
from PIL import Image
B=Path(__file__).resolve().parents[1];folder='Timo - VST - πClideaN MPC';old=B/'build/skin'/folder/'Plugin Skins';new=B/'build/skin-pitonic'/folder/'Plugin Skins'
before=json.loads((B/'Validation/skin-pitonic/before.json').read_text())
for path,digest in before.items():assert hashlib.sha256((B/path).read_bytes()).hexdigest()==digest,path
for p in old.glob('Q-Links*.json'):
 assert json.loads((new/p.name).read_text())['Screen Mode Q-Links']==json.loads(p.read_text())['Screen Mode Q-Links'],p.name
params=json.loads((B/'params.json').read_text());indices={p['key']:i for i,p in enumerate(params)}
physical=[13,9,5,1,14,10,6,2]
expected=[[f'q{i}' for i in range(8)],['mod0_depth','mod0_offset','mod0_phase','mod0_threshold','mod1_depth','mod1_offset','mod1_phase','mod1_threshold'],['line_select16','play_mode','auto_rotate','auto_shift','auto_every'],['line_select16','play_mode','auto_rotate','auto_shift','auto_every'],[]]
maps=[]
for filename in ('Q-Links.json','Q-Links - 8by1.json','Q-Links - 4by4.json'):
 q=json.loads((new/filename).read_text());rows=q['Screen Mode Q-Links']['map'];assert len(rows)==5
 for tab,(row,keys) in enumerate(zip(rows,expected),1):
  target={f'Q-Link {i}':-1 for i in range(1,17)};target.update({f'Q-Link {slot}':indices[key] for slot,key in zip(physical,keys)})
  assert row['Tab']==tab and row['SubTab']==1 and row['Q-Links']==target,(filename,tab)
 assert q['Program Mode Q-Links']==rows[0]['Q-Links']
 maps.append(rows)
assert maps[0]==maps[1]==maps[2]
j=json.loads((new/'TUI.json').read_text())['pageData'];o=json.loads((old/'TUI.json').read_text())['pageData'];defs={d['key']:d['value'] for d in j['componentDefinitions']['localComponentDefinitions']};olddefs={d['key']:d['value'] for d in o['componentDefinitions']['localComponentDefinitions']}
assert [t['tabName'] for t in j['tabs']]==[t['tabName'] for t in o['tabs']]
def controls(d):
 rows=[]
 for name,v in d.items():
  if v['actions']:rows.append((re.sub(r'_(?:394|410)(?=_|$)','_ACTIONY',name),json.dumps(v['actions'],sort_keys=True),v['ignoreMousePresses'],v['disableCoarseDataWheel']))
 return sorted(rows)
assert controls(defs)==controls(olddefs),'Native gesture/focus ownership changed'
for n in range(8):
 for view in (0,1):
  d=defs[f'knob_v{view}_{n}'];assert bool(d['actions'])==(view==0 or n>=3)
  for c in d['componentsData']:
   kind=c['componentData']['type'];data=c['componentData']['data']
   if kind=='Knob':assert c['bounds']['bounds']=='12 32 58 58' and data['numFrames']==127
   if kind=='Label':assert c['bounds']['bounds']=='86 43 210 42' and data['textStyle']['font']['height']==16
  group=next(c for c in defs['PATTERN']['componentsData'] if c['componentData']['type']==f'knob_v{view}_{n}')
  x,y,w,h=map(int,group['bounds']['bounds'].split());assert (x,y,w,h)==(20+n%4*312,132+n//4*144,304,122)
  assert y+h<=400
assert not (new/'scopes.png').exists(),'Outer frame must be removed'
for n in range(8):
 d=defs[f'rand_target{n}'];knobs=[c for c in d['componentsData'] if c['componentData']['type']=='Knob'];assert all(c['bounds']['bounds']=='12 32 58 58' for c in knobs)
for name,v in defs.items():
 for c in v['componentsData']:
  data=c['componentData']['data']
  for key in ('image','onImage','offImage','filmStrip'):
   if data.get(key):assert (new/data[key]).is_file(),data[key]
  if c['componentData']['type']=='Label':assert data['textStyle']['font']['name']=='Titillium Web'
assert Image.open(new/'track_states_128.png').size==(12,22*128)
assert Image.open(new/'mod_segment_atlas.png').size==(4,68*3072)
print('PASS unchanged engine/parameters/native actions; exact Screen mappings, three Q-Link layouts and explicit Program aliases; five tabs, piToniC card geometry, 16px values, lock/RAND strips, no outer frame, shared track/MOD atlases')

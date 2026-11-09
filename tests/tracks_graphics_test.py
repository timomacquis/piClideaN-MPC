"""Compare the optimized native TRACKS pages with the saved candidate.8 skin."""
from pathlib import Path
import hashlib, json, tempfile, zipfile, sys, math
from PIL import Image, ImageChops
B=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(B/'scripts'))
from project_config import FOLDER
baseline=B/'Validation/Before-tracks-performance-c8.zip'
A=B/'build/skin'/FOLDER/'Plugin Skins'
new=json.loads((A/'TUI.json').read_text())['pageData']
with zipfile.ZipFile(baseline) as z, tempfile.TemporaryDirectory() as temp:
 old_a=Path(temp)
 prefix='build/skin/'+FOLDER+'/Plugin Skins/'
 for name in z.namelist():
  if name.startswith(prefix):
   p=old_a/name[len(prefix):];p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(z.read(name))
 old=json.loads((old_a/'TUI.json').read_text())['pageData']
 old_defs={d['key']:d['value'] for d in old['componentDefinitions']['localComponentDefinitions']}
 new_defs={d['key']:d['value'] for d in new['componentDefinitions']['localComponentDefinitions']}
 source=(B/'scripts/preview_skin.py').read_text().split('pages=[dict(p)')[0]
 env={'__file__':str(B/'scripts/preview_skin.py')};exec(compile(source,'preview_skin.py','exec'),env)
 V,K=env['V'],env['K']
 # Parameter identity and engine source must not change for this skin-only work.
 assert (B/'params.lock').read_bytes()==z.read('params.lock')
 for name in z.namelist():
  if name.startswith('src/') and not name.endswith('/'):
   assert (B/name).read_bytes()==z.read(name),name
 report=[];cases=0
 for page,start in [('01 - 08',0),('09 - 16',8)]:
  before,after=old_defs[page]['componentsData'],new_defs[page]['componentsData']
  tiles=[c for c in after if c['componentData']['name'].startswith('trackcell')]
  assert len(tiles)==256
  for c in tiles:
   d=new_defs[c['componentData']['type']]
   assert d['ignoreMousePresses'] and not d['actions']
   assert c['bounds']['acceptsHWFocus']=='No'
   assert len(d['componentsData'])==1
   if c is not tiles[0]:continue
   knob=d['componentsData'][0]['componentData']['data']
   assert knob['numFrames']==128
   strip=Image.open(A/knob['filmStrip']).convert('RGB');assert strip.size==(12,22*128)
   # Native frame-count geometry and both normalized rounding choices must
   # resolve to a complete tile, never a slice from its neighbour.
   for state in range(9):
    original=next(c for c in before if c['componentData']['type']=='Image' and any(g.startswith(f'IndexedEnabling/{state}/9/') for g in c['bounds']['additionalInvalidatingHandles']))
    target=Image.open(old_a/original['componentData']['data']['image']).convert('RGB')
    for frame in {math.floor(state*127/8),round(state*127/8),math.ceil(state*127/8)}:
     tile=strip.crop((0,22*frame,12,22*(frame+1)))
     assert ImageChops.difference(tile,target).getbbox() is None,(state,frame)
  # Every musical control and its tactile/keyboard action remains identical.
  def behavioral(c,defs):
   kind=c['componentData']['type']
   if kind not in defs:return (kind,c.get('handle remapping'))
   d=defs[kind]
   return (c['bounds']['bounds'],c.get('handle remapping'),d['actions'],d['ignoreMousePresses'],d['disableCoarseDataWheel'])
  for key in [f'edit_line{i}' for i in range(start,start+8)]+[f'l{i}_{k}' for i in range(start,start+8) for k in ('mute','solo')]+['open_bank','auto_rotate','auto_shift','auto_every','reset_phase']:
   a=next(c for c in before if c['componentData']['name']==key)
   b=next(c for c in after if c['componentData']['name']==key)
   assert behavioral(a,old_defs)==behavioral(b,new_defs),key
  for case in range(25):
   selected=case%16
   for i in range(16):
    V[K[f'line_on{i}']]=int(i==selected)
    V[K[f'l{i}_mute']]=case%2;V[K[f'l{i}_solo']]=(case//2)%2
    for j in range(32):V[K[f'trackcell{i}_{j}']]=(case if case<9 else (i+j+case)%9)
   for key in ('auto_shift','auto_every'):V[K[key]]=env['P'][K[key]]['min'] if case%2 else env['P'][K[key]]['max']
   V[K['auto_rotate']]=case%2
   renders=[]
   for assets,defs,children in [(old_a,old_defs,before),(A,new_defs,after)]:
    env['A']=assets;env['D']=defs
    canvas=Image.new('RGB',(1280,628),'#171a1e')
    for c in children:env['draw_control'](canvas,c)
    renders.append(canvas)
   assert ImageChops.difference(*renders).getbbox() is None,(page,case)
   cases+=1
  report.append({'page':page,'before_top_level_components':len(before),'after_top_level_components':len(after),'before_step_images':256*9,'after_step_components':256})
 result={'passed':True,'device_tested':False,'pixel_identical_page_cases':cases,'engine_sources_unchanged':True,'parameter_indices_unchanged':True,'touch_and_qlink_actions_preserved':True,'pages':report}
 (B/'Validation/TRACKS-GRAPHICS-TESTS.json').write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps(result,indent=2))

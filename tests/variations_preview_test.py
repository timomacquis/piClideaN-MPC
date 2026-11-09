"""Guard the native short-strip regression visible on Force 2026-11-09.
Checks each complete frame at floor/nearest/ceil host normalization, preserving
active/custom/selection/playhead/unused colours. No device qualification claim.
"""
from pathlib import Path
import json,math,os
from PIL import Image
B=Path(__file__).resolve().parents[1]
A=B/os.environ.get('PICN_PREVIEW_TEST_SKIN','build/label-centering/skin/Timo - VST - πClideaN/Plugin Skins')
page=json.loads((A/'TUI.json').read_text())['pageData'];defs={x['key']:x['value'] for x in page['componentDefinitions']['localComponentDefinitions']}
checks=0
for i in range(32):
 d=defs[f'variation_preview{i}'];assert not d.get('actions') and len(d['componentsData'])==1
 knob=d['componentsData'][0];assert knob['bounds']['acceptsHWFocus']=='No';data=knob['componentData']['data'];assert data['numFrames']==128
 strip=Image.open(A/data['filmStrip']).convert('RGB');assert strip.size==(40,32*128)
 for state in range(18):
  source=defs[f'pad_touch{i}']['componentsData'][state]['componentData']['data']['image'];expected=Image.open(A/source).convert('RGB').resize((40,32),Image.Resampling.LANCZOS)
  for index in {math.floor(state*127/17),round(state*127/17),math.ceil(state*127/17)}:
   actual=strip.crop((0,index*32,40,(index+1)*32));assert actual.tobytes()==expected.tobytes(),(i,state,index);checks+=1
 # The unused state must be the grey original tile, never a purple lock state.
 unused=strip.crop((0,round(16*127/17)*32,40,(round(16*127/17)+1)*32));assert unused.getpixel((4,16))!=tuple(bytes.fromhex('432449'))
placed=[]
for definition in defs.values():
 for c in definition.get('componentsData',[]):
  if c['componentData']['type'].startswith('variation_preview'):placed.append(c)
assert len(placed)==32
for c in placed:
 i=int(c['componentData']['type'].removeprefix('variation_preview'));assert c['bounds']['bounds']==f'{220+(i%16)*52} {192+(i//16)*44} 40 32'
message=defs['read_can_undo_220_436']['componentsData'][0];assert message['componentData']['data']['textStyle']['font']['height']==22
print(f'PASS {checks} complete-frame state checks; both rows preserve active/custom/head/unused colours; 32 passive owners; two non-overlapping rows; status text 22. Hardware rendering remains pending.')

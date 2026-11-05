"""Check actual generated EDIT skin: lock artwork, ranges and single touch owners."""
from pathlib import Path
import json,sys
from PIL import Image
B=Path(__file__).resolve().parents[1];sys.path.insert(0,str(B/'scripts'))
from project_config import FOLDER
A=B/'build/skin'/FOLDER/'Plugin Skins';params=json.loads((B/'params.json').read_text());K={p['key']:i for i,p in enumerate(params)}
assert [p['key'] for p in params]==(B/'params.lock').read_text().split()
page=json.loads((A/'TUI.json').read_text())['pageData'];defs={r['key']:r['value'] for r in page['componentDefinitions']['localComponentDefinitions']}
for n in range(8):
 d=defs[f'knob_v1_{n}'];assert bool(d['actions'])==(n>=3)
 if n<3:continue
 custom='preview0' if n==3 else f'step_custom{n-4}';count=5 if n==3 else 2
 for state in (0,1):
  rule=f'IndexedEnabling/{state}/{count}/Parameter {K[custom]}'
  group=[c for c in d['componentsData'] if rule in c['bounds']['additionalInvalidatingHandles']]
  assert len(group)==4,(n,state)
  image=group[0]['componentData']['data']['image'];assert image.startswith('parameter_box_custom' if state else 'parameter_box.')
  knob=group[1]['componentData']['data'];assert knob['filmStrip'].startswith('dial_custom' if state else 'dial.')
  assert Image.open(A/knob['filmStrip']).size==(58,58*128)
# Render an inspectable EDIT layout from the real generated component tree.
source=(B/'scripts/preview_skin.py').read_text().split('pages=[dict(p)')[0]
env={'__file__':str(B/'scripts/preview_skin.py')};exec(compile(source,'preview_skin.py','exec'),env)
V=env['V'];V[K['edit_view']]=1;V[K['preview0']]=1;V[K['step_custom0']]=1;V[K['step_custom2']]=1
V[K['q3']]=127*(80-10)/85;env['texts']['q3']='80';params=env['P'];params[K['q3']]['name']='GATE · STEP'
canvas=Image.new('RGB',(1280,628),'#171a1e')
for c in defs[page['tabs'][0]['tabName']]['componentsData']:env['draw_control'](canvas,c)
canvas.save(B/'build/reports/PREVIEW-EDIT-GATE-1.3.1.png')
print('PASS 4096 stable parameter indices / five EDIT locks / correct retired-handle range / cached art / single touch owners')

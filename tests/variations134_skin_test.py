from pathlib import Path
import json,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from project_config import FOLDER
B=Path(__file__).resolve().parents[1]
p=json.loads((B/'params.json').read_text());keys=[x['key'] for x in p];old=(B/'Validation/1.3.4/baseline/params.lock').read_text().splitlines();assert len(keys)==4096 and keys==old
assert p[keys.index('undo')]['momentary'] is True
assert not p[keys.index('redo')].get('options') and not p[keys.index('rhythm_choice')].get('options')
a=B/'build/skin'/FOLDER/'Plugin Skins';j=json.loads((a/'TUI.json').read_text())['pageData'];defs={d['key']:d['value'] for d in j['componentDefinitions']['localComponentDefinitions']}
assert len(j['tabs'])==5
for n in ['Q-Links.json','Q-Links - 8by1.json','Q-Links - 4by4.json']:assert (a/n).read_bytes()==(B/'Validation/1.3.4/baseline/Q-Links-4by4.json').read_bytes()
assert (a/'dialog7.png').exists() and len(list(a.glob('variation_step_atlas*.png')))==32
assert not list(a.glob('variation_step*_state*.png'))
rand=defs['rand_two_stage']
assert not any(c['componentData']['type']=='Button' for c in rand['componentsData'])
assert 'Toggle Switch' in str(rand)

line_actions={c['componentData']['name']:c for c in defs['PATTERN']['componentsData'] if c['componentData']['name'] in ('half','double','undo','open_target','randomize') and c['bounds']['bounds'].split()[1]=='410'}
for key,box in {'half':'644 410 70 46','double':'722 410 70 46','undo':'800 410 148 46','open_target':'956 410 148 46','randomize':'1112 410 148 46'}.items():
 assert line_actions[key]['bounds']['bounds']==box
 control=defs[line_actions[key]['componentData']['type']]
 assert not any(c['componentData']['type']=='Button' for c in control['componentsData'])
 assert any(a['handler']=='Toggle Switch' and a['onAction']=='Mouse Down' for a in control['actions'])
def walk(c):
 for h in c['bounds'].get('additionalInvalidatingHandles',[]):
  if h.endswith('Parameter '+str(keys.index('dialog'))):assert '/8/' in h,h
 kind=c['componentData']['type']
 if kind in defs:
  for z in defs[kind]['componentsData']:walk(z)
for t in j['tabs']:
 for c in defs[t['componentName']]['componentsData']:walk(c)
print('PASS exact 4096 indices, reused controls, five pages, unchanged physical Q-Links, cached 32-preview atlases, eight-state dialog gates and single-owner RAND touch action')

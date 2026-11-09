"""Compile original factory states into a bounded table; no runtime file I/O."""
from pathlib import Path
import json,hashlib
B=Path(__file__).resolve().parents[1]
def generate():
 p=B/'presets/factory.json';bank=json.loads(p.read_text());rows=bank['programs'];assert bank['schema']==1 and 1<=len(rows)<=128
 assert rows[0]['name']=='INIT' and len({r['name'] for r in rows})==len(rows)
 out=['/* Generated from presets/factory.json; do not edit. */','#pragma once','typedef struct { const char *name, *state; } picn_factory_program_t;',f'#define PICN_FACTORY_COUNT {len(rows)}','static const picn_factory_program_t PICN_FACTORY[]={']
 for r in rows:
  assert r['name'].isascii() and 0<len(r['name'])<24
  assert r['state'].startswith('PICN7 ') and r['state'].endswith('END') and '\0' not in r['state'] and len(r['state'])<32700
  out.append('{'+json.dumps(r['name'])+','+json.dumps(r['state'])+'},')
 out.append('};');(B/'build').mkdir(exist_ok=True);(B/'build/picn_factory_programs.h').write_text('\n'.join(out)+'\n')
 return len(rows)
if __name__=='__main__':print('Generated',generate(),'native factory programs')

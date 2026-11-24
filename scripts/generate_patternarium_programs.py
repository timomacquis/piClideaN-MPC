"""πClideaN 1.4.1 : programmes d'usine = INIT + les 150 kits Patternarium BEST de πToniC, même ordre, mêmes noms.

Chaque programme part de l'état INIT et reçoit le motif A+B (choix 12 : jusqu'à 32 pas) de la source
.mtpreset, importé exactement comme le bouton IMPORT .MTPRESET (lignes 1-8 ; lignes 9-16 = INIT).
Sources : ../πToniC/Private/PatternariumBest150-sources (MANIFEST.json). Ancienne banque gardée
dans presets/factory-1.4.0.json.
Usage : python3 scripts/generate_patternarium_programs.py
"""
import json,hashlib,subprocess,tempfile,os
from pathlib import Path
B=Path(__file__).resolve().parents[1];SRC=B.parent/'πToniC/Private/PatternariumBest150-sources'
factory=B/'presets/factory.json';old=B/'presets/factory-1.4.0.json'
if not old.exists():old.write_text(factory.read_text())
base=json.loads(old.read_text());init=base['programs'][0];assert init['name']=='INIT'
manifest=json.loads((SRC/'MANIFEST.json').read_text());kits=manifest['kits'];assert len(kits)==150
for k in kits:assert hashlib.sha256((SRC/k['mtpreset']).read_bytes()).hexdigest()==k['sha256'],k['mtpreset']
tool=B/'build/mtpreset_factory_tool'
subprocess.run(['cc','-std=c11','-D_POSIX_C_SOURCE=200809L','-D_GNU_SOURCE','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-Ibuild','-Ithird_party/mpc-vst-wrapper/wrapper','-Ithird_party/alsa-dev/usr/include','tests/mtpreset_factory_tool.c','-l:libasound.so.2','-lm','-lpthread','-ldl','-o',str(tool)],cwd=B,check=True)
with tempfile.NamedTemporaryFile('w',suffix='.state',delete=False) as t:t.write(init['state']);state_file=t.name
try:
 out=subprocess.run([str(tool),state_file,'12',*[str(SRC/k['mtpreset']) for k in kits]],capture_output=True,text=True,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1')).stdout.splitlines()
finally:os.unlink(state_file)
assert len(out)==150
rows=[init];steps={}
for k,line in zip(kits,out):
 assert line.startswith('OK '),(k['mtpreset'],line)
 _,n,choice,state=line.split(' ',3);assert choice=='12'
 steps[int(n)]=steps.get(int(n),0)+1
 rows.append({'name':k['name'],'state':state,'source':{'order':k['order'],'mtpreset':k['mtpreset'],'sha256':k['sha256'],'pattern':'A+B','steps':int(n)}})
assert len({r['name'] for r in rows})==151
bank=dict(base);bank['description']='INIT, puis les 150 kits Patternarium BEST de πToniC dans le même ordre : motif A+B (jusqu\'à 32 pas) sur les lignes 1-8. Sources Sonic Charge / Patternarium, usage privé.'
bank['programs']=rows;factory.write_text(json.dumps(bank,ensure_ascii=False,indent=1)+'\n')
print('151 programmes : INIT + 150 kits ; pas A+B :',dict(sorted(steps.items())))

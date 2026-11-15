"""Exercise the ARM32 ZIP only against disposable local settings/directories."""
from pathlib import Path
import tempfile,zipfile,json,subprocess,os,hashlib,xml.etree.ElementTree as ET
B=Path(__file__).resolve().parents[1];arch=__import__('sys').argv[1] if len(__import__('sys').argv)>1 else 'armv7';assert arch in ('armv7','aarch64');z=B/('Installation/ClideaN-MPC-1.3.7-mpc-'+arch+'.zip')
with tempfile.TemporaryDirectory(prefix='picn-package-',dir='/home/greg/forcework') as tmp:
 T=Path(tmp);extract=T/'package'
 with zipfile.ZipFile(z) as f:f.extractall(extract)
 top=next(extract.iterdir());portable=top/'portable';payload=next(portable.iterdir());pj=json.loads((payload/'plugin.json').read_text())
 assert pj['name']=='πClideaN' and pj['uid']=='0x5069434e' and pj['format']=='vst2'
 filekey='file32' if arch=='armv7' else 'file64';assert pj[filekey]=='piclidean_mpc.so' and ('file64' if arch=='armv7' else 'file32') not in pj
 assert (payload/pj[filekey]).read_bytes()[:5]==(b'\x7fELF\x01' if arch=='armv7' else b'\x7fELF\x02')
 for directory in ['Synths','VST2 Plugins']:
  target=T/directory;settings=T/(directory.replace(' ','-')+'.settings')
  settings.write_text('''<?xml version="1.0" encoding="UTF-8"?>
<PROPERTIES>
 <VALUE name="SynthContentLocations" val="/sdcard/Synths"/>
 <VALUE name="pluginList-arm">
  <KNOWNPLUGINS>
   <PLUGIN name="Other" format="VST" manufacturer="x" version="1.0" file="/sdcard/vst/other.so" uid="6f746872" isInstrument="1"/>
   <PLUGIN name="πClideaN MPC" format="VST" manufacturer="Timo" version="1.3.6" file="/sdcard/Synths/Old/piclidean_mpc.so" uid="5069434e" isInstrument="1"/>
  </KNOWNPLUGINS>
 </VALUE>
</PROPERTIES>'''.replace('pluginList-arm','pluginList-arm-64bit' if arch=='aarch64' else 'pluginList-arm'))
  env=dict(os.environ,MPC_INSTALL_TEST='1',MPC_SETTINGS=str(settings),MPC_LEGACY_ROOT=str(T/'legacy'),MPC_TEST_LOG=str(T/'control.log'))
  def run(script):
   r=subprocess.run(['sh',str(top/script),'-y','-n','-t',str(target)],cwd=top,env=env,text=True,capture_output=True)
   assert not r.returncode,r.stdout+r.stderr
  run('install.sh');run('install.sh')
  found=list(ET.parse(settings).getroot().iter('PLUGIN'));ours=[p for p in found if p.get('uid')=='5069434e']
  assert len(ours)==1 and ours[0].get('file')==str(target/payload.name/'piclidean_mpc.so')
  assert len([p for p in found if p.get('name')=='Other'])==1
  assert (target/payload.name/'plugin.json').read_bytes()==(payload/'plugin.json').read_bytes()
  run('uninstall.sh');found=list(ET.parse(settings).getroot().iter('PLUGIN'))
  assert not [p for p in found if p.get('uid')=='5069434e'] and len(found)==1
  assert not (target/payload.name/'piclidean_mpc.so').exists()
  print('PASS isolated install/reinstall/uninstall:',directory,'; neighbouring entry retained')
report={'passed':True,'hardware_access':False,'architecture':arch,'package':z.name,'sha256':hashlib.sha256(z.read_bytes()).hexdigest(),'checks':['official plugin.json + PiCN identity','ELF architecture','isolated Synths and VST2 Plugins install/reinstall/uninstall','neighbour preserved']}
(B/('Validation/PACKAGE-1.3.7-'+arch+'.json')).write_text(json.dumps(report,indent=2)+'\n')
print('PASS package identity, architecture and isolated installation')

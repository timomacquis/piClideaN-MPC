"""Offline standard installer simulation. No SSH and no real service calls."""
from pathlib import Path
import os,tempfile,zipfile,json,hashlib,subprocess,xml.etree.ElementTree as ET,sys

def check(package):
 with tempfile.TemporaryDirectory(prefix='picn-standard-install-') as td:
  t=Path(td)
  with zipfile.ZipFile(package) as z:
   assert z.testzip() is None;names=z.namelist();prefix=names[0].split('/')[0];z.extractall(t/'package')
  root=t/'package'/prefix;m=json.loads((root/'mpc-plugin.json').read_text());arch=m['arch'];key='pluginList-arm-64bit' if arch=='aarch64' else 'pluginList-arm';folder='Timo - VST - πClideaN MPC'
  original=(root/'portable'/folder/'piclidean_mpc.so').read_bytes();synths=t/'Synths';synths.mkdir();settings=t/'MPC.settings';calls=t/'calls'
  settings.write_text('<PROPERTIES><VALUE name="SynthContentLocations"><Locations><Location>'+str(synths)+'</Location></Locations></VALUE><VALUE name="pluginList-arm"><KNOWNPLUGINS><PLUGIN name="Keep Gen1" file="/keep.so" uid="12345678" /></KNOWNPLUGINS></VALUE><VALUE name="pluginList-arm-64bit"><KNOWNPLUGINS><PLUGIN name="Keep Gen2" file="/keep64.so" uid="87654321" /></KNOWNPLUGINS></VALUE></PROPERTIES>'.replace('><','>\n<'))
  env=dict(os.environ,MPC_INSTALL_TEST='1',MPC_SETTINGS=str(settings),MPC_TEST_LOG=str(calls),MPC_LEGACY_ROOT=str(t/'legacy'))
  for operation in ['install.sh','install.sh','uninstall.sh']:
   r=subprocess.run(['sh',str(root/operation),'-y','-t',str(synths)],env=env,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stdout+r.stderr
   xml=ET.parse(settings);allrows=xml.findall('.//PLUGIN');assert len([p for p in allrows if p.get('uid') in ('12345678','87654321')])==2
   ours=[p for p in allrows if p.get('uid')=='5069434e']
   if operation=='install.sh':
    assert len(ours)==1;assert len(xml.findall('.//VALUE[@name="'+key+'"]//PLUGIN[@uid="5069434e"]'))==1
    assert (synths/folder/'piclidean_mpc.so').read_bytes()==original
    assert ours[0].get('file')==str(synths/folder/'piclidean_mpc.so')
   else:assert not ours and not (synths/folder).exists()
  assert calls.read_text().splitlines()==['stop','start']*3
  print('PASS',arch,'standard install / reinstall / uninstall, correct registration key, unrelated plugins retained; no real device/service touched')
if __name__=='__main__':
 for p in sys.argv[1:]:check(Path(p).resolve())

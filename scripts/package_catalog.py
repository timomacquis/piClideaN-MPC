"""One official packaging path for both device generations; never accesses a device."""
from pathlib import Path
import json,hashlib,subprocess,sys,re,xml.etree.ElementTree as ET
from project_config import VERSION,DISPLAY_NAME,FOLDER
from provenance import inputs,tree
B=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def package(arch):
 runtime_version=VERSION.split('-',1)[0]
 assert arch in ('armv7','aarch64')
 report=B/('Validation/BUILD.json' if arch=='armv7' else 'build/reports/BUILD-aarch64.json');build=json.loads(report.read_text())
 if build['version']!=VERSION or build['input_hashes']!=inputs():raise SystemExit('Source inputs changed; rebuild this architecture first.')
 root=B/('build' if arch=='armv7' else 'build/aarch64');so=root/'piclidean_mpc.so';skin=root/'skin'/FOLDER
 assert sha(so)==build['engine_sha256'] and tree(skin)==build['skin_output_hashes']
 tools=B/'third_party/open-mpc-release';meta=json.loads((tools/'UPSTREAM.json').read_text())
 for n,h in meta['files'].items():assert sha(tools/n)==h,'Pinned release tool changed: '+n
 entry=root/'catalog-plugin-entry.xml';e=ET.Element('PLUGIN',name=DISPLAY_NAME,descriptiveName=DISPLAY_NAME,format='VST',category='Synth',manufacturer='Timo',version=runtime_version,file='%payload-path%/'+FOLDER+'/piclidean_mpc.so',uid='5069434e',isInstrument='1',numInputs='0',numOutputs='2');entry.write_text(ET.tostring(e,encoding='unicode'))
 command=[sys.executable,str(tools/'tools/release.py'),'--so',str(so),'--skin',str(skin),'--entry',str(entry),'--version',runtime_version,'--id','piclidean-mpc','--repo','timomacquis/piClideaN16-MPC','--license','MIT','--about','Sixteen-lane hybrid MIDI sequencer.','-o',str(B/'Installation')]
 bench=B/'docs/bench'/('force-'+VERSION+'.json');bench_report=bench.with_name('force-'+VERSION+'-report.json')
 if arch=='armv7' and bench.exists():
  checked=json.loads(bench_report.read_text());assert checked['version']==VERSION and checked['engine_sha256']==build['engine_sha256'],'CPU measurement does not match this engine'
  command += ['--bench',str(bench)]
 subprocess.run(command,check=True)
 z=B/'Installation'/(re.sub(r'[^A-Za-z0-9]+','-',DISPLAY_NAME).strip('-')+'-'+runtime_version+'-mpc-'+arch+'.zip')
 attach_docs(z,{'version':VERSION,'tools_repo':'open-mpc/mpc-vst-plugins','tools_commit':meta['commit'],'architecture':arch,'engine_sha256':build['engine_sha256'],'build_input_hashes':build['input_hashes'],'hardware_access':False})
 subprocess.run([sys.executable,str(tools/'tools/catalog_check.py'),str(z),'--catalog','--require-portable','--expect-id','piclidean-mpc','--expect-repo','timomacquis/piClideaN16-MPC'],check=True)
 if VERSION!=runtime_version:
  candidate=z.with_name(z.name.replace('-'+runtime_version+'-mpc-', '-'+VERSION+'-mpc-'))
  z.replace(candidate);z=candidate
 if VERSION==runtime_version:
  catalog_name=z.with_name('ClideaN-MPC-'+VERSION+'-mpc-'+arch+'.zip')
  z.replace(catalog_name);z=catalog_name
 z.with_suffix('.zip.sha256').write_text(sha(z)+'  '+z.name+'\n')
 print('Standard catalog package:',z)

def attach_docs(z,origin):
 """Add documentation/provenance, preserving the official scripts and runtime bytes."""
 import zipfile,os
 with zipfile.ZipFile(z) as src:
  top=src.namelist()[0].split('/')[0];rows={n[len(top)+1:]:(src.getinfo(n),src.read(n)) for n in src.namelist() if not n.endswith('/')}
 additions={}
 for name in ['README.md','LICENSE','THIRD_PARTY.md','Cover.png','Cover.JPG']:additions[name]=((B/'docs/PACKAGE-README.md') if name=='README.md' and (B/'docs/PACKAGE-README.md').exists() else (B/name)).read_bytes()
 for directory in ['docs','licenses']:
  for p in (B/directory).rglob('*'):
   if p.is_file() and p.name!='TOOLCHAIN.json' and not (directory=='docs' and p.relative_to(B/directory).parts[0] not in ('images','USER-MANUAL.md','MODE-EMPLOI.md','INSTALLATION.md','CANDIDATE-1.3.4.md','NATIVE-PRESETS.md','SKIN-PITONIC.md','RELEASE-NOTES.md','RELEASE-NOTES-1.3.5.md','PACKAGE-README.md','GROOVE-STATISTICS.md')):additions[p.relative_to(B).as_posix()]=p.read_bytes()
 additions['PROVENANCE.json']=(json.dumps(origin,ensure_ascii=False,indent=2)+'\n').encode()
 for name,data in additions.items():
  info=zipfile.ZipInfo(top+'/'+name,(2026,11,5,0,0,0));info.create_system=3;info.external_attr=0o100644<<16;info.compress_type=zipfile.ZIP_DEFLATED;rows[name]=(info,data)
 sums=''.join(hashlib.sha256(data).hexdigest()+'  '+name+'\n' for name,(info,data) in sorted(rows.items()) if name!='SHA256SUMS').encode();info=rows['SHA256SUMS'][0];rows['SHA256SUMS']=(info,sums)
 temp=z.with_suffix('.zip.partial')
 with zipfile.ZipFile(temp,'w',zipfile.ZIP_DEFLATED) as dst:
  for name,(info,data) in sorted(rows.items()):dst.writestr(info,data)
 with zipfile.ZipFile(temp) as dst:assert dst.testzip() is None
 os.replace(temp,z)

"""Local aarch64 (Gen2) build in an arm64 container. Never accesses or restarts a device."""
from pathlib import Path
import os,subprocess,sys,json,re,hashlib,struct
from project_config import VERSION,FOLDER
from provenance import inputs,tree,timestamp,build_date
B=Path(__file__).resolve().parents[1];tools=B/'third_party/mpc-vst-wrapper/tools'
if os.environ.get('GEN_VST_RELOCK'):raise SystemExit('Forced parameter relock is forbidden in this build')
subprocess.run([sys.executable,str(B/'scripts/generate.py')],check=True)
subprocess.run([sys.executable,str(B/'scripts/prepare_wrapper.py')],check=True)
sys.path.insert(0,str(tools));import gen_vst
params=json.loads((B/'params.json').read_text());keys=[p['key'] for p in params];lock=B/'params.lock'
if lock.exists():
 old=lock.read_text().split();assert len(keys)>=len(old) and keys[:len(old)]==old,'Published parameter indices changed'
gen_vst.check_lock(str(B),params)
(B/'build').mkdir(exist_ok=True);cfg=gen_vst.load(str(B/'vst.json'));gen_vst.gen_params(cfg,params,str(B/'build/params.h'))
auto=[int(bool(re.fullmatch(r'l(?:[0-9]|1[0-5])_(?:[a-z]+|s[0-9]+_[a-z]+|mod_mix|mod[01]_(?:[a-z]+|c[0-7]))',p['key'])) or p['key'] in ('channel','bank','target_mode')) for p in params]
with (B/'build/params.h').open('a') as f:
 f.write('\nstatic const unsigned char PICN_AUTOMATABLE[NPARAMS]={'+','.join(map(str,auto))+'};\n')
 f.write('static const unsigned short PICN_PARAM_LOOKUP[NPARAMS]={'+','.join(map(str,sorted(range(len(keys)),key=lambda i:keys[i])))+'};\n')
IMAGE=os.environ.get('PICN_AARCH64_IMAGE','arm64v8/gcc:12-bookworm')
script=("set -e; apt-get update -qq >/dev/null; apt-get install -y -qq libasound2 >/dev/null; "
 "gcc -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -fPIC -fvisibility=hidden -fno-common -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation "
 "-Ibuild -Isrc -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include -shared src/engine.c third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c "
 "/usr/lib/aarch64-linux-gnu/libasound.so.2 -static-libgcc -lpthread -lm -ldl -Wl,--no-undefined -Wl,-z,relro,-z,now,-z,noexecstack -o build/aarch64/piclidean_mpc.so; "
 "strip build/aarch64/piclidean_mpc.so; readelf -h -d --version-info -Ws build/aarch64/piclidean_mpc.so > build/reports/ELF-aarch64.txt")
(B/'build/aarch64').mkdir(parents=True,exist_ok=True);(B/'build/reports').mkdir(parents=True,exist_ok=True)
source_inputs=inputs()
cross=os.environ.get('PICN_AARCH64_SYSROOT')
if cross:
 root=Path(cross).resolve();cc=root/'usr/bin/aarch64-linux-gnu-gcc-12'
 command=[str(cc),'--sysroot='+str(root),'-B'+str(root/'usr/aarch64-linux-gnu/lib')+'/', '-std=c11','-D_POSIX_C_SOURCE=200809L','-O2','-fPIC','-fvisibility=hidden','-fno-common','-Wall','-Wextra','-Wno-unused-parameter','-Wno-misleading-indentation','-Ibuild','-Isrc','-Ithird_party/mpc-vst-wrapper/wrapper','-Ithird_party/alsa-dev/usr/include','-shared','src/engine.c','third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c',str(root/'usr/lib/aarch64-linux-gnu/libasound.so.2'),'-static-libgcc','-lpthread','-lm','-ldl','-Wl,--no-undefined','-Wl,-z,relro,-z,now,-z,noexecstack','-o','build/aarch64/piclidean_mpc.so']
 env=dict(os.environ,PATH=str(root/'usr/bin')+':'+os.environ.get('PATH',''),LD_LIBRARY_PATH=str(root/'usr/lib/x86_64-linux-gnu')+':'+os.environ.get('LD_LIBRARY_PATH',''))
 result=subprocess.run(command,cwd=B,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 if result.returncode==0:
  subprocess.run([str(root/'usr/bin/aarch64-linux-gnu-strip'),'build/aarch64/piclidean_mpc.so'],cwd=B,env=env,check=True)
  report=subprocess.check_output([str(root/'usr/bin/aarch64-linux-gnu-readelf'),'-h','-d','--version-info','-Ws','build/aarch64/piclidean_mpc.so'],cwd=B,env=env,text=True)
  (B/'build/reports/ELF-aarch64.txt').write_text(report)
else:
 result=subprocess.run(['docker','run','--rm','--platform','linux/arm64','-v',str(B)+':/w','-w','/w',IMAGE,'bash','-c',script],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(B/'build/compile-aarch64.log').write_text(result.stdout);print(result.stdout[-4000:])
if result.returncode:raise SystemExit(result.returncode)
elf=B/'build/aarch64/piclidean_mpc.so';raw=elf.read_bytes()
assert raw[:5]==b'\x7fELF\x02' and int.from_bytes(raw[18:20],'little')==183,'not an aarch64 ELF64'
subprocess.run([os.environ.get('PICN_SKIN_PYTHON',sys.executable),str(B/'scripts/build_skin.py')],check=True,cwd=B,env=dict(os.environ,PICN_SKIN_MIN_IMAGE_AREA='6400',PICN_SKIN_OUTPUT='build/aarch64/skin',PICN_SKIN_RECEIPT='build/aarch64/skin-receipt.json'))
report=(B/'build/reports/ELF-aarch64.txt').read_text()
info={'version':VERSION,'architecture':'ELF64 aarch64','glibc_versions':sorted(set(re.findall(r'GLIBC_[0-9.]+',report))),'engine_sha256':hashlib.sha256(raw).hexdigest(),'hardware_access':False}
assert all(tuple(map(int,v.split('_')[1].split('.'))) <= (2,34) for v in info['glibc_versions']), 'glibc newer than 2.34'
assert source_inputs==inputs(),'Build inputs changed during compilation'
skin_receipt=json.loads((B/'build/aarch64/skin-receipt.json').read_text())
assert skin_receipt['version']==VERSION and skin_receipt['inputs']==inputs(),'Skin inputs changed during build'
assert skin_receipt['outputs']==tree(B/'build/aarch64/skin'/FOLDER),'Skin output changed during build'
info['input_hashes']=inputs();info['skin_output_hashes']=skin_receipt['outputs']
(B/'build/reports/BUILD-aarch64.json').write_text(json.dumps(info,indent=2)+'\n');print('Built',elf,info['engine_sha256'])

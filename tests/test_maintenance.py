"""Offline regression suite. Uses generated params.h; never contacts a device."""
from pathlib import Path
import subprocess,os,json,hashlib,datetime
B=Path(__file__).resolve().parents[1];O=B/'build/maintenance-137';O.mkdir(exist_ok=True)
names=['midi_input_stability_test','recall_snapshot_test','native_programs_test','mod_refresh_wrapper_test','variations134_wrapper_test','release131_test','mod_preview_worker_test','polymeter_loop_test','count_in_test','scales_panel_test']
flags=['gcc','-std=c11','-D_POSIX_C_SOURCE=200809L','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-pthread',*['-I'+str(B/x) for x in ['tests','build','src','third_party/mpc-vst-wrapper/wrapper','third_party/alsa-dev/usr/include']]]
report={'passed':False,'hardware_access':False,'hardware_qualified':False,'timestamp_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'tests':[],'sanitizers':['address','undefined']}
paths=list((B/'src').glob('*'))+[B/'third_party/mpc-vst-wrapper/wrapper/vst2_wrap.c',B/'build/params.h']+list((B/'tests').glob('*.c'))
report['input_sha256']={p.relative_to(B).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}
for name in names:
 subprocess.run([*flags,str(B/'tests'/(name+'.c')),'-l:libasound.so.2','-lm','-o',str(O/name)],cwd=B,check=True)
 r=subprocess.run([str(O/name)],cwd=B,capture_output=True,text=True,timeout=120,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
 (O/(name+'.log')).write_text(r.stdout+r.stderr);print(name,r.returncode,r.stdout[-2000:],r.stderr[-1000:],flush=True);r.check_returncode();report['tests'].append(name)
assert all(hashlib.sha256((B/p).read_bytes()).hexdigest()==h for p,h in report['input_sha256'].items())
report['passed']=True;(B/'Validation/MAINTENANCE-1.3.7.json').write_text(json.dumps(report,indent=2)+'\n')
print('All targeted sanitizer tests passed.')

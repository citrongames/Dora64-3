"""Generate a pinned research-only runtime source; never edit dependency checkouts."""
import hashlib, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
source=ROOT/'build-deps/lib/N64ModernRuntime/ultramodern/src/events.cpp'
data=source.read_bytes()
expected='cd47bb9b332e0c907131310bb3ec8b551b7fb7b2160538ba7fe5aa7554ba2e97'
if hashlib.sha256(data).hexdigest()!=expected:
    raise RuntimeError('Unreviewed events.cpp revision; refusing timing adaptation')
text=data.decode()
before='''                // Keep the original RSP timing; delaying this until parsing completes
                // did not prevent Doraemon's nested-list corruption and stalls the game.
                sp_complete();'''
after='''                // Dora64-3 research only: keep the original scheduler waiting for
                // graphics SP completion until RT64 has consumed this task.
                // This is not a complete memory-lifetime fix or a timing guarantee.'''
point='                renderer_context->send_dl(&task_action->task);'
assert text.count(before)==1 and text.count(point)==1
text=text.replace(before,after).replace(point,point+'\n                sp_complete();')
output=Path(sys.argv[1]).resolve()
if not output.is_relative_to(ROOT/'build-local'):
    raise ValueError('Output must stay in ignored build-local')
output.parent.mkdir(parents=True,exist_ok=True)
if not output.exists() or output.read_text()!=text:
    output.write_text(text)
print('Research graphics SP completion: after RT64 parse; original dependency unchanged')

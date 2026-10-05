"""Content-checked CPU oracle results; cache hits retain their original execution.

The key covers compiler dependency scans, generated translation units/seed,
compiler tools, sanitizer libraries and flags. Failed or incomplete runs are
never reusable. Logs and the executed binary remain beside each result.
"""
from pathlib import Path
import datetime, hashlib, json, os, platform, shlex, shutil, subprocess, uuid

SCHEMA = 1

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()

def atomic_json(path, value):
    path = Path(path)
    temporary = path.with_name(path.name + '.' + uuid.uuid4().hex + '.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    os.replace(temporary, path)

def compilation_inputs(commands, seed, roots, extra_files=()):
    """Ask the actual compilers for every header, including system headers.

    Generation directory names are not executable inputs. Normalize those
    names while retaining the complete contents of their selected files.
    """
    roots = [(str(Path(path).resolve()), label) for path, label in roots]
    roots.sort(key=lambda pair: len(pair[0]), reverse=True)
    def identity(path):
        value = str(Path(path).resolve())
        for base, label in roots:
            if value == base or value.startswith(base + '/'):
                return label + value[len(base):]
        return value
    files = {}
    tools = {}
    compile_options = []
    for command in commands:
        compiler = shutil.which(command[0])
        assert compiler, command[0]
        compiler = Path(compiler).resolve()
        tools[str(compiler)] = sha(compiler)
        version = subprocess.check_output([str(compiler), '--version'], text=True).splitlines()[0]
        options, sources = [], []
        skip = False
        for argument in command[1:]:
            if skip:
                skip = False
                continue
            if argument == '-o':
                skip = True
            elif argument == '-c' or Path(argument).suffix == '.o':
                continue
            elif Path(argument).suffix in ('.c', '.cpp') and not argument.startswith('-'):
                sources.append(argument)
            else:
                options.append(argument)
        assert sources and not skip
        normalized = [('-I' + identity(option[2:])) if option.startswith('-I') else option for option in options]
        compile_options.append(dict(compiler=str(compiler), version=version, flags=normalized,
                                    sources=[identity(path) for path in sources]))
        for source in sources:
            dependencies = subprocess.check_output([str(compiler), *options, '-M', '-MT', 'oracle', source], text=True)
            assert dependencies.startswith('oracle:'), dependencies[:100]
            for dependency in shlex.split(dependencies.removeprefix('oracle:').replace('\\\n', ' ')):
                path = Path(dependency).resolve()
                key = identity(path)
                value = sha(path)
                assert key not in files or files[key] == value, key
                files[key] = value
        for name in ('cc1', 'cc1plus', 'collect2', 'ld', 'as'):
            tool = subprocess.check_output([str(compiler), '-print-prog-name=' + name], text=True).strip()
            path = Path(shutil.which(tool) or tool)
            if path.is_file():
                tools[str(path.resolve())] = sha(path)
        for name in ('libasan.so', 'libubsan.so', 'libstdc++.so.6', 'libgcc_s.so.1', 'libc.so.6', 'libm.so.6',
                     'crt1.o', 'crti.o', 'crtn.o', 'crtbegin.o', 'crtend.o', 'ld-linux-x86-64.so.2'):
            path = Path(subprocess.check_output([str(compiler), '-print-file-name=' + name], text=True).strip())
            assert path.is_file(), (compiler, name, path)
            tools[str(path.resolve())] = sha(path)
    for path in [seed, *extra_files]:
        files[identity(path)] = sha(path)
    return dict(schema=SCHEMA, architecture=platform.machine(), compilation=compile_options,
                files_sha256=dict(sorted(files.items())), tools_sha256=dict(sorted(tools.items())),
                sanitizer_environment=dict(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1'))

def verify_inputs(inputs, roots):
    """Catch an edit during compilation/execution before publishing a result."""
    roots = {label:Path(path).resolve() for path, label in roots}
    for name, value in {**inputs['files_sha256'], **inputs['tools_sha256']}.items():
        if name.startswith('$'):
            label, _, remainder = name.partition('/')
            path = roots[label] / remainder
        else:
            path = Path(name)
        assert sha(path) == value, 'Oracle input changed during execution: ' + name

def read_group(cache, inputs, group):
    """Return a completed result only if every retained artifact still matches."""
    directory = Path(cache) / digest(inputs) / ('group-' + str(group['id']))
    try:
        index = json.loads((directory / 'success.json').read_text())
        attempt = index['attempt']
        if Path(attempt).name != attempt or not attempt.startswith('attempt-'):
            return None
        run = directory / attempt
        record = json.loads((run / 'record.json').read_text())
        if index['record_sha256'] != sha(run / 'record.json') or record['inputs_sha256'] != digest(inputs):
            return None
        if record['group'] != group or record['exit_code'] != 0 or record['status'] != 'PASS':
            return None
        if json.loads((run / 'inputs.json').read_text()) != inputs:
            return None
        for name, value in record['artifacts_sha256'].items():
            if Path(name).name != name or sha(run / name) != value:
                return None
        if set(record['artifacts_sha256']) != {'check', 'stdout.log', 'stderr.log', 'inputs.json'}:
            return None
        lines = (run / 'stdout.log').read_text().strip().splitlines()
        if len(lines) < 2 or not all(line.startswith('PASS:') for line in lines):
            return None
        return lines, dict(record, reused=True, cache_record=str(run / 'record.json'))
    except (OSError, ValueError, KeyError, TypeError):
        return None

def save_group(cache, inputs, group, binary, stdout, stderr, exit_code, elapsed, producer):
    """Publish PASS atomically, after the subprocess and all retained artifacts."""
    directory = Path(cache) / digest(inputs) / ('group-' + str(group['id']))
    run = directory / ('attempt-' + uuid.uuid4().hex)
    run.mkdir(parents=True)
    atomic_json(run / 'inputs.json', inputs)
    shutil.copy2(binary, run / 'check')
    shutil.copy2(stdout, run / 'stdout.log')
    shutil.copy2(stderr, run / 'stderr.log')
    record = dict(group=group, inputs_sha256=digest(inputs), exit_code=exit_code,
                  status='PASS' if exit_code == 0 else 'FAIL', elapsed_seconds=elapsed,
                  completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), producer=producer,
                  artifacts_sha256={name:sha(run / name) for name in ('check', 'stdout.log', 'stderr.log', 'inputs.json')})
    atomic_json(run / 'record.json', record)
    if exit_code == 0:
        lines = (run / 'stdout.log').read_text().strip().splitlines()
        assert len(lines) >= 2 and all(line.startswith('PASS:') for line in lines), 'Missing complete oracle summaries'
        atomic_json(directory / 'success.json', dict(attempt=run.name, record_sha256=sha(run / 'record.json')))
    return record

"""Build the pinned Doraemon recompiler without using a developer's checkout."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]


def run(*args, cwd=None, capture=False):
    return subprocess.run(
        [str(arg) for arg in args], cwd=cwd, check=True,
        stdout=subprocess.PIPE if capture else None,
        encoding="utf-8" if capture else None,
    ).stdout


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_sources(source, lock, patched):
    if run("git", "rev-parse", "HEAD", cwd=source, capture=True).strip() != lock["commit"]:
        raise RuntimeError(f"Unexpected N64Recomp revision in {source}; use a new --work-dir.")
    # Only the documented, unstaged patch is allowed. Never reset user changes.
    status = run("git", "status", "--porcelain", "--untracked-files=normal",
                 "--ignore-submodules=none", cwd=source, capture=True)
    for line in status.splitlines():
        if not (patched and line.startswith(" M ") and line[3:] in lock["files"]):
            raise RuntimeError(f"Unexpected source changes in {source}: {line}")
    key = "patched_sha256" if patched else "original_sha256"
    for name, hashes in lock["files"].items():
        if sha256(source / name) != hashes[key]:
            raise RuntimeError(f"Unexpected contents of {source / name}; no files were reset.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work-dir", type=Path, default=PROJECT_ROOT / "build-tools" / "n64recomp")
    parser.add_argument("--jobs", type=int, default=min(os.cpu_count() or 1, 8))
    parser.add_argument("--cmake", default="cmake", help="CMake executable or absolute path")
    parser.add_argument("--generator", help="Optional CMake generator, e.g. Ninja")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")

    metadata = PROJECT_ROOT / "tools" / "n64recomp"
    lock = json.loads((metadata / "lock.json").read_text(encoding="utf-8"))
    patch = metadata / lock["patch"]
    if sha256(patch) != lock["patch_sha256"]:
        raise RuntimeError("N64Recomp patch does not match lock.json.")

    work = args.work_dir.resolve()
    source = work / "source"
    build = work / "build"
    work.mkdir(parents=True, exist_ok=True)
    if not source.exists():
        run("git", "clone", "--no-checkout", "-c", "core.autocrlf=false",
            lock["repository"], source)
        run("git", "checkout", "--detach", lock["commit"], cwd=source)

    patched = all((source / name).is_file() and
                  sha256(source / name) == hashes["patched_sha256"]
                  for name, hashes in lock["files"].items())
    check_sources(source, lock, patched)
    # Gitlinks in the pinned commit lock all dependencies, including nested ones.
    submodules = run("git", "submodule", "status", "--recursive", cwd=source, capture=True)
    if any(line.startswith(("+", "U")) for line in submodules.splitlines()):
        raise RuntimeError("N64Recomp submodule revision differs from the pin; use a new --work-dir.")
    if any(line.startswith("-") for line in submodules.splitlines()):
        run("git", "-c", "core.autocrlf=false", "submodule", "update", "--init", "--recursive", cwd=source)
    if not patched:
        run("git", "apply", "--check", patch, cwd=source)
        run("git", "apply", patch, cwd=source)
    check_sources(source, lock, patched=True)

    configure = [args.cmake, "-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Release"]
    if args.generator:
        configure += ["-G", args.generator]
    run(*configure)
    run(args.cmake, "--build", build, "--config", "Release", "--target",
        "N64RecompCLI", "RSPRecomp", "--parallel", args.jobs)
    suffix = ".exe" if os.name == "nt" else ""
    executables = {}
    for name in ("N64Recomp", "RSPRecomp"):
        candidates = [build / (name + suffix), build / "Release" / (name + suffix)]
        executable = next((p for p in candidates if p.is_file()), None)
        if executable is None:
            raise RuntimeError(f"Built {name} was not found in {build}.")
        executables[name] = {"path": str(executable), "sha256": sha256(executable)}
        print(f"{name}: {executable}", flush=True)
    receipt = {"upstream_commit": lock["commit"], "patch_sha256": lock["patch_sha256"],
               "executables": executables}
    (work / "build-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"Recompiler setup failed: {error}", file=sys.stderr)
        sys.exit(1)

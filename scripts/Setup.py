#!/usr/bin/env python3
import os
import platform
import shutil
import subprocess
from pathlib import Path


def run_step(cmd, cwd, label, allow_failure=False):
    print(f"[Bolt Setup] {label}...")
    print(f"[Bolt Setup] > {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0 and not allow_failure:
        raise RuntimeError(f"Step failed ({label}) with exit code {result.returncode}.")
    return result.returncode == 0


def resolve_premake_executable(repo_root: Path) -> str | None:
    candidates = []
    if platform.system() == "Windows":
        candidates.append(repo_root / "vendor" / "bin" / "premake5.exe")
    else:
        candidates.append(repo_root / "vendor" / "bin" / "premake5")

    for candidate in candidates:
        if candidate.is_file():
            return str(candidate)

    return shutil.which("premake5")


def main():
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent

    os.chdir(repo_root)
    print(f"[Bolt Setup] Repository root: {repo_root}")

    os.environ["BOLT_DIR"] = str(repo_root)
    print(f"[Bolt Setup] BOLT_DIR set to: {os.environ['BOLT_DIR']}")

    run_step(["git", "submodule", "update", "--init", "--recursive"], repo_root, "Updating git submodules")

    lfs_ok = run_step(["git", "lfs", "pull"], repo_root, "Pulling Git LFS assets", allow_failure=True)
    if not lfs_ok:
        print("[Bolt Setup] Warning: git lfs pull failed or Git LFS is unavailable. Continuing...")

    premake_path = resolve_premake_executable(repo_root)
    if not premake_path:
        print("[Bolt Setup] ERROR: Premake executable was not found.")
        print("[Bolt Setup] Either place premake5(.exe) in vendor/bin or install premake5 in PATH.")
        return 1

    action = "vs2022" if platform.system() == "Windows" else "gmake2"
    run_step([premake_path, action], repo_root, f"Generating {action} files via Premake")

    print("[Bolt Setup] Setup complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[Bolt Setup] ERROR: {exc}")
        raise SystemExit(1)
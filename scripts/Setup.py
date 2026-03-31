#!/usr/bin/env python3
import os
import subprocess
from pathlib import Path


def run_step(cmd, cwd, label, allow_failure=False):
    print(f"[Bolt Setup] {label}...")
    print(f"[Bolt Setup] > {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0 and not allow_failure:
        raise RuntimeError(f"Step failed ({label}) with exit code {result.returncode}.")
    return result.returncode == 0


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

    premake_path = repo_root / "vendor" / "bin" / "premake5.exe"
    if not premake_path.is_file():
        print("[Bolt Setup] ERROR: Premake executable was not found.")
        print(f"[Bolt Setup] Expected path: {premake_path}")
        print("[Bolt Setup] Place premake5.exe at vendor/bin/premake5.exe and run setup again.")
        return 1

    run_step([str(premake_path), "vs2022"], repo_root, "Generating Visual Studio 2022 solution via Premake")

    print("[Bolt Setup] Setup complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[Bolt Setup] ERROR: {exc}")
        raise SystemExit(1)
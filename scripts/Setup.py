#!/usr/bin/env python3
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


# ── Helpers ──────────────────────────────────────────────────────────────────

def run_step(cmd, cwd, label, allow_failure=False):
    print(f"[Bolt Setup] {label}...")
    print(f"[Bolt Setup] > {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0 and not allow_failure:
        raise RuntimeError(f"Step failed ({label}) with exit code {result.returncode}.")
    return result.returncode == 0


def check_tool(name, cmd=None):
    """Return True if *name* is reachable on PATH."""
    exe = shutil.which(cmd or name)
    if exe:
        print(f"  [OK] {name} found: {exe}")
    else:
        print(f"  [!!] {name} NOT found on PATH")
    return exe is not None


def submodules_populated(repo_root: Path) -> bool:
    """Quick check: do all registered submodule paths have content?"""
    gitmodules = repo_root / ".gitmodules"
    if not gitmodules.exists():
        return True
    result = subprocess.run(
        ["git", "submodule", "status"],
        cwd=repo_root,
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        return False
    for line in result.stdout.strip().splitlines():
        # Lines starting with '-' mean uninitialized submodule
        if line.strip().startswith("-"):
            return False
    return True


def dotnet_files_present(repo_root: Path) -> bool:
    """Return True if External/dotnet already has all required files."""
    dotnet_dir = repo_root / "External" / "dotnet"
    required = [
        dotnet_dir / "nethost.h",
        dotnet_dir / "hostfxr.h",
        dotnet_dir / "coreclr_delegates.h",
        dotnet_dir / "lib" / "nethost.lib",
        dotnet_dir / "lib" / "nethost.dll",
    ]
    return all(f.is_file() for f in required)


def resolve_premake_executable(repo_root: Path):
    if platform.system() == "Windows":
        vendored = repo_root / "vendor" / "bin" / "premake5.exe"
    else:
        vendored = repo_root / "vendor" / "bin" / "premake5"
    if vendored.is_file():
        return str(vendored)
    return shutil.which("premake5")


def detect_dotnet_version(dotnet_root: Path) -> str | None:
    """Find the latest installed .NET host pack version."""
    host_packs = dotnet_root / "packs" / "Microsoft.NETCore.App.Host.win-x64"
    if not host_packs.is_dir():
        return None
    versions = sorted(
        [d.name for d in host_packs.iterdir() if d.is_dir()],
        reverse=True,
    )
    return versions[0] if versions else None


# ── Main ─────────────────────────────────────────────────────────────────────

def main():
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent

    os.chdir(repo_root)
    print(f"[Bolt Setup] Repository root: {repo_root}")
    print(f"[Bolt Setup] Platform: {platform.system()}")
    print()

    os.environ["BOLT_DIR"] = str(repo_root)

    # ── 1. Dependency check ──────────────────────────────────────────────
    print("[Bolt Setup] Checking prerequisites...")
    missing = []

    if not check_tool("git"):
        missing.append("git")

    if not check_tool("python", sys.executable):
        missing.append("python")

    premake_path = resolve_premake_executable(repo_root)
    if premake_path:
        print(f"  [OK] premake5 found: {premake_path}")
    else:
        print("  [!!] premake5 NOT found (vendor/bin or PATH)")
        missing.append("premake5")

    is_windows = platform.system() == "Windows"

    if is_windows:
        dotnet_root = Path(os.environ.get("DOTNET_ROOT", r"C:\Program Files\dotnet"))
        dotnet_exe = shutil.which("dotnet")
        if dotnet_exe:
            print(f"  [OK] dotnet found: {dotnet_exe}")
        else:
            print("  [!!] dotnet NOT found on PATH")

        detected_ver = detect_dotnet_version(dotnet_root)
        if detected_ver:
            print(f"  [OK] .NET host pack found: {detected_ver}")
        else:
            print("  [!!] No .NET host pack found — setup-dotnet will fail")
            if not dotnet_files_present(repo_root):
                missing.append("dotnet-host-pack")

    print()
    if missing:
        print(f"[Bolt Setup] WARNING: Missing prerequisites: {', '.join(missing)}")
        print("[Bolt Setup] The setup will continue but some steps may fail.")
        print()

    # ── 2. Git submodules ────────────────────────────────────────────────
    if submodules_populated(repo_root):
        print("[Bolt Setup] Submodules already initialised — skipping update.")
        print("             (Run 'git submodule update --init --recursive' manually to force.)")
    else:
        run_step(
            ["git", "submodule", "update", "--init", "--recursive"],
            repo_root, "Updating git submodules",
        )

    # ── 3. Git LFS ──────────────────────────────────────────────────────
    lfs_ok = run_step(
        ["git", "lfs", "pull"], repo_root,
        "Pulling Git LFS assets", allow_failure=True,
    )
    if not lfs_ok:
        print("[Bolt Setup] Warning: git lfs pull failed or Git LFS is unavailable. Continuing...")

    # ── 4. .NET hosting files (Windows only) ─────────────────────────────
    if is_windows:
        if dotnet_files_present(repo_root):
            print("[Bolt Setup] .NET hosting files already present — skipping.")
        else:
            print("[Bolt Setup] Setting up .NET hosting files...")
            ps_script = script_dir / "setup-dotnet.ps1"
            if not ps_script.is_file():
                print("[Bolt Setup] WARNING: setup-dotnet.ps1 not found, skipping .NET setup.")
            else:
                ps_args = ["powershell", "-ExecutionPolicy", "Bypass", "-File", str(ps_script)]
                if detected_ver:
                    ps_args += ["-DotNetVersion", detected_ver]
                run_step(ps_args, script_dir, "Copying .NET hosting files", allow_failure=True)

    # ── 5. Premake ───────────────────────────────────────────────────────
    if premake_path:
        action = "vs2022" if is_windows else "gmake2"
        run_step([premake_path, action], repo_root, f"Generating {action} files via Premake")
    else:
        print("[Bolt Setup] Skipping project generation — premake5 not available.")

    print()
    print("[Bolt Setup] Setup complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[Bolt Setup] ERROR: {exc}")
        raise SystemExit(1)

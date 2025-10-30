#!/usr/bin/env python3
import subprocess
import os
import datetime

BASE = "/work/doctor"
CRY_DIR = os.path.join(BASE, "cry")
SAW_DIR = os.path.join(BASE, "saw")

REPORT_CRY = os.path.join(BASE, "cryptol_report.md")
REPORT_SAW = os.path.join(BASE, "saw_report.md")

timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def run_command(cmd, cwd=None):
    """Run a shell command and capture stdout/stderr + exit code."""
    try:
        result = subprocess.run(
            cmd, shell=True, text=True, capture_output=True, cwd=cwd, timeout=180
        )
        success = result.returncode == 0
        output = result.stdout.strip() + "\n" + result.stderr.strip()
        return success, output.strip()
    except subprocess.TimeoutExpired:
        return False, "Timeout expired"
    except Exception as e:
        return False, f"Exception: {e}"

def compile_files(tool, folder, report_path):
    """Compile files of a given type (Cryptol or SAW) and write a Markdown report."""
    print(f"\n=== Starting {tool.upper()} batch ===")
    entries = []
    failures = []

    with open(report_path, "w", encoding="utf-8") as rep:
        rep.write(f"# {tool.upper()} Compilation Report\n")
        rep.write(f"Generated: {timestamp}\n\n")
        rep.write("| File | Result | Message |\n")
        rep.write("|------|---------|---------|\n")
        rep.flush()

        try:
            for root, _, files in os.walk(folder):
                for f in files:
                    if not f.endswith((".cry", ".saw")):
                        continue
                    path = os.path.join(root, f)
                    rel = os.path.relpath(path, BASE)
                    cmd = f"{tool} -b {path}" if tool == "cryptol" else f"{tool} {path}"
                    print(f"[BUILD] Compiling {rel} with {tool}...")
                    success, output = run_command(cmd)

                    short = output.replace("\n", " ").strip()
                    if len(short) > 100:
                        short = short[:97] + "..."
                    result_text = "Success" if success else "Failed"
                    rep.write(f"| `{rel}` | {result_text} | {short} |\n")
                    rep.flush()

                    entries.append((rel, success, output))
                    if not success:
                        failures.append((rel, output))

        except KeyboardInterrupt:
            print("\nInterrupted by user. Writing partial report.")
        finally:
            rep.write("\n\n---\n")
            rep.write(f"## Full Failure Logs ({len(failures)} total)\n\n")
            for rel, full_output in failures:
                rep.write(f"### `{rel}`\n")
                rep.write("```text\n")
                rep.write(full_output.strip() + "\n")
                rep.write("```\n\n")
            rep.write("_Report ended._\n")
            rep.flush()

    print(f"\nReport written to {report_path} ({len(entries)} files processed)\n")

def main():
    if os.path.isdir(CRY_DIR):
        compile_files("cryptol", CRY_DIR, REPORT_CRY)
    else:
        print("[!] No Cryptol directory found")

    if os.path.isdir(SAW_DIR):
        compile_files("saw", SAW_DIR, REPORT_SAW)
    else:
        print("[!] No SAW directory found")

if __name__ == "__main__":
    main()
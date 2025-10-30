#!/usr/bin/env python3
import json, os

# Base folder
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CRY_DIR = os.path.join(BASE_DIR, "cry")
SAW_DIR = os.path.join(BASE_DIR, "saw")
JSONL_PATH = os.path.join(BASE_DIR, "failed.jsonl")

# Ensure directories exist
os.makedirs(CRY_DIR, exist_ok=True)
os.makedirs(SAW_DIR, exist_ok=True)

# Process JSONL
count_cry = count_saw = count_skip = 0
with open(JSONL_PATH, "r", encoding="utf-8") as f:
    for i, line in enumerate(f, 1):
        line = line.strip()
        if not line:
            continue
        try:
            obj = json.loads(line)
            ftype = obj.get("filetype", "").lower()
            fname = os.path.basename(obj.get("filename", f"unknown_{i}.{ftype or 'txt'}"))
            content = obj.get("content", "")

            if ftype == "cry":
                path = os.path.join(CRY_DIR, fname)
                count_cry += 1
            elif ftype == "saw":
                path = os.path.join(SAW_DIR, fname)
                count_saw += 1
            else:
                count_skip += 1
                print(f"[!] Skipping line {i}: unknown type '{ftype}'")
                continue

            with open(path, "w", encoding="utf-8") as out:
                out.write(content)
            print(f"[+] Wrote {path}")

        except Exception as e:
            count_skip += 1
            print(f"[!] Error parsing line {i}: {e}")

print(f"\n Done: {count_cry} .cry files, {count_saw} .saw files written, {count_skip} skipped.")
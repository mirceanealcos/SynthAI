# data/preprocess.py

import os, shutil
from utils import parse_filename

RAW_DIR   = os.path.join(os.path.dirname(__file__), "raw_midi")
SPLIT_DIR = os.path.join(os.path.dirname(__file__), "split_midis")
ROLES     = {"bass","lead","pad","pluck"}

def split_by_filename(input_dir: str, output_dir: str):
    os.makedirs(output_dir, exist_ok=True)
    for fn in os.listdir(input_dir):
        if not fn.lower().endswith((".mid",".midi")):
            continue
        info = parse_filename(fn)
        if not info:
            print(f"⚠️  Skipping {fn}: filename not recognized")
            continue
        base, role, tempo, key = info
        if role not in ROLES:
            print(f"⚠️  Skipping {fn}: unknown role '{role}'")
            continue

        role_dir = os.path.join(output_dir, role)
        os.makedirs(role_dir, exist_ok=True)
        src = os.path.join(input_dir, fn)
        dst = os.path.join(role_dir, fn)
        shutil.copy2(src, dst)
        print(f"✅ {fn} → {role}/{fn}")

if __name__ == "__main__":
    split_by_filename(RAW_DIR, SPLIT_DIR)

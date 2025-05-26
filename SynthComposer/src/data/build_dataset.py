# data/build_dataset.py

import os
import numpy as np
from utils import parse_filename

QUANT_DIR = os.path.join(os.path.dirname(__file__), "quantized")
ROLES     = ["bass", "lead", "pad", "pluck"]
SEQ_LEN   = 128
STRIDE    = 64

def collect_full_songs():
    groups = {}
    for role in ROLES:
        role_dir = os.path.join(QUANT_DIR, role)
        if not os.path.isdir(role_dir):
            continue
        for fn in os.listdir(role_dir):
            if not fn.endswith(".npz"):
                continue
            mid_name = os.path.splitext(fn)[0] + ".mid"
            info = parse_filename(mid_name)
            if not info:
                print(f"⚠️  Could not parse '{fn}', skipping")
                continue
            base, role_parsed, _, _ = info
            if role_parsed != role:
                print(f"⚠️  Role mismatch for '{fn}': parsed '{role_parsed}' vs folder '{role}'")
                continue
            groups.setdefault(base, {})[role] = fn

    complete = {b:paths for b, paths in groups.items() if all(r in paths for r in ROLES)}
    print(f"▶ Found {len(complete)} complete songs (of {len(groups)} total bases)")
    return complete

def load_role_roll(base, role):
    role_dir = os.path.join(QUANT_DIR, role)
    rolls = []
    prefix = f"{base}_{role}"
    for fn in os.listdir(role_dir):
        if fn.startswith(prefix) and fn.endswith(".npz"):
            arr = np.load(os.path.join(role_dir, fn))['roll']
            rolls.append(arr)
    if not rolls:
        raise ValueError(f"No data for base='{base}', role='{role}'")
    # pad to max length within this role group
    maxT = max(r.shape[0] for r in rolls)
    padded = [np.pad(r, ((0, maxT-r.shape[0]), (0,0)), 'constant') for r in rolls]
    return sum(padded)

def build_examples(user_role="bass"):
    songs = collect_full_songs()
    if not songs:
        raise RuntimeError("No complete song groups found. Check your quantized folders.")

    # Build key vocabulary
    key_names = []
    for base, paths in songs.items():
        mid_name = os.path.splitext(paths[user_role])[0] + ".mid"
        info = parse_filename(mid_name)
        if info:
            _, _, _, key = info
            key_names.append(key)
    key_names = sorted(set(key_names))
    KEY2IDX = {k:i for i,k in enumerate(key_names)}
    ROLE2IDX= {r:i for i,r in enumerate(ROLES)}

    X_user, X_key, X_role, Y = [], [], [], []

    for base, paths in songs.items():
        # parse key from user_role
        mid_name = os.path.splitext(paths[user_role])[0] + ".mid"
        info = parse_filename(mid_name)
        if not info:
            continue
        _, _, _, key = info
        kidx = KEY2IDX[key]

        # load user and accomp rolls
        user_roll = load_role_roll(base, user_role)  # (T1,128)
        accomp_rolls = [load_role_roll(base, r) for r in ROLES if r != user_role]  # list of (Ti,128)

        # pad all to the same length
        all_rolls = [user_roll] + accomp_rolls
        maxT = max(r.shape[0] for r in all_rolls)
        user_padded = np.pad(user_roll, ((0,maxT-user_roll.shape[0]),(0,0)), 'constant')
        accomp_padded = [np.pad(r, ((0,maxT-r.shape[0]),(0,0)), 'constant') for r in accomp_rolls]

        # stack accompaniment into shape (3, maxT, 128)
        accomp = np.stack(accomp_padded, axis=0)

        # slide windows
        for start in range(0, maxT - SEQ_LEN + 1, STRIDE):
            X_user.append(user_padded[start:start+SEQ_LEN])
            X_key.append(kidx)
            X_role.append(ROLE2IDX[user_role])
            Y.append(accomp[:, start:start+SEQ_LEN, :])

    if not X_user:
        raise RuntimeError("No examples generated; check SEQ_LEN/STRIDE vs data length.")

    X_user = np.stack(X_user)   # (N, SEQ_LEN, 128)
    X_key  = np.array(X_key)    # (N,)
    X_role = np.array(X_role)   # (N,)
    Y      = np.stack(Y)        # (N, 3, SEQ_LEN, 128)

    BASE = os.path.dirname(__file__)
    os.makedirs(BASE, exist_ok=True)
    out_fn = os.path.join(BASE, f"train_{user_role}.npz")
    np.savez_compressed(out_fn,
        X_user=X_user, X_key=X_key, X_role=X_role, Y=Y
    )
    print(f"✅ Saved {out_fn}")
    print("  Shapes:", X_user.shape, X_key.shape, X_role.shape, Y.shape)

if __name__ == "__main__":
    for role in ROLES:
        build_examples(role)

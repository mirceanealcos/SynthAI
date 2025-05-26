# src/ws_client/WebSocketClient.py

import os
import time
import asyncio
import json

import numpy as np
import torch
import websockets

from data.utils import parse_filename
from utils.KeyDetector import KeyDetector
from train_composer import ComposerModel

# ——— CONFIG ——————————————————————————————————————————

WS_URI_IN    = "ws://localhost:8080/user/input"
WS_URI_OUT   = "ws://localhost:8080/composer/output"
ROLENAME     = "bass"
MODEL_PATH   = os.path.join("models", f"composer_train_{ROLENAME}.npz.pt")
QUANT_DIR    = os.path.join("data", "quantized", ROLENAME)

SEQ_LEN           = 128
STEPS_PER_QUARTER = 16
BAR_STEPS         = STEPS_PER_QUARTER * 4

# ——— BUILD KEY2IDX ————————————————————————————————————

key_names = set()
for fn in os.listdir(QUANT_DIR):
    if not fn.endswith(".npz"):
        continue
    info = parse_filename(fn.replace(".npz", ".mid"))
    if info:
        _, _, _, key_name = info
        key_names.add(key_name)
KEY2IDX = {k: i for i, k in enumerate(sorted(key_names))}

# Only one role in this model
ROLE2IDX = {ROLENAME: 0}

# ——— INFER TEMPO & TIMING ————————————————————————————

first_npz = next(fn for fn in os.listdir(QUANT_DIR) if fn.endswith(".npz"))
tempo     = parse_filename(first_npz.replace(".npz", ".mid"))[2]
quarter_sec = 60.0 / tempo
step_sec    = quarter_sec / STEPS_PER_QUARTER
window_secs = (SEQ_LEN / STEPS_PER_QUARTER) * quarter_sec

# ——— MODEL LOADER & UTILITIES ——————————————————————————

def load_model():
    # Load role_vocab from dataset so it matches the checkpoint
    data = np.load(os.path.join("data", f"train_{ROLENAME}.npz"))
    role_vocab = int(data['X_role'].max()) + 1
    model = ComposerModel(len(KEY2IDX), role_vocab)
    model.load_state_dict(torch.load(MODEL_PATH, map_location="cpu"))
    model.eval()
    return model

def events_to_roll(events):
    """
    Convert buffered (t_sec, pitch, vel) events into a piano-roll (SEQ_LEN,128).
    """
    roll = np.zeros((SEQ_LEN, 128), dtype=np.float32)
    if not events:
        return roll
    t0 = events[-1][0] - window_secs
    for t, pitch, vel in events:
        idx = int((t - t0) / step_sec)
        if 0 <= idx < SEQ_LEN:
            roll[idx, pitch] = vel / 127.0
    return roll

# ——— MAIN ASYNC LOOP —————————————————————————————

async def run():
    model       = load_model()
    keydet      = KeyDetector()
    buffer      = []    # (t_sec, pitch, vel)
    pending_offs= []    # (send_time_sec, role, pitch)
    last_bar    = time.time()
    current_key = None  # until detected

    async with websockets.connect(WS_URI_IN) as ws_in, \
               websockets.connect(WS_URI_OUT) as ws_out:
        print("🎶 Connected to WS in/out")
        async for msg in ws_in:
            evt = json.loads(msg)
            now = time.time()

            # 1) flush any pending note_offs
            due = [off for off in pending_offs if off[0] <= now]
            pending_offs[:] = [off for off in pending_offs if off[0] > now]
            for send_t, role, pitch in due:
                await ws_out.send(json.dumps({
                    "type":      "note_off",
                    "role":      role,
                    "note":      pitch,
                    "timestamp": int(send_t * 1000)
                }))

            # 2) key detection
            keydet.feed_event(evt)
            detected = keydet.estimate_key()
            if detected:
                current_key = detected
                print(f"🎹 Key changed → {current_key}")

            # 3) buffer note_on
            if evt.get("type") == "note_on":
                buffer.append((
                    evt["timestamp"]/1000.0,
                    evt["note"],
                    evt["velocity"]
                ))

            # 4) skip generation until we have a valid trained key
            if current_key is None:
                continue
            if current_key not in KEY2IDX:
                print(f"⚠️  Key '{current_key}' not in model keys; skipping")
                continue

            # 5) every bar, run the model
            if now - last_bar >= BAR_STEPS * step_sec:
                last_bar = now

                # prepare inputs
                roll   = events_to_roll(buffer)
                x_user = torch.from_numpy(roll[None,:,:]).float()
                k_idx  = torch.tensor([KEY2IDX[current_key]], dtype=torch.long)
                r_idx  = torch.tensor([ROLE2IDX[ROLENAME]], dtype=torch.long)

                with torch.no_grad():
                    out = model(x_user, k_idx, r_idx).squeeze(0).numpy()

                # reshape to (3 roles, SEQ_LEN, 128)
                out = out.reshape(SEQ_LEN, 3, 128).transpose(1,0,2)
                t0  = now + step_sec
                out_roles = [r for r in ["bass","lead","pad","pluck"] if r != ROLENAME]

                # emit note_on and schedule note_off
                for i, role in enumerate(out_roles):
                    for step in range(SEQ_LEN):
                        pitches = np.where(out[i, step] > 0.5)[0]
                        for p in pitches:
                            t_on = t0 + step * step_sec
                            await ws_out.send(json.dumps({
                                "type":      "note_on",
                                "role":      role,
                                "note":      int(p),
                                "velocity":  int(out[i,step,p]*127),
                                "timestamp": int(t_on * 1000)
                            }))
                            # schedule its note_off one 16th later
                            pending_offs.append((t_on+step_sec, role, int(p)))

                buffer.clear()

if __name__ == "__main__":
    asyncio.run(run())

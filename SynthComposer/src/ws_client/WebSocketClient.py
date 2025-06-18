# src/ws_client/WebSocketClient.py

import os
import time
import asyncio
import json
import re

import torch
import websockets

from src.utils.KeyDetector import KeyDetector

# Import our new RNN and its globals
from src.composer.music_rnn import (
    MusicRNN,
    generate,
    TOKEN2IDX,
    IDX2TOKEN,
    ROLES,
    ROLE2IDX,
    KEY2IDX,
    IDX2KEY,        # we need to update both
)

# ——— CONFIG ——————————————————————————————————————————
WS_URI_IN     = "ws://localhost:8080/user/input"
WS_URI_OUT    = "ws://localhost:8080/composer/output"
MODEL_PATH    = "../composer/music_rnn.pt"
GENERATE_LENGTH = 128
TIME_QUANT_MS = 10   # must match your music_rnn.py

# ——— UTILITIES ——————————————————————————————————————————

def buffer_to_seed_tokens(buffer):
    if not buffer:
        return []
    tokens = []
    prev_t = buffer[0][0]
    for t, pitch, vel in buffer:
        delta_ms = (t - prev_t) * 1000.0
        steps = int(delta_ms / TIME_QUANT_MS + 0.5)
        tokens.extend([TOKEN2IDX[f"TIME_SHIFT_{TIME_QUANT_MS}ms"]] * steps)
        tok = f"NOTE_ON_{pitch}"
        if tok in TOKEN2IDX:
            tokens.append(TOKEN2IDX[tok])
        prev_t = t

    # trim to seq_len=200 (as in training)
    return tokens[-200:]

def token_stream_to_events(tokens, role, start_time):
    """
    Given a list of token-indices or token-strings, expand into
      [(event_time_sec, type, pitch, velocity), ...]
    """
    events = []
    t = start_time
    for item in tokens:
        # Allow either int indices or string tokens
        tok = IDX2TOKEN[item] if isinstance(item, int) else item

        if tok.startswith("TIME_SHIFT_"):
            # "TIME_SHIFT_10ms" → 10
            ms_part = tok[len("TIME_SHIFT_"):-2]  # strip prefix & "ms"
            t += int(ms_part) / 1000.0

        elif tok.startswith("NOTE_ON_"):
            # "NOTE_ON_60" → 60
            pitch = int(tok[len("NOTE_ON_"):])
            events.append((t, "note_on", pitch, 100))

        elif tok.startswith("NOTE_OFF_"):
            # "NOTE_OFF_60" → 60
            pitch = int(tok[len("NOTE_OFF_"):])
            events.append((t, "note_off", pitch, 0))

    return events



def normalize_key_name(raw_key: str) -> str:
    """
    Convert Detected keys like:
      - "C major"    → "c"
      - "c minor"    → "cmin"
      - "C# major"   → "c#"
      - "D# minor"   → "d#min"
      - "Eb major"   → "eb"
    """
    k = raw_key.strip().lower()
    parts = k.split()
    if len(parts) == 2:
        tonic, quality = parts
        # normalize synonyms
        if quality in ("major", "maj"):
            return tonic
        elif quality in ("minor", "min"):
            return tonic + "min"
    # fallback: lowercase, no spaces
    return k.replace(" ", "")

# ——— MAIN ASYNC LOOP —————————————————————————————

async def run():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # 1) Load checkpoint (with key2idx) before building the model
    ckpt = torch.load(MODEL_PATH, map_location=device)
    saved_keys = ckpt.get("key2idx", None)
    if saved_keys is None:
        raise RuntimeError("Checkpoint missing 'key2idx'; re-save your model with key2idx included.")
    # Update the globals in music_rnn
    KEY2IDX.clear(); KEY2IDX.update(saved_keys)
    IDX2KEY.clear(); IDX2KEY.update({v:k for k,v in saved_keys.items()})
    print(f"🔑 Restored {len(KEY2IDX)} keys from checkpoint")

    # 2) Instantiate with correct num_keys
    model = MusicRNN(
        vocab_size = len(TOKEN2IDX),
        num_roles  = len(ROLES),
        num_keys   = len(KEY2IDX)
    ).to(device)
    model.load_state_dict(ckpt["model_state"])
    model.eval()
    print("✅ Loaded RNN checkpoint")

    keydet       = KeyDetector()
    buffer       = []    # (t_sec, pitch, vel)
    pending_offs = []    # (send_time_sec, role, pitch)
    last_gen     = time.time()
    current_key  = None

    async with websockets.connect(WS_URI_IN) as ws_in, \
               websockets.connect(WS_URI_OUT) as ws_out:
        print("🎶 Connected to WS in/out")
        async for msg in ws_in:
            evt = json.loads(msg)
            now = time.time()

            # flush pending note_offs
            due = [off for off in pending_offs if off[0] <= now]
            pending_offs[:] = [off for off in pending_offs if off[0] > now]
            for send_t, role, pitch in due:
                await ws_out.send(json.dumps({
                    "type":      "note_off",
                    "role":      role,
                    "note":      pitch,
                    "velocity":  0,
                    "timestamp": int(send_t * 1000)
                }))

            # key detection
            keydet.feed_event(evt)
            det = keydet.estimate_key()
            if det:
                norm = normalize_key_name(det)
                if norm in KEY2IDX:
                    current_key = norm
                    print(f"🎹 Key → {det}  (normalized to '{current_key}')")
                else:
                    print(f"⚠️ Detected key '{det}' normalized to '{norm}', which is not in model keys")


            # buffer user note_on
            if evt.get("type") == "note_on":
                buffer.append((
                    evt["timestamp"] / 1000.0,
                    evt["note"],
                    evt["velocity"]
                ))

            # wait until we have a valid key
            if current_key is None or current_key not in KEY2IDX:
                continue

            # generate every second
            if now - last_gen >= 1.0:
                last_gen = now

                seed = buffer_to_seed_tokens(buffer)
                if not seed:
                    continue
                for role in ROLES:
                    if role == evt["role"]:
                        continue

                    tok_idxs = generate(
                        model,
                        seed,
                        role,
                        current_key,
                        GENERATE_LENGTH,
                        device=device,
                        temp=1.0
                    )
                    evs = token_stream_to_events(tok_idxs, role, start_time=now)
                    for t, typ, pitch, vel in evs:
                        await ws_out.send(json.dumps({
                            "type":      typ,
                            "role":      role,
                            "note":      pitch,
                            "velocity":  vel,
                            "timestamp": int(t * 1000)
                        }))
                        if typ == "note_on":
                            pending_offs.append((t + 0.1, role, pitch))

                buffer.clear()

if __name__ == "__main__":
    asyncio.run(run())

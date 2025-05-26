# data/quantize.py

import os, math, numpy as np, pretty_midi
from utils import parse_filename

STEPS_PER_QUARTER = 16
SPLIT_DIR  = os.path.join(os.path.dirname(__file__), "split_midis")
QUANT_DIR  = os.path.join(os.path.dirname(__file__), "quantized")

def quantize(path, tempo):
    """
    Given a MIDI path and its BPM, return a piano‐roll array (T,128).
    """
    pm = pretty_midi.PrettyMIDI(path)
    quarter_sec = 60.0 / tempo
    step_sec    = quarter_sec / STEPS_PER_QUARTER
    T = math.ceil(pm.get_end_time() / step_sec)
    roll = np.zeros((T,128), dtype=np.int16)

    for inst in pm.instruments:
        for note in inst.notes:
            s = max(0, min(T-1, int(note.start/step_sec)))
            e = max(0, min(T,   int(note.end  /step_sec)+1))
            roll[s:e, note.pitch] = note.velocity
    return roll

def quantize_all():
    for role in os.listdir(SPLIT_DIR):
        role_in  = os.path.join(SPLIT_DIR, role)
        role_out = os.path.join(QUANT_DIR, role)
        os.makedirs(role_out, exist_ok=True)

        for fn in os.listdir(role_in):
            if not fn.lower().endswith((".mid",".midi")):
                continue
            info = parse_filename(fn)
            if not info:
                print(f"⚠️ {fn}: parse failed")
                continue
            base, _, tempo, key = info
            src = os.path.join(role_in, fn)
            print(f"⏱ Quantizing {role}/{fn} @ {tempo} BPM")
            roll = quantize(src, tempo)
            out_path = os.path.join(role_out, os.path.splitext(fn)[0] + ".npz")
            np.savez_compressed(out_path, roll=roll)

if __name__ == "__main__":
    quantize_all()

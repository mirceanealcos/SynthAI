# music_rnn.py
# =============
# A role- and key-conditioned event RNN that
#  • Recursively finds every .mid under midi_folder
#  • Infers role = parent folder name (bass/lead/pad/pluck)
#  • Infers key from “_key_<name>.mid” in the filename
#  • Trains one LSTM on all roles+keys (or resumes training)
#  • Saves/loads a checkpoint (including key2idx)
#  • Generates new tokens for any role+key

import os, glob, re
import mido
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader

# ──────────────────────────────────────────────────────────────────────────────
# 1) Roles & Vocabulary
# ──────────────────────────────────────────────────────────────────────────────
ROLES     = ["bass", "lead", "pad", "pluck"]
ROLE2IDX  = {r:i for i,r in enumerate(ROLES)}

NOTE_ON    = [f"NOTE_ON_{i}"       for i in range(128)]
NOTE_OFF   = [f"NOTE_OFF_{i}"      for i in range(128)]
TIME_SHIFT = [f"TIME_SHIFT_{i*10}ms" for i in range(101)]  # 0..1000ms in 10ms steps

VOCAB      = NOTE_ON + NOTE_OFF + TIME_SHIFT
TOKEN2IDX  = {tok:idx for idx,tok in enumerate(VOCAB)}
IDX2TOKEN  = {idx:tok for tok,idx in TOKEN2IDX.items()}

# These will be filled once we scan keys
KEY2IDX = {}
IDX2KEY = {}

# ──────────────────────────────────────────────────────────────────────────────
# 2) MIDI → tokens
# ──────────────────────────────────────────────────────────────────────────────
def midi_to_events(path, time_quant=10):
    mid    = mido.MidiFile(path)
    events = []
    for msg in mid:
        dt_ms = msg.time * 1000.0
        if dt_ms > 0:
            steps = int(dt_ms / time_quant + 0.5)
            events.extend([f"TIME_SHIFT_{time_quant}ms"] * steps)
        if msg.type == "note_on" and msg.velocity > 0:
            events.append(f"NOTE_ON_{msg.note}")
        elif msg.type == "note_off" or (msg.type=="note_on" and msg.velocity==0):
            events.append(f"NOTE_OFF_{msg.note}")
    return events

# ──────────────────────────────────────────────────────────────────────────────
# 3) Dataset & DataLoader
# ──────────────────────────────────────────────────────────────────────────────
class EventDataset(Dataset):
    def __init__(self, midi_folder, seq_len=200, time_quant=10):
        self.seq_len    = seq_len
        self.time_quant = time_quant
        self.examples   = []  # (inp_idxs, tgt_idxs, role_idx, key_idx)

        # 1) find all .mid
        pattern = os.path.join(midi_folder, "**", "*.mid")
        files   = glob.glob(pattern, recursive=True)
        print(f"🔍 Scanning {len(files)} files under '{midi_folder}'", flush=True)

        # 2) collect all unique keys
        keys = set()
        info_list = []
        for path in files:
            role = os.path.basename(os.path.dirname(path)).lower()
            if role not in ROLES:
                continue
            m = re.search(r"_key_([^.]+)\.mid$", path, re.IGNORECASE)
            if not m:
                continue
            key = m.group(1).lower()
            keys.add(key)
            info_list.append((path, role, key))

        global KEY2IDX, IDX2KEY
        KEY2IDX = {k:i for i,k in enumerate(sorted(keys))}
        IDX2KEY = {i:k for k,i in KEY2IDX.items()}
        print(f"  • Roles found: {ROLES}", flush=True)
        print(f"  • Keys found:  {sorted(keys)}", flush=True)

        # 3) tokenize & slice windows
        for path, role, key in info_list:
            evs  = midi_to_events(path, time_quant=self.time_quant)
            idxs = [TOKEN2IDX[e] for e in evs if e in TOKEN2IDX]
            r_i  = ROLE2IDX[role]
            k_i  = KEY2IDX[key]
            for i in range(len(idxs) - self.seq_len):
                inp = idxs[i : i + self.seq_len]
                tgt = idxs[i + 1 : i + self.seq_len + 1]
                self.examples.append((inp, tgt, r_i, k_i))

        print(f"🗂 Total windows: {len(self.examples)}", flush=True)

    def __len__(self):
        return len(self.examples)

    def __getitem__(self, idx):
        inp, tgt, r, k = self.examples[idx]
        return (
            torch.tensor(inp, dtype=torch.long),
            torch.tensor(tgt, dtype=torch.long),
            torch.tensor(r,   dtype=torch.long),
            torch.tensor(k,   dtype=torch.long)
        )

def get_dataloader(midi_folder, seq_len=200, batch_size=64,
                   time_quant=10, shuffle=True):
    ds = EventDataset(midi_folder, seq_len, time_quant)
    return DataLoader(ds, batch_size=batch_size,
                      shuffle=shuffle, drop_last=True)

# ──────────────────────────────────────────────────────────────────────────────
# 4) Role+Key-Conditioned RNN Model
# ──────────────────────────────────────────────────────────────────────────────
class MusicRNN(nn.Module):
    def __init__(self, vocab_size, num_roles, num_keys,
                 embed_dim=128, role_dim=32, key_dim=16,
                 hidden_dim=256, num_layers=2, dropout=0.2):
        super().__init__()
        self.embed      = nn.Embedding(vocab_size, embed_dim)
        self.role_embed = nn.Embedding(num_roles, role_dim)
        self.key_embed  = nn.Embedding(num_keys,  key_dim)
        self.lstm       = nn.LSTM(embed_dim + role_dim + key_dim,
                                  hidden_dim,
                                  num_layers=num_layers,
                                  dropout=dropout,
                                  batch_first=True)
        self.fc         = nn.Linear(hidden_dim, vocab_size)

    def forward(self, x, r_idx, k_idx, hidden=None):
        b, s = x.size()
        be = self.embed(x)  # (b, s, embed_dim)
        re = self.role_embed(r_idx).unsqueeze(1).expand(b, s, -1)
        ke = self.key_embed(k_idx).unsqueeze(1).expand(b, s, -1)
        inp = torch.cat([be, re, ke], dim=-1)
        out, h = self.lstm(inp, hidden)
        logits = self.fc(out)
        return logits, h

# ──────────────────────────────────────────────────────────────────────────────
# 5) Training & Generation
# ──────────────────────────────────────────────────────────────────────────────
def train(model, loader, device, epochs=20, lr=1e-3, clip=1.0):
    model.to(device)
    opt = torch.optim.Adam(model.parameters(), lr=lr)
    ce  = nn.CrossEntropyLoss()
    for ep in range(1, epochs+1):
        model.train()
        tot = 0.0
        for inp, tgt, r, k in loader:
            inp, tgt = inp.to(device), tgt.to(device)
            r, k     = r.to(device),   k.to(device)
            opt.zero_grad()
            logits, _ = model(inp, r, k)
            b, s, v   = logits.size()
            loss      = ce(logits.view(b*s, v), tgt.view(b*s))
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), clip)
            opt.step()
            tot += loss.item()
        print(f"Epoch {ep}/{epochs} – Loss: {tot/len(loader):.4f}", flush=True)

def generate(model, seed, role, key, length, device, temp=1.0):
    model.to(device).eval()
    r_idx = torch.tensor([ROLE2IDX[role]], dtype=torch.long, device=device)
    k_idx = torch.tensor([KEY2IDX[key]],   dtype=torch.long, device=device)
    inp   = torch.tensor([seed],           dtype=torch.long, device=device)
    hidden= None
    with torch.no_grad():
        _, hidden = model(inp, r_idx, k_idx)
    last = inp[:, -1:].long()
    out  = []
    for _ in range(length):
        logits, hidden = model(last, r_idx, k_idx, hidden)
        vec   = logits[:, -1, :] / temp
        probs = torch.softmax(vec, dim=-1)
        idx   = torch.multinomial(probs, num_samples=1)
        out.append(idx.item())
        last = idx
    return [IDX2TOKEN[i] for i in out]

# ──────────────────────────────────────────────────────────────────────────────
# 6) __main__: load/train/resume & demo
# ──────────────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    import argparse

    p = argparse.ArgumentParser()
    p.add_argument("--midi_folder", type=str, required=True,
                   help="Root folder containing bass/, lead/, pad/, pluck/")
    p.add_argument("--seq_len",     type=int, default=200)
    p.add_argument("--batch_size",  type=int, default=64)
    p.add_argument("--epochs",      type=int, default=20)
    p.add_argument("--device",      type=str,
                   default="cuda" if torch.cuda.is_available() else "cpu")
    p.add_argument("--model_path",  type=str, default="music_rnn.pt")
    p.add_argument("--train",       action="store_true",
                   help="Resume training if checkpoint exists, or train from scratch")
    args = p.parse_args()

    # 1) load data
    loader = get_dataloader(args.midi_folder,
                            seq_len=args.seq_len,
                            batch_size=args.batch_size)
    print(f"✔️  Roles: {ROLES}", flush=True)
    print(f"✔️  Keys:  {sorted(KEY2IDX.keys())}", flush=True)

    # 2) instantiate model with the discovered key count
    model = MusicRNN(
        vocab_size = len(VOCAB),
        num_roles  = len(ROLES),
        num_keys   = len(KEY2IDX)
    ).to(args.device)

    # 3) load checkpoint if present
    if os.path.exists(args.model_path):
        print("🔄 Loading checkpoint …", flush=True)
        ck = torch.load(args.model_path, map_location=args.device)
        model.load_state_dict(ck["model_state"])
        if "key2idx" in ck:
            KEY2IDX.clear(); KEY2IDX.update(ck["key2idx"])
            IDX2KEY.clear(); IDX2KEY.update({v:k for k,v in ck["key2idx"].items()})
        print("✅ Checkpoint loaded", flush=True)

    # 4) train if requested or if no checkpoint found
    if args.train or not os.path.exists(args.model_path):
        print("🏋️‍♂️  Training …", flush=True)
        train(model, loader, args.device, epochs=args.epochs)
        torch.save({
            "model_state": model.state_dict(),
            "key2idx":     KEY2IDX
        }, args.model_path)
        print(f"💾 Model saved to {args.model_path}", flush=True)
    else:
        print("⏭️  Skipping training", flush=True)

    # 5) demo generation
    print("🎹 Demo generation:", flush=True)
    seed = [TOKEN2IDX["TIME_SHIFT_10ms"]] * 10
    for role in ROLES:
        for key in sorted(KEY2IDX.keys())[:3]:  # show first 3 keys as example
            toks = generate(model, seed, role, key, length=30, device=args.device)
            print(f"  {role}/{key}: {toks[:8]} …", flush=True)

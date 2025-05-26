import os, argparse
import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader
from data.utils import parse_filename

# 1. Dataset definition
class MidiDataset(Dataset):
    def __init__(self, npz_path):
        data = np.load(npz_path)
        self.X_user = data['X_user']      # (N, SEQ_LEN, 128)
        self.X_key  = data['X_key']       # (N,)
        self.X_role = data['X_role']      # (N,)
        Y = data['Y']                     # (N, 3, SEQ_LEN, 128)
        # reshape to (N, SEQ_LEN, 3*128) for simplicity
        self.Y = Y.transpose(0,2,1,3).reshape(Y.shape[0], Y.shape[2], -1)

    def __len__(self):
        return self.X_user.shape[0]

    def __getitem__(self, idx):
        return (
            torch.from_numpy(self.X_user[idx]).float(),
            torch.tensor(self.X_key[idx], dtype=torch.long),
            torch.tensor(self.X_role[idx], dtype=torch.long),
            torch.from_numpy(self.Y[idx]).float(),
        )

# 2. Model definition
class ComposerModel(nn.Module):
    def __init__(self, key_vocab, role_vocab, hidden_dim=256, key_emb=16, role_emb=16):
        super().__init__()
        self.key_emb  = nn.Embedding(key_vocab, key_emb)
        self.role_emb = nn.Embedding(role_vocab, role_emb)
        self.lstm     = nn.LSTM(128 + key_emb + role_emb, hidden_dim, num_layers=2, batch_first=True)
        self.fc       = nn.Linear(hidden_dim, 3*128)

    def forward(self, usr, key, role):
        B, T, _ = usr.size()
        ke = self.key_emb(key).unsqueeze(1).expand(-1,T,-1)
        re = self.role_emb(role).unsqueeze(1).expand(-1,T,-1)
        x = torch.cat([usr, ke, re], dim=-1)
        out, _ = self.lstm(x)
        return self.fc(out)

# 3. Training loop
def train_loop(dataset_path, epochs, batch_size, lr):
    ds = MidiDataset(dataset_path)
    loader = DataLoader(ds, batch_size=batch_size, shuffle=True, pin_memory=True)
    key_vocab = int(ds.X_key.max().item()) + 1
    role_vocab= int(ds.X_role.max().item()) + 1

    model = ComposerModel(key_vocab, role_vocab)
    opt   = torch.optim.Adam(model.parameters(), lr=lr)
    lossf = nn.MSELoss()

    model.train()
    for ep in range(1, epochs+1):
        total, count = 0.0, 0
        for usr, key, role, Y in loader:
            pred = model(usr, key, role)
            loss = lossf(pred, Y)
            opt.zero_grad()
            loss.backward()
            opt.step()
            total += loss.item() * usr.size(0)
            count += usr.size(0)
        print(f"Epoch {ep}/{epochs}  avg loss: {total/count:.4f}")

    os.makedirs("models", exist_ok=True)
    out = os.path.join("models", f"composer_{os.path.basename(dataset_path)}.pt")
    torch.save(model.state_dict(), out)
    print("Saved model to", out)

# 4. CLI
if __name__=="__main__":
    p = argparse.ArgumentParser()
    p.add_argument("dataset", help="Path to train_*.npz")
    p.add_argument("--epochs",   type=int, default=10)
    p.add_argument("--batch",    type=int, default=16)
    p.add_argument("--lr",       type=float, default=1e-3)
    args = p.parse_args()
    train_loop(args.dataset, args.epochs, args.batch, args.lr)

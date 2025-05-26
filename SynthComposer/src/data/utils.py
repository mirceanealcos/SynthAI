# data/utils.py
import re

FNAME_RE = re.compile(
    r"""
    ^(?P<base>.+?)_                              # song base
    (?P<role>bass\d*|lead\d*|pad\d*|pluck\d*)_   # role + optional index
    (?P<tempo>\d+)_bpm_key_                      # tempo
    (?P<tonic>[A-G][#b]?)(?P<mode>maj|major|min|minor)?  # tonic + optional mode
    \.(?:mid|midi)$                              # extension
    """,
    re.IGNORECASE | re.VERBOSE
)

def parse_filename(fn: str):
    m = FNAME_RE.match(fn)
    if not m:
        return None
    base  = m.group('base')
    role  = re.sub(r'\d+$','', m.group('role').lower())
    tempo = int(m.group('tempo'))
    tonic = m.group('tonic').upper().replace('B','♭').replace('#','♯')
    mode  = m.group('mode')
    mode  = 'minor' if mode and mode.lower().startswith('min') else 'major'
    return base, role, tempo, f"{tonic} {mode}"

import time
from collections import deque

from music21 import stream, note, analysis

class KeyDetector:
    """
    Real-time key detector using music21's Krumhansl–Schmuckler,
    with a long window and throttled updates for maximum accuracy.
    """

    def __init__(self,
                 window_size: float      = 5.0,
                 update_interval: float = 2.5):
        """
        window_size: how many seconds of past notes to analyze
        update_interval: how often (sec) to re-estimate the key
        """
        self.window        = window_size
        self.update_int    = update_interval
        self._last_time    = 0.0
        self._last_key     = None

        # music21 Stream holding Note objects with .offset in seconds
        self.stream    = stream.Stream()
        # the KS analyzer
        self.analyzer  = analysis.discrete.KrumhanslSchmuckler()

    def feed_event(self, event: dict):
        """
        Call on each MIDI event dict:
          - event["type"] == "note_on"
          - event["timestamp"] in ms
          - event["note"] MIDI pitch 0–127
        """
        if event.get("type") != "note_on":
            return

        t_sec = event["timestamp"] / 1000.0
        n = note.Note(event["note"])
        n.offset = t_sec
        self.stream.insert(t_sec, n)

        # Purge notes older than window_size
        cutoff = t_sec - self.window
        for old in list(self.stream.notes):
            if old.offset < cutoff:
                self.stream.remove(old)

    def estimate_key(self) -> str | None:
        """
        Re-estimate every update_interval seconds.
        Returns a string like "C major" or "A minor" when it changes,
        otherwise None.
        """
        now = time.time()
        if now - self._last_time < self.update_int:
            return None
        self._last_time = now

        # Need at least a handful of notes to make a call
        if len(self.stream.notes) < 5:
            return None

        try:
            key_obj = self.stream.analyze('KrumhanslSchmuckler')
        except Exception:
            return None

        key_name = f"{key_obj.tonic.name} {key_obj.mode}"
        if key_name != self._last_key:
            self._last_key = key_name
            return key_name
        return None

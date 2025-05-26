# key_detector.py

import math
import time
from collections import deque

class KeyDetector:
    """
    Real-time key detector with flush-on-switch and manual reset.
    """

    MAJOR_PROFILE = [6.35, 2.23, 3.48, 2.33, 4.38, 4.09,
                     2.52, 5.19, 2.39, 3.66, 2.29, 2.88]
    MINOR_PROFILE = [6.33, 2.68, 3.52, 5.38, 2.60, 3.53,
                     2.54, 4.75, 3.98, 2.69, 3.34, 3.17]
    NOTE_NAMES    = ['C', 'C#', 'D', 'D#', 'E', 'F',
                     'F#', 'G', 'G#', 'A', 'A#', 'B']

    def __init__(self,
                 window_size: float         = 4.0,
                 decay_half_life: float     = 2.0,
                 stability: int             = 3,
                 update_interval: float     = 0.25,
                 confidence_threshold: float= 0.3):
        """
        window_size           – seconds of history
        decay_half_life       – seconds for an event’s weight to halve
        stability             – consecutive agrees before switching
        update_interval       – sec between recomputations
        confidence_threshold  – min correlation (0–1) to accept
        """
        self.window     = window_size
        self.decay_rate = math.log(2) / decay_half_life
        self.stability  = stability
        self.update_int = update_interval
        self.conf_thresh= confidence_threshold

        # store note_on events: (time_sec, pitch_class, velocity)
        self.events = deque()

        # hysteresis / state
        self._last_time  = 0.0
        self._current_key= None
        self._pending_key= None
        self._stable_cnt = 0

    def feed(self, event: dict):
        """Add a note_on event; ignore others."""
        if event.get("type") != "note_on":
            return
        t = event["timestamp"] / 1000
        pc = event["note"] % 12
        vel= event.get("velocity",1)/127.0

        self.events.append((t, pc, vel))
        cutoff = t - self.window
        while self.events and self.events[0][0] < cutoff:
            self.events.popleft()

    def reset(self):
        """Manually clear history and state."""
        self.events.clear()
        self._current_key = None
        self._pending_key = None
        self._stable_cnt  = 0
        self._last_time   = 0.0

    def estimate(self) -> str | None:
        """
        Return a new key only when:
          - enough time has passed (update_interval),
          - correlation ≥ confidence_threshold,
          - seen stability count,
        else None.
        On switch, flush history.
        """
        now = time.time()
        if now - self._last_time < self.update_int:
            return None
        self._last_time = now

        # build decayed histogram
        hist = [0.0]*12
        for t, pc, w in self.events:
            age = now - t
            hist[pc] += w * math.exp(-self.decay_rate * age)

        if sum(hist) == 0:
            return None

        # stats
        mean_h = sum(hist)/12
        var_h  = sum((h-mean_h)**2 for h in hist)

        best_score = -1.0
        best_key   = None

        # test both modes
        for mode, profile in (("major", self.MAJOR_PROFILE),
                              ("minor", self.MINOR_PROFILE)):
            for shift in range(12):
                prof = profile[shift:]+profile[:shift]
                mean_p = sum(prof)/12
                var_p  = sum((p-mean_p)**2 for p in prof)
                num = sum((h-mean_h)*(p-mean_p) for h,p in zip(hist, prof))
                den = math.sqrt(var_h * var_p)
                corr= num/den if den>0 else 0.0
                if corr>best_score:
                    best_score, best_key = corr, f"{self.NOTE_NAMES[shift]} {mode}"

        # enforce confidence
        if best_score < self.conf_thresh:
            return None

        # if same as current, reset pending
        if best_key == self._current_key:
            self._pending_key = None
            self._stable_cnt  = 0
            return None

        # hysteresis
        if best_key == self._pending_key:
            self._stable_cnt += 1
        else:
            self._pending_key = best_key
            self._stable_cnt  = 1

        if self._stable_cnt >= self.stability:
            # confirm switch
            self._current_key = self._pending_key
            # flush old events so new key can build fresh
            self.events.clear()
            # reset pending
            self._pending_key = None
            self._stable_cnt  = 0
            return self._current_key

        return None

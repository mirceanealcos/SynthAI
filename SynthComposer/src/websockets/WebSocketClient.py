import asyncio
import json
import websockets
import logging
from src.utils.KeyDetector import KeyDetector

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("midi-ws-client")

WS_URI = "ws://localhost:8080/user/input"

async def connect_and_listen():
    detector = KeyDetector(
        window_size=4.0,
        decay_half_life=2.0,
        confidence_threshold=0.3,
        stability=3,
        update_interval=0.25
    )
    lastKey = None
    while True:
        try:
            logger.info(f"Connecting to {WS_URI}…")
            async with websockets.connect(WS_URI) as ws:
                logger.info("Connected! Listening for MIDI events…")
                async for message in ws:
                    try:
                        event = json.loads(message)
                        detector.feed(event)
                        key = detector.estimate()
                        if key and key != lastKey:
                            logger.info(f"Detected key: {key}")
                            lastKey = key
                    except json.JSONDecodeError:
                        logger.warning("Received non-JSON message:", message)
        except (websockets.ConnectionClosedError, OSError) as e:
            logger.error(f"Connection lost: {e}")
            logger.info("Reconnecting in 2 seconds…")
            await asyncio.sleep(2)

if __name__ == "__main__":
    asyncio.run(connect_and_listen())

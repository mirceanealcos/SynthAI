import asyncio
import json
import websockets
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("midi-ws-client")

WS_URI = "ws://localhost:8080/user/input"

async def connect_and_listen():
    while True:
        try:
            logger.info(f"Connecting to {WS_URI}…")
            async with websockets.connect(WS_URI) as ws:
                logger.info("Connected! Listening for MIDI events…")
                async for message in ws:
                    try:
                        event = json.loads(message)
                        logger.info(f"MIDI event: {event}")
                        # TODO: pass event to key-detector/composer here
                    except json.JSONDecodeError:
                        logger.warning("Received non-JSON message:", message)
        except (websockets.ConnectionClosedError, OSError) as e:
            logger.error(f"Connection lost: {e}")
            logger.info("Reconnecting in 2 seconds…")
            await asyncio.sleep(2)

if __name__ == "__main__":
    asyncio.run(connect_and_listen())

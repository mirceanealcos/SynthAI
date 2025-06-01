import asyncio
import websockets
import json
import sys

WEBSOCKET_URL = "ws://localhost:8080/user/input"
ERROR_MESSAGE_COUNT = 5000
CONCURRENCY = 2
DELAY_BETWEEN_MESSAGES = 0.001


async def flood_type_errors(uri, message_count, delay, idx):
    async with websockets.connect(uri) as ws:
        print(f"[{idx}] Connected, sending type‐mismatch payloads")
        for i in range(message_count):
            bad_payload = {
                "note": f"bad_note_{i}",
                "velocity": f"bad_vel_{i}",
                "type": "note_on"
            }
            await ws.send(json.dumps(bad_payload))
            await asyncio.sleep(delay)
        print(f"[{idx}] Done sending {message_count} bad messages")


async def main():
    tasks = [
        asyncio.create_task(
            flood_type_errors(WEBSOCKET_URL, ERROR_MESSAGE_COUNT, DELAY_BETWEEN_MESSAGES, i + 1)
        )
        for i in range(CONCURRENCY)
    ]
    await asyncio.gather(*tasks)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Interrupted by user")
        sys.exit(0)

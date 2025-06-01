import asyncio
import websockets

WEBSOCKET_URL = "ws://localhost:8080/user/input"
MESSAGE_COUNT = 5000
CONCURRENCY = 2
DELAY_BETWEEN_MESSAGES = 0.001

async def flood_messages(uri, message_count, delay):
    async with websockets.connect(uri) as ws:
        print(f"[+] Connected to {uri}")
        for i in range(message_count):
            payload = {
                "note": 60,
                "velocity": 127,
                "type": "note_on"
            }
            await ws.send(json.dumps(payload))
            await asyncio.sleep(delay)
        print(f"[✓] Sent {message_count} messages on {uri}")

async def main():
    tasks = [
        flood_messages(WEBSOCKET_URL, MESSAGE_COUNT, DELAY_BETWEEN_MESSAGES)
        for _ in range(CONCURRENCY)
    ]
    await asyncio.gather(*tasks)

if __name__ == "__main__":
    import json, sys
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Interrupted by user, exiting.")
        sys.exit(0)

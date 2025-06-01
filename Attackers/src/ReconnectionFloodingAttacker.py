import asyncio
import websockets

WEBSOCKET_URL = "ws://localhost:8080/user/input"
TOTAL_RECONNECTS = 15
CONCURRENCY = 1
CONNECT_HOLD_TIME = 0.05
DELAY_BETWEEN_RECONNECTS = 0.1

async def reconnect_flood(uri, total_reconnects, hold_time, delay):

    for i in range(total_reconnects):
        try:
            ws = await websockets.connect(uri)
            await asyncio.sleep(hold_time)
            await ws.close()
            print(f"[{i+1}/{total_reconnects}] Reconnected and closed")
        except Exception as e:
            print(f"[!] Attempt {i+1} error: {e}")
        await asyncio.sleep(delay)

async def main():
    tasks = [
        asyncio.create_task(
            reconnect_flood(WEBSOCKET_URL, TOTAL_RECONNECTS, CONNECT_HOLD_TIME, DELAY_BETWEEN_RECONNECTS)
        )
        for _ in range(CONCURRENCY)
    ]
    await asyncio.gather(*tasks)

if __name__ == "__main__":
    import json, sys
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Interrupted by user")
        sys.exit(0)

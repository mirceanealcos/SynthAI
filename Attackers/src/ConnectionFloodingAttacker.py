import asyncio
import websockets

WEBSOCKET_URL           = "ws://localhost:8080/user/input"
TOTAL_CONNECTIONS       = 500
CONNECTION_BATCH_SIZE   = 50
DELAY_BETWEEN_BATCHES   = 0.5
CONNECTION_LIFETIME     = 60

async def hold_connection(uri, lifetime, idx):
    try:
        async with websockets.connect(uri) as ws:
            print(f"[{idx:04d}] Connected")
            await asyncio.sleep(lifetime)
            print(f"[{idx:04d}] Closing after {lifetime}s")
    except Exception as e:
        print(f"[{idx:04d}] Error: {e}")

async def main():
    tasks = []
    for i in range(TOTAL_CONNECTIONS):
        tasks.append(asyncio.create_task(hold_connection(
            WEBSOCKET_URL, CONNECTION_LIFETIME, i+1
        )))
        if (i + 1) % CONNECTION_BATCH_SIZE == 0:
            await asyncio.sleep(DELAY_BETWEEN_BATCHES)

    await asyncio.gather(*tasks)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Interrupted by user")

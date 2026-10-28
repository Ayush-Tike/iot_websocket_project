
import asyncio
import json
import websockets

clients = set()


async def handler(websocket):
    clients.add(websocket)
    print(f"Client connected. Total: {len(clients)}")

    try:
        async for message in websocket:
            try:
                data = json.loads(message)

                if (
                    "temperature" not in data
                    or "humidity" not in data
                ):
                    continue

                print(
                    f"Temperature: {data['temperature']} °C | "
                    f"Humidity: {data['humidity']} %"
                )

                # Share the readings with every other client.
                payload = json.dumps(data)
                for client in list(clients):
                    if client != websocket:
                        try:
                            await client.send(payload)
                        except websockets.exceptions.ConnectionClosed:
                            pass

            except (json.JSONDecodeError, TypeError):
                print("Invalid sensor message:", message)

    except websockets.exceptions.ConnectionClosed:
        pass

    finally:
        clients.discard(websocket)
        print(f"Client disconnected. Total: {len(clients)}")


async def main():
    async with websockets.serve(
        handler,
        "0.0.0.0",
        8765
    ):
        print("WebSocket server running on port 8765")
        print("Waiting for ESP32 and browser connections...")
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())

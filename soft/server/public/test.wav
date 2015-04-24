#!/usr/bin/env python

import sys
import asyncio
import websockets

@asyncio.coroutine
def hello():
    websocket = yield from websockets.connect('ws://localhost:9000/streaming')
    f = open("test.wav", "rb")
    try:
        byte = f.read(48)
        byte = f.read(16)
        while byte:
            yield from websocket.send(byte)
            byte = f.read(16)
    finally:
        f.close()
    print ("Start receiving")
    f = open("receive.wav", "wb")
    try:
        data = yield from websocket.recv()
        while data:
            f.write(data)
            data = yield from websocket.recv()
    finally:
        f.close()

asyncio.get_event_loop().run_until_complete(hello())

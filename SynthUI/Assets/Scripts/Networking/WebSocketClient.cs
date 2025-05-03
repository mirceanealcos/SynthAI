using UnityEngine;
using NativeWebSocket;
using System;
using System.Threading.Tasks;

public class WebSocketClient
{
    private WebSocket websocket;
    public event Action<byte[]> OnBinaryMessageReceived;
    
    public WebSocketClient(string uri)
    {
        websocket = new WebSocket(uri);

        websocket.OnOpen += () => UnityEngine.Debug.Log("WebSocket Opened");
        websocket.OnError += (e) => UnityEngine.Debug.Log("WebSocket Error: " + e);
        websocket.OnClose += (e) => UnityEngine.Debug.Log("WebSocket Closed: " + e);
        websocket.OnMessage += (bytes) =>
        {
            OnBinaryMessageReceived?.Invoke(bytes);
        };
    }

     public async Task Connect()
    {
        await websocket.Connect();
    }

    public void Dispatch()
    {
        websocket.DispatchMessageQueue();
    }

    public async void Close()
    {
        await websocket.Close();
    }

}

using System;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using UnityEngine;

[Serializable]
public class PresetMessage {
    public string preset;
}

public class PresetWebSocketClient : MonoBehaviour
{
    public string host = "localhost";
    public int port = 8080;
    public string endpoint = "/user/preset";
    
    private ClientWebSocket ws;
    private CancellationTokenSource cts;

    async void Start()
    {
        ws = new ClientWebSocket();
        cts = new CancellationTokenSource();
        var uri = new Uri($"ws://{host}:{port}{endpoint}");
        try {
            await ws.ConnectAsync(uri, cts.Token);
            _ = ReceiveLoop();
        }
        catch (Exception e) {
            Debug.LogError("WebSocket connect failed: " + e);
        }
    }

    async Task ReceiveLoop()
    {
        var buffer = new ArraySegment<byte>(new byte[1024]);
        try
        {
            while (ws.State == WebSocketState.Open)
            {
                var result = await ws.ReceiveAsync(buffer, cts.Token);
                if (result.MessageType == WebSocketMessageType.Close)
                {
                    await ws.CloseAsync(WebSocketCloseStatus.NormalClosure, 
                                        "Closing", cts.Token);
                    Debug.Log("WebSocket closed by server.");
                }
                else
                {
                    var msg = Encoding.UTF8.GetString(buffer.Array, 0, result.Count);
                    Debug.Log($"[WS recv] {msg}");
                }
            }
        }
        catch (Exception e)
        {
            Debug.LogError("WebSocket receive error: " + e);
        }
    }

    public async void SendPreset(string presetName)
    {
        if (ws?.State != WebSocketState.Open)
        {
            Debug.LogWarning("WebSocket not open, cannot send.");
            return;
        }

        var pm = new PresetMessage { preset = presetName };
        var json = JsonUtility.ToJson(pm);
        var bytes = Encoding.UTF8.GetBytes(json);
        var seg   = new ArraySegment<byte>(bytes);

        try
        {
            await ws.SendAsync(seg, WebSocketMessageType.Text, true, cts.Token);
            // Debug.Log($"[WS send] {json}");
        }
        catch (Exception e)
        {
            Debug.LogError("WebSocket send error: " + e);
        }
    }

    void OnDestroy()
    {
        cts.Cancel();
        ws?.Dispose();
    }

    void Update()
    {
        
    }
}

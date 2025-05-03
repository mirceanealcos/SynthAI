using UnityEngine;

public class AudioStreamManager : MonoBehaviour
{
    private WebSocketClient wsClient;
    private OpusDecoderWrapper decoder;
    private AudioPlayerStream audioPlayer;

    [SerializeField] private string websocketUrl = "ws://localhost:8080/user/audio";

    async void Start()
    {
        decoder = new OpusDecoderWrapper(48000, 2);
        audioPlayer = new AudioPlayerStream(48000, 2);

        wsClient = new WebSocketClient(websocketUrl);
        wsClient.OnBinaryMessageReceived += OnAudioDataReceived;

        await wsClient.Connect();
    }

    void OnAudioDataReceived(byte[] opusBytes)
    {
        Debug.Log("RECEIVED BYTES: " + opusBytes);
        var pcm = decoder.Decode(opusBytes);
        audioPlayer.Enqueue(pcm);
    }

    void Update()
    {
        wsClient?.Dispatch();
    }

    private void OnApplicationQuit()
    {
        wsClient?.Close();
    }
}

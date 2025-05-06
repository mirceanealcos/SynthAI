using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class AudioStreamManager : MonoBehaviour
{
    private WebSocketClient wsClient;
    private AudioPlayerStream audioPlayer;
    private OpusDecoderWrapper decoder;

    [SerializeField] private string websocketUrl = "ws://localhost:8080/user/audio";

    private Queue<float[]> decodeQueue = new Queue<float[]>();
    private int bufferedFrames = 0;

    void Start()
    {
        StartCoroutine(Initialize());
    }

    private IEnumerator Initialize()
    {
        GameObject playerGO = new GameObject("AudioStreamPlayer");
        audioPlayer = playerGO.AddComponent<AudioPlayerStream>();
        audioPlayer.Init(48000, 1, 48000); // 1 second buffer

        decoder = new OpusDecoderWrapper(48000, 1);

        var audioSource = playerGO.AddComponent<AudioSource>();
        audioSource.spatialBlend = 0;
        audioSource.loop = true;
        audioSource.playOnAwake = true;
        audioSource.Play();

        DontDestroyOnLoad(playerGO);

        wsClient = new WebSocketClient(websocketUrl);
        wsClient.OnBinaryMessageReceived += OnAudioDataReceived;
        yield return wsClient.Connect();

        Debug.Log("✅ Starting PCM buffer loop...");
        StartCoroutine(BufferLoop());
    }

    void OnAudioDataReceived(byte[] bytes)
    {
        short[] decodedShorts = decoder.Decode(bytes);
        float[] decodedFloats = new float[decodedShorts.Length];

        for (int i = 0; i < decodedShorts.Length; i++)
            decodedFloats[i] = decodedShorts[i] / 32768f;

        lock (decodeQueue)
        {
            decodeQueue.Enqueue(decodedFloats);
        }
    }

    IEnumerator BufferLoop()
    {
        Debug.Log("🌀 BufferLoop started");
        while (true)
        {
            yield return new WaitForSeconds(0.01f);

            float[] block = null;
            lock (decodeQueue)
            {
                if (decodeQueue.Count > 0)
                    block = decodeQueue.Dequeue();
            }

            if (block != null)
            {
                if (audioPlayer.GetBufferedSampleCount() > 8192)
                {
                    Debug.Log("⚠️ Skipped block to avoid overbuffering");
                    continue;
                }

                audioPlayer.Enqueue(block);
                bufferedFrames++;

                if (bufferedFrames >= 4) // reduced latency
                {
                    Debug.Log("🟢 Unpausing playback");
                    audioPlayer.UnpausePlayback();
                }
            }
        }
    }

    void Update()
    {
        wsClient?.Dispatch();
    }

    void OnApplicationQuit()
    {
        wsClient?.Close();
    }
}

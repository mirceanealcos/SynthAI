using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System;

public class AudioStreamManager : MonoBehaviour
{
    private AudioPlayerStream audioPlayer;
    private UdpClient udpClient;
    private Thread receiveThread;

    private Queue<float[]> decodeQueue = new Queue<float[]>();
    private int bufferedFrames = 0;

    [SerializeField] private int udpPort = 9000;
    [SerializeField] private int sampleRate = 48000;
    [SerializeField] private int channels = 1;

    void Start()
    {
        // Log current DSP buffer configuration
        var cfg = AudioSettings.GetConfiguration();
        Debug.Log($"🎚 DSP buffer size: {cfg.dspBufferSize} samples");

        StartCoroutine(Initialize());
    }

    private IEnumerator Initialize()
    {
        // Setup audio player
        GameObject playerGO = new GameObject("AudioStreamPlayer");
        audioPlayer = playerGO.AddComponent<AudioPlayerStream>();
        audioPlayer.Init(sampleRate, channels, sampleRate); // 1 second buffer

        // Create and configure AudioSource
        var audioSource = playerGO.AddComponent<AudioSource>();
        audioSource.spatialBlend = 0;
        audioSource.loop = true;
        audioSource.playOnAwake = false; // start manually
        DontDestroyOnLoad(playerGO);

        // Setup UDP
        udpClient = new UdpClient(udpPort);
        udpClient.Client.ReceiveBufferSize = 1024 * 1024;

        // Start receive thread
        receiveThread = new Thread(ReceiveLoop) { IsBackground = true };
        receiveThread.Start();

        Debug.Log("✅ Starting UDP PCM buffer loop...");
        StartCoroutine(BufferLoop());
        yield return null;
    }

    void ReceiveLoop()
    {
        IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, udpPort);
        while (true)
        {
            try
            {
                byte[] data = udpClient.Receive(ref remoteEP);
                int floatCount = data.Length / 4;
                float[] samples = new float[floatCount];
                Buffer.BlockCopy(data, 0, samples, 0, data.Length);

                Debug.Log($"📥 Received frame: {floatCount} samples");
                lock (decodeQueue)
                {
                    decodeQueue.Enqueue(samples);
                }
            }
            catch (Exception ex)
            {
                Debug.LogError("UDP receive error: " + ex.Message);
            }
        }
    }

    IEnumerator BufferLoop()
    {
        Debug.Log("🌀 BufferLoop started");
        while (true)
        {
            yield return null; // minimal polling delay

            // Dequeue next block if available
            float[] block = null;
            lock (decodeQueue)
            {
                if (decodeQueue.Count > 0)
                    block = decodeQueue.Dequeue();
            }

            if (block == null)
                continue;

            Debug.Log($"▶ Dequeued block: {block.Length} samples");

            // Compute thresholds based on DSP buffer size
            int dspSize = AudioSettings.GetConfiguration().dspBufferSize;
            int slack = dspSize / 2;             // half-block slack (~256)
            int maxBuffered = dspSize * 3;       // allow up to ~3 blocks (~32ms)
            int startThreshold = dspSize + slack; // ~1.5 blocks (~768 samples ~16ms)

            int buffered = audioPlayer.GetBufferedSampleCount();

            // Drop new blocks if buffer is too full
            if (audioPlayer.IsPlaying() && buffered > maxBuffered)
            {
                Debug.Log($"⚠️ Skipping block — buffer too full: {buffered} samples");
                continue;
            }

            // Enqueue and track
            audioPlayer.Enqueue(block);
            bufferedFrames++;
            Debug.Log($"⏱ Enqueued block. Frames enqueued: {bufferedFrames}, Buffered samples: {audioPlayer.GetBufferedSampleCount()}");

            // Start playback once buffer has warmed sufficiently
            if (!audioPlayer.IsPlaying() && audioPlayer.GetBufferedSampleCount() >= startThreshold)
            {
                Debug.Log("🟢 Starting playback at warm buffer level");
                var source = audioPlayer.GetComponent<AudioSource>();
                source.Play();
                audioPlayer.UnpausePlayback();
            }
        }
    }

    void OnApplicationQuit()
    {
        receiveThread?.Abort();
        udpClient?.Close();
    }
}

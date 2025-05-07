using UnityEngine;
using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class AudioStreamManager : MonoBehaviour
{
    [SerializeField] int udpPort    = 9000;
    [SerializeField] int sampleRate = 48000;
    [SerializeField] int channels   = 2;

    private AudioPlayerStream audioPlayer;
    private UdpClient         udpClient;
    private Thread            receiveThread;

    // we’ll start playback after this many floats are buffered:
    private int startThresholdSamples;

    void Start()
    {
        // Compute warm-up threshold: 3 × Unity’s DSP buffer (per channel)
        int dspSize             = AudioSettings.GetConfiguration().dspBufferSize;  // e.g. 512
        startThresholdSamples   = dspSize * 3 * channels;                       // e.g. 512*3*2 = 3072 floats
        Debug.Log($"🎚 DSP buffer size: {dspSize} samples/ch → startThreshold = {startThresholdSamples} floats");

        // Create player GameObject
        var go = new GameObject("AudioStreamPlayer");
        audioPlayer = go.AddComponent<AudioPlayerStream>();
        audioPlayer.Init(sampleRate, channels, sampleRate);  // 1s buffer

        var src = go.AddComponent<AudioSource>();
        src.spatialBlend = 0;
        src.loop         = true;
        src.playOnAwake  = false;   // we’ll call Play() now, but audio doesn’t actually pass through until UnpausePlayback()
        src.Play();

        DontDestroyOnLoad(go);

        // UDP setup
        udpClient = new UdpClient(udpPort) {
            Client = { ReceiveBufferSize = 1024 * 1024 }
        };
        receiveThread = new Thread(ReceiveLoop) { IsBackground = true };
        receiveThread.Start();
    }

    void ReceiveLoop()
    {
        var endpoint = new IPEndPoint(IPAddress.Any, udpPort);
        int blockCount = 0;

        while (true)
        {
            try
            {
                byte[] data = udpClient.Receive(ref endpoint);
                int floatCount = data.Length / sizeof(float);
                var samples    = new float[floatCount];
                Buffer.BlockCopy(data, 0, samples, 0, data.Length);

                blockCount++;
                Debug.Log($"📥 Received block #{blockCount}: {floatCount} floats");

                // Direct write to ring buffer
                audioPlayer.WriteRawSamples(samples);

                int buffered = audioPlayer.GetBufferedSampleCount();
                Debug.Log($"   → Buffered after write: {buffered} floats");

                // Warm-up: unpause once we have enough
                if (!audioPlayer.IsPlaying() && buffered >= startThresholdSamples)
                {
                    Debug.Log($"🟢 Buffer warmed ({buffered} ≥ {startThresholdSamples}). Unpausing.");
                    audioPlayer.UnpausePlayback();
                }
            }
            catch (Exception ex)
            {
                Debug.LogError($"UDP receive error: {ex.Message}");
            }
        }
    }

    void OnApplicationQuit()
    {
        receiveThread?.Abort();
        udpClient?.Close();
    }
}

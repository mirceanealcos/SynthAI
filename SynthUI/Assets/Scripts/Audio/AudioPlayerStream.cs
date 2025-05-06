using UnityEngine;

public class AudioPlayerStream : MonoBehaviour
{
    private float[] ringBuffer;
    private int writePos = 0;
    private int readPos = 0;
    private int bufferSize;
    private int channels;
    private object lockObj = new object();
    private bool canPlay = false;
    private float lastSample = 0f;

    public void Init(int sampleRate, int channels, int bufferSizeOverride = -1)
{
    this.channels = channels;
    this.bufferSize = (bufferSizeOverride > 0)
        ? bufferSizeOverride
        : sampleRate * channels * 5; // default 5 seconds

    this.ringBuffer = new float[bufferSize];
}

    public void UnpausePlayback()
    {
        canPlay = true;
    }

    public int GetBufferedSampleCount()
    {
        lock (lockObj)
        {
            return (writePos - readPos + bufferSize) % bufferSize;
        }
    }

    public void Enqueue(float[] samples)
    {
        Debug.Log($"📤 Enqueuing {samples.Length} float samples");
        if (samples.Length >= 10)
{
    string firstSamples = string.Join(", ", samples[..10]);
    Debug.Log($"🧪 First 10 samples: {firstSamples}");
}
        lock (lockObj)
        {
            for (int i = 0; i < samples.Length; i++)
            {
                ringBuffer[writePos] = samples[i];
                writePos = (writePos + 1) % bufferSize;

                if (writePos == readPos)
                    readPos = (readPos + 1) % bufferSize;
            }
        }
    }

    public void OnAudioFilterRead(float[] data, int channels)
    {
        if (!canPlay)
        {
            for (int i = 0; i < data.Length; i++)
                data[i] = 0f;
            return;
        }

        lock (lockObj)
        {
            int available = (writePos - readPos + bufferSize) % bufferSize;
            int delta = available - data.Length;
            Debug.Log($"🔊 Unity requested: {data.Length} | Buffered: {available} | Delta: {delta} | Channels: {channels}");


            for (int i = 0; i < data.Length; i++)
            {
                if (readPos != writePos)
                {
                    data[i] = ringBuffer[readPos];
                    lastSample = data[i];
                    readPos = (readPos + 1) % bufferSize;
                }
                else
                {
                    data[i] = lastSample * 0.98f;
                    lastSample = data[i];
                }
            }
        }
    }
}

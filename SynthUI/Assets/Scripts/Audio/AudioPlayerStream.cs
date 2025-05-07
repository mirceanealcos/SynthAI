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
    private int minSafeBufferSamples = 512; // threshold for underrun warnings

    public void Init(int sampleRate, int channels, int bufferSizeOverride = -1)
    {
        this.channels = channels;
        this.bufferSize = (bufferSizeOverride > 0)
            ? bufferSizeOverride
            : sampleRate * channels * 5; // default 5 seconds buffer

        ringBuffer = new float[bufferSize];
    }

    public void UnpausePlayback()
    {
        canPlay = true;
    }

    public bool IsPlaying() => canPlay;

    public int GetBufferedSampleCount()
    {
        lock (lockObj)
        {
            return (writePos - readPos + bufferSize) % bufferSize;
        }
    }

    public void Enqueue(float[] samples)
    {
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
            // If not playing, output silence
            for (int i = 0; i < data.Length; i++)
                data[i] = 0f;
            return;
        }

        lock (lockObj)
        {
            int available = (writePos - readPos + bufferSize) % bufferSize;
            int delta = available - data.Length;
            Debug.Log($"🔊 Unity requested: {data.Length} | Buffered: {available} | Delta: {delta} | Channels: {channels}");

            if (available < minSafeBufferSamples)
                Debug.LogWarning($"⚠️ Buffer very low: {available} samples");

            for (int i = 0; i < data.Length; i++)
            {
                if (readPos != writePos)
                {
                    float raw = ringBuffer[readPos];
                    float sample;
                    if (i == 0)
                    {
                        // Crossfade first sample to avoid boundary pop
                        sample = (lastSample + raw) * 0.5f;
                    }
                    else
                    {
                        sample = raw;
                    }
                    data[i] = sample;
                    lastSample = sample;
                    readPos = (readPos + 1) % bufferSize;
                }
                else
                {
                    // Underrun: apply soft tailing
                    lastSample *= 0.95f;
                    data[i] = lastSample;
                }
            }
        }
    }
}

using UnityEngine;
using System;

public class AudioPlayerStream : MonoBehaviour
{
    private float[] ringBuffer;
    private int writePos, readPos, bufferSize, channels;
    private object lockObj = new object();
    private bool canPlay = false;
    private float lastSample = 0f;

    /// <summary>
    /// Must be called once before Play(): sampleRate in Hz, number of interleaved channels, total buffer size in floats.
    /// </summary>
    public void Init(int sampleRate, int channels, int bufferSizeOverride = -1)
    {
        this.channels = channels;
        bufferSize = (bufferSizeOverride > 0)
            ? bufferSizeOverride
            : sampleRate * channels * 5;  // default 5s
        ringBuffer = new float[bufferSize];
        writePos = readPos = 0;
    }

    /// <summary>
    /// Called by network thread to inject decoded PCM floats.
    /// </summary>
    public void WriteRawSamples(float[] samples)
    {
        lock (lockObj)
        {
            for (int i = 0; i < samples.Length; ++i)
            {
                ringBuffer[writePos] = samples[i];
                writePos = (writePos + 1) % bufferSize;
                if (writePos == readPos)
                    readPos = (readPos + channels) % bufferSize;  // drop oldest
            }
        }
    }

    public void UnpausePlayback() => canPlay = true;
    public bool IsPlaying()       => canPlay;

    /// <summary>
    /// Number of floats currently buffered.
    /// </summary>
    public int GetBufferedSampleCount()
    {
        lock (lockObj)
        {
            return (writePos - readPos + bufferSize) % bufferSize;
        }
    }

    /// <summary>
    /// Unity callback: fill the provided data[] (interleaved) with PCM floats.
    /// </summary>
    void OnAudioFilterRead(float[] data, int ch)
    {
    if (!canPlay)
    {
        Array.Clear(data, 0, data.Length);
        return;
    }

    lock (lockObj)
    {
        int available = GetBufferedSampleCount();
        int delta     = available - data.Length;
        // Debug.Log($"🔊 OnAudioFilterRead: req={data.Length} buf={available} delta={delta} ch={ch}");

        // if (available < data.Length)
            // Debug.LogWarning($"⚠️ Underrun: only {available}/{data.Length} samples buffered");

        for (int i = 0; i < data.Length; ++i)
        {
            if (available > 0)
            {
                data[i] = ringBuffer[readPos];
                readPos = (readPos + 1) % bufferSize;
                available--;
            }
            else
            {
                // underrun fallback: just output silence
                data[i] = 0f;
            }
        }
    }
    }
}

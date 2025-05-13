using UnityEngine;

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
            for (int i = 0; i < data.Length; ++i)
                data[i] = 0f;
            return;
        }

        lock (lockObj)
        {
            int available = GetBufferedSampleCount();
            int delta     = available - data.Length;
            // Debug.Log($"🔊 OnAudioFilterRead: requested {data.Length} | buffered {available} | delta {delta} | ch {ch}");

            // if (available < data.Length)
                // Debug.LogWarning($"⚠️ Underrun imminent: available {available} < need {data.Length}");

            // Peek next sample for interpolation if underflow
            float nextSample = available > 0
                ? ringBuffer[readPos]
                : lastSample;

            for (int i = 0; i < data.Length; ++i)
            {
                if (available > 1)
                {
                    float raw = ringBuffer[readPos];
                    float output = (i == 0)
                        ? 0.5f * (lastSample + raw)   // crossfade first sample
                        : raw;

                    data[i] = output;
                    lastSample = output;

                    readPos = (readPos + 1) % bufferSize;
                    available--;
                }
                else
                {
                    // Sub-block underrun: interpolate soft tail
                    float t = (data.Length > 1)
                        ? (float)i / (data.Length - 1)
                        : 0f;
                    float interp = Mathf.Lerp(lastSample, nextSample, t);
                    data[i] = interp;
                    lastSample = interp;
                }
            }
        }
    }
}

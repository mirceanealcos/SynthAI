using UnityEngine;
using System.Collections.Concurrent;

public class AudioPlayerStream : MonoBehaviour
{
    private ConcurrentQueue<short[]> queue = new ConcurrentQueue<short[]>();
    private float[] floatBuffer = new float[0];
    private int channels;

    public AudioPlayerStream(int sampleRate, int channels)
    {
        this.channels = channels;

        GameObject go = new GameObject("AudioStreamPlayer");
        go.AddComponent<AudioSource>();
        go.AddComponent<InternalStreamBehaviour>().Init(this, sampleRate, channels);
        GameObject.DontDestroyOnLoad(go);
    }

    public void Enqueue(short[] samples)
    {
        queue.Enqueue(samples);
    }

    private void FillBuffer(float[] data)
    {
        int offset = 0;
        while (offset < data.Length && queue.TryDequeue(out var block))
        {
            for (int i = 0; i < block.Length && offset < data.Length; i++)
                data[offset++] = block[i] / 32768f;
        }

        while (offset < data.Length)
            data[offset++] = 0f;
    }

    private class InternalStreamBehaviour : MonoBehaviour
    {
        private AudioPlayerStream parent;
        private AudioSource source;

        public void Init(AudioPlayerStream parent, int sampleRate, int channels)
        {
            this.parent = parent;
            source = GetComponent<AudioSource>();
            source.clip = AudioClip.Create("StreamClip", sampleRate * 10, channels, sampleRate, true, OnAudioRead);
            source.loop = true;
            source.Play();
            source.volume = 1.0f;
        }

        private void OnAudioRead(float[] data)
        {
            parent.FillBuffer(data);
        }
    }
}

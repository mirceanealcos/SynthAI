using Concentus.Structs;
using UnityEngine;

public class OpusDecoderWrapper
{
    private OpusDecoder decoder;
    private int channels;

    public OpusDecoderWrapper(int sampleRate, int channels)
    {
        this.channels = channels;
        decoder = OpusDecoder.Create(sampleRate, channels);
    }

    public short[] Decode(byte[] opusBytes)
    {
        short[] buffer = new short[1500 * channels]; // enough for max frame
        int decodedSamples = decoder.Decode(opusBytes, 0, opusBytes.Length, buffer, 0, buffer.Length, false);

        short[] output = new short[decodedSamples * channels];
        System.Array.Copy(buffer, output, output.Length);
        return output;
    }
}
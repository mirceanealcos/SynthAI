using Concentus.Structs;

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
        short[] pcm = new short[4096]; // Max for 1024 frames mono
        int decoded = decoder.Decode(opusBytes, 0, opusBytes.Length, pcm, 0, pcm.Length, false);

        short[] output = new short[decoded * channels];
        System.Array.Copy(pcm, output, output.Length);
        return output;
    }
}

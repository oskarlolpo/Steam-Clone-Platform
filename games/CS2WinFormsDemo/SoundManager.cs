using System;
using System.IO;
using System.Media;
using System.Threading.Tasks;

namespace CS2WinFormsDemo
{
    public static class SoundManager
    {
        private static SoundPlayer? _shootPlayer;
        private static SoundPlayer? _deaglePlayer;
        private static SoundPlayer? _hitPlayer;
        private static SoundPlayer? _reloadPlayer;
        private static SoundPlayer? _emptyPlayer;
        private static SoundPlayer? _headshotPlayer;
        private static SoundPlayer? _doorPlayer;
        private static SoundPlayer? _jumpPlayer;

        public static void Initialize()
        {
            Task.Run(() =>
            {
                try
                {
                    _shootPlayer = CreatePlayer(GenerateAk47Sound());
                    _deaglePlayer = CreatePlayer(GenerateDeagleSound());
                    _hitPlayer = CreatePlayer(GenerateHitSound());
                    _headshotPlayer = CreatePlayer(GenerateHeadshotSound());
                    _reloadPlayer = CreatePlayer(GenerateReloadSound());
                    _emptyPlayer = CreatePlayer(GenerateEmptySound());
                    _doorPlayer = CreatePlayer(GenerateDoorSound());
                    _jumpPlayer = CreatePlayer(GenerateJumpSound());
                }
                catch { }
            });
        }

        private static SoundPlayer CreatePlayer(byte[] wavData)
        {
            var ms = new MemoryStream(wavData);
            var player = new SoundPlayer(ms);
            player.Load();
            return player;
        }

        public static void PlayShoot()
        {
            Task.Run(() => { try { _shootPlayer?.Play(); } catch { } });
        }

        public static void PlayDeagle()
        {
            Task.Run(() => { try { _deaglePlayer?.Play(); } catch { } });
        }

        public static void PlayHit(bool headshot = false)
        {
            Task.Run(() =>
            {
                try
                {
                    if (headshot)
                        _headshotPlayer?.Play();
                    else
                        _hitPlayer?.Play();
                }
                catch { }
            });
        }

        public static void PlayReload()
        {
            Task.Run(() => { try { _reloadPlayer?.Play(); } catch { } });
        }

        public static void PlayEmpty()
        {
            Task.Run(() => { try { _emptyPlayer?.Play(); } catch { } });
        }

        public static void PlayDoor()
        {
            Task.Run(() => { try { _doorPlayer?.Play(); } catch { } });
        }

        public static void PlayJump()
        {
            Task.Run(() => { try { _jumpPlayer?.Play(); } catch { } });
        }

        private static byte[] GenerateAk47Sound()
        {
            int sampleRate = 22050;
            double duration = 0.35;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];
            var rng = new Random(42);

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double noise = (rng.NextDouble() * 2.0 - 1.0) * Math.Exp(-18.0 * t);
                double freq = 90.0 * Math.Exp(-8.0 * t) + 40.0;
                double punch = Math.Sin(2.0 * Math.PI * freq * t) * Math.Exp(-12.0 * t);

                double combined = (noise * 0.7 + punch * 0.8);
                combined = Math.Clamp(combined, -1.0, 1.0);
                samples[i] = (short)(combined * 30000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateDeagleSound()
        {
            int sampleRate = 22050;
            double duration = 0.4;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];
            var rng = new Random(1337);

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double noise = (rng.NextDouble() * 2.0 - 1.0) * Math.Exp(-14.0 * t);
                double freq = 120.0 * Math.Exp(-10.0 * t) + 50.0;
                double punch = Math.Sin(2.0 * Math.PI * freq * t) * Math.Exp(-9.0 * t);

                double combined = (noise * 0.6 + punch * 0.9);
                combined = Math.Clamp(combined, -1.0, 1.0);
                samples[i] = (short)(combined * 32000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateHitSound()
        {
            int sampleRate = 22050;
            double duration = 0.12;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double tone = Math.Sin(2.0 * Math.PI * 400.0 * t) * Math.Exp(-25.0 * t);
                samples[i] = (short)(tone * 20000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateHeadshotSound()
        {
            int sampleRate = 22050;
            double duration = 0.25;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double bell = (Math.Sin(2.0 * Math.PI * 1800.0 * t) * 0.6 +
                               Math.Sin(2.0 * Math.PI * 2400.0 * t) * 0.4) * Math.Exp(-15.0 * t);
                samples[i] = (short)(bell * 28000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateReloadSound()
        {
            int sampleRate = 22050;
            double duration = 0.5;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];
            var rng = new Random(777);

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double click1 = 0, click2 = 0;
                if (t >= 0.05 && t <= 0.12)
                {
                    double t1 = t - 0.05;
                    click1 = (rng.NextDouble() * 2.0 - 1.0) * Math.Exp(-40.0 * t1);
                }
                if (t >= 0.35 && t <= 0.45)
                {
                    double t2 = t - 0.35;
                    click2 = (rng.NextDouble() * 2.0 - 1.0) * Math.Exp(-45.0 * t2);
                }
                double val = Math.Clamp(click1 + click2, -1.0, 1.0);
                samples[i] = (short)(val * 24000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateEmptySound()
        {
            int sampleRate = 22050;
            double duration = 0.08;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double tone = Math.Sin(2.0 * Math.PI * 900.0 * t) * Math.Exp(-60.0 * t);
                samples[i] = (short)(tone * 18000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateDoorSound()
        {
            // Heavy mechanical sliding door sound
            int sampleRate = 22050;
            double duration = 0.5;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];
            var rng = new Random(888);

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double scrape = (rng.NextDouble() * 2.0 - 1.0) * 0.4;
                double hum = Math.Sin(2.0 * Math.PI * (110.0 + t * 40.0) * t) * 0.6;
                double env = Math.Sin(Math.PI * (t / duration));
                samples[i] = (short)((scrape + hum) * env * 22000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] GenerateJumpSound()
        {
            // Jump grunt / swoosh
            int sampleRate = 22050;
            double duration = 0.18;
            int totalSamples = (int)(sampleRate * duration);
            short[] samples = new short[totalSamples];

            for (int i = 0; i < totalSamples; i++)
            {
                double t = (double)i / sampleRate;
                double freq = 160.0 * (1.0 - t / duration) + 60.0;
                double tone = Math.Sin(2.0 * Math.PI * freq * t) * Math.Exp(-12.0 * t);
                samples[i] = (short)(tone * 20000);
            }
            return CreateWav(samples, sampleRate);
        }

        private static byte[] CreateWav(short[] samples, int sampleRate)
        {
            using var ms = new MemoryStream();
            using var bw = new BinaryWriter(ms);
            int byteRate = sampleRate * 2;

            bw.Write(System.Text.Encoding.ASCII.GetBytes("RIFF"));
            bw.Write(36 + samples.Length * 2);
            bw.Write(System.Text.Encoding.ASCII.GetBytes("WAVE"));
            bw.Write(System.Text.Encoding.ASCII.GetBytes("fmt "));
            bw.Write(16);
            bw.Write((short)1); // PCM
            bw.Write((short)1); // Mono
            bw.Write(sampleRate);
            bw.Write(byteRate);
            bw.Write((short)2); // BlockAlign
            bw.Write((short)16); // BitsPerSample
            bw.Write(System.Text.Encoding.ASCII.GetBytes("data"));
            bw.Write(samples.Length * 2);
            for (int i = 0; i < samples.Length; i++)
            {
                bw.Write(samples[i]);
            }
            return ms.ToArray();
        }
    }
}

# whisper.cpp

C/C++ port of [OpenAI's Whisper](https://github.com/openai/whisper) speech-to-text model

- Plain C/C++ implementation without dependencies
- ARM_NEON and AVX intrinsics support
- Mixed F16 / F32 support
- Low memory usage (Flash Attention + Flash Forward)
- Zero memory allocations at runtime

## Usage

To build the main program, run `make`. You can then transcribe a `.wav` file like this:

```bash
$ ./main -f input.wav
```

Before running the program, make sure to download one of the ggml Whisper models. For example:

```bash
bash ./download-ggml-model.sh base.en
```

---

For a quick demo, simply run `make base.en`:

```bash
$ make base.en

gcc -pthread -O3 -mavx -mavx2 -mfma -mf16c -c ggml.c
g++ -pthread -O3 -std=c++11 -c main.cpp
g++ -pthread -o main ggml.o main.o
./main -h

usage: ./main [options]

options:
  -h,       --help           show this help message and exit
  -s SEED,  --seed SEED      RNG seed (default: -1)
  -t N,     --threads N      number of threads to use during computation (default: 4)
  -v,       --verbose        verbose output
            --translate      translate from source language to english
  -ps,      --print_special  print special tokens
  -nt,      --no_timestamps  do not print timestamps
  -l LANG,  --language LANG  spoken language (default: en)
  -m FNAME, --model FNAME    model path (default: models/ggml-base.en.bin)
  -f FNAME, --file FNAME     input WAV file path (default: samples/jfk.wav)

bash ./download-ggml-model.sh base.en
Downloading ggml model base.en ...
Model base.en already exists. Skipping download.

===============================================
Running base.en on all samples in ./samples ...
===============================================

----------------------------------------------
[+] Running base.en on samples/jfk.wav ... (run 'ffplay samples/jfk.wav' to listen)
----------------------------------------------

whisper_model_load: loading model from 'models/ggml-base.en.bin'
whisper_model_load: n_vocab       = 51864
whisper_model_load: n_audio_ctx   = 1500
whisper_model_load: n_audio_state = 512
whisper_model_load: n_audio_head  = 8
whisper_model_load: n_audio_layer = 6
whisper_model_load: n_text_ctx    = 448
whisper_model_load: n_text_state  = 512
whisper_model_load: n_text_head   = 8
whisper_model_load: n_text_layer  = 6
whisper_model_load: n_mels        = 80
whisper_model_load: f16           = 1
whisper_model_load: type          = 2
whisper_model_load: mem_required  = 611.00 MB
whisper_model_load: adding 1607 extra tokens
whisper_model_load: ggml ctx size = 163.43 MB
whisper_model_load: memory size =    22.83 MB
whisper_model_load: model size  =   140.54 MB
log_mel_spectrogram: n_sample = 176000, n_len = 1100
log_mel_spectrogram: recording length: 11.000000 s

main: processing 176000 samples (11.0 sec), 4 threads, lang = english, task = transcribe, timestamps = 1 ...

[00:00.000 --> 00:11.000]   And so my fellow Americans ask not what your country can do for you. Ask what you can do for your country.


main:     load time =    61.78 ms
main:      mel time =    41.74 ms
main:   sample time =     2.10 ms
main:   encode time =   718.60 ms / 119.77 ms per layer
main:   decode time =    83.55 ms
main:    total time =   908.15 ms
```

The command downloads the `base.en` model converted to custom `ggml` format and runs the inference on all `.wav` samples in the folder `samples`.

If you want some extra audio samples to play with, simply run:

```
make samples
```

This will download a few more audio files from Wikipedia and convert them to 16-bit WAV format via `ffmpeg`.

You can download and run the other models as follows:

```
make tiny.en
make tiny
make base.en
make base
make small.en
make small
make medium.en
make medium
make large
```

For detailed usage instructions, run: `./main -h`

Note that `whisper.cpp` runs only with 16-bit WAV files, so make sure to convert your input before running the tool.
For example, you can use `ffmpeg` like this:

```bash
ffmpeg -i input.mp3 -ar 16000 -ac 1 -c:a pcm_s16le output.wav
```

Here is another example of transcribing a [3:24 min speech](https://upload.wikimedia.org/wikipedia/commons/1/1f/George_W_Bush_Columbia_FINAL.ogg) in less than a minute, using `medium.en` model:

```bash
$ ./main -m models/ggml-medium.en.bin -f samples/gb1.wav -t 8
whisper_model_load: loading model from 'models/ggml-medium.en.bin'
whisper_model_load: n_vocab       = 51864
whisper_model_load: n_audio_ctx   = 1500
whisper_model_load: n_audio_state = 1024
whisper_model_load: n_audio_head  = 16
whisper_model_load: n_audio_layer = 24
whisper_model_load: n_text_ctx    = 448
whisper_model_load: n_text_state  = 1024
whisper_model_load: n_text_head   = 16
whisper_model_load: n_text_layer  = 24
whisper_model_load: n_mels        = 80
whisper_model_load: f16           = 1
whisper_model_load: type          = 4
whisper_model_load: mem_required  = 2786.00 MB
whisper_model_load: adding 1607 extra tokens
whisper_model_load: ggml ctx size = 1644.97 MB
whisper_model_load: memory size =   182.62 MB
whisper_model_load: model size  =  1462.12 MB
log_mel_spectrogram: n_sample = 3179750, n_len = 19873
log_mel_spectrogram: recording length: 198.734375 s

main: processing 3179750 samples (198.7 sec), 8 threads, lang = english, task = transcribe, timestamps = 1 ...

[00:00.000 --> 00:08.000]   My fellow Americans, this day has brought terrible news and great sadness to our country.
[00:08.000 --> 00:17.000]   At 9 o'clock this morning, Mission Control in Houston lost contact with our Space Shuttle Columbia.
[00:17.000 --> 00:24.000]   A short time later, debris was seen falling from the skies above Texas.
[00:24.000 --> 00:29.000]   The Columbia's lost. There are no survivors.
[00:29.000 --> 00:32.000]   On board was a crew of seven.
[00:32.000 --> 00:43.000]   Colonel Rick Husband, Lieutenant Colonel Michael Anderson, Commander Laurel Clark, Captain David Brown, Commander William McCool,
[00:43.000 --> 00:52.000]   Dr. Kultner Aschavla, and Elon Ramon, a Colonel in the Israeli Air Force.
[00:52.000 --> 00:58.000]   These men and women assumed great risk in the service to all humanity.
[00:58.000 --> 01:06.000]   In an age when space flight has come to seem almost routine, it is easy to overlook the dangers of travel by rocket
[01:06.000 --> 01:12.000]   and the difficulties of navigating the fierce outer atmosphere of the Earth.
[01:12.000 --> 01:22.000]   These astronauts knew the dangers, and they faced them willingly, knowing they had a high and noble purpose in life.
[01:22.000 --> 01:30.000]   Because of their courage, endearing, and idealism, we will miss them all the more.
[01:30.000 --> 01:40.000]   All Americans today are thinking as well of the families of these men and women who have been given this sudden shock and grief.
[01:40.000 --> 01:45.000]   You're not alone. Our entire nation agrees with you.
[01:45.000 --> 01:52.000]   And those you love will always have the respect and gratitude of this country.
[01:52.000 --> 01:56.000]   The cause in which they died will continue.
[01:56.000 --> 02:07.000]   Mankind is led into the darkness beyond our world by the inspiration of discovery and the longing to understand.
[02:07.000 --> 02:11.000]   Our journey into space will go on.
[02:11.000 --> 02:16.000]   In the skies today, we saw destruction and tragedy.
[02:16.000 --> 02:22.000]   Yet farther than we can see, there is comfort and hope.
[02:22.000 --> 02:31.000]   In the words of the prophet Isaiah, "Lift your eyes and look to the heavens who created all these.
[02:31.000 --> 02:39.000]   He who brings out the starry hosts one by one and calls them each by name."
[02:39.000 --> 02:46.000]   Because of his great power and mighty strength, not one of them is missing.
[02:46.000 --> 02:55.000]   The same creator who names the stars also knows the names of the seven souls we mourn today.
[02:55.000 --> 03:05.000]   The crew of the shuttle Columbia did not return safely to Earth, yet we can pray that all are safely home.
[03:05.000 --> 03:14.000]   May God bless the grieving families and may God continue to bless America.
[03:14.000 --> 03:24.000]   [Music]


main:     load time =   438.55 ms
main:      mel time =   440.22 ms
main:   sample time =    32.23 ms
main:   encode time = 42329.63 ms / 1763.73 ms per layer
main:   decode time = 15190.00 ms
main:    total time = 58444.63 ms
```

## Limitations

- Very basic greedy sampling scheme - always pick up the top token
- Inference only
- Runs on the CPU
- Only mono-channel 16-bit WAV is supported

## Memory usage

| Model | Disk | Mem |
| ---   | --- | --- |
| tiny | 75 MB | ~460 MB |
| base | 142 MB | ~620 MB |
| small | 466 MB | ~1.3 GB |
| medium | 1.5 GB | ~2.8 GB |
| large | 2.9 GB | ~4.9 GB |

## ggml format

The original models are converted to a custom binary format. This allows to pack everything needed into a single file:

- model parameters
- mel filters
- vocabulary
- weights

You can download the converted models using the [download-ggml-model.sh](download-ggml-model.sh) script.

For more details, see the conversion script [convert-pt-to-ggml.py](convert-pt-to-ggml.py)

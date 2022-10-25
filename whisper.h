#ifndef WHISPER_H
#define WHISPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef WHISPER_SHARED
#    ifdef _WIN32
#        ifdef WHISPER_BUILD
#            define WHISPER_API __declspec(dllexport)
#        else
#            define WHISPER_API __declspec(dllimport)
#        endif
#    else
#        define WHISPER_API __attribute__ ((visibility ("default")))
#    endif
#else
#    define WHISPER_API
#endif

#define WHISPER_SAMPLE_RATE 16000
#define WHISPER_N_FFT       400
#define WHISPER_N_MEL       80
#define WHISPER_HOP_LENGTH  160
#define WHISPER_CHUNK_SIZE  30

#ifdef __cplusplus
extern "C" {
#endif

    //
    // C interface
    //
    // The following interface is thread-safe as long as the sample whisper_context is not used by multiple threads
    // concurrently.
    //
    // Basic usage:
    //
    //     #include "whisper.h"
    //
    //     ...
    //
    //     struct whisper_context * ctx = whisper_init("/path/to/ggml-base.en.bin");
    //
    //     if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
    //         fprintf(stderr, "failed to process audio\n");
    //         return 7;
    //     }
    //
    //     const int n_segments = whisper_full_n_segments(ctx);
    //     for (int i = 0; i < n_segments; ++i) {
    //         const char * text = whisper_full_get_segment_text(ctx, i);
    //         printf("%s", text);
    //     }
    //
    //     whisper_free(ctx);
    //
    //     ...
    //
    // This is a demonstration of the most straightforward usage of the library.
    // "pcmf32" contains the RAW audio data in 32-bit floating point format.
    //
    // The interface also allows for more fine-grained control over the computation, but it requires a deeper
    // understanding of how the model works.
    //

    struct whisper_context;

    typedef int whisper_token;

    // Allocates all memory needed for the model and loads the model from the given file.
    // Returns NULL on failure.
    WHISPER_API struct whisper_context * whisper_init(const char * path_model);

    // Frees all memory allocated by the model.
    WHISPER_API void whisper_free(struct whisper_context * ctx);

    // Convert RAW PCM audio to log mel spectrogram.
    // The resulting spectrogram is stored inside the provided whisper context.
    // Returns 0 on success
    WHISPER_API int whisper_pcm_to_mel(
            struct whisper_context * ctx,
            const float * samples,
            int n_samples,
            int n_threads);

    // This can be used to set a custom log mel spectrogram inside the provided whisper context.
    // Use this instead of whisper_pcm_to_mel() if you want to provide your own log mel spectrogram.
    // n_mel must be 80
    // Returns 0 on success
    WHISPER_API int whisper_set_mel(
            struct whisper_context * ctx,
            const float * data,
            int n_len,
            int n_mel);

    // Run the Whisper encoder on the log mel spectrogram stored inside the provided whisper context.
    // Make sure to call whisper_pcm_to_mel() or whisper_set_mel() first.
    // offset can be used to specify the offset of the first frame in the spectrogram.
    // Returns 0 on success
    WHISPER_API int whisper_encode(
            struct whisper_context * ctx,
            int offset,
            int n_threads);

    // Run the Whisper decoder to obtain the logits and probabilities for the next token.
    // Make sure to call whisper_encode() first.
    // tokens + n_tokens is the provided context for the decoder.
    // n_past is the number of tokens to use from previous decoder calls.
    // Returns 0 on success
    WHISPER_API int whisper_decode(
            struct whisper_context * ctx,
            const whisper_token * tokens,
            int n_tokens,
            int n_past,
            int n_threads);

    // Token sampling methods.
    // These are provided for convenience and can be used after each call to whisper_decode().
    // You can also implement your own sampling method using the whisper_get_probs() function.
    // whisper_sample_best() returns the token with the highest probability
    // whisper_sample_timestamp() returns the most probable timestamp token
    WHISPER_API whisper_token whisper_sample_best(struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_sample_timestamp(struct whisper_context * ctx);

    // Return the id of the specified language, returns -1 if not found
    WHISPER_API int whisper_lang_id(const char * lang);

    WHISPER_API int whisper_n_len          (struct whisper_context * ctx); // mel length
    WHISPER_API int whisper_n_vocab        (struct whisper_context * ctx);
    WHISPER_API int whisper_n_text_ctx     (struct whisper_context * ctx);
    WHISPER_API int whisper_is_multilingual(struct whisper_context * ctx);

    // The probabilities for the next token
    WHISPER_API float * whisper_get_probs(struct whisper_context * ctx);

    // Token Id -> String. Uses the vocabulary in the provided context
    WHISPER_API const char * whisper_token_to_str(struct whisper_context * ctx, whisper_token token);

    // Special tokens
    WHISPER_API whisper_token whisper_token_eot (struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_token_sot (struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_token_prev(struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_token_solm(struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_token_not (struct whisper_context * ctx);
    WHISPER_API whisper_token whisper_token_beg (struct whisper_context * ctx);

    // Task tokens
    WHISPER_API whisper_token whisper_token_translate ();
    WHISPER_API whisper_token whisper_token_transcribe();

    // Performance information
    WHISPER_API void whisper_print_timings(struct whisper_context * ctx);

    ////////////////////////////////////////////////////////////////////////////

    // Available sampling strategies
    enum whisper_sampling_strategy {
        WHISPER_SAMPLING_GREEDY,      // Always select the most probable token
        WHISPER_SAMPLING_BEAM_SEARCH, // TODO: not implemented yet!
    };

    // Text segment callback
    // Called on every newly generated text segment
    // Use the whisper_full_...() functions to obtain the text segments
    typedef void (*whisper_new_segment_callback)(struct whisper_context * ctx, void * user_data);

    struct whisper_full_params {
        enum whisper_sampling_strategy strategy;

        int n_threads;
        int offset_ms;

        bool translate;
        bool no_context;
        bool print_special_tokens;
        bool print_progress;
        bool print_realtime;
        bool print_timestamps;

        const char * language;

        struct {
            int n_past;
        } greedy;

        struct {
            int n_past;
            int beam_width;
            int n_best;
        } beam_search;

        whisper_new_segment_callback new_segment_callback;
        void * new_segment_callback_user_data;
    };

    WHISPER_API struct whisper_full_params whisper_full_default_params(enum whisper_sampling_strategy strategy);

    // Run the entire model: PCM -> log mel spectrogram -> encoder -> decoder -> text
    // Uses the specified decoding strategy to obtain the text.
    WHISPER_API int whisper_full(
            struct whisper_context * ctx,
            struct whisper_full_params params,
            const float * samples,
            int n_samples);

    // Number of generated text segments.
    // A segment can be a few words, a sentence, or even a paragraph.
    WHISPER_API int whisper_full_n_segments(struct whisper_context * ctx);

    // Get the start and end time of the specified segment.
    WHISPER_API int64_t whisper_full_get_segment_t0(struct whisper_context * ctx, int i_segment);
    WHISPER_API int64_t whisper_full_get_segment_t1(struct whisper_context * ctx, int i_segment);

    // Get the text of the specified segment.
    WHISPER_API const char * whisper_full_get_segment_text(struct whisper_context * ctx, int i_segment);

    // Get number of tokens in the specified segment.
    WHISPER_API int whisper_full_n_tokens(struct whisper_context * ctx, int i_segment);

    // Get the token text of the specified token in the specified segment.
    WHISPER_API const char * whisper_full_get_token_text(struct whisper_context * ctx, int i_segment, int i_token);
    WHISPER_API whisper_token whisper_full_get_token_id (struct whisper_context * ctx, int i_segment, int i_token);

    // Get the probability of the specified token in the specified segment.
    WHISPER_API float whisper_full_get_token_p(struct whisper_context * ctx, int i_segment, int i_token);

    // Print system information
    WHISPER_API const char * whisper_print_system_info();

#ifdef __cplusplus
}
#endif

#endif

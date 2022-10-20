#include "audio.h"

static const int SAMPLE_RATE = 48000;            // Opus is fixed at 48kHz
static const int SAMPLES_PER_BUF = SAMPLE_RATE * 120 / 1000;  // 120ms buffer
static const int CHANNELS_PER_SAMPLE = 2;
static const size_t WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE * sizeof(int16_t);

ndspWaveBuf s_waveBufs[3];
int16_t* s_audioBuffer = NULL;
LightEvent s_event;
volatile bool s_quit = false;  // Quit flag
OggOpusFile* opusFile;
Thread threadId;

// Retrieve strings for libopusfile errors
// Sourced from David Gow's example code: https://davidgow.net/files/opusal.cpp
const char* opusStrError(int error)
{
    switch (error) {
    case OP_FALSE:
        return "OP_FALSE: A request did not succeed.";
    case OP_HOLE:
        return "OP_HOLE: There was a hole in the page sequence numbers.";
    case OP_EREAD:
        return "OP_EREAD: An underlying read, seek or tell operation "
            "failed.";
    case OP_EFAULT:
        return "OP_EFAULT: A NULL pointer was passed where none was "
            "expected, or an internal library error was encountered.";
    case OP_EIMPL:
        return "OP_EIMPL: The stream used a feature which is not "
            "implemented.";
    case OP_EINVAL:
        return "OP_EINVAL: One or more parameters to a function were "
            "invalid.";
    case OP_ENOTFORMAT:
        return "OP_ENOTFORMAT: This is not a valid Ogg Opus stream.";
    case OP_EBADHEADER:
        return "OP_EBADHEADER: A required header packet was not properly "
            "formatted.";
    case OP_EVERSION:
        return "OP_EVERSION: The ID header contained an unrecognised "
            "version number.";
    case OP_EBADPACKET:
        return "OP_EBADPACKET: An audio packet failed to decode properly.";
    case OP_EBADLINK:
        return "OP_EBADLINK: We failed to find data we had seen before or "
            "the stream was sufficiently corrupt that seeking is "
            "impossible.";
    case OP_ENOSEEK:
        return "OP_ENOSEEK: An operation that requires seeking was "
            "requested on an unseekable stream.";
    case OP_EBADTIMESTAMP:
        return "OP_EBADTIMESTAMP: The first or last granule position of a "
            "link failed basic validity checks.";
    default:
        return "Unknown error.";
    }
}

// This sets up NDSP and our primary audio buffer
bool audioInit(void) {
    // Setup NDSP
    ndspChnReset(0);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(0, SAMPLE_RATE);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

    // Allocate audio buffer
    const size_t bufferSize = WAVEBUF_SIZE * ARRAY_SIZE(s_waveBufs);
    s_audioBuffer = (int16_t*)linearAlloc(bufferSize);
    if (!s_audioBuffer) {
        printf("Failed to allocate audio buffer\n");
        return false;
    }

    // Setup waveBufs for NDSP
    memset(&s_waveBufs, 0, sizeof(s_waveBufs));
    int16_t* buffer = s_audioBuffer;

    for (size_t i = 0; i < ARRAY_SIZE(s_waveBufs); ++i) {
        s_waveBufs[i].data_vaddr = buffer;
        s_waveBufs[i].status = NDSP_WBUF_DONE;

        buffer += WAVEBUF_SIZE / sizeof(buffer[0]);
    }

    return true;
}


// Stops playback and frees the primary audio buffer
void audioExit(void) {
    // Signal audio thread to quit
    s_quit = true;
    LightEvent_Signal(&s_event);

    // Free the audio thread
    threadJoin(threadId, UINT64_MAX);
    threadFree(threadId);

    // Cleanup audio things and de-init platform features

    ndspChnReset(0);
    linearFree(s_audioBuffer);

    op_free(opusFile);
}

// Main audio decoding logic
// This function pulls and decodes audio samples from opusFile_ to fill waveBuf_
bool fillBuffer(OggOpusFile* opusFile_, ndspWaveBuf* waveBuf_) {
#ifdef DEBUG
    // Setup timer for performance stats
    TickCounter timer;
    osTickCounterStart(&timer);
#endif  // DEBUG

    // Decode samples until our waveBuf is full
    int totalSamples = 0;
    while (totalSamples < SAMPLES_PER_BUF) {
        int16_t* buffer = waveBuf_->data_pcm16 + (totalSamples *
            CHANNELS_PER_SAMPLE);
        const size_t bufferSize = (SAMPLES_PER_BUF - totalSamples) *
            CHANNELS_PER_SAMPLE;

        // Decode bufferSize samples from opusFile_ into buffer,
        // storing the number of samples that were decoded (or error)
        const int samples = op_read_stereo(opusFile_, buffer, bufferSize);
        if (samples <= 0) {
            if (samples == 0) break;  // No error here

            printf("op_read_stereo: error %d (%s)", samples,
                opusStrError(samples));
            break;
        }

        totalSamples += samples;
    }

    // If no samples were read in the last decode cycle, we're done
    if (totalSamples == 0) {
        printf("Playback complete\n");
        return false;
    }

    // Pass samples to NDSP
    waveBuf_->nsamples = totalSamples;
    ndspChnWaveBufAdd(0, waveBuf_);
    DSP_FlushDataCache(waveBuf_->data_pcm16,
        totalSamples * CHANNELS_PER_SAMPLE * sizeof(int16_t));

#ifdef DEBUG
    // Print timing info
    osTickCounterUpdate(&timer);
    printf("fillBuffer %lfms in %lfms\n", totalSamples * 1000.0 / SAMPLE_RATE,
        osTickCounterRead(&timer));
#endif  // DEBUG

    return true;
}

// NDSP audio frame callback
// This signals the audioThread to decode more things
// once NDSP has played a sound frame, meaning that there should be
// one or more available waveBufs to fill with more data.
void audioCallback(void* const nul_) {
    (void)nul_;  // Unused

    if (s_quit) { // Quit flag
        return;
    }

    LightEvent_Signal(&s_event);
}

// Audio thread
// This handles calling the decoder function to fill NDSP buffers as necessary
void audioThread(void* const opusFile_) {
    OggOpusFile* const opusFile = (OggOpusFile*)opusFile_;

    while (!s_quit) {  // Whilst the quit flag is unset,
        // search our waveBufs and fill any that aren't currently
        // queued for playback (i.e, those that are 'done')
        for (size_t i = 0; i < ARRAY_SIZE(s_waveBufs); ++i) {
            if (s_waveBufs[i].status != NDSP_WBUF_DONE) {
                continue;
            }

            if (!fillBuffer(opusFile, &s_waveBufs[i])) {   // Playback complete
                return;
            }
        }

        // Wait for a signal that we're needed again before continuing,
        // so that we can yield to other things that want to run
        // (Note that the 3DS uses cooperative threading)
        LightEvent_Wait(&s_event);
    }
}
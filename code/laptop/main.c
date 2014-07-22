#include <stdint.h>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include "serialreader.h"
#include "sinegen.h"

#define PERIOD_SIZE 256
#define BUFFER_SIZE (PERIOD_SIZE*2)

snd_output_t *output = NULL;

static const char *device = "fileout"; /* playback device */
// static const char *device = "file:'/tmp/out.wav',wav"; /* playback device */
static const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; /* sample format */
static const unsigned int rate = 48000; /* stream rate */
static const unsigned int channels = 1; /* count of channels */

#define MEGA 256
int16_t buffer[MEGA];


static int set_hwparams(snd_pcm_t *handle,
snd_pcm_hw_params_t *params,
snd_pcm_access_t access)
{
    unsigned int rrate;
    int err;
    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
    printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
    return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, 0);
    if (err < 0) {
        printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
        printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
        return err;
    }
    /* set the stream rate */
    // rrate = rate;
    err = snd_pcm_hw_params_set_rate(handle, params, rate, 0);
    if (err < 0) {
        printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }


    err = snd_pcm_hw_params_set_buffer_size(handle, params, BUFFER_SIZE);
    if (err < 0) {
        printf("Unable to set buffer size %lu for playback: %s\n", PERIOD_SIZE, snd_strerror(err));
        return err;
    }

    int dir;
    err = snd_pcm_hw_params_set_period_size(handle, params, PERIOD_SIZE, 0);
    if (err < 0) {
        printf("Unable to set period size %lu for playback: %s\n", PERIOD_SIZE, snd_strerror(err));
        return err;
    }

    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }

    return 0;
}

serial_vars sv;


static inline double makeangular(uint16_t range) {
    double r = range > 600 ? 600 : (double) range;
    return TAU*(exp((600-r)/100) + 200); 
}

static inline double makelux(uint16_t lux) {
    return TAU * lux * 20/1024 ;
}

int main(void) {
    pthread_t serial_th;

    state sinestate = {.phase=0, .w=500*TAU, .vib_w=5*TAU, .vib_phase=0, .s =0};

    int err;
    unsigned int i;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;
    snd_pcm_hw_params_t *hwparams;

    pthread_create(&serial_th, NULL, (void* (*)(void *))serialreader, &sv);

    snd_pcm_hw_params_alloca(&hwparams);


    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    if (set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        return 1;
    }

    for (;;) {
        sinewave(&sinestate, buffer, PERIOD_SIZE, makeangular(sv.range), makelux(sv.lux));
        frames = snd_pcm_writei(handle, buffer, PERIOD_SIZE);

        // printf("%d\n", frames);
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
            // break;
        }
        if (frames > 0 && frames < PERIOD_SIZE)
            printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
    }

    snd_pcm_close(handle);
    return 0;
}

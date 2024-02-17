#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#define DURATION 5

// Function to write a 16-bit PCM WAV file header
void writeWAVHeader(FILE *file, int sample_rate, int num_samples) {
    uint16_t num_channels = 1; // Mono
    uint16_t bits_per_sample = 16; // 16-bit PCM
    uint16_t block_align = num_channels * (bits_per_sample / 8);
    uint32_t subchunk2_size = num_samples * num_channels * (bits_per_sample / 8);
    uint32_t chunk_size = 36 + subchunk2_size;

    fwrite("RIFF", 4, 1, file);
    fwrite(&chunk_size, 4, 1, file);
    fwrite("WAVE", 4, 1, file);
    fwrite("fmt ", 4, 1, file);
    uint32_t subchunk1_size = 16;
    fwrite(&subchunk1_size, 4, 1, file);
    uint16_t audio_format = 1;
    fwrite(&audio_format, 2, 1, file);
    fwrite(&num_channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    uint32_t byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);
    fwrite("data", 4, 1, file);
    fwrite(&subchunk2_size, 4, 1, file);
}

// Function to generate a sine wave
int16_t generateSineWave(double t, double amplitude, double frequency) {
    return (int16_t)(amplitude * SHRT_MAX * sin(2.0 * M_PI * frequency * t));
}

// Function to generate a sawtooth wave with a specified frequency
int16_t generateSawtoothWave(double t, double amplitude, double frequency) {
    return (int16_t)(amplitude * SHRT_MAX * (2.0 * fmod(t * frequency, 1.0) - 1.0));
}

// Function to generate a bipolar square wave with a specified frequency

int16_t generateSquareWave(double t, double amplitude, double frequency) {
    double value = sin(2.0 * M_PI * frequency * t);
    return (int16_t)(amplitude * SHRT_MAX * (fmod(value, 1.0) >= 0.5 ? 1.0 : -1.0));
}



// Function to generate a triangle wave with a specified frequency
int16_t generateTriangleWave(double t, double amplitude, double frequency) {
    return (int16_t)(amplitude * SHRT_MAX * (1.0 - 4.0 * fabs(fmod(t * frequency, 1.0) - 0.5)));
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        printf("Usage: %s fs m wavetype f A T fn.wav sqnr.txt\n", argv[0]);
        return 1;
    }

    int sample_rate;
    int bits_per_sample;
    double amplitude;
    double frequency;
    char waveform_type[20];
    int duration;
    float SQNR;

    sscanf(argv[1], "%d", &sample_rate);
    sscanf(argv[2], "%d", &bits_per_sample);
    sscanf(argv[4], "%lf", &frequency);
    sscanf(argv[5], "%lf", &amplitude);
    sscanf(argv[6], "%d", &duration);
    strncpy(waveform_type, argv[3], sizeof(waveform_type));

    // Calculate the number of samples for a specified duration
    int num_samples = sample_rate * duration;

    FILE *wav_file = fopen(argv[7], "wb");
    FILE *qf_file = fopen(argv[8], "w");

    if (wav_file == NULL || qf_file == NULL) {
        printf("Error opening the WAV or SQNR file for writing.\n");
        return 1;
    }

    writeWAVHeader(wav_file, sample_rate, num_samples);

    double t;
    int16_t sample;
    int16_t (*generateWaveform)(double, double, double) = NULL;

    if (strcmp(waveform_type, "sine") == 0) {
        generateWaveform = generateSineWave;
    } else if (strcmp(waveform_type, "sawtooth") == 0) {
        generateWaveform = generateSawtoothWave;
    } else if (strcmp(waveform_type, "bipolarsquare") == 0) {
        generateWaveform = generateSquareWave;
    } else if (strcmp(waveform_type, "triangle") == 0) {
        generateWaveform = generateTriangleWave;
    } else {
        printf("Unsupported waveform type. Using sine wave by default.\n");
        generateWaveform = generateSineWave;
    }

    for (int i = 0; i < num_samples; i++) {
        t = (double)i / sample_rate;
        sample = generateWaveform(t, amplitude, frequency);
        fwrite(&sample, 2, 1, wav_file);
    }

    SQNR = 6.02 * bits_per_sample;
    fprintf(qf_file, "%.15lf", SQNR);

    fclose(qf_file);
    fclose(wav_file);

    printf("WAV file and SQNR file generated successfully.\n");

    return 0;
}
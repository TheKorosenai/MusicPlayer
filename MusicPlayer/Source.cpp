#include<iostream>
#include<string>
#include<portaudio.h>
#include<sndfile.h>

#define FRAMES_PER_BUFFER (512)

using namespace std;

struct AudioData {
    SNDFILE* file;
    SF_INFO info;
};

static int paCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusflags, void* userData) {
    AudioData* audioData = (AudioData*)userData;
    float* out = (float*)outputBuffer;
    sf_count_t numRead = sf_readf_float(audioData->file, out, framesPerBuffer);

    if (numRead < framesPerBuffer) {
        for (unsigned long i = numRead; i < framesPerBuffer; ++i) {
            *out++ = 0.0f;
        }
        return paComplete;
    }
    return paContinue;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage : " << argv[0] << " <audio-file-path> " << endl;
        return 1;
    }
    string filePath = argv[1];

    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        cerr << "Failed to open file: " << sf_error(file) << endl;
        return 1;
    }

    AudioData audioData = { file, sfinfo };

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        cerr << "PortAudio Error: " << Pa_GetErrorText(err) << endl;
        sf_close(file);
        return 1;
    }

    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream, 0, sfinfo.channels, paFloat32, sfinfo.samplerate, FRAMES_PER_BUFFER, paCallback, &audioData);

    if (err != paNoError) {
        cerr << "PortAudio Error: " << Pa_GetErrorText(err) << endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        cerr << "PortAudio error: " << Pa_GetErrorText(err) << endl;
        Pa_CloseStream(stream);
        Pa_Terminate;
        sf_close(file);
        return 1;
    }

    cout << "Playing audio. Press enter to stop..." << endl;
    cin.get();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);

    Pa_Terminate();
    sf_close(file);

    return EXIT_SUCCESS;
}
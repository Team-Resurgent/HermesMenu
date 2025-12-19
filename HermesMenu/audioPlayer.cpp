#include "audioPlayer.h"
#include "stringUtility.h"
#include "context.h"
#include "utils.h"
#include "Assets\music_ogg.h"

#include <dmusici.h>
#include <dsound.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.cpp"
#include "stb.h"

#define AUDIO_PACKETS 64
#define AUDIO_OUTPUT_BUF_SIZE 8192

extern "C" {
    extern BOOL g_fDirectSoundDisableBusyWaitWarning;
}

namespace {
    LPDIRECTSOUND8 mDirectSoundDevice;
}

bool audioPlayer::init() {
    g_fDirectSoundDisableBusyWaitWarning = TRUE;
    HRESULT hr = DirectSoundCreate(NULL, &mDirectSoundDevice, NULL);
    HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)process, NULL, 0, NULL);
    return SUCCEEDED(hr);
}

uint64_t WINAPI audioPlayer::process(void* param) {

    int error;
    stb_vorbis* vorbis = stb_vorbis_open_memory(music_ogg, music_ogg_len, &error, NULL);
    if (vorbis == NULL) {
        return 0;
    }

    stb_vorbis_info vorbisInfo = stb_vorbis_get_info(vorbis);

    WAVEFORMATEX waveFormat;
    memset(&waveFormat, 0, sizeof(waveFormat));
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = vorbisInfo.channels;
    waveFormat.nSamplesPerSec = vorbisInfo.sample_rate;
    waveFormat.wBitsPerSample = sizeof(short) * 8;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample >> 3) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;

    DSSTREAMDESC streamDesc;
    memset(&streamDesc, 0, sizeof(streamDesc));
    streamDesc.dwMaxAttachedPackets = AUDIO_PACKETS;
    streamDesc.lpwfxFormat = &waveFormat;

    LPDIRECTSOUNDSTREAM directSoundStream;
    mDirectSoundDevice->CreateSoundStream(&streamDesc, &directSoundStream, NULL);

    directSoundStream->SetVolume(0);
    directSoundStream->SetHeadroom(0);

    DSMIXBINVOLUMEPAIR mixBinVolumePair[6];
    mixBinVolumePair[0].dwMixBin = DSMIXBIN_FRONT_LEFT;
    mixBinVolumePair[0].lVolume = DSBVOLUME_MAX;
    mixBinVolumePair[1].dwMixBin = DSMIXBIN_FRONT_RIGHT;
    mixBinVolumePair[1].lVolume = DSBVOLUME_MAX;
    mixBinVolumePair[2].dwMixBin = DSMIXBIN_FRONT_CENTER;
    mixBinVolumePair[2].lVolume = DSBVOLUME_MIN;
    mixBinVolumePair[3].dwMixBin = DSMIXBIN_LOW_FREQUENCY;
    mixBinVolumePair[3].lVolume = DSBVOLUME_MIN;
    mixBinVolumePair[4].dwMixBin = DSMIXBIN_BACK_LEFT;
    mixBinVolumePair[4].lVolume = DSBVOLUME_MIN;
    mixBinVolumePair[5].dwMixBin = DSMIXBIN_BACK_RIGHT;
    mixBinVolumePair[5].lVolume = DSBVOLUME_MIN;

    DSMIXBINS mixBins;
    mixBins.dwMixBinCount = 6;
    mixBins.lpMixBinVolumePairs = mixBinVolumePair;
    directSoundStream->SetMixBins(&mixBins);

    uint32_t* packetStatus = (uint32_t*)malloc(AUDIO_PACKETS * sizeof(uint32_t));
    memset(packetStatus, 0, AUDIO_PACKETS * sizeof(uint32_t));
    uint32_t* completedSize = (uint32_t*)malloc(AUDIO_PACKETS * sizeof(uint32_t));
    memset(completedSize, 0, AUDIO_PACKETS * sizeof(uint32_t));

    char* decodeBuffer = (char*)malloc(AUDIO_PACKETS * AUDIO_OUTPUT_BUF_SIZE);

    bool eof = false;
    while (true) {

        if (eof == true) {
            DirectSoundDoWork();
            Sleep(10);
            break;
        }

        for (int i = 0; i < AUDIO_PACKETS; i++) {
            if (packetStatus[i] != XMEDIAPACKET_STATUS_PENDING) {
                char* decodeBufferOffset = decodeBuffer + (i * AUDIO_OUTPUT_BUF_SIZE);
                const int bytesRead = (int)stb_vorbis_get_samples_short_interleaved(vorbis, vorbisInfo.channels,
                                        (short*)decodeBufferOffset, AUDIO_OUTPUT_BUF_SIZE / sizeof(short)) *
                    vorbisInfo.channels * sizeof(short);
                if (bytesRead != AUDIO_OUTPUT_BUF_SIZE) {
                    memset(decodeBufferOffset + bytesRead, 0, AUDIO_OUTPUT_BUF_SIZE - bytesRead);
                }
                if (bytesRead != 0) {
                    XMEDIAPACKET mediaPacket;
                    memset(&mediaPacket, 0, sizeof(mediaPacket));
                    mediaPacket.pvBuffer = decodeBufferOffset;
                    mediaPacket.dwMaxSize = bytesRead;
                    mediaPacket.pdwCompletedSize = &completedSize[i];
                    mediaPacket.pdwStatus = &packetStatus[i];
                    directSoundStream->Process(&mediaPacket, NULL);
                } else {
                    stb_vorbis_seek(vorbis, 0);
                }
            }
        }
        DirectSoundDoWork();
        Sleep(100);
    }

    int maxSamples = ((AUDIO_PACKETS * AUDIO_OUTPUT_BUF_SIZE) / sizeof(short)) / vorbisInfo.channels;
    int waitTime = (int)(maxSamples / (vorbisInfo.sample_rate / 1000.0f));
    Sleep(waitTime);

    directSoundStream->FlushEx(0, DSSTREAMFLUSHEX_ASYNC);
    directSoundStream->Release();

    free(packetStatus);
    free(completedSize);
    free(decodeBuffer);
    stb_vorbis_close(vorbis);

    return 1;
}

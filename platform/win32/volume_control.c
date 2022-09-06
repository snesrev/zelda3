#include "volume_control.h"

#define COBJMACROS
#define CINTERFACE

#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#include <initguid.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

HRESULT GetSimpleAudioVolume(ISimpleAudioVolume **simple_audio_volume);

DEFINE_GUID(IID_IMMDeviceEnumerator, 0XA95664D2, 0X9614, 0X4F35, 0XA7, 0X46, 0XDE, 0X8D, 0XB6, 0X36, 0X17, 0XE6);
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0XBCDE0395, 0XE52F, 0X467C, 0X8E, 0X3D, 0XC4, 0X57, 0X92, 0X91, 0X69, 0X2E);
DEFINE_GUID(IID_IAudioSessionManager, 0X77AA99A0, 0X1BD6, 0X484F, 0X8B, 0XC7, 0X2C, 0X65, 0X4C, 0X9A, 0X9B, 0X6F);

bool SetApplicationVolume(int volume_level) {
  ISimpleAudioVolume *simple_audio_volume = NULL;
  HRESULT result = GetSimpleAudioVolume(&simple_audio_volume);
  if (FAILED(result) || simple_audio_volume == NULL) {
    return false;
  }

  result = ISimpleAudioVolume_SetMasterVolume(simple_audio_volume, (float)(volume_level / 100.0), NULL);

  ISimpleAudioVolume_Release(simple_audio_volume);

  return SUCCEEDED(result);
}

bool SetApplicationMuted(bool muted) {
  ISimpleAudioVolume *simple_audio_volume = NULL;
  HRESULT result = GetSimpleAudioVolume(&simple_audio_volume);
  if (FAILED(result) || simple_audio_volume == NULL) {
    return false;
  }

  result = ISimpleAudioVolume_SetMute(simple_audio_volume, muted, NULL);

  ISimpleAudioVolume_Release(simple_audio_volume);

  return SUCCEEDED(result);
}

HRESULT GetSimpleAudioVolume(ISimpleAudioVolume **simple_audio_volume) {
  HRESULT result = S_OK;

  result = CoInitialize(NULL);

  IMMDeviceEnumerator *device_enumerator = NULL;
  result = CoCreateInstance(&CLSID_MMDeviceEnumerator,
    NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator, &device_enumerator);

  if (FAILED(result) || device_enumerator == NULL) {
    goto done;
  }

  IMMDevice *device = NULL;
  result = IMMDeviceEnumerator_GetDefaultAudioEndpoint(device_enumerator, eRender, eConsole, &device);

  if (FAILED(result) || device == NULL) {
    goto done;
  }

  IAudioSessionManager *audio_session_manager = NULL;
  result = IMMDevice_Activate(device, &IID_IAudioSessionManager, CLSCTX_INPROC_SERVER, NULL, &audio_session_manager);

  if (FAILED(result) || audio_session_manager == NULL) {
    goto done;
  }

  result = IAudioSessionManager_GetSimpleAudioVolume(audio_session_manager, &GUID_NULL, 0, simple_audio_volume);

done:
#pragma warning(push)
#pragma warning(disable: 6001)
  if (device_enumerator) IMMDeviceEnumerator_Release(device_enumerator);
  if (device) IMMDevice_Release(device);
  if (audio_session_manager) IAudioSessionManager_Release(audio_session_manager);
#pragma warning(pop)

  CoUninitialize();

  return SUCCEEDED(result);
}

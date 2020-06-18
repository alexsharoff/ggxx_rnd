#pragma once

#include <Windows.h>
#include <objbase.h>


typedef struct XACT_WAVE_INSTANCE_PROPERTIES
{
} XACT_WAVE_INSTANCE_PROPERTIES, *LPXACT_WAVE_INSTANCE_PROPERTIES;

DECLARE_INTERFACE(IXACT3Wave)
{
    STDMETHOD(Destroy)(THIS) PURE;
    STDMETHOD(Play)(THIS) PURE;
    STDMETHOD(Stop)(THIS_ DWORD dwFlags) PURE;
    STDMETHOD(Pause)(THIS_ BOOL fPause) PURE;
    STDMETHOD(GetState)(THIS_ __out DWORD* pdwState) PURE;
    STDMETHOD(SetPitch)(THIS_ SHORT pitch) PURE;
    STDMETHOD(SetVolume)(THIS_ FLOAT volume) PURE;
    STDMETHOD(SetMatrixCoefficients)(THIS_ UINT32 uSrcChannelCount, UINT32 uDstChannelCount, __in float* pMatrixCoefficients) PURE;
    STDMETHOD(GetProperties)(THIS_ __out LPXACT_WAVE_INSTANCE_PROPERTIES pProperties) PURE;
};

struct IXACT3WaveBank {};

// -----------------------------------------------------------------------------
// XACT State flags
// -----------------------------------------------------------------------------
static const DWORD XACT_STATE_CREATED           = 0x00000001; // Created, but nothing else
static const DWORD XACT_STATE_PREPARING         = 0x00000002; // In the middle of preparing
static const DWORD XACT_STATE_PREPARED          = 0x00000004; // Prepared, but not yet played
static const DWORD XACT_STATE_PLAYING           = 0x00000008; // Playing (though could be paused)
static const DWORD XACT_STATE_STOPPING          = 0x00000010; // Stopping
static const DWORD XACT_STATE_STOPPED           = 0x00000020; // Stopped
static const DWORD XACT_STATE_PAUSED            = 0x00000040; // Paused (Can be combined with some of the other state flags above)
static const DWORD XACT_STATE_INUSE             = 0x00000080; // Object is in use (used by wavebanks and soundbanks).
static const DWORD XACT_STATE_PREPAREFAILED     = 0x80000000; // Object preparation failed.

struct XACT3Wave : public IXACT3Wave
{
    STDMETHOD(Destroy)(THIS)
    {
        return S_OK;
    }
    STDMETHOD(Play)(THIS)
    {
        return S_OK;
    }
    STDMETHOD(Stop)(THIS_ DWORD /* dwFlags */)
    {
        return S_OK;
    }
    STDMETHOD(Pause)(THIS_ BOOL /* fPause */)
    {
        return S_OK;
    }
    STDMETHOD(GetState)(THIS_ __out DWORD* pdwState)
    {
        *pdwState = XACT_STATE_STOPPED;
        return S_OK;
    }
    STDMETHOD(SetPitch)(THIS_ SHORT /* pitch */)
    {
        return S_OK;
    }
    STDMETHOD(SetVolume)(THIS_ FLOAT /* volume */)
    {
        return S_OK;
    }
    STDMETHOD(SetMatrixCoefficients)(THIS_ UINT32 /* uSrcChannelCount*/ , UINT32 /* uDstChannelCount */, __in float* /* pMatrixCoefficients */)
    {
        return S_OK;
    }
    STDMETHOD(GetProperties)(THIS_ __out LPXACT_WAVE_INSTANCE_PROPERTIES /* pProperties */)
    {
        return S_OK;
    }
};

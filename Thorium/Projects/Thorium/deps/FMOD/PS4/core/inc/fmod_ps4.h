#pragma once

#include <user_service.h>

typedef enum FMOD_PS4_PORT_TYPE
{
    FMOD_PS4_PORT_TYPE_MUSIC,
    FMOD_PS4_PORT_TYPE_VOICE,
    FMOD_PS4_PORT_TYPE_PERSONAL,
    FMOD_PS4_PORT_TYPE_CONTROLLER,
    FMOD_PS4_PORT_TYPE_COPYRIGHT_MUSIC,
    FMOD_PS4_PORT_TYPE_SOCIAL,
    
    FMOD_PS4_PORT_TYPE_MAX
} FMOD_PS4_PORT_TYPE;

extern "C" FMOD_RESULT F_API FMOD_PS4_GetPadVolume(FMOD_SYSTEM* system, SceUserServiceUserId userID, float* volume);
extern "C" FMOD_RESULT F_API FMOD_PS4_SetComputeDevice(int computePipe, int computeQueue, void* garlicMem, size_t garlicMemSize, void* onionMem, size_t onionMemSize);
extern "C" FMOD_RESULT F_API FMOD_PS4_ReleaseComputeDevice();

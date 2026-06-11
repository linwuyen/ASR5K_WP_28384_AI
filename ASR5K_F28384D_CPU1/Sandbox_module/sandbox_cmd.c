/*
 * sandbox_cmd.c
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX_PHASE1)
 */

#include "Sandbox_module/sandbox_cmd.h"
#include "Wave_module/wave_memory_backend.h"
#include "dds/dds_api.h"

ST_SANDBOX_CMD g_sSandboxCmd;

void SandboxCmd_Init(void)
{
    g_sSandboxCmd.u16Cmd           = (uint16_t)SANDBOX_CMD_NONE;
    g_sSandboxCmd.u32Param         = 0UL;
    g_sSandboxCmd.u16Pending       = 0U;
    g_sSandboxCmd.u16LastCmd       = (uint16_t)SANDBOX_CMD_NONE;
    g_sSandboxCmd.u32LastParam     = 0UL;
    g_sSandboxCmd.u16LastResult    = 0U;
    g_sSandboxCmd.u32DispatchCount = 0UL;
    g_sSandboxCmd.u32RejectCount   = 0UL;
}

void SandboxCmd_Inject(uint16_t u16Cmd, uint32_t u32Param)
{
    g_sSandboxCmd.u32Param   = u32Param;
    g_sSandboxCmd.u16Cmd     = u16Cmd;
    g_sSandboxCmd.u16Pending = 1U;
}

/*
 * WAVE_ACTIVATE handler: validate -> activate -> re-attach the DDS pointer.
 * Runs in main-loop context; the pointer republish is one 32-bit MOVL so the
 * 100kHz ISR can never observe a half-updated pointer.
 */
static uint16_t sandboxCmdWaveActivate(uint16_t u16PageId)
{
    if (WaveMem_ValidatePage(u16PageId) == 0U) {
        return 0U;
    }
    if (WaveMem_ActivatePage(u16PageId) == 0U) {
        return 0U;
    }
    sDDS.pu16WaveTable = WaveMem_GetActiveWavePtr();
    return 1U;
}

void SandboxCmd_Poll(void)
{
    uint16_t u16Cmd;
    uint32_t u32Param;
    uint16_t u16Result;

    if (g_sSandboxCmd.u16Pending == 0U) {
        return;
    }

    /* Latch then clear pending so a debugger re-fire is unambiguous */
    u16Cmd   = g_sSandboxCmd.u16Cmd;
    u32Param = g_sSandboxCmd.u32Param;
    g_sSandboxCmd.u16Pending = 0U;

    switch (u16Cmd) {
    case SANDBOX_CMD_OUTPUT_ON:
        DDS_Start();
        u16Result = 1U;
        break;

    case SANDBOX_CMD_OUTPUT_OFF:
        DDS_Stop();
        u16Result = 1U;
        break;

    case SANDBOX_CMD_DDS_FREQ:
        DDS_SetFrequency(u32Param);
        u16Result = 1U;
        break;

    case SANDBOX_CMD_DDS_AMP:
        if (u32Param <= 65535UL) {
            DDS_SetAmplitude((uint16_t)u32Param);
            u16Result = 1U;
        } else {
            u16Result = 0U;
        }
        break;

    case SANDBOX_CMD_DDS_OFFSET:
        if (u32Param <= 65535UL) {
            DDS_SetOffset((uint16_t)u32Param);
            u16Result = 1U;
        } else {
            u16Result = 0U;
        }
        break;

    case SANDBOX_CMD_WAVE_ACTIVATE:
        u16Result = sandboxCmdWaveActivate((uint16_t)u32Param);
        break;

    default:
        u16Result = 0U;
        break;
    }

    g_sSandboxCmd.u16LastCmd    = u16Cmd;
    g_sSandboxCmd.u32LastParam  = u32Param;
    g_sSandboxCmd.u16LastResult = u16Result;
    if (u16Result != 0U) {
        g_sSandboxCmd.u32DispatchCount++;
    } else {
        g_sSandboxCmd.u32RejectCount++;
    }
}

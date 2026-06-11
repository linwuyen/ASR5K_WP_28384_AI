/*
 * am3352_sandbox.c
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 */

#include "Sandbox_module/am3352_sandbox.h"
#include "Sandbox_module/sandbox_cmd.h"

ST_AM3352_SANDBOX g_sAm3352Sandbox;

void Am3352Sandbox_Init(void)
{
    g_sAm3352Sandbox.u16CmdId        = 0U;
    g_sAm3352Sandbox.u32Param        = 0UL;
    g_sAm3352Sandbox.u16Pending      = 0U;
    g_sAm3352Sandbox.u16LastCmdId    = 0U;
    g_sAm3352Sandbox.u16LastMapOk    = 0U;
    g_sAm3352Sandbox.u32ForwardCount = 0UL;
    g_sAm3352Sandbox.u32UnknownCount = 0UL;
    g_sAm3352Sandbox.u32PollCount    = 0UL;
}

void Am3352Sandbox_Inject(uint16_t u16CmdId, uint32_t u32Param)
{
    g_sAm3352Sandbox.u32Param   = u32Param;
    g_sAm3352Sandbox.u16CmdId   = u16CmdId;
    g_sAm3352Sandbox.u16Pending = 1U;
}

void Am3352Sandbox_Poll(void)
{
    uint16_t u16CmdId;
    uint32_t u32Param;
    uint16_t u16Mapped;

    g_sAm3352Sandbox.u32PollCount++;

    if (g_sAm3352Sandbox.u16Pending == 0U) {
        return;
    }
    u16CmdId = g_sAm3352Sandbox.u16CmdId;
    u32Param = g_sAm3352Sandbox.u32Param;
    g_sAm3352Sandbox.u16Pending = 0U;

    /* Protocol-layer id -> sandbox dispatcher command. The sandbox ids are
     * deliberately 1:1 today; the future real SPIB bridge maps the official
     * cmd_id.h register writes onto the SAME SandboxCmd_Inject entry. */
    switch (u16CmdId) {
    case AM3352_CMD_OUTPUT_ON:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_OUTPUT_ON, 0UL);
        u16Mapped = 1U;
        break;
    case AM3352_CMD_OUTPUT_OFF:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_OUTPUT_OFF, 0UL);
        u16Mapped = 1U;
        break;
    case AM3352_CMD_DDS_FREQ:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_DDS_FREQ, u32Param);
        u16Mapped = 1U;
        break;
    case AM3352_CMD_DDS_AMP:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_DDS_AMP, u32Param);
        u16Mapped = 1U;
        break;
    case AM3352_CMD_DDS_OFFSET:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_DDS_OFFSET, u32Param);
        u16Mapped = 1U;
        break;
    case AM3352_CMD_WAVE_ACTIVATE:
        SandboxCmd_Inject((uint16_t)SANDBOX_CMD_WAVE_ACTIVATE, u32Param);
        u16Mapped = 1U;
        break;
    default:
        u16Mapped = 0U;
        break;
    }

    g_sAm3352Sandbox.u16LastCmdId = u16CmdId;
    g_sAm3352Sandbox.u16LastMapOk = u16Mapped;
    if (u16Mapped != 0U) {
        g_sAm3352Sandbox.u32ForwardCount++;
    } else {
        g_sAm3352Sandbox.u32UnknownCount++;
    }
}

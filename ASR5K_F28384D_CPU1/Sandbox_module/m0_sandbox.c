/*
 * m0_sandbox.c
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 * Updated: 2026-06-12 - fake plant behavior for fans / ADC / protections.
 */

#include "Sandbox_module/m0_sandbox.h"

ST_M0_SANDBOX g_sM0Sandbox;

static uint32_t s_u32PollDivider = 0UL;
static int16_t  s_i16IlPhase     = 0;

/* Read-only registers (writes rejected, like the real device would NACK) */
static uint16_t m0RegIsReadOnly(uint16_t u16Reg)
{
    switch (u16Reg) {
    case M0_REG_WHOAMI:
    case M0_REG_GROUP_IN:
    case M0_REG_ILA_AD:
    case M0_REG_ILB_AD:
    case M0_REG_TEMP:
    case M0_REG_FAN1_CAP:
    case M0_REG_FAN2_CAP:
    case M0_REG_FAN3_CAP:
    case M0_REG_FAN4_CAP:
    case M0_REG_FANFAIL:
    case M0_REG_HEARTBEAT:
        return 1U;
    default:
        return 0U;
    }
}

void M0Sandbox_Init(void)
{
    uint16_t u16I;

    g_sM0Sandbox.u16Mode        = M0_SANDBOX_MODE_FAKE;
    g_sM0Sandbox.u16LastReg     = 0U;
    g_sM0Sandbox.u16LastData    = 0U;
    g_sM0Sandbox.u32XferCount   = 0UL;
    g_sM0Sandbox.u32NackCount   = 0UL;
    g_sM0Sandbox.u32RejectCount = 0UL;
    for (u16I = 0U; u16I < M0_SANDBOX_REG_COUNT; u16I++) {
        g_sM0Sandbox.au16Reg[u16I] = 0U;
    }
    g_sM0Sandbox.au16Reg[M0_REG_WHOAMI] = M0_SANDBOX_WHOAMI;
    g_sM0Sandbox.au16Reg[M0_REG_TEMP]   = 250U;   /* fake 25.0 C            */
    g_sM0Sandbox.au16Reg[M0_REG_IPK_POS] = 0x0FFFU; /* Ipeak limits wide open */
    g_sM0Sandbox.au16Reg[M0_REG_IPK_NEG] = 0x0FFFU;

    s_u32PollDivider = 0UL;
    s_i16IlPhase     = 0;
}

uint16_t M0Sandbox_ReadReg(uint16_t u16Reg)
{
    if (u16Reg >= M0_SANDBOX_REG_COUNT) {
        g_sM0Sandbox.u32RejectCount++;
        return 0xFFFFU;
    }
    g_sM0Sandbox.u16LastReg = u16Reg;
    g_sM0Sandbox.u32XferCount++;
    return g_sM0Sandbox.au16Reg[u16Reg];
}

uint16_t M0Sandbox_WriteReg(uint16_t u16Reg, uint16_t u16Data)
{
    if ((u16Reg >= M0_SANDBOX_REG_COUNT) || (m0RegIsReadOnly(u16Reg) != 0U)) {
        g_sM0Sandbox.u32RejectCount++;
        return 0U;
    }
    /* DAC7612 channels are 12-bit */
    if ((u16Reg == M0_REG_IPK_POS) || (u16Reg == M0_REG_IPK_NEG)) {
        u16Data &= 0x0FFFU;
    }
    g_sM0Sandbox.au16Reg[u16Reg] = u16Data;
    g_sM0Sandbox.u16LastReg  = u16Reg;
    g_sM0Sandbox.u16LastData = u16Data;
    g_sM0Sandbox.u32XferCount++;
    return 1U;
}

/*
 * Fake plant, stepped every 4096 polls:
 *   - fan tach feedback tracks the commanded duty (no FANFAIL)
 *   - ILA/ILB wander as a slow triangle so the watch shows live current
 *   - protections stay clear (fault injection: poke GROUP_IN via debugger
 *     watch is intentionally possible - it is a plain RAM register here)
 *   - heartbeat increments (liveness for the future real-M0 health check)
 */
void M0Sandbox_Poll(void)
{
    uint16_t u16Fan;

    s_u32PollDivider++;
    if ((s_u32PollDivider & 0x0FFFUL) != 0UL) {
        return;
    }
    if (g_sM0Sandbox.u16Mode != M0_SANDBOX_MODE_FAKE) {
        return;   /* REAL mode: plant is the real M0 */
    }

    for (u16Fan = 0U; u16Fan < 4U; u16Fan++) {
        g_sM0Sandbox.au16Reg[M0_REG_FAN1_CAP + u16Fan] =
            g_sM0Sandbox.au16Reg[M0_REG_FAN1_PWM + u16Fan];
    }

    s_i16IlPhase += 64;
    if (s_i16IlPhase > 4000) {
        s_i16IlPhase = -4000;
    }
    g_sM0Sandbox.au16Reg[M0_REG_ILA_AD] = (uint16_t)(2048 + (s_i16IlPhase >> 1));
    g_sM0Sandbox.au16Reg[M0_REG_ILB_AD] = (uint16_t)(2048 - (s_i16IlPhase >> 1));

    g_sM0Sandbox.au16Reg[M0_REG_HEARTBEAT]++;
}

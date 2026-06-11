/*
 * fsi_sandbox.c
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 * Updated: 2026-06-12 - product 14-word frame + current-share loop model.
 */

#include "Sandbox_module/fsi_sandbox.h"
#include "Sandbox_module/mcbsp_sandbox.h"

ST_FSI_SANDBOX g_sFsiSandbox;

void FsiSandbox_Init(void)
{
    uint16_t u16I;

    g_sFsiSandbox.u16Mode       = FSI_SANDBOX_MODE_FAKE;
    g_sFsiSandbox.u16Seq        = 0U;
    g_sFsiSandbox.u16SharePct   = 0U;
    g_sFsiSandbox.u32TxCount    = 0UL;
    g_sFsiSandbox.u32RxCount    = 0UL;
    g_sFsiSandbox.u32ErrorCount = 0UL;
    g_sFsiSandbox.u32PollCount  = 0UL;
    for (u16I = 0U; u16I < FSI_SANDBOX_FRAME_WORDS; u16I++) {
        g_sFsiSandbox.au16TxFrame[u16I] = 0U;
        g_sFsiSandbox.au16RxFrame[u16I] = 0U;
    }
}

uint16_t FsiSandbox_GetSharePct(void)
{
    return g_sFsiSandbox.u16SharePct;
}

static uint16_t fsiFrameChecksum(const uint16_t *pu16Frame)
{
    uint16_t u16Sum = 0U;
    uint16_t u16I;

    for (u16I = 0U; u16I < FSI_FRM_CHECKSUM; u16I++) {
        u16Sum = (uint16_t)(u16Sum + pu16Frame[u16I]);
    }
    return u16Sum;
}

void FsiSandbox_Poll(void)
{
    uint16_t u16I;

    g_sFsiSandbox.u32PollCount++;

    if (g_sFsiSandbox.u16Mode != FSI_SANDBOX_MODE_FAKE) {
        /* HW_LOOP / DAISY are future bring-up placeholders */
        return;
    }

    /* Build the product-layout frame. CV feedback comes from the MCBSP
     * sandbox (AD7915 CV_AD model) -> unit2 current slot; setpoints and
     * unit3 slots carry recognizable fake patterns. */
    g_sFsiSandbox.u16Seq++;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_SEQ]       = g_sFsiSandbox.u16Seq;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_SETP_A]    = 0x1111U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_SETP_B]    = 0x2222U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_SETP_C]    = 0x3333U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_U2_VOLT]   = 0x0E10U;  /* fake 360.0 */
    g_sFsiSandbox.au16TxFrame[FSI_FRM_U2_CURR]   = McbspSandbox_ReadCvAd();
    g_sFsiSandbox.au16TxFrame[FSI_FRM_U3_VOLT]   = 0x0E10U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_U3_CURR]   = 0U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_CMD1]      = 0U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_DATA1]     = 0U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_CMD2]      = 0U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_DATA2]     = 0U;
    /* Lead unit answer modeled as an even 50.00% share */
    g_sFsiSandbox.au16TxFrame[FSI_FRM_SHARE_PCT] = 5000U;
    g_sFsiSandbox.au16TxFrame[FSI_FRM_CHECKSUM]  =
        fsiFrameChecksum(g_sFsiSandbox.au16TxFrame);
    g_sFsiSandbox.u32TxCount++;

    /* Software loopback "wire" */
    for (u16I = 0U; u16I < FSI_SANDBOX_FRAME_WORDS; u16I++) {
        g_sFsiSandbox.au16RxFrame[u16I] = g_sFsiSandbox.au16TxFrame[u16I];
    }
    g_sFsiSandbox.u32RxCount++;

    /* Integrity check + consume the share% word (CC loop input) */
    if (g_sFsiSandbox.au16RxFrame[FSI_FRM_CHECKSUM] !=
        fsiFrameChecksum(g_sFsiSandbox.au16RxFrame)) {
        g_sFsiSandbox.u32ErrorCount++;
    } else {
        g_sFsiSandbox.u16SharePct =
            g_sFsiSandbox.au16RxFrame[FSI_FRM_SHARE_PCT];
    }
}

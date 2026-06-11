/*
 * am3352_sandbox.h
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 *
 * Software AM3352 command injector (SPIB system path stand-in).
 *
 * Product use: the AM3352 drives CPU1 over SPIB (legacy register protocol,
 * spi_b_slave.c + cmd_parser). No AM3352 exists on the home board, and the
 * official SPIB parser must not be modified, so this module simulates the
 * COMMAND LAYER ONLY: commands enter a mailbox exactly as if parsed from a
 * SPIB frame, then are forwarded to the sandbox command dispatcher
 * (SandboxCmd_Inject), which the future real SPIB->sandbox bridge will call
 * through the very same entry point.
 *
 * Fire from the debugger:
 *   g_sAm3352Sandbox.u32Param   = <param>
 *   g_sAm3352Sandbox.u16CmdId   = <AM3352_CMD_*>
 *   g_sAm3352Sandbox.u16Pending = 1
 */

#ifndef SANDBOX_AM3352_SANDBOX_H_
#define SANDBOX_AM3352_SANDBOX_H_

#include <stdint.h>

/* Protocol-layer command ids (sandbox numbering, NOT the official cmd_id.h
 * register map - mapping to the real map happens in the future bridge). */
#define AM3352_CMD_OUTPUT_ON      1U
#define AM3352_CMD_OUTPUT_OFF     2U
#define AM3352_CMD_DDS_FREQ       3U   /* param = freq x100 */
#define AM3352_CMD_DDS_AMP        4U   /* param = 0..65535  */
#define AM3352_CMD_DDS_OFFSET     5U   /* param = 0..65535  */
#define AM3352_CMD_WAVE_ACTIVATE  6U   /* param = page id   */

typedef struct {
    volatile uint16_t u16CmdId;     /* AM3352_CMD_*                        */
    volatile uint32_t u32Param;
    volatile uint16_t u16Pending;   /* write 1 to fire                     */

    uint16_t u16LastCmdId;
    uint16_t u16LastMapOk;          /* 1 = mapped & forwarded              */
    uint32_t u32ForwardCount;       /* commands forwarded to dispatcher    */
    uint32_t u32UnknownCount;       /* unmapped command ids                */
    uint32_t u32PollCount;
} ST_AM3352_SANDBOX;

extern ST_AM3352_SANDBOX g_sAm3352Sandbox;

void Am3352Sandbox_Init(void);

/** @brief Inject a protocol-layer command from code. */
void Am3352Sandbox_Inject(uint16_t u16CmdId, uint32_t u32Param);

/** @brief Map and forward a pending command. Call from the main loop. */
void Am3352Sandbox_Poll(void);

#endif /* SANDBOX_AM3352_SANDBOX_H_ */

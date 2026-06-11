/*
 * m0_sandbox.h
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 * Updated: 2026-06-12 - register map expanded to mirror the real MSPM0G3519
 *          duties from the product control-card system diagram.
 *
 * I2CA / MSPM0G3519 management controller fake backend.
 *
 * Product duties (system diagram):
 *   - 4x fan control: PWM out + CAP feedback (FANCTRL1-4 / FANFAIL1-4)
 *   - Ipeak limitation: SPI1 -> DAC7612 dual 12-bit (IPK_POS / IPK_NEG)
 *   - GROUP OUTPUT: MASTER, OUT_HI_LO, IM_HI_LO, VM_HI_LO, P_RANG_HL,
 *     RANGE_HL, OUT_ON, REMOTE, DCDC1_ON, PFC1_ON, PFC2_ON, DCDC2_ON,
 *     AC_OCPA, AC_ON
 *   - GROUP INPUT (protections): IPK_PWM, PFC1_UVP/OVP/OTP,
 *     PFC2_UVP/OVP/OTP, DCOVP
 *   - ADC: ILA_AD, ILB_AD, TEMP
 *   - PWM carry clocks CLK_0 / CLK_180
 *   - I2C0 BSL + NRST/INVOKE + UART debug
 *
 * The fake backend emulates this as a register file behind the SAME
 * read/write interface the real I2C driver will use. Porting to the real
 * M0 = implement M0_SANDBOX_MODE_REAL with I2CA transactions; register
 * semantics stay identical.
 *
 * Home board note: no M0 and no I2C pull-ups - the real bus must NOT be
 * driven, FAKE mode never touches the peripheral.
 */

#ifndef SANDBOX_M0_SANDBOX_H_
#define SANDBOX_M0_SANDBOX_H_

#include <stdint.h>

#define M0_SANDBOX_MODE_FAKE   0U
#define M0_SANDBOX_MODE_REAL   1U   /* placeholder */

#define M0_SANDBOX_REG_COUNT   32U
#define M0_SANDBOX_WHOAMI      0x3519U

//-----------------------------------------------------------------------------
// Register map (fake == future real map proposal)
//-----------------------------------------------------------------------------
#define M0_REG_WHOAMI          0U   /* RO: 0x3519                           */
#define M0_REG_GROUP_OUT       1U   /* RW: output control bitmap            */
#define M0_REG_GROUP_IN        2U   /* RO: protection input bitmap          */
#define M0_REG_ILA_AD          3U   /* RO: phase A current ADC              */
#define M0_REG_ILB_AD          4U   /* RO: phase B current ADC              */
#define M0_REG_TEMP            5U   /* RO: temperature ADC                  */
#define M0_REG_IPK_POS         6U   /* RW: DAC7612 ch A (Ipeak +), 12 bit   */
#define M0_REG_IPK_NEG         7U   /* RW: DAC7612 ch B (Ipeak -), 12 bit   */
#define M0_REG_FAN1_PWM        8U   /* RW: fan duty 0..1000                 */
#define M0_REG_FAN2_PWM        9U
#define M0_REG_FAN3_PWM        10U
#define M0_REG_FAN4_PWM        11U
#define M0_REG_FAN1_CAP        12U  /* RO: tach feedback (fake: ~duty)      */
#define M0_REG_FAN2_CAP        13U
#define M0_REG_FAN3_CAP        14U
#define M0_REG_FAN4_CAP        15U
#define M0_REG_FANFAIL         16U  /* RO: bit0..3 = FANFAIL1..4            */
#define M0_REG_PWM_CARRY       17U  /* RW: bit0 CLK_0 en, bit1 CLK_180 en   */
#define M0_REG_HEARTBEAT       31U  /* RO: fake M0 liveness counter         */

/* M0_REG_GROUP_OUT bits (per system diagram order) */
#define M0_GO_MASTER       0x0001U
#define M0_GO_OUT_HI_LO    0x0002U
#define M0_GO_IM_HI_LO     0x0004U
#define M0_GO_VM_HI_LO     0x0008U
#define M0_GO_P_RANG_HL    0x0010U
#define M0_GO_RANGE_HL     0x0020U
#define M0_GO_OUT_ON       0x0040U
#define M0_GO_REMOTE       0x0080U
#define M0_GO_DCDC1_ON     0x0100U
#define M0_GO_PFC1_ON      0x0200U
#define M0_GO_PFC2_ON      0x0400U
#define M0_GO_DCDC2_ON     0x0800U
#define M0_GO_AC_OCPA      0x1000U
#define M0_GO_AC_ON        0x2000U

/* M0_REG_GROUP_IN bits (protections, 1 = tripped; fake keeps all 0) */
#define M0_GI_IPK_PWM      0x0001U
#define M0_GI_PFC1_UVP     0x0002U
#define M0_GI_PFC1_OVP     0x0004U
#define M0_GI_PFC1_OTP     0x0008U
#define M0_GI_PFC2_UVP     0x0010U
#define M0_GI_PFC2_OVP     0x0020U
#define M0_GI_PFC2_OTP     0x0040U
#define M0_GI_DCOVP        0x0080U

typedef struct {
    uint16_t u16Mode;          /* M0_SANDBOX_MODE_*                        */
    uint16_t u16LastReg;
    uint16_t u16LastData;
    uint32_t u32XferCount;     /* completed transactions                   */
    uint32_t u32NackCount;     /* fake: stays 0; real: I2C NACK counter    */
    uint32_t u32RejectCount;   /* out-of-range / read-only violations      */
    uint16_t au16Reg[M0_SANDBOX_REG_COUNT];   /* register file             */
} ST_M0_SANDBOX;

extern ST_M0_SANDBOX g_sM0Sandbox;

void     M0Sandbox_Init(void);
uint16_t M0Sandbox_ReadReg(uint16_t u16Reg);                 /* 0xFFFF on err */
uint16_t M0Sandbox_WriteReg(uint16_t u16Reg, uint16_t u16Data); /* 1 = ok  */
void     M0Sandbox_Poll(void);   /* fake plant behavior (slow)             */

#endif /* SANDBOX_M0_SANDBOX_H_ */

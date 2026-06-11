/*
 * fsi_sandbox.h
 *
 * Created: 2026-06-12 (ASR5K_SANDBOX)
 * Updated: 2026-06-12 - frame upgraded to the product 14-word layout
 *          (系統圖「並聯訊息同步」: 14筆 x 16Bits / 100MHz = 2.24usec).
 *
 * FSI daisy-chain communication stub.
 *
 * Product use: FSI_DAISY_TX (FSITXA GPIO0/1/2) / FSI_DAISY_RX (FSIRXA
 * GPIO12/11/13) + SYNC (GPIO6 out / GPIO90 in) link paralleled units.
 * Frame content per system diagram:
 *   - 三相每點設定電壓資料 (3 phase voltage setpoints)
 *   - 第二台輸出電壓電流 / 第三台輸出電壓電流 (unit2/unit3 V & I)
 *   - 三組自定義操作命令與資料 (3 custom cmd/data pairs -> here 2 pairs
 *     + current-share word, see layout)
 *   - current-share percentage feeding the CC_DA loop (電流環 via MCBSP)
 *
 * 14-word frame layout used by this sandbox (offset = word index):
 *   [0]  sequence number
 *   [1]  phase A setpoint     [2]  phase B setpoint   [3]  phase C setpoint
 *   [4]  unit2 voltage        [5]  unit2 current
 *   [6]  unit3 voltage        [7]  unit3 current
 *   [8]  custom cmd1          [9]  custom data1
 *   [10] custom cmd2          [11] custom data2
 *   [12] current-share %x100 (0..10000)
 *   [13] additive checksum of words 0..12
 *
 * Modes:
 *   FSI_SANDBOX_MODE_FAKE    - software frame loopback + integrity check;
 *                              CV sample is picked up from the MCBSP
 *                              sandbox (CV_AD) into unit2-current, and the
 *                              share% word is generated, modeling the
 *                              CV -> FSI -> share% -> CC chain
 *   FSI_SANDBOX_MODE_HW_LOOP - placeholder: FSITXA->FSIRXA loopback
 *   FSI_SANDBOX_MODE_DAISY   - placeholder: real daisy-chain
 */

#ifndef SANDBOX_FSI_SANDBOX_H_
#define SANDBOX_FSI_SANDBOX_H_

#include <stdint.h>

#define FSI_SANDBOX_MODE_FAKE     0U
#define FSI_SANDBOX_MODE_HW_LOOP  1U   /* placeholder */
#define FSI_SANDBOX_MODE_DAISY    2U   /* placeholder */

#define FSI_SANDBOX_FRAME_WORDS   14U  /* product frame size */

/* Frame word offsets */
#define FSI_FRM_SEQ        0U
#define FSI_FRM_SETP_A     1U
#define FSI_FRM_SETP_B     2U
#define FSI_FRM_SETP_C     3U
#define FSI_FRM_U2_VOLT    4U
#define FSI_FRM_U2_CURR    5U
#define FSI_FRM_U3_VOLT    6U
#define FSI_FRM_U3_CURR    7U
#define FSI_FRM_CMD1       8U
#define FSI_FRM_DATA1      9U
#define FSI_FRM_CMD2       10U
#define FSI_FRM_DATA2      11U
#define FSI_FRM_SHARE_PCT  12U
#define FSI_FRM_CHECKSUM   13U

typedef struct {
    uint16_t u16Mode;          /* FSI_SANDBOX_MODE_*                       */
    uint16_t u16Seq;           /* frame sequence number                    */
    uint16_t au16TxFrame[FSI_SANDBOX_FRAME_WORDS];
    uint16_t au16RxFrame[FSI_SANDBOX_FRAME_WORDS];  /* last_frame          */
    uint16_t u16SharePct;      /* received current-share %x100             */
    uint32_t u32TxCount;
    uint32_t u32RxCount;
    uint32_t u32ErrorCount;    /* checksum mismatches (expect 0)           */
    uint32_t u32PollCount;
} ST_FSI_SANDBOX;

extern ST_FSI_SANDBOX g_sFsiSandbox;

void FsiSandbox_Init(void);
void FsiSandbox_Poll(void);   /* FAKE: build->send->receive->verify        */

/** @brief Received current-share percentage x100 (CC loop consumer). */
uint16_t FsiSandbox_GetSharePct(void);

#endif /* SANDBOX_FSI_SANDBOX_H_ */

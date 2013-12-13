/*
 * Copyright (c) 2008 Carsten Schlote <c.schlote@konzeptpark.de>
 * See file CREDITS for list of people who contributed to this project.
 *
 * This file is part of barebox.
 *
 * barebox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * barebox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with barebox.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Register and bit definitions for the MCF548X and MCF547x
 *  Fast Ethernet Controller (FEC)
 */
#ifndef __MCF548X_FEC_H__
#define __MCF548X_FEC_H__

/*
 *  Fast Ethernet Controller (FEC)
 */

/* Register read/write macros */
#define MCF_FEC_EIR0                     (*(vuint32_t*)(&__MBAR[0x009004]))
#define MCF_FEC_EIMR0                    (*(vuint32_t*)(&__MBAR[0x009008]))
#define MCF_FEC_ECR0                     (*(vuint32_t*)(&__MBAR[0x009024]))
#define MCF_FEC_MMFR0                    (*(vuint32_t*)(&__MBAR[0x009040]))
#define MCF_FEC_MSCR0                    (*(vuint32_t*)(&__MBAR[0x009044]))
#define MCF_FEC_MIBC0                    (*(vuint32_t*)(&__MBAR[0x009064]))
#define MCF_FEC_RCR0                     (*(vuint32_t*)(&__MBAR[0x009084]))
#define MCF_FEC_R_HASH0                  (*(vuint32_t*)(&__MBAR[0x009088]))
#define MCF_FEC_TCR0                     (*(vuint32_t*)(&__MBAR[0x0090C4]))
#define MCF_FEC_PALR0                    (*(vuint32_t*)(&__MBAR[0x0090E4]))
#define MCF_FEC_PAUR0                    (*(vuint32_t*)(&__MBAR[0x0090E8]))
#define MCF_FEC_OPD0                     (*(vuint32_t*)(&__MBAR[0x0090EC]))
#define MCF_FEC_IAUR0                    (*(vuint32_t*)(&__MBAR[0x009118]))
#define MCF_FEC_IALR0                    (*(vuint32_t*)(&__MBAR[0x00911C]))
#define MCF_FEC_GAUR0                    (*(vuint32_t*)(&__MBAR[0x009120]))
#define MCF_FEC_GALR0                    (*(vuint32_t*)(&__MBAR[0x009124]))
#define MCF_FEC_FECTFWR0                 (*(vuint32_t*)(&__MBAR[0x009144]))
#define MCF_FEC_FECRFDR0                 (*(vuint32_t*)(&__MBAR[0x009184]))
#define MCF_FEC_FECRFSR0                 (*(vuint32_t*)(&__MBAR[0x009188]))
#define MCF_FEC_FECRFCR0                 (*(vuint32_t*)(&__MBAR[0x00918C]))
#define MCF_FEC_FECRLRFP0                (*(vuint32_t*)(&__MBAR[0x009190]))
#define MCF_FEC_FECRLWFP0                (*(vuint32_t*)(&__MBAR[0x009194]))
#define MCF_FEC_FECRFAR0                 (*(vuint32_t*)(&__MBAR[0x009198]))
#define MCF_FEC_FECRFRP0                 (*(vuint32_t*)(&__MBAR[0x00919C]))
#define MCF_FEC_FECRFWP0                 (*(vuint32_t*)(&__MBAR[0x0091A0]))
#define MCF_FEC_FECTFDR0                 (*(vuint32_t*)(&__MBAR[0x0091A4]))
#define MCF_FEC_FECTFSR0                 (*(vuint32_t*)(&__MBAR[0x0091A8]))
#define MCF_FEC_FECTFCR0                 (*(vuint32_t*)(&__MBAR[0x0091AC]))
#define MCF_FEC_FECTLRFP0                (*(vuint32_t*)(&__MBAR[0x0091B0]))
#define MCF_FEC_FECTLWFP0                (*(vuint32_t*)(&__MBAR[0x0091B4]))
#define MCF_FEC_FECTFAR0                 (*(vuint32_t*)(&__MBAR[0x0091B8]))
#define MCF_FEC_FECTFRP0                 (*(vuint32_t*)(&__MBAR[0x0091BC]))
#define MCF_FEC_FECTFWP0                 (*(vuint32_t*)(&__MBAR[0x0091C0]))
#define MCF_FEC_FRST0                    (*(vuint32_t*)(&__MBAR[0x0091C4]))
#define MCF_FEC_CTCWR0                   (*(vuint32_t*)(&__MBAR[0x0091C8]))
#define MCF_FEC_RMON_T_DROP0             (*(vuint32_t*)(&__MBAR[0x009200]))
#define MCF_FEC_RMON_T_PACKETS0          (*(vuint32_t*)(&__MBAR[0x009204]))
#define MCF_FEC_RMON_T_BC_PKT0           (*(vuint32_t*)(&__MBAR[0x009208]))
#define MCF_FEC_RMON_T_MC_PKT0           (*(vuint32_t*)(&__MBAR[0x00920C]))
#define MCF_FEC_RMON_T_CRC_ALIGN0        (*(vuint32_t*)(&__MBAR[0x009210]))
#define MCF_FEC_RMON_T_UNDERSIZE0        (*(vuint32_t*)(&__MBAR[0x009214]))
#define MCF_FEC_RMON_T_OVERSIZE0         (*(vuint32_t*)(&__MBAR[0x009218]))
#define MCF_FEC_RMON_T_FRAG0             (*(vuint32_t*)(&__MBAR[0x00921C]))
#define MCF_FEC_RMON_T_JAB0              (*(vuint32_t*)(&__MBAR[0x009220]))
#define MCF_FEC_RMON_T_COL0              (*(vuint32_t*)(&__MBAR[0x009224]))
#define MCF_FEC_RMON_T_P640              (*(vuint32_t*)(&__MBAR[0x009228]))
#define MCF_FEC_RMON_T_P65TO1270         (*(vuint32_t*)(&__MBAR[0x00922C]))
#define MCF_FEC_RMON_T_P128TO2550        (*(vuint32_t*)(&__MBAR[0x009230]))
#define MCF_FEC_RMON_T_P256TO5110        (*(vuint32_t*)(&__MBAR[0x009234]))
#define MCF_FEC_RMON_T_P512TO10230       (*(vuint32_t*)(&__MBAR[0x009238]))
#define MCF_FEC_RMON_T_P1024TO20470      (*(vuint32_t*)(&__MBAR[0x00923C]))
#define MCF_FEC_RMON_T_P_GTE20480        (*(vuint32_t*)(&__MBAR[0x009240]))
#define MCF_FEC_RMON_T_OCTETS0           (*(vuint32_t*)(&__MBAR[0x009244]))
#define MCF_FEC_IEEE_T_DROP0             (*(vuint32_t*)(&__MBAR[0x009248]))
#define MCF_FEC_IEEE_T_FRAME_OK0         (*(vuint32_t*)(&__MBAR[0x00924C]))
#define MCF_FEC_IEEE_T_1COL0             (*(vuint32_t*)(&__MBAR[0x009250]))
#define MCF_FEC_IEEE_T_MCOL0             (*(vuint32_t*)(&__MBAR[0x009254]))
#define MCF_FEC_IEEE_T_DEF0              (*(vuint32_t*)(&__MBAR[0x009258]))
#define MCF_FEC_IEEE_T_LCOL0             (*(vuint32_t*)(&__MBAR[0x00925C]))
#define MCF_FEC_IEEE_T_EXCOL0            (*(vuint32_t*)(&__MBAR[0x009260]))
#define MCF_FEC_IEEE_T_MACERR0           (*(vuint32_t*)(&__MBAR[0x009264]))
#define MCF_FEC_IEEE_T_CSERR0            (*(vuint32_t*)(&__MBAR[0x009268]))
#define MCF_FEC_IEEE_T_SQE0              (*(vuint32_t*)(&__MBAR[0x00926C]))
#define MCF_FEC_IEEE_T_FDXFC0            (*(vuint32_t*)(&__MBAR[0x009270]))
#define MCF_FEC_IEEE_T_OCTETS_OK0        (*(vuint32_t*)(&__MBAR[0x009274]))
#define MCF_FEC_RMON_R_DROP0             (*(vuint32_t*)(&__MBAR[0x009280]))
#define MCF_FEC_RMON_R_PACKETS0          (*(vuint32_t*)(&__MBAR[0x009284]))
#define MCF_FEC_RMON_R_BC_PKT0           (*(vuint32_t*)(&__MBAR[0x009288]))
#define MCF_FEC_RMON_R_MC_PKT0           (*(vuint32_t*)(&__MBAR[0x00928C]))
#define MCF_FEC_RMON_R_CRC_ALIGN0        (*(vuint32_t*)(&__MBAR[0x009290]))
#define MCF_FEC_RMON_R_UNDERSIZE0        (*(vuint32_t*)(&__MBAR[0x009294]))
#define MCF_FEC_RMON_R_OVERSIZE0         (*(vuint32_t*)(&__MBAR[0x009298]))
#define MCF_FEC_RMON_R_FRAG0             (*(vuint32_t*)(&__MBAR[0x00929C]))
#define MCF_FEC_RMON_R_JAB0              (*(vuint32_t*)(&__MBAR[0x0092A0]))
#define MCF_FEC_RMON_R_RESVD_00          (*(vuint32_t*)(&__MBAR[0x0092A4]))
#define MCF_FEC_RMON_R_P640              (*(vuint32_t*)(&__MBAR[0x0092A8]))
#define MCF_FEC_RMON_R_P65TO1270         (*(vuint32_t*)(&__MBAR[0x0092AC]))
#define MCF_FEC_RMON_R_P128TO2550        (*(vuint32_t*)(&__MBAR[0x0092B0]))
#define MCF_FEC_RMON_R_P256TO5110        (*(vuint32_t*)(&__MBAR[0x0092B4]))
#define MCF_FEC_RMON_R_512TO10230        (*(vuint32_t*)(&__MBAR[0x0092B8]))
#define MCF_FEC_RMON_R_1024TO20470       (*(vuint32_t*)(&__MBAR[0x0092BC]))
#define MCF_FEC_RMON_R_P_GTE20480        (*(vuint32_t*)(&__MBAR[0x0092C0]))
#define MCF_FEC_RMON_R_OCTETS0           (*(vuint32_t*)(&__MBAR[0x0092C4]))
#define MCF_FEC_IEEE_R_DROP0             (*(vuint32_t*)(&__MBAR[0x0092C8]))
#define MCF_FEC_IEEE_R_FRAME_OK0         (*(vuint32_t*)(&__MBAR[0x0092CC]))
#define MCF_FEC_IEEE_R_CRC0              (*(vuint32_t*)(&__MBAR[0x0092D0]))
#define MCF_FEC_IEEE_R_ALIGN0            (*(vuint32_t*)(&__MBAR[0x0092D4]))
#define MCF_FEC_IEEE_R_MACERR0           (*(vuint32_t*)(&__MBAR[0x0092D8]))
#define MCF_FEC_IEEE_R_FDXFC0            (*(vuint32_t*)(&__MBAR[0x0092DC]))
#define MCF_FEC_IEEE_R_OCTETS_OK0        (*(vuint32_t*)(&__MBAR[0x0092E0]))
#define MCF_FEC_EIR1                     (*(vuint32_t*)(&__MBAR[0x009804]))
#define MCF_FEC_EIMR1                    (*(vuint32_t*)(&__MBAR[0x009808]))
#define MCF_FEC_ECR1                     (*(vuint32_t*)(&__MBAR[0x009824]))
#define MCF_FEC_MMFR1                    (*(vuint32_t*)(&__MBAR[0x009840]))
#define MCF_FEC_MSCR1                    (*(vuint32_t*)(&__MBAR[0x009844]))
#define MCF_FEC_MIBC1                    (*(vuint32_t*)(&__MBAR[0x009864]))
#define MCF_FEC_RCR1                     (*(vuint32_t*)(&__MBAR[0x009884]))
#define MCF_FEC_R_HASH1                  (*(vuint32_t*)(&__MBAR[0x009888]))
#define MCF_FEC_TCR1                     (*(vuint32_t*)(&__MBAR[0x0098C4]))
#define MCF_FEC_PALR1                    (*(vuint32_t*)(&__MBAR[0x0098E4]))
#define MCF_FEC_PAUR1                    (*(vuint32_t*)(&__MBAR[0x0098E8]))
#define MCF_FEC_OPD1                     (*(vuint32_t*)(&__MBAR[0x0098EC]))
#define MCF_FEC_IAUR1                    (*(vuint32_t*)(&__MBAR[0x009918]))
#define MCF_FEC_IALR1                    (*(vuint32_t*)(&__MBAR[0x00991C]))
#define MCF_FEC_GAUR1                    (*(vuint32_t*)(&__MBAR[0x009920]))
#define MCF_FEC_GALR1                    (*(vuint32_t*)(&__MBAR[0x009924]))
#define MCF_FEC_FECTFWR1                 (*(vuint32_t*)(&__MBAR[0x009944]))
#define MCF_FEC_FECRFDR1                 (*(vuint32_t*)(&__MBAR[0x009984]))
#define MCF_FEC_FECRFSR1                 (*(vuint32_t*)(&__MBAR[0x009988]))
#define MCF_FEC_FECRFCR1                 (*(vuint32_t*)(&__MBAR[0x00998C]))
#define MCF_FEC_FECRLRFP1                (*(vuint32_t*)(&__MBAR[0x009990]))
#define MCF_FEC_FECRLWFP1                (*(vuint32_t*)(&__MBAR[0x009994]))
#define MCF_FEC_FECRFAR1                 (*(vuint32_t*)(&__MBAR[0x009998]))
#define MCF_FEC_FECRFRP1                 (*(vuint32_t*)(&__MBAR[0x00999C]))
#define MCF_FEC_FECRFWP1                 (*(vuint32_t*)(&__MBAR[0x0099A0]))
#define MCF_FEC_FECTFDR1                 (*(vuint32_t*)(&__MBAR[0x0099A4]))
#define MCF_FEC_FECTFSR1                 (*(vuint32_t*)(&__MBAR[0x0099A8]))
#define MCF_FEC_FECTFCR1                 (*(vuint32_t*)(&__MBAR[0x0099AC]))
#define MCF_FEC_FECTLRFP1                (*(vuint32_t*)(&__MBAR[0x0099B0]))
#define MCF_FEC_FECTLWFP1                (*(vuint32_t*)(&__MBAR[0x0099B4]))
#define MCF_FEC_FECTFAR1                 (*(vuint32_t*)(&__MBAR[0x0099B8]))
#define MCF_FEC_FECTFRP1                 (*(vuint32_t*)(&__MBAR[0x0099BC]))
#define MCF_FEC_FECTFWP1                 (*(vuint32_t*)(&__MBAR[0x0099C0]))
#define MCF_FEC_FRST1                    (*(vuint32_t*)(&__MBAR[0x0099C4]))
#define MCF_FEC_CTCWR1                   (*(vuint32_t*)(&__MBAR[0x0099C8]))
#define MCF_FEC_RMON_T_DROP1             (*(vuint32_t*)(&__MBAR[0x009A00]))
#define MCF_FEC_RMON_T_PACKETS1          (*(vuint32_t*)(&__MBAR[0x009A04]))
#define MCF_FEC_RMON_T_BC_PKT1           (*(vuint32_t*)(&__MBAR[0x009A08]))
#define MCF_FEC_RMON_T_MC_PKT1           (*(vuint32_t*)(&__MBAR[0x009A0C]))
#define MCF_FEC_RMON_T_CRC_ALIGN1        (*(vuint32_t*)(&__MBAR[0x009A10]))
#define MCF_FEC_RMON_T_UNDERSIZE1        (*(vuint32_t*)(&__MBAR[0x009A14]))
#define MCF_FEC_RMON_T_OVERSIZE1         (*(vuint32_t*)(&__MBAR[0x009A18]))
#define MCF_FEC_RMON_T_FRAG1             (*(vuint32_t*)(&__MBAR[0x009A1C]))
#define MCF_FEC_RMON_T_JAB1              (*(vuint32_t*)(&__MBAR[0x009A20]))
#define MCF_FEC_RMON_T_COL1              (*(vuint32_t*)(&__MBAR[0x009A24]))
#define MCF_FEC_RMON_T_P641              (*(vuint32_t*)(&__MBAR[0x009A28]))
#define MCF_FEC_RMON_T_P65TO1271         (*(vuint32_t*)(&__MBAR[0x009A2C]))
#define MCF_FEC_RMON_T_P128TO2551        (*(vuint32_t*)(&__MBAR[0x009A30]))
#define MCF_FEC_RMON_T_P256TO5111        (*(vuint32_t*)(&__MBAR[0x009A34]))
#define MCF_FEC_RMON_T_P512TO10231       (*(vuint32_t*)(&__MBAR[0x009A38]))
#define MCF_FEC_RMON_T_P1024TO20471      (*(vuint32_t*)(&__MBAR[0x009A3C]))
#define MCF_FEC_RMON_T_P_GTE20481        (*(vuint32_t*)(&__MBAR[0x009A40]))
#define MCF_FEC_RMON_T_OCTETS1           (*(vuint32_t*)(&__MBAR[0x009A44]))
#define MCF_FEC_IEEE_T_DROP1             (*(vuint32_t*)(&__MBAR[0x009A48]))
#define MCF_FEC_IEEE_T_FRAME_OK1         (*(vuint32_t*)(&__MBAR[0x009A4C]))
#define MCF_FEC_IEEE_T_1COL1             (*(vuint32_t*)(&__MBAR[0x009A50]))
#define MCF_FEC_IEEE_T_MCOL1             (*(vuint32_t*)(&__MBAR[0x009A54]))
#define MCF_FEC_IEEE_T_DEF1              (*(vuint32_t*)(&__MBAR[0x009A58]))
#define MCF_FEC_IEEE_T_LCOL1             (*(vuint32_t*)(&__MBAR[0x009A5C]))
#define MCF_FEC_IEEE_T_EXCOL1            (*(vuint32_t*)(&__MBAR[0x009A60]))
#define MCF_FEC_IEEE_T_MACERR1           (*(vuint32_t*)(&__MBAR[0x009A64]))
#define MCF_FEC_IEEE_T_CSERR1            (*(vuint32_t*)(&__MBAR[0x009A68]))
#define MCF_FEC_IEEE_T_SQE1              (*(vuint32_t*)(&__MBAR[0x009A6C]))
#define MCF_FEC_IEEE_T_FDXFC1            (*(vuint32_t*)(&__MBAR[0x009A70]))
#define MCF_FEC_IEEE_T_OCTETS_OK1        (*(vuint32_t*)(&__MBAR[0x009A74]))
#define MCF_FEC_RMON_R_DROP1             (*(vuint32_t*)(&__MBAR[0x009A80]))
#define MCF_FEC_RMON_R_PACKETS1          (*(vuint32_t*)(&__MBAR[0x009A84]))
#define MCF_FEC_RMON_R_BC_PKT1           (*(vuint32_t*)(&__MBAR[0x009A88]))
#define MCF_FEC_RMON_R_MC_PKT1           (*(vuint32_t*)(&__MBAR[0x009A8C]))
#define MCF_FEC_RMON_R_CRC_ALIGN1        (*(vuint32_t*)(&__MBAR[0x009A90]))
#define MCF_FEC_RMON_R_UNDERSIZE1        (*(vuint32_t*)(&__MBAR[0x009A94]))
#define MCF_FEC_RMON_R_OVERSIZE1         (*(vuint32_t*)(&__MBAR[0x009A98]))
#define MCF_FEC_RMON_R_FRAG1             (*(vuint32_t*)(&__MBAR[0x009A9C]))
#define MCF_FEC_RMON_R_JAB1              (*(vuint32_t*)(&__MBAR[0x009AA0]))
#define MCF_FEC_RMON_R_RESVD_01          (*(vuint32_t*)(&__MBAR[0x009AA4]))
#define MCF_FEC_RMON_R_P641              (*(vuint32_t*)(&__MBAR[0x009AA8]))
#define MCF_FEC_RMON_R_P65TO1271         (*(vuint32_t*)(&__MBAR[0x009AAC]))
#define MCF_FEC_RMON_R_P128TO2551        (*(vuint32_t*)(&__MBAR[0x009AB0]))
#define MCF_FEC_RMON_R_P256TO5111        (*(vuint32_t*)(&__MBAR[0x009AB4]))
#define MCF_FEC_RMON_R_512TO10231        (*(vuint32_t*)(&__MBAR[0x009AB8]))
#define MCF_FEC_RMON_R_1024TO20471       (*(vuint32_t*)(&__MBAR[0x009ABC]))
#define MCF_FEC_RMON_R_P_GTE20481        (*(vuint32_t*)(&__MBAR[0x009AC0]))
#define MCF_FEC_RMON_R_OCTETS1           (*(vuint32_t*)(&__MBAR[0x009AC4]))
#define MCF_FEC_IEEE_R_DROP1             (*(vuint32_t*)(&__MBAR[0x009AC8]))
#define MCF_FEC_IEEE_R_FRAME_OK1         (*(vuint32_t*)(&__MBAR[0x009ACC]))
#define MCF_FEC_IEEE_R_CRC1              (*(vuint32_t*)(&__MBAR[0x009AD0]))
#define MCF_FEC_IEEE_R_ALIGN1            (*(vuint32_t*)(&__MBAR[0x009AD4]))
#define MCF_FEC_IEEE_R_MACERR1           (*(vuint32_t*)(&__MBAR[0x009AD8]))
#define MCF_FEC_IEEE_R_FDXFC1            (*(vuint32_t*)(&__MBAR[0x009ADC]))
#define MCF_FEC_IEEE_R_OCTETS_OK1        (*(vuint32_t*)(&__MBAR[0x009AE0]))
#define MCF_FEC_EIR(x)                   (*(vuint32_t*)(&__MBAR[0x009004+((x)*0x800)]))
#define MCF_FEC_EIMR(x)                  (*(vuint32_t*)(&__MBAR[0x009008+((x)*0x800)]))
#define MCF_FEC_ECR(x)                   (*(vuint32_t*)(&__MBAR[0x009024+((x)*0x800)]))
#define MCF_FEC_MMFR(x)                  (*(vuint32_t*)(&__MBAR[0x009040+((x)*0x800)]))
#define MCF_FEC_MSCR(x)                  (*(vuint32_t*)(&__MBAR[0x009044+((x)*0x800)]))
#define MCF_FEC_MIBC(x)                  (*(vuint32_t*)(&__MBAR[0x009064+((x)*0x800)]))
#define MCF_FEC_RCR(x)                   (*(vuint32_t*)(&__MBAR[0x009084+((x)*0x800)]))
#define MCF_FEC_R_HASH(x)                (*(vuint32_t*)(&__MBAR[0x009088+((x)*0x800)]))
#define MCF_FEC_TCR(x)                   (*(vuint32_t*)(&__MBAR[0x0090C4+((x)*0x800)]))
#define MCF_FEC_PALR(x)                  (*(vuint32_t*)(&__MBAR[0x0090E4+((x)*0x800)]))
#define MCF_FEC_PAUR(x)                  (*(vuint32_t*)(&__MBAR[0x0090E8+((x)*0x800)]))
#define MCF_FEC_OPD(x)                   (*(vuint32_t*)(&__MBAR[0x0090EC+((x)*0x800)]))
#define MCF_FEC_IAUR(x)                  (*(vuint32_t*)(&__MBAR[0x009118+((x)*0x800)]))
#define MCF_FEC_IALR(x)                  (*(vuint32_t*)(&__MBAR[0x00911C+((x)*0x800)]))
#define MCF_FEC_GAUR(x)                  (*(vuint32_t*)(&__MBAR[0x009120+((x)*0x800)]))
#define MCF_FEC_GALR(x)                  (*(vuint32_t*)(&__MBAR[0x009124+((x)*0x800)]))
#define MCF_FEC_FECTFWR(x)               (*(vuint32_t*)(&__MBAR[0x009144+((x)*0x800)]))
#define MCF_FEC_FECRFDR(x)               (*(vuint32_t*)(&__MBAR[0x009184+((x)*0x800)]))
#define MCF_FEC_FECRFSR(x)               (*(vuint32_t*)(&__MBAR[0x009188+((x)*0x800)]))
#define MCF_FEC_FECRFCR(x)               (*(vuint32_t*)(&__MBAR[0x00918C+((x)*0x800)]))
#define MCF_FEC_FECRLRFP(x)              (*(vuint32_t*)(&__MBAR[0x009190+((x)*0x800)]))
#define MCF_FEC_FECRLWFP(x)              (*(vuint32_t*)(&__MBAR[0x009194+((x)*0x800)]))
#define MCF_FEC_FECRFAR(x)               (*(vuint32_t*)(&__MBAR[0x009198+((x)*0x800)]))
#define MCF_FEC_FECRFRP(x)               (*(vuint32_t*)(&__MBAR[0x00919C+((x)*0x800)]))
#define MCF_FEC_FECRFWP(x)               (*(vuint32_t*)(&__MBAR[0x0091A0+((x)*0x800)]))
#define MCF_FEC_FECTFDR(x)               (*(vuint32_t*)(&__MBAR[0x0091A4+((x)*0x800)]))
#define MCF_FEC_FECTFSR(x)               (*(vuint32_t*)(&__MBAR[0x0091A8+((x)*0x800)]))
#define MCF_FEC_FECTFCR(x)               (*(vuint32_t*)(&__MBAR[0x0091AC+((x)*0x800)]))
#define MCF_FEC_FECTLRFP(x)              (*(vuint32_t*)(&__MBAR[0x0091B0+((x)*0x800)]))
#define MCF_FEC_FECTLWFP(x)              (*(vuint32_t*)(&__MBAR[0x0091B4+((x)*0x800)]))
#define MCF_FEC_FECTFAR(x)               (*(vuint32_t*)(&__MBAR[0x0091B8+((x)*0x800)]))
#define MCF_FEC_FECTFRP(x)               (*(vuint32_t*)(&__MBAR[0x0091BC+((x)*0x800)]))
#define MCF_FEC_FECTFWP(x)               (*(vuint32_t*)(&__MBAR[0x0091C0+((x)*0x800)]))
#define MCF_FEC_FRST(x)                  (*(vuint32_t*)(&__MBAR[0x0091C4+((x)*0x800)]))
#define MCF_FEC_CTCWR(x)                 (*(vuint32_t*)(&__MBAR[0x0091C8+((x)*0x800)]))
#define MCF_FEC_RMON_T_DROP(x)           (*(vuint32_t*)(&__MBAR[0x009200+((x)*0x800)]))
#define MCF_FEC_RMON_T_PACKETS(x)        (*(vuint32_t*)(&__MBAR[0x009204+((x)*0x800)]))
#define MCF_FEC_RMON_T_BC_PKT(x)         (*(vuint32_t*)(&__MBAR[0x009208+((x)*0x800)]))
#define MCF_FEC_RMON_T_MC_PKT(x)         (*(vuint32_t*)(&__MBAR[0x00920C+((x)*0x800)]))
#define MCF_FEC_RMON_T_CRC_ALIGN(x)      (*(vuint32_t*)(&__MBAR[0x009210+((x)*0x800)]))
#define MCF_FEC_RMON_T_UNDERSIZE(x)      (*(vuint32_t*)(&__MBAR[0x009214+((x)*0x800)]))
#define MCF_FEC_RMON_T_OVERSIZE(x)       (*(vuint32_t*)(&__MBAR[0x009218+((x)*0x800)]))
#define MCF_FEC_RMON_T_FRAG(x)           (*(vuint32_t*)(&__MBAR[0x00921C+((x)*0x800)]))
#define MCF_FEC_RMON_T_JAB(x)            (*(vuint32_t*)(&__MBAR[0x009220+((x)*0x800)]))
#define MCF_FEC_RMON_T_COL(x)            (*(vuint32_t*)(&__MBAR[0x009224+((x)*0x800)]))
#define MCF_FEC_RMON_T_P64(x)            (*(vuint32_t*)(&__MBAR[0x009228+((x)*0x800)]))
#define MCF_FEC_RMON_T_P65TO127(x)       (*(vuint32_t*)(&__MBAR[0x00922C+((x)*0x800)]))
#define MCF_FEC_RMON_T_P128TO255(x)      (*(vuint32_t*)(&__MBAR[0x009230+((x)*0x800)]))
#define MCF_FEC_RMON_T_P256TO511(x)      (*(vuint32_t*)(&__MBAR[0x009234+((x)*0x800)]))
#define MCF_FEC_RMON_T_P512TO1023(x)     (*(vuint32_t*)(&__MBAR[0x009238+((x)*0x800)]))
#define MCF_FEC_RMON_T_P1024TO2047(x)    (*(vuint32_t*)(&__MBAR[0x00923C+((x)*0x800)]))
#define MCF_FEC_RMON_T_P_GTE2048(x)      (*(vuint32_t*)(&__MBAR[0x009240+((x)*0x800)]))
#define MCF_FEC_RMON_T_OCTETS(x)         (*(vuint32_t*)(&__MBAR[0x009244+((x)*0x800)]))
#define MCF_FEC_IEEE_T_DROP(x)           (*(vuint32_t*)(&__MBAR[0x009248+((x)*0x800)]))
#define MCF_FEC_IEEE_T_FRAME_OK(x)       (*(vuint32_t*)(&__MBAR[0x00924C+((x)*0x800)]))
#define MCF_FEC_IEEE_T_1COL(x)           (*(vuint32_t*)(&__MBAR[0x009250+((x)*0x800)]))
#define MCF_FEC_IEEE_T_MCOL(x)           (*(vuint32_t*)(&__MBAR[0x009254+((x)*0x800)]))
#define MCF_FEC_IEEE_T_DEF(x)            (*(vuint32_t*)(&__MBAR[0x009258+((x)*0x800)]))
#define MCF_FEC_IEEE_T_LCOL(x)           (*(vuint32_t*)(&__MBAR[0x00925C+((x)*0x800)]))
#define MCF_FEC_IEEE_T_EXCOL(x)          (*(vuint32_t*)(&__MBAR[0x009260+((x)*0x800)]))
#define MCF_FEC_IEEE_T_MACERR(x)         (*(vuint32_t*)(&__MBAR[0x009264+((x)*0x800)]))
#define MCF_FEC_IEEE_T_CSERR(x)          (*(vuint32_t*)(&__MBAR[0x009268+((x)*0x800)]))
#define MCF_FEC_IEEE_T_SQE(x)            (*(vuint32_t*)(&__MBAR[0x00926C+((x)*0x800)]))
#define MCF_FEC_IEEE_T_FDXFC(x)          (*(vuint32_t*)(&__MBAR[0x009270+((x)*0x800)]))
#define MCF_FEC_IEEE_T_OCTETS_OK(x)      (*(vuint32_t*)(&__MBAR[0x009274+((x)*0x800)]))
#define MCF_FEC_RMON_R_DROP(x)           (*(vuint32_t*)(&__MBAR[0x009280+((x)*0x800)]))
#define MCF_FEC_RMON_R_PACKETS(x)        (*(vuint32_t*)(&__MBAR[0x009284+((x)*0x800)]))
#define MCF_FEC_RMON_R_BC_PKT(x)         (*(vuint32_t*)(&__MBAR[0x009288+((x)*0x800)]))
#define MCF_FEC_RMON_R_MC_PKT(x)         (*(vuint32_t*)(&__MBAR[0x00928C+((x)*0x800)]))
#define MCF_FEC_RMON_R_CRC_ALIGN(x)      (*(vuint32_t*)(&__MBAR[0x009290+((x)*0x800)]))
#define MCF_FEC_RMON_R_UNDERSIZE(x)      (*(vuint32_t*)(&__MBAR[0x009294+((x)*0x800)]))
#define MCF_FEC_RMON_R_OVERSIZE(x)       (*(vuint32_t*)(&__MBAR[0x009298+((x)*0x800)]))
#define MCF_FEC_RMON_R_FRAG(x)           (*(vuint32_t*)(&__MBAR[0x00929C+((x)*0x800)]))
#define MCF_FEC_RMON_R_JAB(x)            (*(vuint32_t*)(&__MBAR[0x0092A0+((x)*0x800)]))
#define MCF_FEC_RMON_R_RESVD_0(x)        (*(vuint32_t*)(&__MBAR[0x0092A4+((x)*0x800)]))
#define MCF_FEC_RMON_R_P64(x)            (*(vuint32_t*)(&__MBAR[0x0092A8+((x)*0x800)]))
#define MCF_FEC_RMON_R_P65TO127(x)       (*(vuint32_t*)(&__MBAR[0x0092AC+((x)*0x800)]))
#define MCF_FEC_RMON_R_P128TO255(x)      (*(vuint32_t*)(&__MBAR[0x0092B0+((x)*0x800)]))
#define MCF_FEC_RMON_R_P256TO511(x)      (*(vuint32_t*)(&__MBAR[0x0092B4+((x)*0x800)]))
#define MCF_FEC_RMON_R_512TO1023(x)      (*(vuint32_t*)(&__MBAR[0x0092B8+((x)*0x800)]))
#define MCF_FEC_RMON_R_1024TO2047(x)     (*(vuint32_t*)(&__MBAR[0x0092BC+((x)*0x800)]))
#define MCF_FEC_RMON_R_P_GTE2048(x)      (*(vuint32_t*)(&__MBAR[0x0092C0+((x)*0x800)]))
#define MCF_FEC_RMON_R_OCTETS(x)         (*(vuint32_t*)(&__MBAR[0x0092C4+((x)*0x800)]))
#define MCF_FEC_IEEE_R_DROP(x)           (*(vuint32_t*)(&__MBAR[0x0092C8+((x)*0x800)]))
#define MCF_FEC_IEEE_R_FRAME_OK(x)       (*(vuint32_t*)(&__MBAR[0x0092CC+((x)*0x800)]))
#define MCF_FEC_IEEE_R_CRC(x)            (*(vuint32_t*)(&__MBAR[0x0092D0+((x)*0x800)]))
#define MCF_FEC_IEEE_R_ALIGN(x)          (*(vuint32_t*)(&__MBAR[0x0092D4+((x)*0x800)]))
#define MCF_FEC_IEEE_R_MACERR(x)         (*(vuint32_t*)(&__MBAR[0x0092D8+((x)*0x800)]))
#define MCF_FEC_IEEE_R_FDXFC(x)          (*(vuint32_t*)(&__MBAR[0x0092DC+((x)*0x800)]))
#define MCF_FEC_IEEE_R_OCTETS_OK(x)      (*(vuint32_t*)(&__MBAR[0x0092E0+((x)*0x800)]))

/* Bit definitions and macros for MCF_FEC_EIR */
#define MCF_FEC_EIR_RFERR              (0x00020000)
#define MCF_FEC_EIR_XFERR              (0x00040000)
#define MCF_FEC_EIR_XFUN               (0x00080000)
#define MCF_FEC_EIR_RL                 (0x00100000)
#define MCF_FEC_EIR_LC                 (0x00200000)
#define MCF_FEC_EIR_MII                (0x00800000)
#define MCF_FEC_EIR_TXF                (0x08000000)
#define MCF_FEC_EIR_GRA                (0x10000000)
#define MCF_FEC_EIR_BABT               (0x20000000)
#define MCF_FEC_EIR_BABR               (0x40000000)
#define MCF_FEC_EIR_HBERR              (0x80000000)
#define MCF_FEC_EIR_CLEAR_ALL          (0xFFFFFFFF)

/* Bit definitions and macros for MCF_FEC_EIMR */
#define MCF_FEC_EIMR_RFERR             (0x00020000)
#define MCF_FEC_EIMR_XFERR             (0x00040000)
#define MCF_FEC_EIMR_XFUN              (0x00080000)
#define MCF_FEC_EIMR_RL                (0x00100000)
#define MCF_FEC_EIMR_LC                (0x00200000)
#define MCF_FEC_EIMR_MII               (0x00800000)
#define MCF_FEC_EIMR_TXF               (0x08000000)
#define MCF_FEC_EIMR_GRA               (0x10000000)
#define MCF_FEC_EIMR_BABT              (0x20000000)
#define MCF_FEC_EIMR_BABR              (0x40000000)
#define MCF_FEC_EIMR_HBERR             (0x80000000)
#define MCF_FEC_EIMR_MASK_ALL          (0x00000000)
#define MCF_FEC_EIMR_UNMASK_ALL        (0xFFFFFFFF)

/* Bit definitions and macros for MCF_FEC_ECR */
#define MCF_FEC_ECR_RESET              (0x00000001)
#define MCF_FEC_ECR_ETHER_EN           (0x00000002)

/* Bit definitions and macros for MCF_FEC_MMFR */
#define MCF_FEC_MMFR_DATA(x)           (((x)&0x0000FFFF)<<0)
#define MCF_FEC_MMFR_TA(x)             (((x)&0x00000003)<<16)
#define MCF_FEC_MMFR_RA(x)             (((x)&0x0000001F)<<18)
#define MCF_FEC_MMFR_PA(x)             (((x)&0x0000001F)<<23)
#define MCF_FEC_MMFR_OP(x)             (((x)&0x00000003)<<28)
#define MCF_FEC_MMFR_ST(x)             (((x)&0x00000003)<<30)
#define MCF_FEC_MMFR_ST_01             (0x40000000)
#define MCF_FEC_MMFR_OP_READ           (0x20000000)
#define MCF_FEC_MMFR_OP_WRITE          (0x10000000)
#define MCF_FEC_MMFR_TA_10             (0x00020000)

/* Bit definitions and macros for MCF_FEC_MSCR */
#define MCF_FEC_MSCR_MII_SPEED(x)      (((x)&0x0000003F)<<1)
#define MCF_FEC_MSCR_DIS_PREAMBLE      (0x00000080)
#define MCF_FEC_MSCR_MII_SPEED_133     (0x1B<<1)
#define MCF_FEC_MSCR_MII_SPEED_120     (0x18<<1)
#define MCF_FEC_MSCR_MII_SPEED_66      (0xE<<1)
#define MCF_FEC_MSCR_MII_SPEED_60      (0xC<<1)

/* Bit definitions and macros for MCF_FEC_MIBC */
#define MCF_FEC_MIBC_MIB_IDLE          (0x40000000)
#define MCF_FEC_MIBC_MIB_DISABLE       (0x80000000)

/* Bit definitions and macros for MCF_FEC_RCR */
#define MCF_FEC_RCR_LOOP               (0x00000001)
#define MCF_FEC_RCR_DRT                (0x00000002)
#define MCF_FEC_RCR_MII_MODE           (0x00000004)
#define MCF_FEC_RCR_PROM               (0x00000008)
#define MCF_FEC_RCR_BC_REJ             (0x00000010)
#define MCF_FEC_RCR_FCE                (0x00000020)
#define MCF_FEC_RCR_MAX_FL(x)          (((x)&0x000007FF)<<16)

/* Bit definitions and macros for MCF_FEC_R_HASH */
#define MCF_FEC_R_HASH_HASH(x)         (((x)&0x0000003F)<<24)
#define MCF_FEC_R_HASH_MULTCAST        (0x40000000)
#define MCF_FEC_R_HASH_FCE_DC          (0x80000000)

/* Bit definitions and macros for MCF_FEC_TCR */
#define MCF_FEC_TCR_GTS                (0x00000001)
#define MCF_FEC_TCR_HBC                (0x00000002)
#define MCF_FEC_TCR_FDEN               (0x00000004)
#define MCF_FEC_TCR_TFC_PAUSE          (0x00000008)
#define MCF_FEC_TCR_RFC_PAUSE          (0x00000010)

/* Bit definitions and macros for MCF_FEC_PAUR */
#define MCF_FEC_PAUR_TYPE(x)           (((x)&0x0000FFFF)<<0)
#define MCF_FEC_PAUR_PADDR2(x)         (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for MCF_FEC_OPD */
#define MCF_FEC_OPD_OP_PAUSE(x)        (((x)&0x0000FFFF)<<0)
#define MCF_FEC_OPD_OPCODE(x)          (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for MCF_FEC_FECTFWR */
#define MCF_FEC_FECTFWR_X_WMRK(x)      (((x)&0x0000000F)<<0)
#define MCF_FEC_FECTFWR_X_WMRK_64      (0x00000000)
#define MCF_FEC_FECTFWR_X_WMRK_128     (0x00000001)
#define MCF_FEC_FECTFWR_X_WMRK_192     (0x00000002)
#define MCF_FEC_FECTFWR_X_WMRK_256     (0x00000003)
#define MCF_FEC_FECTFWR_X_WMRK_320     (0x00000004)
#define MCF_FEC_FECTFWR_X_WMRK_384     (0x00000005)
#define MCF_FEC_FECTFWR_X_WMRK_448     (0x00000006)
#define MCF_FEC_FECTFWR_X_WMRK_512     (0x00000007)
#define MCF_FEC_FECTFWR_X_WMRK_576     (0x00000008)
#define MCF_FEC_FECTFWR_X_WMRK_640     (0x00000009)
#define MCF_FEC_FECTFWR_X_WMRK_704     (0x0000000A)
#define MCF_FEC_FECTFWR_X_WMRK_768     (0x0000000B)
#define MCF_FEC_FECTFWR_X_WMRK_832     (0x0000000C)
#define MCF_FEC_FECTFWR_X_WMRK_896     (0x0000000D)
#define MCF_FEC_FECTFWR_X_WMRK_960     (0x0000000E)
#define MCF_FEC_FECTFWR_X_WMRK_1024    (0x0000000F)

/* Bit definitions and macros for MCF_FEC_FECRFDR */
#define MCF_FEC_FECRFDR_ADDR0            ((void*)(&__MBAR[0x009184]))
#define MCF_FEC_FECRFDR_ADDR1            ((void*)(&__MBAR[0x009984]))
#define MCF_FEC_FECRFDR_ADDR(x)          ((void*)(&__MBAR[0x009184+(0x800*ch)]))

/* Bit definitions and macros for MCF_FEC_FECRFSR */
#define MCF_FEC_FECRFSR_EMT            (0x00010000)
#define MCF_FEC_FECRFSR_ALARM          (0x00020000)
#define MCF_FEC_FECRFSR_FU             (0x00040000)
#define MCF_FEC_FECRFSR_FR             (0x00080000)
#define MCF_FEC_FECRFSR_OF             (0x00100000)
#define MCF_FEC_FECRFSR_UF             (0x00200000)
#define MCF_FEC_FECRFSR_RXW            (0x00400000)
#define MCF_FEC_FECRFSR_FAE            (0x00800000)
#define MCF_FEC_FECRFSR_FRM(x)         (((x)&0x0000000F)<<24)
#define MCF_FEC_FECRFSR_IP             (0x80000000)

/* Bit definitions and macros for MCF_FEC_FECRFCR */
#define MCF_FEC_FECRFCR_COUNTER(x)     (((x)&0x0000FFFF)<<0)
#define MCF_FEC_FECRFCR_OF_MSK         (0x00080000)
#define MCF_FEC_FECRFCR_UF_MSK         (0x00100000)
#define MCF_FEC_FECRFCR_RXW_MSK        (0x00200000)
#define MCF_FEC_FECRFCR_FAE_MSK        (0x00400000)
#define MCF_FEC_FECRFCR_IP_MSK         (0x00800000)
#define MCF_FEC_FECRFCR_GR(x)          (((x)&0x00000007)<<24)
#define MCF_FEC_FECRFCR_FRM            (0x08000000)
#define MCF_FEC_FECRFCR_TIMER          (0x10000000)
#define MCF_FEC_FECRFCR_WFR            (0x20000000)
#define MCF_FEC_FECRFCR_WCTL           (0x40000000)

/* Bit definitions and macros for MCF_FEC_FECRLRFP */
#define MCF_FEC_FECRLRFP_LRFP(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECRLWFP */
#define MCF_FEC_FECRLWFP_LWFP(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECRFAR */
#define MCF_FEC_FECRFAR_ALARM(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECRFRP */
#define MCF_FEC_FECRFRP_READ(x)        (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECRFWP */
#define MCF_FEC_FECRFWP_WRITE(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECTFDR */
#define MCF_FEC_FECTFDR_TFCW_TC        (0x04000000)
#define MCF_FEC_FECTFDR_TFCW_ABC       (0x02000000)
#define MCF_FEC_FECTFDR_ADDR0            ((void*)(&__MBAR[0x0091A4]))
#define MCF_FEC_FECTFDR_ADDR1            ((void*)(&__MBAR[0x0099A4]))
#define MCF_FEC_FECTFDR_ADDR(x)          ((void*)(&__MBAR[0x0091A4+(0x800*ch)]))

/* Bit definitions and macros for MCF_FEC_FECTFSR */
#define MCF_FEC_FECTFSR_EMT            (0x00010000)
#define MCF_FEC_FECTFSR_ALARM          (0x00020000)
#define MCF_FEC_FECTFSR_FU             (0x00040000)
#define MCF_FEC_FECTFSR_FR             (0x00080000)
#define MCF_FEC_FECTFSR_OF             (0x00100000)
#define MCF_FEC_FECTFSR_UP             (0x00200000)
#define MCF_FEC_FECTFSR_FAE            (0x00800000)
#define MCF_FEC_FECTFSR_FRM(x)         (((x)&0x0000000F)<<24)
#define MCF_FEC_FECTFSR_TXW            (0x40000000)
#define MCF_FEC_FECTFSR_IP             (0x80000000)

/* Bit definitions and macros for MCF_FEC_FECTFCR */
#define MCF_FEC_FECTFCR_RESERVED       (0x00200000)
#define MCF_FEC_FECTFCR_COUNTER(x)     (((x)&0x0000FFFF)<<0|0x00200000)
#define MCF_FEC_FECTFCR_TXW_MSK        (0x00240000)
#define MCF_FEC_FECTFCR_OF_MSK         (0x00280000)
#define MCF_FEC_FECTFCR_UF_MSK         (0x00300000)
#define MCF_FEC_FECTFCR_FAE_MSK        (0x00600000)
#define MCF_FEC_FECTFCR_IP_MSK         (0x00A00000)
#define MCF_FEC_FECTFCR_GR(x)          (((x)&0x00000007)<<24|0x00200000)
#define MCF_FEC_FECTFCR_FRM            (0x08200000)
#define MCF_FEC_FECTFCR_TIMER          (0x10200000)
#define MCF_FEC_FECTFCR_WFR            (0x20200000)
#define MCF_FEC_FECTFCR_WCTL           (0x40200000)

/* Bit definitions and macros for MCF_FEC_FECTLRFP */
#define MCF_FEC_FECTLRFP_LRFP(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECTLWFP */
#define MCF_FEC_FECTLWFP_LWFP(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECTFAR */
#define MCF_FEC_FECTFAR_ALARM(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECTFRP */
#define MCF_FEC_FECTFRP_READ(x)        (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FECTFWP */
#define MCF_FEC_FECTFWP_WRITE(x)       (((x)&0x00000FFF)<<0)

/* Bit definitions and macros for MCF_FEC_FRST */
#define MCF_FEC_FRST_RST_CTL           (0x01000000)
#define MCF_FEC_FRST_SW_RST            (0x02000000)

/* Bit definitions and macros for MCF_FEC_CTCWR */
#define MCF_FEC_CTCWR_TFCW             (0x01000000)
#define MCF_FEC_CTCWR_CRC              (0x02000000)


struct mcf54xx_fec
{
	vuint32_t RES0;
	vuint32_t	eir;                   // 004
	vuint32_t	eimr;                  // 008
	vuint32_t RES1[6];
	vuint32_t	ecr;                   // 024
	vuint32_t RES2[6];
	vuint32_t	mmfr;                  // 040
	vuint32_t	mscr;                  // 044
	vuint32_t RES3[7];
	vuint32_t	mibc;                  // 064
	vuint32_t RES4[7];
	vuint32_t	rcr;                   // 084
	vuint32_t	r_hash;                // 088
	vuint32_t RES5[14];
	vuint32_t	tcr;                   // 0c4
	vuint32_t RES6[7];
	vuint32_t	palr;                  // 0e4
	vuint32_t	paur;                  // 0e8
	vuint32_t	opd;                   // 0ec
	vuint32_t RES7[10];
	vuint32_t	iaur;                  // 118
	vuint32_t	ialr;                  // 11c
	vuint32_t	gaur;                  // 120
	vuint32_t	galr;                  // 124
	vuint32_t RES8[7];
	vuint32_t	fectfwr;               // 144
	vuint32_t RES8a[15];
	vuint32_t	fecrfdr;               // 184
	vuint32_t	fecrfsr;               // 188
	vuint32_t	fecrfcr;               // 18c
	vuint32_t	fecrlrfp;              // 190
	vuint32_t	fecrlwfp;              // 194
	vuint32_t	fecrfar;               // 198
	vuint32_t	fecrfrp;               // 19c
	vuint32_t	fecrfwp;               // 1a0
	vuint32_t	fectfdr;               // 1a4
	vuint32_t	fectfsr;               // 1a8
	vuint32_t	fectfcr;               // 1ac
	vuint32_t	fectlrfp;              // 1b0
	vuint32_t	fectlwfp;              // 1b4
	vuint32_t	fectfar;               // 1b8
	vuint32_t	fectfrp;               // 1bc
	vuint32_t	fectfwp;               // 1c0
	vuint32_t	frst;                  // 1c4
	vuint32_t	ctcwr;                 // 1c8
	vuint32_t RES9[13];

	/* MIB Counters Memory Map */
	vuint32_t	rmon_t_drop;           // 200
	vuint32_t	rmon_t_packets;        // 204
	vuint32_t	rmon_t_bc_pkt;         // 208
	vuint32_t	rmon_t_mc_pkt;         // 20C
	vuint32_t	rmon_t_crc_align;      // 210
	vuint32_t	rmon_t_undersize;      // 214
	vuint32_t	rmon_t_oversize;       // 218
	vuint32_t	rmon_t_frag;           // 21C
	vuint32_t	rmon_t_jab;            // 220
	vuint32_t	rmon_t_col;            // 224
	vuint32_t	rmon_t_p64;            // 228
	vuint32_t	rmon_t_p65to127;       // 22C
	vuint32_t	rmon_t_p128to255;      // 230
	vuint32_t	rmon_t_p256to511;      // 234
	vuint32_t	rmon_t_p512to1023;     // 238
	vuint32_t	rmon_t_p1024to2047;    // 23C
	vuint32_t	rmon_t_p_gte2048;      // 240
	vuint32_t	rmon_t_octets;         // 244
	vuint32_t	ieee_t_drop;           // 248
	vuint32_t	ieee_t_frame_ok;       // 24C
	vuint32_t	ieee_t_1col;           // 250
	vuint32_t	ieee_t_mcol;           // 254
	vuint32_t	ieee_t_def;            // 258
	vuint32_t	ieee_t_lcol;           // 25C
	vuint32_t	ieee_t_excol;          // 260
	vuint32_t	ieee_t_macerr;         // 264
	vuint32_t	ieee_t_cserr;          // 268
	vuint32_t	ieee_t_sqe;            // 26C
	vuint32_t	ieee_t_fdxfc;          // 270
	vuint32_t	ieee_t_octets_ok;      // 274
	vuint32_t RES10[2];

      	vuint32_t rmon_r_drop;           // 280
	vuint32_t	rmon_r_packets;        // 284
	vuint32_t	rmon_r_bc_pkt;         // 288
	vuint32_t	rmon_r_mc_pkt;         // 28C
	vuint32_t	rmon_r_crc_align;      // 290
	vuint32_t	rmon_r_undersize;      // 294
	vuint32_t	rmon_r_oversize;       // 298
	vuint32_t	rmon_r_frag;           // 29C
	vuint32_t	rmon_r_jab;            // 2A0
	vuint32_t	rmon_r_resvd_0;        // 2A4
	vuint32_t	rmon_r_p64;            // 2A8
	vuint32_t	rmon_r_p65to127;       // 2AC
	vuint32_t	rmon_r_p128to255;      // 2B0
	vuint32_t	rmon_r_p256to511;      // 2B4
	vuint32_t	rmon_r_512to1023;      // 2B8
	vuint32_t	rmon_r_1024to2047;     // 2BC
	vuint32_t	rmon_r_p_gte2048;      // 2C0
	vuint32_t	rmon_r_octets;         // 2C4
	vuint32_t	ieee_r_drop;           // 2C8
	vuint32_t	ieee_r_frame_ok;       // 2CC
	vuint32_t	ieee_r_crc;            // 2D0
	vuint32_t	ieee_r_align;          // 2D4
	vuint32_t	ieee_r_macerr;         // 2D8
	vuint32_t	ieee_r_fdxfc;          // 2DC
	vuint32_t	ieee_r_octets_ok;      // 2e0
};

#define MCF_FEC_ADDR(ch)          ((void*)(&__MBAR[0x009000+(0x800*ch)]))
#define MCF_FEC_SIZE(ch)          ((uint32_t)(0x800))

#endif /* __MCF548X_FEC_H__ */

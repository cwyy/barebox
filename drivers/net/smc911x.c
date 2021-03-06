/*
 * SMSC LAN9[12]1[567] Network driver
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifdef CONFIG_ENABLE_DEVICE_NOISE
# define DEBUG
#endif

#include <common.h>

#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <init.h>
#include <xfuncs.h>
#include <errno.h>
#include <clock.h>
#include <asm/io.h>

#define AS CONFIG_DRIVER_NET_SMC911X_ADDRESS_SHIFT

/* Below are the register offsets and bit definitions
 * of the Lan911x memory space
 */
#define RX_DATA_FIFO		 (0x00 << AS)

#define TX_DATA_FIFO		 (0x20 << AS)
#define	TX_CMD_A_INT_ON_COMP			0x80000000
#define	TX_CMD_A_INT_BUF_END_ALGN		0x03000000
#define	TX_CMD_A_INT_4_BYTE_ALGN		0x00000000
#define	TX_CMD_A_INT_16_BYTE_ALGN		0x01000000
#define	TX_CMD_A_INT_32_BYTE_ALGN		0x02000000
#define	TX_CMD_A_INT_DATA_OFFSET		0x001F0000
#define	TX_CMD_A_INT_FIRST_SEG			0x00002000
#define	TX_CMD_A_INT_LAST_SEG			0x00001000
#define	TX_CMD_A_BUF_SIZE			0x000007FF
#define	TX_CMD_B_PKT_TAG			0xFFFF0000
#define	TX_CMD_B_ADD_CRC_DISABLE		0x00002000
#define	TX_CMD_B_DISABLE_PADDING		0x00001000
#define	TX_CMD_B_PKT_BYTE_LENGTH		0x000007FF

#define RX_STATUS_FIFO		(0x40 << AS)
#define	RX_STS_PKT_LEN				0x3FFF0000
#define	RX_STS_ES				0x00008000
#define	RX_STS_BCST				0x00002000
#define	RX_STS_LEN_ERR				0x00001000
#define	RX_STS_RUNT_ERR				0x00000800
#define	RX_STS_MCAST				0x00000400
#define	RX_STS_TOO_LONG				0x00000080
#define	RX_STS_COLL				0x00000040
#define	RX_STS_ETH_TYPE				0x00000020
#define	RX_STS_WDOG_TMT				0x00000010
#define	RX_STS_MII_ERR				0x00000008
#define	RX_STS_DRIBBLING			0x00000004
#define	RX_STS_CRC_ERR				0x00000002
#define RX_STATUS_FIFO_PEEK 	(0x44 << AS)
#define TX_STATUS_FIFO		(0x48 << AS)
#define	TX_STS_TAG				0xFFFF0000
#define	TX_STS_ES				0x00008000
#define	TX_STS_LOC				0x00000800
#define	TX_STS_NO_CARR				0x00000400
#define	TX_STS_LATE_COLL			0x00000200
#define	TX_STS_MANY_COLL			0x00000100
#define	TX_STS_COLL_CNT				0x00000078
#define	TX_STS_MANY_DEFER			0x00000004
#define	TX_STS_UNDERRUN				0x00000002
#define	TX_STS_DEFERRED				0x00000001
#define TX_STATUS_FIFO_PEEK	(0x4C << AS)
#define ID_REV			(0x50 << AS)
#define	ID_REV_CHIP_ID				0xFFFF0000  /* RO */
#define	ID_REV_REV_ID				0x0000FFFF  /* RO */

#define INT_CFG			(0x54 << AS)
#define	INT_CFG_INT_DEAS			0xFF000000  /* R/W */
#define	INT_CFG_INT_DEAS_CLR			0x00004000
#define	INT_CFG_INT_DEAS_STS			0x00002000
#define	INT_CFG_IRQ_INT				0x00001000  /* RO */
#define	INT_CFG_IRQ_EN				0x00000100  /* R/W */
#define	INT_CFG_IRQ_POL				0x00000010  /* R/W Not Affected by SW Reset */
#define	INT_CFG_IRQ_TYPE			0x00000001  /* R/W Not Affected by SW Reset */

#define INT_STS			(0x58 << AS)
#define	INT_STS_SW_INT				0x80000000  /* R/WC */
#define	INT_STS_TXSTOP_INT			0x02000000  /* R/WC */
#define	INT_STS_RXSTOP_INT			0x01000000  /* R/WC */
#define	INT_STS_RXDFH_INT			0x00800000  /* R/WC */
#define	INT_STS_RXDF_INT			0x00400000  /* R/WC */
#define	INT_STS_TX_IOC				0x00200000  /* R/WC */
#define	INT_STS_RXD_INT				0x00100000  /* R/WC */
#define	INT_STS_GPT_INT				0x00080000  /* R/WC */
#define	INT_STS_PHY_INT				0x00040000  /* RO */
#define	INT_STS_PME_INT				0x00020000  /* R/WC */
#define	INT_STS_TXSO				0x00010000  /* R/WC */
#define	INT_STS_RWT				0x00008000  /* R/WC */
#define	INT_STS_RXE				0x00004000  /* R/WC */
#define	INT_STS_TXE				0x00002000  /* R/WC */
//#define	INT_STS_ERX		0x00001000  /* R/WC */
#define	INT_STS_TDFU				0x00000800  /* R/WC */
#define	INT_STS_TDFO				0x00000400  /* R/WC */
#define	INT_STS_TDFA				0x00000200  /* R/WC */
#define	INT_STS_TSFF				0x00000100  /* R/WC */
#define	INT_STS_TSFL				0x00000080  /* R/WC */
//#define	INT_STS_RXDF		0x00000040  /* R/WC */
#define	INT_STS_RDFO				0x00000040  /* R/WC */
#define	INT_STS_RDFL				0x00000020  /* R/WC */
#define	INT_STS_RSFF				0x00000010  /* R/WC */
#define	INT_STS_RSFL				0x00000008  /* R/WC */
#define	INT_STS_GPIO2_INT			0x00000004  /* R/WC */
#define	INT_STS_GPIO1_INT			0x00000002  /* R/WC */
#define	INT_STS_GPIO0_INT			0x00000001  /* R/WC */
#define INT_EN			(0x5C << AS)
#define	INT_EN_SW_INT_EN			0x80000000  /* R/W */
#define	INT_EN_TXSTOP_INT_EN			0x02000000  /* R/W */
#define	INT_EN_RXSTOP_INT_EN			0x01000000  /* R/W */
#define	INT_EN_RXDFH_INT_EN			0x00800000  /* R/W */
//#define	INT_EN_RXDF_INT_EN		0x00400000  /* R/W */
#define	INT_EN_TIOC_INT_EN			0x00200000  /* R/W */
#define	INT_EN_RXD_INT_EN			0x00100000  /* R/W */
#define	INT_EN_GPT_INT_EN			0x00080000  /* R/W */
#define	INT_EN_PHY_INT_EN			0x00040000  /* R/W */
#define	INT_EN_PME_INT_EN			0x00020000  /* R/W */
#define	INT_EN_TXSO_EN				0x00010000  /* R/W */
#define	INT_EN_RWT_EN				0x00008000  /* R/W */
#define	INT_EN_RXE_EN				0x00004000  /* R/W */
#define	INT_EN_TXE_EN				0x00002000  /* R/W */
//#define	INT_EN_ERX_EN			0x00001000  /* R/W */
#define	INT_EN_TDFU_EN				0x00000800  /* R/W */
#define	INT_EN_TDFO_EN				0x00000400  /* R/W */
#define	INT_EN_TDFA_EN				0x00000200  /* R/W */
#define	INT_EN_TSFF_EN				0x00000100  /* R/W */
#define	INT_EN_TSFL_EN				0x00000080  /* R/W */
//#define	INT_EN_RXDF_EN			0x00000040  /* R/W */
#define	INT_EN_RDFO_EN				0x00000040  /* R/W */
#define	INT_EN_RDFL_EN				0x00000020  /* R/W */
#define	INT_EN_RSFF_EN				0x00000010  /* R/W */
#define	INT_EN_RSFL_EN				0x00000008  /* R/W */
#define	INT_EN_GPIO2_INT			0x00000004  /* R/W */
#define	INT_EN_GPIO1_INT			0x00000002  /* R/W */
#define	INT_EN_GPIO0_INT			0x00000001  /* R/W */

#define BYTE_TEST		(0x64 << AS)
#define FIFO_INT		(0x68 << AS)
#define	FIFO_INT_TX_AVAIL_LEVEL			0xFF000000  /* R/W */
#define	FIFO_INT_TX_STS_LEVEL			0x00FF0000  /* R/W */
#define	FIFO_INT_RX_AVAIL_LEVEL			0x0000FF00  /* R/W */
#define	FIFO_INT_RX_STS_LEVEL			0x000000FF  /* R/W */

#define RX_CFG			(0x6C << AS)
#define	RX_CFG_RX_END_ALGN			0xC0000000  /* R/W */
#define		RX_CFG_RX_END_ALGN4		0x00000000  /* R/W */
#define		RX_CFG_RX_END_ALGN16		0x40000000  /* R/W */
#define		RX_CFG_RX_END_ALGN32		0x80000000  /* R/W */
#define	RX_CFG_RX_DMA_CNT			0x0FFF0000  /* R/W */
#define	RX_CFG_RX_DUMP				0x00008000  /* R/W */
#define	RX_CFG_RXDOFF				0x00001F00  /* R/W */
//#define	RX_CFG_RXBAD			0x00000001  /* R/W */

#define TX_CFG			(0x70 << AS)
//#define	TX_CFG_TX_DMA_LVL		0xE0000000	 /* R/W */
//#define	TX_CFG_TX_DMA_CNT		0x0FFF0000	 /* R/W Self Clearing */
#define	TX_CFG_TXS_DUMP				0x00008000  /* Self Clearing */
#define	TX_CFG_TXD_DUMP				0x00004000  /* Self Clearing */
#define	TX_CFG_TXSAO				0x00000004  /* R/W */
#define	TX_CFG_TX_ON				0x00000002  /* R/W */
#define	TX_CFG_STOP_TX				0x00000001  /* Self Clearing */

#define HW_CFG			(0x74 << AS)
#define	HW_CFG_TTM				0x00200000  /* R/W */
#define	HW_CFG_SF				0x00100000  /* R/W */
#define	HW_CFG_TX_FIF_SZ			0x000F0000  /* R/W */
#define	HW_CFG_TR				0x00003000  /* R/W */
#define	HW_CFG_PHY_CLK_SEL			0x00000060  /* R/W */
#define	HW_CFG_PHY_CLK_SEL_INT_PHY 		0x00000000  /* R/W */
#define	HW_CFG_PHY_CLK_SEL_EXT_PHY 		0x00000020  /* R/W */
#define	HW_CFG_PHY_CLK_SEL_CLK_DIS 		0x00000040  /* R/W */
#define	HW_CFG_SMI_SEL				0x00000010  /* R/W */
#define	HW_CFG_EXT_PHY_DET			0x00000008  /* RO */
#define	HW_CFG_EXT_PHY_EN			0x00000004  /* R/W */
#define	HW_CFG_32_16_BIT_MODE			0x00000004  /* RO */
#define	HW_CFG_SRST_TO				0x00000002  /* RO */
#define	HW_CFG_SRST				0x00000001  /* Self Clearing */

#define RX_DP_CTRL		(0x78 << AS)
#define	RX_DP_CTRL_RX_FFWD			0x80000000  /* R/W */
#define	RX_DP_CTRL_FFWD_BUSY			0x80000000  /* RO */

#define RX_FIFO_INF		(0x7C << AS)
#define	 RX_FIFO_INF_RXSUSED			0x00FF0000  /* RO */
#define	 RX_FIFO_INF_RXDUSED			0x0000FFFF  /* RO */

#define TX_FIFO_INF		(0x80 << AS)
#define	TX_FIFO_INF_TSUSED			0x00FF0000  /* RO */
#define	TX_FIFO_INF_TDFREE			0x0000FFFF  /* RO */

#define PMT_CTRL		(0x84 << AS)
#define	PMT_CTRL_PM_MODE			0x00003000  /* Self Clearing */
#define	PMT_CTRL_PHY_RST			0x00000400  /* Self Clearing */
#define	PMT_CTRL_WOL_EN				0x00000200  /* R/W */
#define	PMT_CTRL_ED_EN				0x00000100  /* R/W */
#define	PMT_CTRL_PME_TYPE			0x00000040  /* R/W Not Affected by SW Reset */
#define	PMT_CTRL_WUPS				0x00000030  /* R/WC */
#define	PMT_CTRL_WUPS_NOWAKE			0x00000000  /* R/WC */
#define	PMT_CTRL_WUPS_ED			0x00000010  /* R/WC */
#define	PMT_CTRL_WUPS_WOL			0x00000020  /* R/WC */
#define	PMT_CTRL_WUPS_MULTI			0x00000030  /* R/WC */
#define	PMT_CTRL_PME_IND			0x00000008  /* R/W */
#define	PMT_CTRL_PME_POL			0x00000004  /* R/W */
#define	PMT_CTRL_PME_EN				0x00000002  /* R/W Not Affected by SW Reset */
#define	PMT_CTRL_READY				0x00000001  /* RO */

#define GPIO_CFG		(0x88 << AS)
#define	GPIO_CFG_LED3_EN			0x40000000  /* R/W */
#define	GPIO_CFG_LED2_EN			0x20000000  /* R/W */
#define	GPIO_CFG_LED1_EN			0x10000000  /* R/W */
#define	GPIO_CFG_GPIO2_INT_POL			0x04000000  /* R/W */
#define	GPIO_CFG_GPIO1_INT_POL			0x02000000  /* R/W */
#define	GPIO_CFG_GPIO0_INT_POL			0x01000000  /* R/W */
#define	GPIO_CFG_EEPR_EN			0x00700000  /* R/W */
#define	GPIO_CFG_GPIOBUF2			0x00040000  /* R/W */
#define	GPIO_CFG_GPIOBUF1			0x00020000  /* R/W */
#define	GPIO_CFG_GPIOBUF0			0x00010000  /* R/W */
#define	GPIO_CFG_GPIODIR2			0x00000400  /* R/W */
#define	GPIO_CFG_GPIODIR1			0x00000200  /* R/W */
#define	GPIO_CFG_GPIODIR0			0x00000100  /* R/W */
#define	GPIO_CFG_GPIOD4				0x00000010  /* R/W */
#define	GPIO_CFG_GPIOD3				0x00000008  /* R/W */
#define	GPIO_CFG_GPIOD2				0x00000004  /* R/W */
#define	GPIO_CFG_GPIOD1				0x00000002  /* R/W */
#define	GPIO_CFG_GPIOD0				0x00000001  /* R/W */

#define GPT_CFG			(0x8C << AS)
#define	GPT_CFG_TIMER_EN			0x20000000  /* R/W */
#define	GPT_CFG_GPT_LOAD			0x0000FFFF  /* R/W */

#define GPT_CNT			(0x90 << AS)
#define	GPT_CNT_GPT_CNT				0x0000FFFF  /* RO */

#define ENDIAN			(0x98 << AS)
#define FREE_RUN		(0x9C << AS)
#define RX_DROP			(0xA0 << AS)
#define MAC_CSR_CMD		(0xA4 << AS)
#define	 MAC_CSR_CMD_CSR_BUSY			0x80000000  /* Self Clearing */
#define	 MAC_CSR_CMD_R_NOT_W			0x40000000  /* R/W */
#define	 MAC_CSR_CMD_CSR_ADDR			0x000000FF  /* R/W */

#define MAC_CSR_DATA		(0xA8 << AS)
#define AFC_CFG			(0xAC << AS)
#define		AFC_CFG_AFC_HI			0x00FF0000  /* R/W */
#define		AFC_CFG_AFC_LO			0x0000FF00  /* R/W */
#define		AFC_CFG_BACK_DUR		0x000000F0  /* R/W */
#define		AFC_CFG_FCMULT			0x00000008  /* R/W */
#define		AFC_CFG_FCBRD			0x00000004  /* R/W */
#define		AFC_CFG_FCADD			0x00000002  /* R/W */
#define		AFC_CFG_FCANY			0x00000001  /* R/W */

#define E2P_CMD			(0xB0 << AS)
#define		E2P_CMD_EPC_BUSY		0x80000000  /* Self Clearing */
#define		E2P_CMD_EPC_CMD			0x70000000  /* R/W */
#define		E2P_CMD_EPC_CMD_READ		0x00000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWDS		0x10000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWEN		0x20000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRITE		0x30000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRAL		0x40000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERASE		0x50000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERAL		0x60000000  /* R/W */
#define		E2P_CMD_EPC_CMD_RELOAD		0x70000000  /* R/W */
#define		E2P_CMD_EPC_TIMEOUT		0x00000200  /* RO */
#define		E2P_CMD_MAC_ADDR_LOADED		0x00000100  /* RO */
#define		E2P_CMD_EPC_ADDR		0x000000FF  /* R/W */

#define E2P_DATA		(0xB4 << AS)
#define	E2P_DATA_EEPROM_DATA			0x000000FF  /* R/W */
/* end of LAN register offsets and bit definitions */

/* MAC Control and Status registers */
#define MAC_CR			0x01  /* R/W */

/* MAC_CR - MAC Control Register */
#define MAC_CR_RXALL			0x80000000
// TODO: delete this bit? It is not described in the data sheet.
#define MAC_CR_HBDIS			0x10000000
#define MAC_CR_RCVOWN			0x00800000
#define MAC_CR_LOOPBK			0x00200000
#define MAC_CR_FDPX			0x00100000
#define MAC_CR_MCPAS			0x00080000
#define MAC_CR_PRMS			0x00040000
#define MAC_CR_INVFILT			0x00020000
#define MAC_CR_PASSBAD			0x00010000
#define MAC_CR_HFILT			0x00008000
#define MAC_CR_HPFILT			0x00002000
#define MAC_CR_LCOLL			0x00001000
#define MAC_CR_BCAST			0x00000800
#define MAC_CR_DISRTY			0x00000400
#define MAC_CR_PADSTR			0x00000100
#define MAC_CR_BOLMT_MASK		0x000000C0
#define MAC_CR_DFCHK			0x00000020
#define MAC_CR_TXEN			0x00000008
#define MAC_CR_RXEN			0x00000004

#define ADDRH			0x02	  /* R/W mask 0x0000FFFFUL */
#define ADDRL			0x03	  /* R/W mask 0xFFFFFFFFUL */
#define HASHH			0x04	  /* R/W */
#define HASHL			0x05	  /* R/W */

#define MII_ACC			0x06	  /* R/W */
#define MII_ACC_PHY_ADDR		0x0000F800
#define MII_ACC_MIIRINDA		0x000007C0
#define MII_ACC_MII_WRITE		0x00000002
#define MII_ACC_MII_BUSY		0x00000001

#define MII_DATA		0x07	  /* R/W mask 0x0000FFFFUL */

#ifdef FLOW
#undef FLOW	/* clashed with include/asm/cpu/defBF561.h:1654 */
#endif

#define FLOW			0x08	  /* R/W */
#define FLOW_FCPT			0xFFFF0000
#define FLOW_FCPASS			0x00000004
#define FLOW_FCEN			0x00000002
#define FLOW_FCBSY			0x00000001

#define VLAN1			0x09	  /* R/W mask 0x0000FFFFUL */
#define VLAN1_VTI1			0x0000ffff

#define VLAN2			0x0A	  /* R/W mask 0x0000FFFFUL */
#define VLAN2_VTI2			0x0000ffff

#define WUFF			0x0B	  /* WO */

#define WUCSR			0x0C	  /* R/W */
#define WUCSR_GUE			0x00000200
#define WUCSR_WUFR			0x00000040
#define WUCSR_MPR			0x00000020
#define WUCSR_WAKE_EN			0x00000004
#define WUCSR_MPEN			0x00000002

/* Chip ID values */
#define CHIP_9115	0x115
#define CHIP_9116	0x116
#define CHIP_9117	0x117
#define CHIP_9118	0x118
#define CHIP_9215	0x115a
#define CHIP_9216	0x116a
#define CHIP_9217	0x117a
#define CHIP_9218	0x118a

struct smc911x_priv {
	struct miiphy_device miiphy;
	unsigned long base;
};

struct chip_id {
	u16 id;
	char *name;
};

static const struct chip_id chip_ids[] =  {
	{ CHIP_9115, "LAN9115" },
	{ CHIP_9116, "LAN9116" },
	{ CHIP_9117, "LAN9117" },
	{ CHIP_9118, "LAN9118" },
	{ CHIP_9215, "LAN9215" },
	{ CHIP_9216, "LAN9216" },
	{ CHIP_9217, "LAN9217" },
	{ CHIP_9218, "LAN9218" },
	{ 0, NULL },
};

#define DRIVERNAME "smc911x"

static int smc911x_mac_wait_busy(struct smc911x_priv *priv)
{
	uint64_t start = get_time_ns();

	while (!is_timeout(start, MSECOND)) {
		if (!(readl(priv->base + MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY))
			return 0;
	}

	printf("%s: mac timeout\n", __FUNCTION__);
	return -1;
}

static u32 smc911x_get_mac_csr(struct eth_device *edev, u8 reg)
{
	struct smc911x_priv *priv = edev->priv;
	ulong val;

	smc911x_mac_wait_busy(priv);

	writel(MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_R_NOT_W | reg,
			priv->base + MAC_CSR_CMD);

	smc911x_mac_wait_busy(priv);

	val = readl(priv->base + MAC_CSR_DATA);

	return val;
}

static void smc911x_set_mac_csr(struct eth_device *edev, u8 reg, u32 data)
{
	struct smc911x_priv *priv = edev->priv;

	smc911x_mac_wait_busy(priv);

	writel(data, priv->base + MAC_CSR_DATA);
	writel(MAC_CSR_CMD_CSR_BUSY | reg, priv->base + MAC_CSR_CMD);

	smc911x_mac_wait_busy(priv);
}

static int smc911x_get_ethaddr(struct eth_device *edev, unsigned char *m)
{
	unsigned long addrh, addrl;

	addrh = smc911x_get_mac_csr(edev, ADDRH);
	addrl = smc911x_get_mac_csr(edev, ADDRL);

	m[0] = (addrl       ) & 0xff;
	m[1] = (addrl >>  8 ) & 0xff;
	m[2] = (addrl >> 16 ) & 0xff;
	m[3] = (addrl >> 24 ) & 0xff;
	m[4] = (addrh       ) & 0xff;
	m[5] = (addrh >>  8 ) & 0xff;

	/* we get 0xff when there is no eeprom connected */
	if ((m[0] & m[1] & m[2] & m[3] & m[4] & m[5]) == 0xff)
		return -1;

	return 0;
}

static int smc911x_set_ethaddr(struct eth_device *edev, unsigned char *m)
{
	unsigned long addrh, addrl;

	addrl = m[0] | m[1] << 8 | m[2] << 16 | m[3] << 24;
	addrh = m[4] | m[5] << 8;
	smc911x_set_mac_csr(edev, ADDRH, addrh);
	smc911x_set_mac_csr(edev, ADDRL, addrl);

	return 0;
}

static int smc911x_phy_read(struct miiphy_device *mdev, uint8_t phy_addr,
		uint8_t reg, uint16_t * val)
{
	struct eth_device *edev = mdev->edev;

	while (smc911x_get_mac_csr(edev, MII_ACC) & MII_ACC_MII_BUSY);

	smc911x_set_mac_csr(edev, MII_ACC, phy_addr << 11 | reg << 6 |
			MII_ACC_MII_BUSY);

	while (smc911x_get_mac_csr(edev, MII_ACC) & MII_ACC_MII_BUSY);

	*val = smc911x_get_mac_csr(edev, MII_DATA);

	return 0;
}

static int smc911x_phy_write(struct miiphy_device *mdev, uint8_t phy_addr,
	uint8_t reg, uint16_t val)
{
	struct eth_device *edev = mdev->edev;

	while (smc911x_get_mac_csr(edev, MII_ACC) & MII_ACC_MII_BUSY);

	smc911x_set_mac_csr(edev, MII_DATA, val);
	smc911x_set_mac_csr(edev, MII_ACC,
		phy_addr << 11 | reg << 6 | MII_ACC_MII_BUSY |
		MII_ACC_MII_WRITE);

	while (smc911x_get_mac_csr(edev, MII_ACC) & MII_ACC_MII_BUSY);

	return 0;
}

static int smc911x_phy_reset(struct eth_device *edev)
{
	struct smc911x_priv *priv = edev->priv;
	u32 reg;

	reg = readl(priv->base + PMT_CTRL);
	reg &= 0xfcf;
	reg |= PMT_CTRL_PHY_RST;
	writel(reg, priv->base + PMT_CTRL);

	mdelay(100);

	return 0;
}

static void smc911x_reset(struct eth_device *edev)
{
	struct smc911x_priv *priv = edev->priv;
	uint64_t start;

	/* Take out of PM setting first */
	if (readl(priv->base + PMT_CTRL) & PMT_CTRL_READY) {
		/* Write to the bytetest will take out of powerdown */
		writel(0, priv->base + BYTE_TEST);

		start = get_time_ns();
		while(1) {
			if ((readl(priv->base + PMT_CTRL) & PMT_CTRL_READY))
				break;
			if (is_timeout(start, 100 * USECOND)) {
				printf(DRIVERNAME
					": timeout waiting for PM restore\n");
				return;
			}
		}
	}

	/* Disable interrupts */
	writel(0, priv->base + INT_EN);

	writel(HW_CFG_SRST, priv->base + HW_CFG);

	start = get_time_ns();
	while(1) {
		if (!(readl(priv->base + E2P_CMD) & E2P_CMD_EPC_BUSY))
			break;
		if (is_timeout(start, 10 * MSECOND)) {
			printf(DRIVERNAME ": reset timeout\n");
			return;
		}
	}

	/* Reset the FIFO level and flow control settings */
	smc911x_set_mac_csr(edev, FLOW, FLOW_FCPT | FLOW_FCEN);

	writel(0x0050287F, priv->base + AFC_CFG);

	/* Set to LED outputs */
	writel(0x70070000, priv->base + GPIO_CFG);
}

static void smc911x_enable(struct eth_device *edev)
{
	struct smc911x_priv *priv = edev->priv;

	/* Enable TX */
	writel(8 << 16 | HW_CFG_SF, priv->base + HW_CFG);

	writel(GPT_CFG_TIMER_EN | 10000, priv->base + GPT_CFG);

	writel(TX_CFG_TX_ON, priv->base + TX_CFG);

	/* no padding to start of packets */
	writel(RX_CFG_RX_DUMP, priv->base + RX_CFG);
}

static int smc911x_eth_open(struct eth_device *edev)
{
	struct smc911x_priv *priv = (struct smc911x_priv *)edev->priv;

	miiphy_wait_aneg(&priv->miiphy);
	miiphy_print_status(&priv->miiphy);

	/* Turn on Tx + Rx */
	smc911x_enable(edev);
	return 0;
}

static int smc911x_eth_send(struct eth_device *edev, void *packet, int length)
{
	struct smc911x_priv *priv = (struct smc911x_priv *)edev->priv;
	u32 *data = (u32*)packet;
	u32 tmplen;
	u32 status;
	uint64_t start;

	writel(TX_CMD_A_INT_FIRST_SEG | TX_CMD_A_INT_LAST_SEG | length,
			priv->base + TX_DATA_FIFO);
	writel(length, priv->base + TX_DATA_FIFO);

	tmplen = (length + 3) / 4;

	while(tmplen--)
		writel(*data++, priv->base + TX_DATA_FIFO);

	/* wait for transmission */
	start = get_time_ns();
	while (1) {
		if ((readl(priv->base + TX_FIFO_INF) &
					TX_FIFO_INF_TSUSED) >> 16)
			break;
		if (is_timeout(start, 100 * MSECOND)) {
			printf("TX timeout\n");
			return -1;
		}
	}

	/* get status. Ignore 'no carrier' error, it has no meaning for
	 * full duplex operation
	 */
	status = readl(priv->base + TX_STATUS_FIFO) & (TX_STS_LOC |
		TX_STS_LATE_COLL | TX_STS_MANY_COLL | TX_STS_MANY_DEFER |
		TX_STS_UNDERRUN);

	if(!status)
		return 0;

	printf(DRIVERNAME ": failed to send packet: %s%s%s%s%s\n",
		status & TX_STS_LOC ? "TX_STS_LOC " : "",
		status & TX_STS_LATE_COLL ? "TX_STS_LATE_COLL " : "",
		status & TX_STS_MANY_COLL ? "TX_STS_MANY_COLL " : "",
		status & TX_STS_MANY_DEFER ? "TX_STS_MANY_DEFER " : "",
		status & TX_STS_UNDERRUN ? "TX_STS_UNDERRUN" : "");

	return -1;
}

static void smc911x_eth_halt(struct eth_device *edev)
{
	struct smc911x_priv *priv = (struct smc911x_priv *)edev->priv;

	/* Disable TX */
	writel(TX_CFG_STOP_TX, priv->base + TX_CFG);

//	smc911x_reset(edev);
}

static int smc911x_eth_rx(struct eth_device *edev)
{
	struct smc911x_priv *priv = (struct smc911x_priv *)edev->priv;
	u32 *data = (u32 *)NetRxPackets[0];
	u32 pktlen, tmplen;
	u32 status;

	if((readl(priv->base + RX_FIFO_INF) & RX_FIFO_INF_RXSUSED) >> 16) {
		status = readl(priv->base + RX_STATUS_FIFO);
		pktlen = (status & RX_STS_PKT_LEN) >> 16;

		writel(0, priv->base + RX_CFG);

		tmplen = (pktlen + 2 + 3) / 4;
		while(tmplen--)
			*data++ = readl(priv->base + RX_DATA_FIFO);

		if(status & RX_STS_ES)
			printf(DRIVERNAME
				": dropped bad packet. Status: 0x%08x\n",
				status);
		else
			NetReceive(NetRxPackets[0], pktlen);
	}

	return 0;
}

static int smc911x_init_dev(struct eth_device *edev)
{
	struct smc911x_priv *priv = (struct smc911x_priv *)edev->priv;

	smc911x_set_mac_csr(edev, MAC_CR, MAC_CR_TXEN | MAC_CR_RXEN |
			MAC_CR_HBDIS);

	miiphy_restart_aneg(&priv->miiphy);

	return 0;
}

static int smc911x_probe(struct device_d *dev)
{
	struct eth_device *edev;
	struct smc911x_priv *priv;
	ulong val;
	int i;

	debug("smc911x_eth_init()\n");

	val = readl(dev->map_base + BYTE_TEST);
	if(val != 0x87654321) {
		printf(DRIVERNAME
			": no smc911x found on 0x%08x (byte_test=0x%08x)\n",
			dev->map_base, val);
		return -ENODEV;
	}

	val = readl(dev->map_base + ID_REV) >> 16;
	for(i = 0; chip_ids[i].id != 0; i++) {
		if (chip_ids[i].id == val) break;
	}
	if (!chip_ids[i].id) {
		printf(DRIVERNAME ": Unknown chip ID %04x\n", val);
		return -ENODEV;
	}

	debug(DRIVERNAME ": detected %s controller\n", chip_ids[i].name);

	edev = xzalloc(sizeof(struct eth_device) +
			sizeof(struct smc911x_priv));
	dev->type_data = edev;
	edev->priv = (struct smc911x_priv *)(edev + 1);

	priv = edev->priv;

	edev->init = smc911x_init_dev;
	edev->open = smc911x_eth_open;
	edev->send = smc911x_eth_send;
	edev->recv = smc911x_eth_rx;
	edev->halt = smc911x_eth_halt;
	edev->get_ethaddr = smc911x_get_ethaddr;
	edev->set_ethaddr = smc911x_set_ethaddr;

	priv->miiphy.read = smc911x_phy_read;
	priv->miiphy.write = smc911x_phy_write;
	priv->miiphy.address = 1;
	priv->miiphy.flags = 0;
	priv->miiphy.edev = edev;
	priv->base = dev->map_base;

	smc911x_reset(edev);
	smc911x_phy_reset(edev);

	miiphy_register(&priv->miiphy);
	eth_register(edev);

        return 0;
}

static struct driver_d smc911x_driver = {
        .name  = "smc911x",
        .probe = smc911x_probe,
};

static int smc911x_init(void)
{
        register_driver(&smc911x_driver);
        return 0;
}

device_initcall(smc911x_init);


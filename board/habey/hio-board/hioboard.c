/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2014 O.S. Systems Software LTDA.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/video.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <miiphy.h>
#include <netdev.h>
#include <phy.h>
#include <input.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define USDHC1_CD_GPIO		IMX_GPIO_NR(1, 2)
#define USDHC3_CD_GPIO		IMX_GPIO_NR(3, 9)
#define ETH_PHY_RESET		IMX_GPIO_NR(3, 29)

static int hdmi_detect(void);

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	//IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	//IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
    IOMUX_PADS(PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
    IOMUX_PADS(PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

//add boot pins
#define PIN1_GPIO	IMX_GPIO_NR(3, 5)
#define PIN2_GPIO	IMX_GPIO_NR(3, 11)

iomux_v3_cfg_t const BootPin_pads[] = {
	IOMUX_PADS(PAD_EIM_DA5__GPIO3_IO05 | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA11__GPIO3_IO11 | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

//iob3200 usb-hub
#define USBHUB_GPIO	IMX_GPIO_NR(4, 6)
iomux_v3_cfg_t const usbhub_pads[] = {
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	
 	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_NANDF_D4__SD2_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_NANDF_D5__SD2_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_NANDF_D6__SD2_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_NANDF_D7__SD2_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_NANDF_D2__GPIO2_IO02	| MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
 	IOMUX_PADS(PAD_CSI0_DAT10__GPIO5_IO28 | MUX_PAD_CTRL(UART_PAD_CTRL)),/*MQ*/
};
 
//emmc flash 
iomux_v3_cfg_t const usdhc3_pads[] = {
 	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	//IOMUX_PADS(PAD_NANDF_D0__GPIO2_IO00    | MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
};

//sd card  
iomux_v3_cfg_t const usdhc4_pads[] = {
 	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	//IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	//IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	//IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
 	//IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC    | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* AR8031 PHY Reset */
	IOMUX_PADS(PAD_EIM_D29__GPIO3_IO29    | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
	SETUP_IOMUX_PADS(BootPin_pads);
	SETUP_IOMUX_PADS(usbhub_pads);
}

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);

	/* Reset AR8031 PHY */
	gpio_direction_output(ETH_PHY_RESET, 0);
	udelay(500);
	gpio_set_value(ETH_PHY_RESET, 1);
}

struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 2)
#define USDHC3_CD_GPIO	IMX_GPIO_NR(2, 0)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		//ret = !gpio_get_value(USDHC3_CD_GPIO);
		ret = 1;
		break;
	case USDHC4_BASE_ADDR:
		ret = 1; /* eMMC/uSDHC4 is always present */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int ret;
	u32 index = 0;
	int pin1 = 0, pin2 = 0;

	//add iob3200 usb-hub gpio
	gpio_direction_output(USBHUB_GPIO, 0);
	udelay(1000);
	gpio_set_value(USBHUB_GPIO, 1);
	
	
	//add boot pins
	gpio_direction_input(PIN1_GPIO);
	gpio_direction_input(PIN2_GPIO);
	
	pin1 = gpio_get_value(PIN1_GPIO);
	pin2 = gpio_get_value(PIN2_GPIO);
	
	if (pin1 == 0 && pin2 == 1) {
		printf("boot from tf....\n");	
		
		//sd4--sd card
		//setenv("mmcroot", "/dev/mmcblk3p2 rootwait rw");
        //setenv("mmcdev", "2");
 
		SETUP_IOMUX_PADS(usdhc4_pads);
		usdhc_cfg[1].esdhc_base = USDHC4_BASE_ADDR;
		usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
		usdhc_cfg[1].max_bus_width = 4;	
		gd->arch.sdhc_clk = usdhc_cfg[1].sdhc_clk;
		
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[1]);
		if (ret)
			return ret;
	
	} else {
		printf("boot from inand....\n");

		//sd3--emmc flash
        //setenv("mmcroot", "/dev/mmcblk2p2 rootwait rw");
        //setenv("mmcdev", "1");
 
		SETUP_IOMUX_PADS(usdhc3_pads);
		usdhc_cfg[2].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		usdhc_cfg[2].max_bus_width = 4;	
		gd->arch.sdhc_clk = usdhc_cfg[2].sdhc_clk;
		
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[2]);
		if (ret)
			return ret;	
	}

		
	return 0;
}
#if 0
int board_mmc_init(bd_t *bis)
{
	printf("board_mmc_init 1 \n");
#ifndef CONFIG_SPL_BUILD
	int ret;
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    SD2
	 * mmc1                    SD3
	 * mmc2                    eMMC
	 */
#if 0	 
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			gpio_direction_input(USDHC2_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			gpio_direction_input(USDHC3_CD_GPIO);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		case 2:
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       i + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[2]);
		if (ret)
			return ret;
	}
#endif

printf("board_mmc_init 2 \n");
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

printf("board_mmc_init 3 \n");			
			ret = fsl_esdhc_initialize(bis, &usdhc_cfg[2]);
printf("board_mmc_init 4 %d \n", ret);			
		if (ret)
			return ret;
		
	return 0;
#else
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */
printf("board_mmc_init 5 \n");
	switch (reg & 0x3) {
	case 0x1:
		imx_iomux_v3_setup_multiple_pads(
			usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x2:
		imx_iomux_v3_setup_multiple_pads(
			usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x3:
		imx_iomux_v3_setup_multiple_pads(
			usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
		usdhc_cfg[0].esdhc_base = USDHC4_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}
#endif

static int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
struct i2c_pads_info mx6q_i2c2_pad_info = {
	.scl = {
		.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 13)
	}
};

struct i2c_pads_info mx6dl_i2c2_pad_info = {
	.scl = {
		.i2c_mode = MX6DL_PAD_KEY_COL3__I2C2_SCL
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_KEY_COL3__GPIO4_IO12
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13
			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 13)
	}
};

static iomux_v3_cfg_t const fwadapt_7wvga_pads[] = {
	IOMUX_PADS(PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK),
	IOMUX_PADS(PAD_DI0_PIN2__IPU1_DI0_PIN02), /* HSync */
	IOMUX_PADS(PAD_DI0_PIN3__IPU1_DI0_PIN03), /* VSync */
	//IOMUX_PADS(PAD_DI0_PIN4__IPU1_DI0_PIN04	| MUX_PAD_CTRL(PAD_CTL_DSE_120ohm)), /* Contrast */
	//IOMUX_PADS(PAD_DI0_PIN15__IPU1_DI0_PIN15), /* DISP0_DRDY */
	IOMUX_PADS(PAD_DISP0_DAT0__IPU1_DISP0_DATA00),
	IOMUX_PADS(PAD_DISP0_DAT1__IPU1_DISP0_DATA01),
	IOMUX_PADS(PAD_DISP0_DAT2__IPU1_DISP0_DATA02),
	IOMUX_PADS(PAD_DISP0_DAT3__IPU1_DISP0_DATA03),
	IOMUX_PADS(PAD_DISP0_DAT4__IPU1_DISP0_DATA04),
	IOMUX_PADS(PAD_DISP0_DAT5__IPU1_DISP0_DATA05),
	IOMUX_PADS(PAD_DISP0_DAT6__IPU1_DISP0_DATA06),
	IOMUX_PADS(PAD_DISP0_DAT7__IPU1_DISP0_DATA07),
	IOMUX_PADS(PAD_DISP0_DAT8__IPU1_DISP0_DATA08),
	IOMUX_PADS(PAD_DISP0_DAT9__IPU1_DISP0_DATA09),
	IOMUX_PADS(PAD_DISP0_DAT10__IPU1_DISP0_DATA10),
	IOMUX_PADS(PAD_DISP0_DAT11__IPU1_DISP0_DATA11),
	IOMUX_PADS(PAD_DISP0_DAT12__IPU1_DISP0_DATA12),
	IOMUX_PADS(PAD_DISP0_DAT13__IPU1_DISP0_DATA13),
	IOMUX_PADS(PAD_DISP0_DAT14__IPU1_DISP0_DATA14),
	IOMUX_PADS(PAD_DISP0_DAT15__IPU1_DISP0_DATA15),
	IOMUX_PADS(PAD_DISP0_DAT16__IPU1_DISP0_DATA16),
	IOMUX_PADS(PAD_DISP0_DAT17__IPU1_DISP0_DATA17),
	//IOMUX_PADS(PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL)), /* DISP0_BKLEN */
	//IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL)), /* DISP0_VDDEN */
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int detect_i2c(struct display_info_t const *dev)
{
	return (0 == i2c_set_bus_num(dev->bus)) &&
			(0 == i2c_probe(dev->addr));
}

static void enable_fwadapt_7wvga(struct display_info_t const *dev)
{
	//SETUP_IOMUX_PADS(fwadapt_7wvga_pads);

	gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
	gpio_direction_output(IMX_GPIO_NR(2, 11), 1);
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0x10,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= detect_i2c,
	.enable	= enable_fwadapt_7wvga,
	.mode	= {
		.name           = "FWBADAPT-LCD-F07A-0102",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 33260,
		.left_margin    = 128,
		.right_margin   = 128,
		.upper_margin   = 22,
		.lower_margin   = 22,
		.hsync_len      = 1,
		.vsync_len      = 1,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static int hdmi_detect(void)
{
	return detect_hdmi(&displays[0]);
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	/* Disable LCD backlight */
	//SETUP_IOMUX_PAD(PAD_DI0_PIN4__GPIO4_IO20);
	//gpio_direction_input(IMX_GPIO_NR(4, 20));
}
#endif /* CONFIG_VIDEO_IPUV3 */

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

	return cpu_eth_init(bis);
}

I2C_PADS(i2c_pad_info2,
	PAD_KEY_COL3__I2C2_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	PAD_KEY_COL3__GPIO4_IO12 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	IMX_GPIO_NR(4, 12),
	PAD_KEY_ROW3__I2C2_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	PAD_KEY_ROW3__GPIO4_IO13 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	IMX_GPIO_NR(4, 13));

void setup_local_i2c(void) 
{
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, I2C_PADS_INFO(i2c_pad_info2));
}

int board_early_init_f(void)
{
	setup_iomux_uart();
#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif

	//add ben i2c
	setup_local_i2c();
	
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	  MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
	int pin1 = 0, pin2 = 0;
	
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
		setenv("board_rev", "MX6Q");
	else
		setenv("board_rev", "MX6DL");
#endif

	//add boot pins
    gpio_direction_input(PIN1_GPIO);
    gpio_direction_input(PIN2_GPIO);

    pin1 = gpio_get_value(PIN1_GPIO);
    pin2 = gpio_get_value(PIN2_GPIO);

	//hdmi_detect
	if(hdmi_detect())
	{
		printf("hdmi state 1, set mmcargs use hdmi\n");	
		//setenv("mmcargs", "setenv bootargs console=${console},${baudrate} root=${mmcroot} video=mxcfb1:dev=hdmi,1280x720@60,if=RGB24");		
		setenv("mmcargs", "setenv bootargs console=${console},${baudrate} root=${mmcroot} video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24");	
	}	
	
    if (pin1 == 0 && pin2 == 1) {
        printf("boot from tf env ....\n");

        //sd4--sd card
        setenv("mmcroot", "/dev/mmcblk3p2 rootwait rw");
        setenv("mmcdev", "0");
    } else {
        printf("boot from inand env....\n");

        //sd3--emmc flash
        setenv("mmcroot", "/dev/mmcblk2p2 rootwait rw");
        setenv("mmcdev", "0");
    }
	
	
	//add usb-update
	//run_command("fatcheckupdate", 1);
	
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c2_pad_info);
	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c2_pad_info);
	else
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c2_pad_info);

	return 0;
}

int checkboard(void)
{
	puts("Board: HIO board\n");

	return 0;
}

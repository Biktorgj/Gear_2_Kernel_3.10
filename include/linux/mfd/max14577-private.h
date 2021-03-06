/*
 * max14577-private.h - Common API for the Maxim 14577/77836 internal sub chip
 *
 * Copyright (C) 2013,2014 Samsung Electrnoics
 * Chanwoo Choi <cw00.choi@samsung.com>
 * Krzysztof Kozlowski <k.kozlowski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MAX14577_PRIVATE_H__
#define __MAX14577_PRIVATE_H__

#include <linux/i2c.h>
#include <linux/regmap.h>

#define I2C_ADDR_PMIC	(0x46 >> 1)
#define I2C_ADDR_MUIC	(0x4A >> 1)
#define I2C_ADDR_FG	(0x6C >> 1)

enum maxim_device_type {
	MAXIM_DEVICE_TYPE_UNKNOWN	= 0,
	MAXIM_DEVICE_TYPE_MAX14577,
	MAXIM_DEVICE_TYPE_MAX77836,

	MAXIM_DEVICE_TYPE_NUM,
};

/* Slave addr = 0x4A: MUIC and Charger */
enum maxim_muic_reg {
	MAXIM_MUIC_REG_DEVICEID			= 0x00,
	MAXIM_MUIC_REG_INT1			= 0x01,
	MAXIM_MUIC_REG_INT2			= 0x02,
	MAXIM_MUIC_REG_INT3			= 0x03,
	MAXIM_MUIC_REG_STATUS1			= 0x04,
	MAXIM_MUIC_REG_STATUS2			= 0x05,
	MAXIM_MUIC_REG_STATUS3			= 0x06,
	MAXIM_MUIC_REG_INTMASK1			= 0x07,
	MAXIM_MUIC_REG_INTMASK2			= 0x08,
	MAXIM_MUIC_REG_INTMASK3			= 0x09,
	MAXIM_MUIC_REG_CDETCTRL1		= 0x0A,
	MAXIM_MUIC_REG_RFU			= 0x0B,
	MAXIM_MUIC_REG_CONTROL1			= 0x0C,
	MAXIM_MUIC_REG_CONTROL2			= 0x0D,
	MAXIM_MUIC_REG_CONTROL3			= 0x0E,
	MAXIM_CHG_REG_CHGCTRL1			= 0x0F,
	MAXIM_CHG_REG_CHGCTRL2			= 0x10,
	MAXIM_CHG_REG_CHGCTRL3			= 0x11,
	MAXIM_CHG_REG_CHGCTRL4			= 0x12,
	MAXIM_CHG_REG_CHGCTRL5			= 0x13,
	MAXIM_CHG_REG_CHGCTRL6			= 0x14,
	MAXIM_CHG_REG_CHGCTRL7			= 0x15,

	MAXIM_MUIC_REG_END,
};

/*
 * Combined charger types for max14577 and max77836.
 *
 * On max14577 three lower bits map to STATUS2/CHGTYP field.
 * On max77836 two values of this field are bitwise OR-ed with 0x8 to indicate
 * the difference.
 */
enum maxim_muic_charger_type {
	MAXIM_CHARGER_TYPE_NONE			= 0x0,
	MAXIM_CHARGER_TYPE_USB			= 0x1,
	MAXIM_CHARGER_TYPE_DOWNSTREAM_PORT	= 0x2,
	MAXIM_CHARGER_TYPE_DEDICATED_CHG	= 0x3,
	MAXIM_CHARGER_TYPE_SPECIAL_500MA	= 0x4,
	/* Special 1A or 2A charger */
	MAXIM_CHARGER_TYPE_SPECIAL_1A		= 0x5,
	/* max14577: reserved, used on max77836 */
	MAX14577_CHARGER_TYPE_RESERVED		= 0x6,
	/* max14577: dead-battery charing with maximum current 100mA */
	MAX14577_CHARGER_TYPE_DEAD_BATTERY	= 0x7,
	/*
	 * max77836: special charger (bias on D+/D-),
	 * register value 0x6 | 0x8 (4th bit set)
	 */
	MAX77836_CHARGER_TYPE_SPECIAL_BIAS	= 0xe,
	/* max77836: reserved, register value 0x7 | 0x8 (4th bit set) */
	MAX77836_CHARGER_TYPE_RESERVED		= 0xf,
};

/* MAX14577 interrupts */
#define MAXIM_INT1_ADC_MASK			(0x1 << 0)
#define MAXIM_INT1_ADCLOW_MASK			(0x1 << 1)
#define MAXIM_INT1_ADCERR_MASK			(0x1 << 2)
#define MAX77836_INT1_ADC1K_MASK		(0x1 << 3)

#define MAXIM_INT2_CHGTYP_MASK			(0x1 << 0)
#define MAXIM_INT2_CHGDETRUN_MASK		(0x1 << 1)
#define MAXIM_INT2_DCDTMR_MASK			(0x1 << 2)
#define MAXIM_INT2_DBCHG_MASK			(0x1 << 3)
#define MAXIM_INT2_VBVOLT_MASK			(0x1 << 4)
#define MAX77836_INT2_VIDRM_MASK		(0x1 << 5)

#define MAXIM_INT3_EOC_MASK			(0x1 << 0)
#define MAXIM_INT3_CGMBC_MASK			(0x1 << 1)
#define MAXIM_INT3_OVP_MASK			(0x1 << 2)
#define MAXIM_INT3_MBCCHGERR_MASK		(0x1 << 3)

/* MAX14577 DEVICE ID register */
#define MAXIM_DEVID_VENDORID_SHIFT		0
#define MAXIM_DEVID_DEVICEID_SHIFT		3
#define MAXIM_DEVID_VENDORID_MASK		(0x07 << MAXIM_DEVID_VENDORID_SHIFT)
#define MAXIM_DEVID_DEVICEID_MASK		(0x1f << MAXIM_DEVID_DEVICEID_SHIFT)

/* MAX14577 STATUS1 register */
#define MAXIM_STATUS1_ADC_SHIFT			0
#define MAXIM_STATUS1_ADCLOW_SHIFT		5
#define MAXIM_STATUS1_ADCERR_SHIFT		6
#define MAX77836_STATUS1_ADC1K_SHIFT		7
#define MAXIM_STATUS1_ADC_MASK			(0x1f << MAXIM_STATUS1_ADC_SHIFT)
#define MAXIM_STATUS1_ADCLOW_MASK		(0x1 << MAXIM_STATUS1_ADCLOW_SHIFT)
#define MAXIM_STATUS1_ADCERR_MASK		(0x1 << MAXIM_STATUS1_ADCERR_SHIFT)
#define MAX77836_STATUS1_ADC1K_MASK		(0x1 << MAX77836_STATUS1_ADC1K_SHIFT)

/* MAX14577 STATUS2 register */
#define MAXIM_STATUS2_CHGTYP_SHIFT		0
#define MAXIM_STATUS2_CHGDETRUN_SHIFT		3
#define MAXIM_STATUS2_DCDTMR_SHIFT		4
#define MAX14577_STATUS2_DBCHG_SHIFT		5
#define MAX77836_STATUS2_DXOVP_SHIFT		5
#define MAXIM_STATUS2_VBVOLT_SHIFT		6
#define MAX77836_STATUS2_VIDRM_SHIFT		7
#define MAXIM_STATUS2_CHGTYP_MASK		(0x7 << MAXIM_STATUS2_CHGTYP_SHIFT)
#define MAXIM_STATUS2_CHGDETRUN_MASK		(0x1 << MAXIM_STATUS2_CHGDETRUN_SHIFT)
#define MAXIM_STATUS2_DCDTMR_MASK		(0x1 << MAXIM_STATUS2_DCDTMR_SHIFT)
#define MAX14577_STATUS2_DBCHG_MASK		(0x1 << MAX14577_STATUS2_DBCHG_SHIFT)
#define MAX77836_STATUS2_DXOVP_MASK		(0x1 << MAX77836_STATUS2_DXOVP_SHIFT)
#define MAXIM_STATUS2_VBVOLT_MASK		(0x1 << MAXIM_STATUS2_VBVOLT_SHIFT)
#define MAX77836_STATUS2_VIDRM_MASK		(0x1 << MAX77836_STATUS2_VIDRM_SHIFT)

/* MAX14577 STATUS3 register */
#define MAXIM_STATUS3_EOC_SHIFT			0
#define MAXIM_STATUS3_CGMBC_SHIFT		1
#define MAXIM_STATUS3_OVP_SHIFT			2
#define MAXIM_STATUS3_MBCCHGERR_SHIFT		3
#define MAXIM_STATUS3_EOC_MASK			(0x1 << MAXIM_STATUS3_EOC_SHIFT)
#define MAXIM_STATUS3_CGMBC_MASK		(0x1 << MAXIM_STATUS3_CGMBC_SHIFT)
#define MAXIM_STATUS3_OVP_MASK			(0x1 << MAXIM_STATUS3_OVP_SHIFT)
#define MAXIM_STATUS3_MBCCHGERR_MASK		(0x1 << MAXIM_STATUS3_MBCCHGERR_SHIFT)

/* MAX14577 CONTROL1 register */
#define MAXIM_CONTROL1_COMN1SW_SHIFT		0
#define MAXIM_CONTROL1_COMP2SW_SHIFT		3
#define MAXIM_CONTROL1_MICEN_SHIFT		6
#define MAXIM_CONTROL1_IDBEN_SHIFT		7
#define MAXIM_CONTROL1_COMN1SW_MASK		(0x7 << MAXIM_CONTROL1_COMN1SW_SHIFT)
#define MAXIM_CONTROL1_COMP2SW_MASK		(0x7 << MAXIM_CONTROL1_COMP2SW_SHIFT)
#define MAXIM_CONTROL1_MICEN_MASK		(0x1 << MAXIM_CONTROL1_MICEN_SHIFT)
#define MAXIM_CONTROL1_IDBEN_MASK		(0x1 << MAXIM_CONTROL1_IDBEN_SHIFT)
#define MAXIM_CONTROL1_CLEAR_IDBEN_MICEN_MASK	(MAXIM_CONTROL1_COMN1SW_MASK | MAXIM_CONTROL1_COMP2SW_MASK)
#define MAXIM_CONTROL1_SW_USB			((1 << MAXIM_CONTROL1_COMP2SW_SHIFT) \
						| (1 << MAXIM_CONTROL1_COMN1SW_SHIFT))
#define MAXIM_CONTROL1_SW_AUDIO			((2 << MAXIM_CONTROL1_COMP2SW_SHIFT) \
						| (2 << MAXIM_CONTROL1_COMN1SW_SHIFT))
#define MAXIM_CONTROL1_SW_UART			((3 << MAXIM_CONTROL1_COMP2SW_SHIFT) \
						| (3 << MAXIM_CONTROL1_COMN1SW_SHIFT))
#define MAXIM_CONTROL1_SW_OPEN			((0 << MAXIM_CONTROL1_COMP2SW_SHIFT) \
						| (0 << MAXIM_CONTROL1_COMN1SW_SHIFT))

/* MAX14577 CONTROL2 register */
#define MAXIM_CONTROL2_LOWPWR_SHIFT		0
#define MAXIM_CONTROL2_ADCEN_SHIFT		1
#define MAXIM_CONTROL2_CPEN_SHIFT		2
#define MAXIM_CONTROL2_SFOUTASRT_SHIFT		3
#define MAXIM_CONTROL2_SFOUTORD_SHIFT		4
#define MAXIM_CONTROL2_ACCDET_SHIFT		5
#define MAXIM_CONTROL2_USBCPINT_SHIFT		6
#define MAXIM_CONTROL2_RCPS_SHIFT		7
#define MAXIM_CONTROL2_LOWPWR_MASK		(0x1 << MAXIM_CONTROL2_LOWPWR_SHIFT)
#define MAXIM_CONTROL2_ADCEN_MASK		(0x1 << MAXIM_CONTROL2_ADCEN_SHIFT)
#define MAXIM_CONTROL2_CPEN_MASK		(0x1 << MAXIM_CONTROL2_CPEN_SHIFT)
#define MAXIM_CONTROL2_SFOUTASRT_MASK		(0x1 << MAXIM_CONTROL2_SFOUTASRT_SHIFT)
#define MAXIM_CONTROL2_SFOUTORD_MASK		(0x1 << MAXIM_CONTROL2_SFOUTORD_SHIFT)
#define MAXIM_CONTROL2_ACCDET_MASK		(0x1 << MAXIM_CONTROL2_ACCDET_SHIFT)
#define MAXIM_CONTROL2_USBCPINT_MASK		(0x1 << MAXIM_CONTROL2_USBCPINT_SHIFT)
#define MAXIM_CONTROL2_RCPS_MASK		(0x1 << MAXIM_CONTROL2_RCPS_SHIFT)

#define MAXIM_CONTROL2_CPEN1_LOWPWR0		((1 << MAXIM_CONTROL2_CPEN_SHIFT) | \
						(0 << MAXIM_CONTROL2_LOWPWR_SHIFT))
#define MAXIM_CONTROL2_CPEN0_LOWPWR1		((0 << MAXIM_CONTROL2_CPEN_SHIFT) | \
						(1 << MAXIM_CONTROL2_LOWPWR_SHIFT))

/* MAX14577 CONTROL3 register */
#define MAXIM_CONTROL3_JIGSET_SHIFT		0
#define MAXIM_CONTROL3_BOOTSET_SHIFT		2
#define MAXIM_CONTROL3_ADCDBSET_SHIFT		4
#define MAX14577_CONTROL3_WBTH_SHIFT		6
#define MAXIM_CONTROL3_JIGSET_MASK		(0x3 << MAXIM_CONTROL3_JIGSET_SHIFT)
#define MAXIM_CONTROL3_BOOTSET_MASK		(0x3 << MAXIM_CONTROL3_BOOTSET_SHIFT)
#define MAXIM_CONTROL3_ADCDBSET_MASK		(0x3 << MAXIM_CONTROL3_ADCDBSET_SHIFT)
#define MAX14577_CONTROL3_WBTH_MASK		(0x3 << MAX14577_CONTROL3_WBTH_SHIFT)

/* MAX14577 CDETCTRL1 register */
#define MAXIM_CDETCTRL1_CHGDETEN_SHIFT		0
#define MAXIM_CDETCTRL1_CHGTYPMAN_SHIFT		1
#define MAXIM_CDETCTRL1_DCDEN_SHIFT		2
#define MAXIM_CDETCTRL1_DCD2SCT_SHIFT		3
#define MAX14577_CDETCTRL1_DCHKTM_SHIFT		4
#define MAX77836_CDETCTRL1_CDDELAY_SHIFT	4
#define MAX14577_CDETCTRL1_DBEXIT_SHIFT		5
#define MAX77836_CDETCTRL1_DCDCPL_SHIFT		5
#define MAX14577_CDETCTRL1_DBIDLE_SHIFT		6
#define MAXIM_CDETCTRL1_CDPDET_SHIFT		7
#define MAXIM_CDETCTRL1_CHGDETEN_MASK		(0x1 << MAXIM_CDETCTRL1_CHGDETEN_SHIFT)
#define MAXIM_CDETCTRL1_CHGTYPMAN_MASK		(0x1 << MAXIM_CDETCTRL1_CHGTYPMAN_SHIFT)
#define MAXIM_CDETCTRL1_DCDEN_MASK		(0x1 << MAXIM_CDETCTRL1_DCDEN_SHIFT)
#define MAXIM_CDETCTRL1_DCD2SCT_MASK		(0x1 << MAXIM_CDETCTRL1_DCD2SCT_SHIFT)
#define MAX14577_CDETCTRL1_DCHKTM_MASK		(0x1 << MAX14577_CDETCTRL1_DCHKTM_SHIFT)
#define MAX77836_CDETCTRL1_CDDELAY_MASK		(0x1 << MAX77836_CDETCTRL1_CDDELAY_SHIFT)
#define MAX14577_CDETCTRL1_DBEXIT_MASK		(0x1 << MAX14577_CDETCTRL1_DBEXIT_SHIFT)
#define MAX77836_CDETCTRL1_DCDCPL_MASK		(0x1 << MAX77836_CDETCTRL1_DCDCPL_SHIFT)
#define MAX14577_CDETCTRL1_DBIDLE_MASK		(0x1 << MAX14577_CDETCTRL1_DBIDLE_SHIFT)
#define MAXIM_CDETCTRL1_CDPDET_MASK		(0x1 << MAXIM_CDETCTRL1_CDPDET_SHIFT)

/* MAX14577 MAXIM_CHGCTRL1 register */
#define MAXIM_CHGCTRL1_TCHW_SHIFT		4
#define MAXIM_CHGCTRL1_TCHW_MASK		(0x7 << MAXIM_CHGCTRL1_TCHW_SHIFT)

/* MAX14577 MAXIM_CHGCTRL2 register */
#define MAXIM_CHGCTRL2_MBCHOSTEN_SHIFT		6
#define MAXIM_CHGCTRL2_MBCHOSTEN_MASK		(0x1 << MAXIM_CHGCTRL2_MBCHOSTEN_SHIFT)
#define MAXIM_CHGCTRL2_VCHGR_RC_SHIFT		7
#define MAXIM_CHGCTRL2_VCHGR_RC_MASK		(0x1 << MAXIM_CHGCTRL2_VCHGR_RC_SHIFT)

/* MAX14577 MAXIM_CHGCTRL3 register */
#define MAXIM_CHGCTRL3_MBCCVWRC_SHIFT		0
#define MAXIM_CHGCTRL3_MBCCVWRC_MASK		(0xf << MAXIM_CHGCTRL3_MBCCVWRC_SHIFT)

/* MAX14577 MAXIM_CHGCTRL4 register */
#define MAXIM_CHGCTRL4_MBCICHWRCH_SHIFT		0
#define MAXIM_CHGCTRL4_MBCICHWRCH_MASK		(0xf << MAXIM_CHGCTRL4_MBCICHWRCH_SHIFT)
#define MAXIM_CHGCTRL4_MBCICHWRCL_SHIFT		4
#define MAXIM_CHGCTRL4_MBCICHWRCL_MASK		(0x1 << MAXIM_CHGCTRL4_MBCICHWRCL_SHIFT)

/* MAX14577 MAXIM_CHGCTRL5 register */
#define MAXIM_CHGCTRL5_EOCS_SHIFT		0
#define MAXIM_CHGCTRL5_EOCS_MASK		(0xf << MAXIM_CHGCTRL5_EOCS_SHIFT)

/* MAX14577 MAXIM_CHGCTRL6 register */
#define MAXIM_CHGCTRL6_AUTOSTOP_SHIFT		5
#define MAXIM_CHGCTRL6_AUTOSTOP_MASK		(0x1 << MAXIM_CHGCTRL6_AUTOSTOP_SHIFT)

/* MAX14577 MAXIM_CHGCTRL7 register */
#define MAXIM_CHGCTRL7_OTPCGHCVS_SHIFT		0
#define MAXIM_CHGCTRL7_OTPCGHCVS_MASK		(0x3 << MAXIM_CHGCTRL7_OTPCGHCVS_SHIFT)

/* MAX14577 regulator current limits (as in MAXIM_CHGCTRL4 register), uA */
#define MAX14577_REGULATOR_CURRENT_LIMIT_MIN		 90000
#define MAX14577_REGULATOR_CURRENT_LIMIT_HIGH_START	200000
#define MAX14577_REGULATOR_CURRENT_LIMIT_HIGH_STEP	 50000
#define MAX14577_REGULATOR_CURRENT_LIMIT_MAX		950000

/* MAX77836 regulator current limits (as in MAXIM_CHGCTRL4 register), uA */
#define MAX77836_REGULATOR_CURRENT_LIMIT_MIN		 45000
#define MAX77836_REGULATOR_CURRENT_LIMIT_HIGH_START	100000
#define MAX77836_REGULATOR_CURRENT_LIMIT_HIGH_STEP	 25000
#define MAX77836_REGULATOR_CURRENT_LIMIT_MAX		475000

/* MAX14577 regulator SFOUT LDO voltage, fixed, uV */
#define MAX14577_REGULATOR_SAFEOUT_VOLTAGE		4900000

/* MAX77836 regulator LDOx voltage, uV */
#define MAX77836_REGULATOR_LDO_VOLTAGE_MIN		800000
#define MAX77836_REGULATOR_LDO_VOLTAGE_MAX		3950000
#define MAX77836_REGULATOR_LDO_VOLTAGE_STEP		50000
#define MAX77836_REGULATOR_LDO_VOLTAGE_STEPS_NUM	64

/* Slave addr = 0x46: PMIC */
enum max77836_pmic_reg {
	MAX77836_COMP_REG_COMP1			= 0x60,

	MAX77836_LDO_REG_CNFG1_LDO1		= 0x51,
	MAX77836_LDO_REG_CNFG2_LDO1		= 0x52,
	MAX77836_LDO_REG_CNFG1_LDO2		= 0x53,
	MAX77836_LDO_REG_CNFG2_LDO2		= 0x54,
	MAX77836_LDO_REG_CNFG_LDO_BIAS		= 0x55,

	MAX77836_PMIC_REG_PMIC_ID		= 0x20,
	MAX77836_PMIC_REG_PMIC_REV		= 0x21,
	MAX77836_PMIC_REG_INTSRC		= 0x22,
	MAX77836_PMIC_REG_INTSRC_MASK		= 0x23,
	MAX77836_PMIC_REG_TOPSYS_INT		= 0x24,
	MAX77836_PMIC_REG_TOPSYS_INT_MASK	= 0x26,
	MAX77836_PMIC_REG_TOPSYS_STAT		= 0x28,
	MAX77836_PMIC_REG_MRSTB_CNTL		= 0x2A,
	MAX77836_PMIC_REG_LSCNFG		= 0x2B,

	MAX77836_PMIC_REG_END,
};

#define MAX77836_INTSRC_MASK_TOP_INT_SHIFT	1
#define MAX77836_INTSRC_MASK_MUIC_CHG_INT_SHIFT	3
#define MAX77836_INTSRC_MASK_TOP_INT_MASK	(0x1 << MAX77836_INTSRC_MASK_TOP_INT_SHIFT)
#define MAX77836_INTSRC_MASK_MUIC_CHG_INT_MASK	(0x1 << MAX77836_INTSRC_MASK_MUIC_CHG_INT_SHIFT)

/* MAX77836 PMIC interrupts */
#define MAX77836_TOPSYS_INT_T120C_SHIFT		0
#define MAX77836_TOPSYS_INT_T140C_SHIFT		1
#define MAX77836_TOPSYS_INT_T120C_MASK		(0x1 << MAX77836_TOPSYS_INT_T120C_SHIFT)
#define MAX77836_TOPSYS_INT_T140C_MASK		(0x1 << MAX77836_TOPSYS_INT_T140C_SHIFT)

/* LDO1/LDO2 CONFIG1 register */
#define MAX77836_CNFG1_LDO_PWRMD_SHIFT		6
#define MAX77836_CNFG1_LDO_TV_SHIFT		0
#define MAX77836_CNFG1_LDO_PWRMD_MASK		(0x3 << MAX77836_CNFG1_LDO_PWRMD_SHIFT)
#define MAX77836_CNFG1_LDO_TV_MASK		(0x3f << MAX77836_CNFG1_LDO_TV_SHIFT)

/* LDO1/LDO2 CONFIG2 register */
#define MAX77836_CNFG2_LDO_OVCLMPEN_SHIFT	7
#define MAX77836_CNFG2_LDO_ALPMEN_SHIFT		6
#define MAX77836_CNFG2_LDO_COMP_SHIFT		4
#define MAX77836_CNFG2_LDO_POK_SHIFT		3
#define MAX77836_CNFG2_LDO_ADE_SHIFT		1
#define MAX77836_CNFG2_LDO_SS_SHIFT		0
#define MAX77836_CNFG2_LDO_OVCLMPEN_MASK	(0x1 << MAX77836_CNFG2_LDO_OVCLMPEN_SHIFT)
#define MAX77836_CNFG2_LDO_ALPMEN_MASK		(0x1 << MAX77836_CNFG2_LDO_ALPMEN_SHIFT)
#define MAX77836_CNFG2_LDO_COMP_MASK		(0x3 << MAX77836_CNFG2_LDO_COMP_SHIFT)
#define MAX77836_CNFG2_LDO_POK_MASK		(0x1 << MAX77836_CNFG2_LDO_POK_SHIFT)
#define MAX77836_CNFG2_LDO_ADE_MASK		(0x1 << MAX77836_CNFG2_LDO_ADE_SHIFT)
#define MAX77836_CNFG2_LDO_SS_MASK		(0x1 << MAX77836_CNFG2_LDO_SS_SHIFT)

/* Slave addr = 0x6C: Fuel-Gauge/Battery */
enum max77836_fg_reg {
	MAX77836_FG_REG_VCELL_MSB	= 0x02,
	MAX77836_FG_REG_VCELL_LSB	= 0x03,
	MAX77836_FG_REG_SOC_MSB		= 0x04,
	MAX77836_FG_REG_SOC_LSB		= 0x05,
	MAX77836_FG_REG_MODE_H		= 0x06,
	MAX77836_FG_REG_MODE_L		= 0x07,
	MAX77836_FG_REG_VERSION_MSB	= 0x08,
	MAX77836_FG_REG_VERSION_LSB	= 0x09,
	MAX77836_FG_REG_HIBRT_H		= 0x0A,
	MAX77836_FG_REG_HIBRT_L		= 0x0B,
	MAX77836_FG_REG_CONFIG_H	= 0x0C,
	MAX77836_FG_REG_CONFIG_L	= 0x0D,
	MAX77836_FG_REG_VALRT_MIN	= 0x14,
	MAX77836_FG_REG_VALRT_MAX	= 0x15,
	MAX77836_FG_REG_CRATE_MSB	= 0x16,
	MAX77836_FG_REG_CRATE_LSB	= 0x17,
	MAX77836_FG_REG_VRESET		= 0x18,
	MAX77836_FG_REG_FGID		= 0x19,
	MAX77836_FG_REG_STATUS_H	= 0x1A,
	MAX77836_FG_REG_STATUS_L	= 0x1B,
	/*
	 * TODO: TABLE registers
	 * TODO: CMD register
	 */

	MAX77836_FG_REG_END,
};

enum maxim_irq {
	/* INT1 */
	MAXIM_IRQ_INT1_ADC,
	MAXIM_IRQ_INT1_ADCLOW,
	MAXIM_IRQ_INT1_ADCERR,
	MAX77836_IRQ_INT1_ADC1K,

	/* INT2 */
	MAXIM_IRQ_INT2_CHGTYP,
	MAXIM_IRQ_INT2_CHGDETRUN,
	MAXIM_IRQ_INT2_DCDTMR,
	MAXIM_IRQ_INT2_DBCHG,
	MAXIM_IRQ_INT2_VBVOLT,
	MAX77836_IRQ_INT2_VIDRM,

	/* INT3 */
	MAXIM_IRQ_INT3_EOC,
	MAXIM_IRQ_INT3_CGMBC,
	MAXIM_IRQ_INT3_OVP,
	MAXIM_IRQ_INT3_MBCCHGERR,

	/* TOPSYS_INT */
	MAXIM_IRQ_TOPSYS_T140C,
	MAXIM_IRQ_TOPSYS_T120C,

	MAXIM_IRQ_NUM,
};

/*
 * State container for max14577-like drivers.
 */
struct maxim_core {
	struct device *dev;
	struct i2c_client *i2c; /* Slave addr = 0x4A */
	struct i2c_client *i2c_pmic; /* Slave addr = 0x46 */
	enum maxim_device_type dev_type;

	struct regmap *regmap_muic;
	struct regmap *regmap_pmic;

	struct regmap_irq_chip_data *irq_data_muic;
	struct regmap_irq_chip_data *irq_data_pmic;
	int irq;
};

/* MAX14577 shared regmap API function */
static inline int max14577_read_reg(struct regmap *map, u8 reg, u8 *dest)
{
	unsigned int val;
	int ret;

	ret = regmap_read(map, reg, &val);
	*dest = val;

	return ret;
}

static inline int max14577_bulk_read(struct regmap *map, u8 reg, u8 *buf,
		int count)
{
	return regmap_bulk_read(map, reg, buf, count);
}

static inline int max14577_write_reg(struct regmap *map, u8 reg, u8 value)
{
	return regmap_write(map, reg, value);
}

static inline int max14577_bulk_write(struct regmap *map, u8 reg, u8 *buf,
		int count)
{
	return regmap_bulk_write(map, reg, buf, count);
}

static inline int max14577_update_reg(struct regmap *map, u8 reg, u8 mask,
		u8 val)
{
	return regmap_update_bits(map, reg, mask, val);
}

#endif /* __MAX14577_PRIVATE_H__ */

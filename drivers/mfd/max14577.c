/*
 * max14577.c - mfd core driver for the Maxim 14577/77836
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
 *
 * This driver is based on max8997.c
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/mfd/core.h>
#include <linux/mfd/max14577.h>
#include <linux/mfd/max14577-private.h>

static struct mfd_cell max14577_devs[] = {
	{
		.name = "max14577-muic",
		.of_compatible = "maxim,max14577-muic",
	},
	{
		.name = "max14577-regulator",
		.of_compatible = "maxim,max14577-regulator",
	},
	{ .name = "max14577-charger", },
};

static struct mfd_cell max77836_devs[] = {
	{
		.name = "max77836-muic",
		.of_compatible = "maxim,max77836-muic",
	},
	{
		.name = "max77836-regulator",
		.of_compatible = "maxim,max77836-regulator",
	},
	{	 .name = "max77836-charger",
		.of_compatible = "maxim,max77836-charger",
	},
	{
		.name = "max77836-battery",
		.of_compatible = "maxim,max77836-battery",
	},
};

static struct of_device_id max14577_dt_match[] = {
	{ .compatible = "maxim,max14577", .data = (void *)MAXIM_DEVICE_TYPE_MAX14577, },
	{ .compatible = "maxim,max77836", .data = (void *)MAXIM_DEVICE_TYPE_MAX77836, },
	{},
};

static bool max14577_muic_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MAXIM_MUIC_REG_INT1 ... MAXIM_MUIC_REG_STATUS3:
		return true;
	default:
		break;
	}
	return false;
}

static bool max77836_muic_volatile_reg(struct device *dev, unsigned int reg)
{
	/* Any max14577 volatile registers are also max77836 volatile. */
	if (max14577_muic_volatile_reg(dev, reg))
		return true;
	switch (reg) {
	case MAX77836_FG_REG_VCELL_MSB ... MAX77836_FG_REG_SOC_LSB:
	case MAX77836_FG_REG_CRATE_MSB ... MAX77836_FG_REG_CRATE_LSB:
	case MAX77836_FG_REG_STATUS_H ... MAX77836_FG_REG_STATUS_L:
	case MAX77836_PMIC_REG_INTSRC:
	case MAX77836_PMIC_REG_TOPSYS_INT:
	case MAX77836_PMIC_REG_TOPSYS_STAT:
		return true;
	default:
		break;
	}
	return false;
}


static const struct regmap_config max14577_muic_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.volatile_reg	= max14577_muic_volatile_reg,
	.max_register	= MAXIM_MUIC_REG_END,
};

static const struct regmap_config max77836_pmic_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.volatile_reg	= max77836_muic_volatile_reg,
	.max_register	= MAX77836_PMIC_REG_END,
};

static const struct regmap_irq max14577_irqs[] = {
	/* INT1 interrupts */
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADC_MASK, },
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADCLOW_MASK, },
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADCERR_MASK, },
	/* INT2 interrupts */
	{ .reg_offset = 1, .mask = MAXIM_INT2_CHGTYP_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_CHGDETRUN_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_DCDTMR_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_DBCHG_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_VBVOLT_MASK, },
	/* INT3 interrupts */
	{ .reg_offset = 2, .mask = MAXIM_INT3_EOC_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_CGMBC_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_OVP_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_MBCCHGERR_MASK, },
};

static const struct regmap_irq_chip max14577_irq_chip = {
	.name			= "max14577",
	.status_base		= MAXIM_MUIC_REG_INT1,
	.mask_base		= MAXIM_MUIC_REG_INTMASK1,
	.mask_invert		= 1,
	.num_regs		= 3,
	.irqs			= max14577_irqs,
	.num_irqs		= ARRAY_SIZE(max14577_irqs),
};

static const struct regmap_irq max77836_muic_irqs[] = {
	/* INT1 interrupts */
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADC_MASK, },
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADCLOW_MASK, },
	{ .reg_offset = 0, .mask = MAXIM_INT1_ADCERR_MASK, },
	{ .reg_offset = 0, .mask = MAX77836_INT1_ADC1K_MASK, },
	/* INT2 interrupts */
	{ .reg_offset = 1, .mask = MAXIM_INT2_CHGTYP_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_CHGDETRUN_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_DCDTMR_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_DBCHG_MASK, },
	{ .reg_offset = 1, .mask = MAXIM_INT2_VBVOLT_MASK, },
	{ .reg_offset = 1, .mask = MAX77836_INT2_VIDRM_MASK, },
	/* INT3 interrupts */
	{ .reg_offset = 2, .mask = MAXIM_INT3_EOC_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_CGMBC_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_OVP_MASK, },
	{ .reg_offset = 2, .mask = MAXIM_INT3_MBCCHGERR_MASK, },
};

static const struct regmap_irq_chip max77836_muic_irq_chip = {
	.name			= "max77836-muic",
	.status_base		= MAXIM_MUIC_REG_INT1,
	.mask_base		= MAXIM_MUIC_REG_INTMASK1,
	.mask_invert		= 1,
	.num_regs		= 3,
	.irqs			= max77836_muic_irqs,
	.num_irqs		= ARRAY_SIZE(max77836_muic_irqs),
};

static const struct regmap_irq max77836_pmic_irqs[] = {
	{ .reg_offset = 0, .mask = MAX77836_TOPSYS_INT_T120C_MASK, },
	{ .reg_offset = 0, .mask = MAX77836_TOPSYS_INT_T140C_MASK, },
};

static const struct regmap_irq_chip max77836_pmic_irq_chip = {
	.name			= "max77836-pmic",
	.status_base		= MAX77836_PMIC_REG_TOPSYS_INT,
	.mask_base		= MAX77836_PMIC_REG_TOPSYS_INT_MASK,
	.mask_invert		= 0,
	.num_regs		= 1,
	.irqs			= max77836_pmic_irqs,
	.num_irqs		= ARRAY_SIZE(max77836_pmic_irqs),
};

static void max14577_print_dev_type(struct maxim_core *maxim_core)
{
	u8 reg_data, vendor_id, device_id;
	int ret = max14577_read_reg(maxim_core->regmap_muic, MAXIM_MUIC_REG_DEVICEID,
			&reg_data);
	if (ret) {
		dev_err(maxim_core->dev, "Failed to read DEVICEID register: %d\n",
				ret);
		return;
	}

	vendor_id = ((reg_data & MAXIM_DEVID_VENDORID_MASK) >>
				MAXIM_DEVID_VENDORID_SHIFT);
	device_id = ((reg_data & MAXIM_DEVID_DEVICEID_MASK) >>
				MAXIM_DEVID_DEVICEID_SHIFT);

	dev_info(maxim_core->dev, "Device type: %u (ID: 0x%x, vendor: 0x%x)\n",
			maxim_core->dev_type, device_id, vendor_id);
}

/*
 * Max77836 specific initialization code for driver probe.
 * Adds new I2C dummy device, regmap and regmap IRQ chip.
 * Unmasks Interrupt Source register.
 *
 * On success returns 0.
 * On failure returns errno and reverts any changes done so far (e.g. remove
 * I2C dummy device), except masking the INT SRC register.
 */
static int max77836_init(struct maxim_core *maxim_core)
{
	int ret;
	u8 intsrc_mask;

	maxim_core->i2c_pmic = i2c_new_dummy(maxim_core->i2c->adapter, I2C_ADDR_PMIC);
	if (!maxim_core->i2c_pmic) {
		dev_err(maxim_core->dev, "Failed to register PMIC I2C device\n");
		return -EPERM;
	}
	i2c_set_clientdata(maxim_core->i2c_pmic, maxim_core);

	maxim_core->regmap_pmic = devm_regmap_init_i2c(maxim_core->i2c_pmic,
			&max77836_pmic_regmap_config);
	if (IS_ERR(maxim_core->regmap_pmic)) {
		ret = PTR_ERR(maxim_core->regmap_pmic);
		dev_err(maxim_core->dev, "Failed to allocate PMIC register map: %d\n",
				ret);
		goto err;
	}

	/* Un-mask MAX77836 Interrupt Source register */
	ret = max14577_read_reg(maxim_core->regmap_pmic,
			MAX77836_PMIC_REG_INTSRC_MASK, &intsrc_mask);
	if (ret < 0) {
		dev_err(maxim_core->dev, "Failed to read PMIC register\n");
		goto err;
	}

	intsrc_mask &= ~(MAX77836_INTSRC_MASK_TOP_INT_MASK);
	intsrc_mask &= ~(MAX77836_INTSRC_MASK_MUIC_CHG_INT_MASK);
	ret = max14577_write_reg(maxim_core->regmap_pmic,
			MAX77836_PMIC_REG_INTSRC_MASK, intsrc_mask);
	if (ret < 0) {
		dev_err(maxim_core->dev, "Failed to write PMIC register\n");
		goto err;
	}

	ret = regmap_add_irq_chip(maxim_core->regmap_pmic, maxim_core->irq,
				  IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_SHARED,
				  0, &max77836_pmic_irq_chip,
				  &maxim_core->irq_data_pmic);
	if (ret != 0) {
		dev_err(maxim_core->dev, "Failed to request PMIC IRQ %d: %d\n",
				maxim_core->irq, ret);
		goto err;
	}

	return 0;

err:
	i2c_unregister_device(maxim_core->i2c_pmic);

	return ret;
}

/*
 * Max77836 specific de-initialization code for driver remove.
 */
static void max77836_remove(struct maxim_core *maxim_core)
{
	regmap_del_irq_chip(maxim_core->irq, maxim_core->irq_data_pmic);
	i2c_unregister_device(maxim_core->i2c_pmic);
}

static int max14577_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	struct maxim_core *maxim_core;
	struct max14577_platform_data *pdata = dev_get_platdata(&i2c->dev);
	struct device_node *np = i2c->dev.of_node;
	int ret = 0;
	const struct regmap_irq_chip *irq_chip;
	struct mfd_cell *mfd_devs;
	unsigned int mfd_devs_size;
	int irq_flags;

	if (np) {
		pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
		i2c->dev.platform_data = pdata;
	}

	if (!pdata) {
		dev_err(&i2c->dev, "No platform data found.\n");
		return -EINVAL;
	}

	maxim_core = devm_kzalloc(&i2c->dev, sizeof(*maxim_core), GFP_KERNEL);
	if (!maxim_core)
		return -ENOMEM;

	i2c_set_clientdata(i2c, maxim_core);
	maxim_core->dev = &i2c->dev;
	maxim_core->i2c = i2c;
	maxim_core->irq = i2c->irq;

	maxim_core->regmap_muic = devm_regmap_init_i2c(i2c, &max14577_muic_regmap_config);
	if (IS_ERR(maxim_core->regmap_muic)) {
		ret = PTR_ERR(maxim_core->regmap_muic);
		dev_err(maxim_core->dev, "Failed to allocate register map: %d\n",
				ret);
		return ret;
	}

	if (np) {
		const struct of_device_id *of_id = of_match_device(max14577_dt_match, &i2c->dev);
		if (of_id)
			maxim_core->dev_type = (unsigned int)of_id->data;
	} else {
		maxim_core->dev_type = id->driver_data;
	}

	max14577_print_dev_type(maxim_core);

	switch (maxim_core->dev_type) {
	case MAXIM_DEVICE_TYPE_MAX77836:
		irq_chip = &max77836_muic_irq_chip;
		mfd_devs = max77836_devs;
		mfd_devs_size = ARRAY_SIZE(max77836_devs);
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_SHARED;
		break;
	case MAXIM_DEVICE_TYPE_MAX14577:
	default:
		irq_chip = &max14577_irq_chip;
		mfd_devs = max14577_devs;
		mfd_devs_size = ARRAY_SIZE(max14577_devs);
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
		break;
	}

	ret = regmap_add_irq_chip(maxim_core->regmap_muic, maxim_core->irq,
				  irq_flags, 0, irq_chip,
				  &maxim_core->irq_data_muic);
	if (ret != 0) {
		dev_err(&i2c->dev, "Failed to request IRQ %d: %d\n",
				maxim_core->irq, ret);
		return ret;
	}

	/* Max77836 specific initialization code (additional regmap) */
	if (maxim_core->dev_type == MAXIM_DEVICE_TYPE_MAX77836) {
		ret = max77836_init(maxim_core);
		if (ret < 0)
			goto err_max77836;
	}

	ret = mfd_add_devices(maxim_core->dev, -1, mfd_devs,
			mfd_devs_size, NULL, 0,
			regmap_irq_get_domain(maxim_core->irq_data_muic));
	if (ret < 0)
		goto err_mfd;

	device_init_wakeup(maxim_core->dev, 1);

	return 0;

err_mfd:
	if (maxim_core->dev_type == MAXIM_DEVICE_TYPE_MAX77836)
		max77836_remove(maxim_core);
err_max77836:
	regmap_del_irq_chip(maxim_core->irq, maxim_core->irq_data_muic);

	return ret;
}

static int max14577_i2c_remove(struct i2c_client *i2c)
{
	struct maxim_core *maxim_core = i2c_get_clientdata(i2c);

	mfd_remove_devices(maxim_core->dev);
	regmap_del_irq_chip(maxim_core->irq, maxim_core->irq_data_muic);
	if (maxim_core->dev_type == MAXIM_DEVICE_TYPE_MAX77836)
		max77836_remove(maxim_core);

	return 0;
}

static const struct i2c_device_id max14577_i2c_id[] = {
	{ "max14577", MAXIM_DEVICE_TYPE_MAX14577, },
	{ "max77836", MAXIM_DEVICE_TYPE_MAX77836, },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max14577_i2c_id);

#ifdef CONFIG_PM_SLEEP
static int max14577_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct maxim_core *maxim_core = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		enable_irq_wake(maxim_core->irq);

	return 0;
}

static int max14577_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct maxim_core *maxim_core = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		disable_irq_wake(maxim_core->irq);

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static SIMPLE_DEV_PM_OPS(max14577_pm, max14577_suspend, max14577_resume);

static struct i2c_driver max14577_i2c_driver = {
	.driver = {
		.name = "max14577",
		.owner = THIS_MODULE,
		.pm = &max14577_pm,
		.of_match_table = max14577_dt_match,
	},
	.probe = max14577_i2c_probe,
	.remove = max14577_i2c_remove,
	.id_table = max14577_i2c_id,
};

static int __init max14577_i2c_init(void)
{
	BUILD_BUG_ON(ARRAY_SIZE(max14577_i2c_id) != MAXIM_DEVICE_TYPE_NUM);
	BUILD_BUG_ON(ARRAY_SIZE(max14577_dt_match) != MAXIM_DEVICE_TYPE_NUM);

	return i2c_add_driver(&max14577_i2c_driver);
}
subsys_initcall(max14577_i2c_init);

static void __exit max14577_i2c_exit(void)
{
	i2c_del_driver(&max14577_i2c_driver);
}
module_exit(max14577_i2c_exit);

MODULE_AUTHOR("Chanwoo Choi <cw00.choi@samsung.com>, Krzysztof Kozlowski <k.kozlowski@samsung.com>");
MODULE_DESCRIPTION("Maxim 14577/77836 multi-function core driver");
MODULE_LICENSE("GPL");
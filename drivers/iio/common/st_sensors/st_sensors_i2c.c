/*
 * STMicroelectronics sensors i2c library driver
 *
 * Copyright 2012-2013 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/iio/iio.h>

#include <linux/iio/common/st_sensors_i2c.h>


#define ST_SENSORS_I2C_MULTIREAD	0x80

static unsigned int st_sensors_i2c_get_data_rdy_irq(struct iio_dev *indio_dev)
{
	struct st_sensor_data *sdata = iio_priv(indio_dev);

	return sdata->irq_map[ST_SENSORS_INT1];
}

static unsigned int st_sensors_i2c_get_event_irq(struct iio_dev *indio_dev)
{
	struct st_sensor_data *sdata = iio_priv(indio_dev);

	return sdata->irq_map[ST_SENSORS_INT2];
}

static void st_sensors_i2c_map_irqs(struct i2c_client *client,
		struct iio_dev *indio_dev)
{
	struct st_sensor_data *sdata = iio_priv(indio_dev);
	struct device_node *np = client->dev.of_node;
	u32 irq_policy = 0;

	of_property_read_u32(np, "irq-map-policy", &irq_policy);

	switch (irq_policy) {
	case ST_SENSORS_MAP_ONLY_DRDY_IRQ:
		sdata->irq_map[ST_SENSORS_INT1] = irq_of_parse_and_map(np, 0);
		break;
	case ST_SENSORS_MAP_ONLY_EVENT_IRQ:
		sdata->irq_map[ST_SENSORS_INT2] = irq_of_parse_and_map(np, 0);
		break;
	default:
		sdata->irq_map[ST_SENSORS_INT1] = 0;
		sdata->irq_map[ST_SENSORS_INT2] = 0;
	}
}

static int st_sensors_i2c_read_byte(struct st_sensor_transfer_buffer *tb,
				struct device *dev, u8 reg_addr, u8 *res_byte)
{
	int err;

	err = i2c_smbus_read_byte_data(to_i2c_client(dev), reg_addr);
	if (err < 0)
		goto st_accel_i2c_read_byte_error;

	*res_byte = err & 0xff;

st_accel_i2c_read_byte_error:
	return err < 0 ? err : 0;
}

static int st_sensors_i2c_read_multiple_byte(
		struct st_sensor_transfer_buffer *tb, struct device *dev,
			u8 reg_addr, int len, u8 *data, bool multiread_bit)
{
	if (multiread_bit)
		reg_addr |= ST_SENSORS_I2C_MULTIREAD;

	return i2c_smbus_read_i2c_block_data(to_i2c_client(dev),
							reg_addr, len, data);
}

static int st_sensors_i2c_write_byte(struct st_sensor_transfer_buffer *tb,
				struct device *dev, u8 reg_addr, u8 data)
{
	return i2c_smbus_write_byte_data(to_i2c_client(dev), reg_addr, data);
}

static const struct st_sensor_transfer_function st_sensors_tf_i2c = {
	.read_byte = st_sensors_i2c_read_byte,
	.write_byte = st_sensors_i2c_write_byte,
	.read_multiple_byte = st_sensors_i2c_read_multiple_byte,
};

void st_sensors_i2c_configure(struct iio_dev *indio_dev,
		struct i2c_client *client, struct st_sensor_data *sdata)
{
	i2c_set_clientdata(client, indio_dev);

	indio_dev->dev.parent = &client->dev;
	indio_dev->name = client->name;

	sdata->tf = &st_sensors_tf_i2c;

	st_sensors_i2c_map_irqs(client, indio_dev);
	sdata->get_irq_data_ready = st_sensors_i2c_get_data_rdy_irq;
	sdata->get_irq_event = st_sensors_i2c_get_event_irq;
}
EXPORT_SYMBOL(st_sensors_i2c_configure);

MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics ST-sensors i2c driver");
MODULE_LICENSE("GPL v2");

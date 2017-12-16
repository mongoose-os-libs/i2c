/*
 * Copyright 2015 Cesanta
 * All rights reserved
 */

/*
 * I2C API.
 *
 * See https://en.wikipedia.org/wiki/I%C2%B2C for the background information.
 */

#ifndef CS_FW_SRC_MGOS_I2C_H_
#define CS_FW_SRC_MGOS_I2C_H_

#include "mgos_features.h"

#include <stdbool.h>
#include <stdint.h>

#include "mgos_init.h"
#include "mgos_sys_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each platform defines its own I2C connection parameters. */
struct mgos_i2c;

/*
 * Initialize I2C master with the given params. Typically clients don't need to
 * do that manually: mgos has a global I2C instance created with the params
 * given in system config, use `mgos_i2c_get_global()` to get the global
 * instance.
 *
 * Example:
 * ```c
 * const struct mgos_config_i2c cfg = {
 *   .enable = true,
 *   .freq: 400,
 *   .debug: 0,
 *   .sda_gpio: 13,
 *   .scl_gpio: 12,
 * };
 * struct mgos_i2c *myi2c = mgos_i2c_create(&cfg);
 * ```
 */
struct mgos_i2c *mgos_i2c_create(const struct mgos_config_i2c *cfg);

/* If this special address is passed to read or write, START is not generated
 * and address is not put on the bus. It is assumed that this is a continuation
 * of a previous operation which (after read or write with stop = false). */
#define MGOS_I2C_ADDR_CONTINUE ((uint16_t) -1)

/*
 * Read specified number of bytes from the specified address.
 * Address should not include the R/W bit. If addr is -1, START is not
 * performed.
 * If |stop| is true, then at the end of the operation bus will be released.
 */
bool mgos_i2c_read(struct mgos_i2c *i2c, uint16_t addr, void *data, size_t len,
                   bool stop);

/*
 * Write specified number of bytes from the specified address.
 * Address should not include the R/W bit. If addr is -1, START is not
 * performed.
 * If |stop| is true, then at the end of the operation bus will be released.
 */
bool mgos_i2c_write(struct mgos_i2c *i2c, uint16_t addr, const void *data,
                    size_t len, bool stop);

/*
 * Release the bus (when left unreleased after read or write).
 */
void mgos_i2c_stop(struct mgos_i2c *i2c);

/* Most implementations should support these two, support for other frequencies
 * is platform-dependent. */
#define MGOS_I2C_FREQ_100KHZ 100000
#define MGOS_I2C_FREQ_400KHZ 400000

/*
 * Get I2C interface frequency.
 */
int mgos_i2c_get_freq(struct mgos_i2c *i2c);

/*
 * Set I2C interface frequency.
 */
bool mgos_i2c_set_freq(struct mgos_i2c *i2c, int freq);

/*
 * Helper for reading 1-byte register `reg` from a device at address `addr`.
 * In case of success return a numeric byte value from 0x00 to 0xff; otherwise
 * return -1.
 */
int mgos_i2c_read_reg_b(struct mgos_i2c *conn, uint16_t addr, uint8_t reg);

/*
 * Helper for reading 2-byte register `reg` from a device at address `addr`.
 * In case of success returns a numeric big-endian value: e.g. if 0x01, 0x02
 * was read from a device, 0x0102 will be returned.
 *
 * In case of error returns -1.
 */
int mgos_i2c_read_reg_w(struct mgos_i2c *conn, uint16_t addr, uint8_t reg);

/*
 * Helper for reading `n`-byte register value from a device. Returns true on
 * success, false on error. Data is written to `buf`, which should be large
 * enough.
 */
bool mgos_i2c_read_reg_n(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                         size_t n, uint8_t *buf);

/*
 * Helper for writing 1-byte register `reg` to a device at address `addr`.
 * Returns `true` in case of success, `false` otherwise.
 */
bool mgos_i2c_write_reg_b(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          uint8_t value);

/*
 * Helper for writing 2-byte register `reg` to a device at address `addr`.
 * The value is big-endian: e.g. if `value` is `0x0102`, then `0x01, 0x02`
 * will be written.
 * Returns `true` in case of success, `false` otherwise.
 */
bool mgos_i2c_write_reg_w(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          uint16_t value);

/*
 * Helper for writing `n`-byte register `reg` to a device at address `addr`.
 * Returns `true` in case of success, `false` otherwise.
 */
bool mgos_i2c_write_reg_n(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          size_t n, const uint8_t *buf);

/* Close i2c connection and free resources. */
void mgos_i2c_close(struct mgos_i2c *conn);

/* Return i2c bus handle that is set up via the sysconfig. */
struct mgos_i2c *mgos_i2c_get_global(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_FW_SRC_MGOS_I2C_H_ */

/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 */

#include "mgos_i2c.h"

static struct mgos_i2c *s_global_i2c;

int mgos_i2c_read_reg_b(struct mgos_i2c *conn, uint16_t addr, uint8_t reg) {
  uint8_t value;
  if (!mgos_i2c_read_reg_n(conn, addr, reg, 1, &value)) {
    return -1;
  }
  return value;
}

int mgos_i2c_read_reg_w(struct mgos_i2c *conn, uint16_t addr, uint8_t reg) {
  uint8_t tmp[2];
  if (!mgos_i2c_read_reg_n(conn, addr, reg, 2, tmp)) {
    return -1;
  }
  return (((uint16_t) tmp[0]) << 8) | tmp[1];
}

bool mgos_i2c_read_reg_n(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                         size_t n, uint8_t *buf) {
  return mgos_i2c_write(conn, addr, &reg, 1, false /* stop */) &&
         mgos_i2c_read(conn, addr, buf, n, true /* stop */);
}

bool mgos_i2c_write_reg_b(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          uint8_t value) {
  uint8_t tmp[2] = {reg, value};
  return mgos_i2c_write(conn, addr, tmp, sizeof(tmp), true /* stop */);
}

bool mgos_i2c_write_reg_w(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          uint16_t value) {
  uint8_t tmp[3] = {reg, (uint8_t)(value >> 8), (uint8_t) value};
  return mgos_i2c_write(conn, addr, tmp, sizeof(tmp), true /* stop */);
}

bool mgos_i2c_write_reg_n(struct mgos_i2c *conn, uint16_t addr, uint8_t reg,
                          size_t n, const uint8_t *buf) {
  bool res = false;
  uint8_t *tmp = calloc(n + 1, 1);
  if (tmp) {
    *tmp = reg;
    memcpy(tmp + 1, buf, n);
    res = mgos_i2c_write(conn, addr, tmp, n + 1, true /* stop */);
    free(tmp);
  }
  return res;
}

bool mgos_i2c_init(void) {
  if (!mgos_sys_config_get_i2c_enable()) return true;
  s_global_i2c = mgos_i2c_create(mgos_sys_config_get_i2c());
  return (s_global_i2c != NULL);
}

struct mgos_i2c *mgos_i2c_get_global(void) {
  return s_global_i2c;
}

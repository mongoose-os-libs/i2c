/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos_i2c.h"

#include "mgos_gpio.h"
#include "mgos_hal.h"

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

#define HALF_DELAY() (mgos_nsleep100)(100 / 2); /* 100 KHz */

bool mgos_i2c_reset_bus(int sda_gpio, int scl_gpio) {
  if (!mgos_gpio_setup_output(sda_gpio, 1) ||
      !mgos_gpio_set_pull(sda_gpio, MGOS_GPIO_PULL_UP) ||
      !mgos_gpio_set_mode(sda_gpio, MGOS_GPIO_MODE_OUTPUT_OD)) {
    return false;
  }
  if (!mgos_gpio_setup_output(scl_gpio, 1) ||
      !mgos_gpio_set_pull(scl_gpio, MGOS_GPIO_PULL_UP) ||
      !mgos_gpio_set_mode(scl_gpio, MGOS_GPIO_MODE_OUTPUT_OD)) {
    return false;
  }

  /*
   * Send some dummy clocks to reset the bus.
   * https://www.i2c-bus.org/i2c-primer/analysing-obscure-problems/blocked-bus/
   */
  HALF_DELAY();
  HALF_DELAY();
  for (int i = 0; i < 16; i++) {
    mgos_gpio_write(scl_gpio, 0);
    HALF_DELAY();
    mgos_gpio_write(sda_gpio, 0);
    HALF_DELAY();
    mgos_gpio_write(scl_gpio, 1);
    HALF_DELAY();
    HALF_DELAY();
  }
  /* STOP condition. */
  mgos_gpio_write(sda_gpio, 1);
  HALF_DELAY();
  return true;
}

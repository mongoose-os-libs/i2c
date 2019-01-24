/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "mgos_i2c.h"
#include "ubuntu_ipc.h"
#include "common/cs_dbg.h"

struct mgos_i2c {
  struct mgos_config_i2c cfg;
  int                    freq;
  int                    read_timeout_ms;
  int                    fd;
};

static const char *mgos_i2c_dev_filename(struct mgos_i2c *c) {
  static char fn[256];

  if (!c) {
    return "NULL";
  }

  snprintf(fn, sizeof(fn) - 1, "/dev/i2c-%d", c->cfg.bus_no);
  return (const char *)fn;
}

bool mgos_i2c_set_freq(struct mgos_i2c *c, int freq) {
  if (!c) {
    return false;
  }
  if (freq != MGOS_I2C_FREQ_100KHZ) {
    LOG(LL_ERROR, ("This driver only supports 100KHz I2C"));
    return false;
  }
  c->freq = freq;
  return true;
}

bool mgos_i2c_write(struct mgos_i2c *c, uint16_t addr, const void *data, size_t len, bool stop) {
  int ret;

  if (!c) {
    return false;
  }

  if (ioctl(c->fd, I2C_SLAVE, addr) < 0) {
    LOG(LL_ERROR, ("Cannot select slave 0x%02x on I2C%d bus", addr, c->cfg.bus_no));
    return false;
  }
  ret = write(c->fd, data, len);
  if (ret != (int)len) {
    LOG(LL_ERROR, ("Sent %d bytes (wanted %lu) to 0x%02x on I2C%d bus", ret, len, addr, c->cfg.bus_no));
    return false;
  }

  return true;

  (void)stop;
}

bool mgos_i2c_read(struct mgos_i2c *c, uint16_t addr, void *data, size_t len, bool stop) {
  fd_set         rfds;
  struct timeval tv;
  int            ret;

  if (!c) {
    return false;
  }

  if (ioctl(c->fd, I2C_SLAVE, addr) < 0) {
    LOG(LL_ERROR, ("Cannot select slave 0x%02x on I2C%d bus", addr, c->cfg.bus_no));
    return false;
  }

  FD_ZERO(&rfds);
  FD_SET(c->fd, &rfds);

  tv.tv_sec  = 0;
  tv.tv_usec = c->read_timeout_ms * 1000;
  ret        = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
  if (ret < 0) {
    LOG(LL_ERROR, ("Cannot not select(): %s", strerror(errno)));
    return false;
  } else if (ret == 0) {
    LOG(LL_ERROR, ("Read timeout (%dms) from slave 0x%02x on I2C%d bus", c->read_timeout_ms, addr, c->cfg.bus_no));
    return false;
  }

  ret = read(c->fd, data, len);
  if (ret != (int)len) {
    LOG(LL_ERROR, ("Received %d bytes (wanted %lu) from 0x%02x on I2C%d bus", ret, len, addr, c->cfg.bus_no));
    return false;
  }
  return true;

  (void)stop;
}

void mgos_i2c_stop(struct mgos_i2c *c) {
  if (!c) {
    return;
  }
  LOG(LL_WARN, ("Not implemented"));
}

int mgos_i2c_get_freq(struct mgos_i2c *c) {
  if (!c) {
    return 0;
  }
  return c->freq;
}

struct mgos_i2c *mgos_i2c_create(const struct mgos_config_i2c *cfg) {
  struct mgos_i2c *c = NULL;
  int fd;

  if (!cfg) {
    return NULL;
  }

  c = calloc(1, sizeof(*c));
  if (c == NULL) {
    return NULL;
  }

  memset(c, 0, sizeof(struct mgos_i2c));
  memcpy(&c->cfg, cfg, sizeof(c->cfg));
  fd = ubuntu_ipc_open(mgos_i2c_dev_filename(c), O_RDWR);
  if (fd < 0) {
    LOG(LL_ERROR, ("Could not open %s", mgos_i2c_dev_filename(c)));
    goto out_err;
  }

  c->fd = fd;
  c->read_timeout_ms = 150;
  if (!mgos_i2c_set_freq(c, cfg->freq)) {
    goto out_err;
  }

  LOG(LL_INFO, ("I2C%d init ok (dev: %s, freq: %d)", c->cfg.bus_no, mgos_i2c_dev_filename(c), c->freq));

  return c;

out_err:
  if (c) {
    free(c);
  }
  if (fd > 0) {
    close(fd);
  }
  LOG(LL_ERROR, ("Invalid I2C settings"));
  return NULL;
}

void mgos_i2c_close(struct mgos_i2c *c) {
  if (!c) {
    return;
  }
  if (c->fd > 0) {
    close(c->fd);
  }
  free(c);
}
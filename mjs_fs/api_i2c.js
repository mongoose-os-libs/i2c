let I2C = {
  _rrn: ffi('int mgos_i2c_read_reg_n(void *, int, int, int, char *)'),
  _r: ffi('bool mgos_i2c_read(void *, int, char *, int, bool)'),

  // ## **`I2C.get()`**
  // Get I2C bus handle. Return value: opaque pointer.
  get: ffi('void *mgos_i2c_get_global(void)'),
  get_default: ffi('void *mgos_i2c_get_global(void)'), // deprecated

  // ## **`I2C.close()`**
  // Close I2C handle. Return value: none.
  close: ffi('void mgos_i2c_close(void *conn)'),

  // ## **`I2C.write(handle, addr, buf, size, stop)`**
  // Send a byte array to I2C.
  // If stop is true, the bus will be released at the end.
  // Return value: success, true/false.
  write: ffi('bool mgos_i2c_write(void *, int, char *, int, bool)'),

  // ## **`I2C.read(handle, addr, len, stop)`**
  // Read specified number of bytes from the specified address.
  // If stop is true, the bus will be released at the end.
  // Return value: null on error, string with data on success. Example:
  // ```javascript
  // let data = I2C.read(bus, 31, 3, true);  // Read 3 bytes
  // if (data) print(JSON.stringify([data.at(0), data.at(1), data.at(2)]));
  // ```
  read: function(h, addr, len, stop) {
    let chunk = '          ', buf = chunk;
    while (buf.length < len) buf += chunk;
    let ok = this._r(h, addr, buf, len, stop);
    return ok ? buf.slice(0, len) : null;
  },

  // ## **`I2C.stop(handle)`**
  // Set i2c Stop condition. Releases the bus.
  // Return value: none.
  stop: ffi('void mgos_i2c_stop(void *)'),

  // ## **`I2C.readRegB(handle, addr, reg)`**
  // Read 1-byte register `reg` from the device at address `addr`.
  readRegB: ffi('int mgos_i2c_read_reg_b(void *, int, int)'),

  // ## **`I2C.readRegW(handle, addr, reg)`**
  // Read 2-byte register `reg` from the device at address `addr`.
  readRegW: ffi('int mgos_i2c_read_reg_w(void *, int, int)'),

  readRegN: function(i2c, addr, reg, num) {
    let buf = '';
    for (let i = 0; i < num; i++) buf += ' ';
    return (this._rrn(i2c, addr, reg, num, buf) === 1) ? buf : '';
  },

  // ## **`I2C.writeRegB(handle, addr, reg, val)`**
  // Write `val` into 1-byte register `reg` at address `addr`.
  // Return `true` on success, `false` on failure.
  writeRegB: ffi('bool mgos_i2c_write_reg_b(void *, int, int, int)'),

  // ## **`I2C.writeRegW(handle, addr, reg, val)`**
  // Write `val` into 2-byte register `reg` at address `addr`.
  // Return `true` on success, `false` on failure.
  writeRegW: ffi('bool mgos_i2c_write_reg_w(void *, int, int, int)'),
  writeRegN: ffi('int mgos_i2c_write_reg_n(void *, int, int, int, char *)'),
};

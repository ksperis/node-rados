var rados = require('bindings')('rados');

var ReadableStream = require('stream').Readable;
var WritableStream = require('stream').Writable;

rados.Ioctx.prototype.createReadStream = function(oid, options) {
  var ioctx = this;

  var stream = new ReadableStream(options);
  stream._offset = 0;

  stream._read = function(size) {
    ioctx.aio_read(oid, size, stream._offset, function(err, chunk) {
      if (err) return stream.emit('error', err);
      stream._offset += chunk.length;
      stream.push((chunk.length > 0) ? chunk : null);
    });
  };

  return stream;
}

rados.Ioctx.prototype.createWriteStream = function(oid, options) {
  var ioctx = this;

  var stream = new WritableStream(oid, options);
  stream._offset = 0;

  stream._write = function(data, encoding, cb) {
    if (!(data instanceof Buffer))
      return this.emit('error', new Error('Invalid data'));

    ioctx.aio_write(oid, data, data.length, stream._offset, function(err) {
      if (err) return stream.emit('error', err);
      stream._offset += data.length;
      cb();
    });
  };

  stream.flush = function(cb) {
    ioctx.aio_flush_async(function (err) {
      if (err) stream.emit('error', err);
      cb();
    });
  }

  return stream;
}

exports = module.exports = rados;

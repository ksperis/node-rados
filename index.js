var rados = require('bindings')('rados');

var ReadableStream = require('stream').Readable;

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

exports = module.exports = rados;

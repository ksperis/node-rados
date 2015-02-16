var rados = require('../');

var CEPH_CLUSTER = 'ceph';
var CEPH_ID = 'client.admin';
var CEPH_CONF = '/etc/ceph/ceph.conf';
var CEPH_POOL = 'rbd';

module.exports = {
  setUp: function(callback) {
    // Connect to cluster
    this.cluster = new rados.Rados(CEPH_CLUSTER, CEPH_ID, CEPH_CONF);
    var err = this.cluster.connect();
    if (err) {
      throw new Error('Could not connect to Ceph cluster');
    }

    // Create Ioctx
    this.ioctx = new rados.Ioctx(this.cluster, CEPH_POOL);

    callback();
  },

  tearDown: function(callback) {
    // Clean up
    if (this.ioctx) this.ioctx.destroy();
    this.cluster.shutdown();
    callback();
  },

  'stream': function(test) {
    var ioctx = this.ioctx;
    var buffer = new Buffer('01234567ABCDEF');

    // Write buffer to testfile
    test.equal(this.ioctx.write_full('testfile', buffer), 0);

    var stream = ioctx.createReadStream('testfile', {highWaterMark:1}); // 1 char at the time

    stream.on('data', function(data) {
      if (stream._offset > 14) throw new Error('Expected no more than 14 characters');

      test.ok(data.length === 1);
      test.equal(data[0], buffer[stream._offset-1]);
    });

    stream.on('end', function() {
      test.ok(stream._offset === 14);
      test.equal(ioctx.remove('testfile'), 0);
      test.done();
    });
  },
};

process.on('uncaughtException', function(err) {
    console.error(err.stack);
});

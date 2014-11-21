var rados = require('../build/Release/rados');

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

  'get_fsid': function(test) {
    test.equal(typeof this.cluster.get_fsid(), 'string');
    test.done();
  },

  'pool_list': function(test) {
    var pools = this.cluster.pool_list();
    test.ok(pools instanceof Array);
    test.ok(pools.indexOf(CEPH_POOL) >= 0);
    test.done();
  },

  'write_full': function(test) {
    test.equal(this.ioctx.write_full('testfile', new Buffer('01234567ABCDEF')), 0);
    test.equal(this.ioctx.read('testfile'), '01234567ABCDEF');
    test.done();
  },

  'write': function(test) {
    test.equal(this.ioctx.write('testfile', new Buffer('89A'), 3, 8), 0);
    test.equal(this.ioctx.write('testfile', new Buffer('BCD'), null, 11), 0); // len null
    test.equal(this.ioctx.read('testfile'), '0123456789ABCD');
    test.done();
  },

  'append': function(test) {
    test.equal(this.ioctx.append('testfile', new Buffer('EFG')), 0);
    test.equal(this.ioctx.read('testfile'), '0123456789ABCDEFG');
    test.done();
  },

  'stat': function(test) {
    var stat = this.ioctx.stat('testfile');
    test.equal(stat.oid, 'testfile');
    test.equal(stat.psize, 17);
    test.ok(stat.pmtime > (Date.now() / 1000) - 10); // 10 seconds
    test.done();
  },

  'trunc': function(test) {
    test.equal(this.ioctx.trunc('testfile', 20), 0); // enlarge by 3
    // offset read
    test.equal(this.ioctx.read('testfile', 3, 17).toString(), '\0\0\0');
    test.done();
  },

  'setxattr': function(test) {
    test.equal(this.ioctx.setxattr('testfile', 'attr1', 'first attr'), 0);
    test.equal(this.ioctx.setxattr('testfile', 'attr2', 'second attr'), 0);
    test.equal(this.ioctx.setxattr('testfile', 'attr3', 'third attr'), 0);
    test.done();
  },

  'rmxattr': function(test) {
    test.equal(this.ioctx.rmxattr('testfile', 'attr2'), 0);
    test.done();
  },

  'getxattr': function(test) {
    var attrs = this.ioctx.getxattrs('testfile');
    test.equal(attrs.attr1, 'first attr');
    test.equal(attrs.attr2, undefined); // was rmxattr
    test.equal(attrs.attr3, 'third attr');
    test.done();
  },

  'aio_read': function(test) {
    var ioctx = this.ioctx;
    ioctx.aio_read('testfile', 3, 10, function(err, data) {
      test.ifError(err);
      test.equal(data.toString(), 'ABC');
      test.done();
    });
  },

  'aio_write': function(test) {
    var ioctx = this.ioctx;
    ioctx.aio_write('testfile', new Buffer('ABCDEFG'), null, 3, function(err) {
      test.ifError(err);
      test.equal(ioctx.read('testfile').toString(), '012ABCDEFGABCDEFG\0\0\0');
      test.done();
    });
  },

  'aio_append': function(test) {
    var ioctx = this.ioctx;
    ioctx.aio_append('testfile', new Buffer([0,0,0]), null, function(err) {
      test.ifError(err);
      test.equal(ioctx.read('testfile').toString(), '012ABCDEFGABCDEFG\0\0\0\0\0\0');
      test.done();
    });
  },

  'aio_write_full': function(test) {
    var ioctx = this.ioctx;
    ioctx.aio_write_full('testfile', new Buffer('ABC'), null, function(err) {
      test.ifError(err);
      test.equal(ioctx.read('testfile').toString(), 'ABC');
      test.done();
    });
  },

  'aio_flush': function(test) {
    test.equal(this.ioctx.aio_flush(), 0);
    test.done();
  },

  'aio_flush_async': function(test) {
    var ioctx = this.ioctx;
    ioctx.aio_flush_async(function(err) {
      test.ifError(err);
      test.done();
    });
  },

  'remove': function(test) {
    test.equal(this.ioctx.remove('testfile'), 0);
    test.equal(this.ioctx.remove('testfile'), 2); // ENOENT
    test.done();
  },
};

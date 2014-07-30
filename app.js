var rados = require('./build/Release/rados');

//var cluster = new rados.Rados( "ceph", "client.admin", "/etc/ceph/ceph.conf");
var cluster = new rados.Rados()
cluster.connect();
console.log( "fsid : " + cluster.get_fsid() );
console.log( "pools : \n" + cluster.pool_list() );

var ioctx = new rados.Ioctx(cluster, "data");
ioctx.write_full("node-rados", new Buffer("01234567"));
console.log( ioctx.read("node-rados", ioctx.stat("node-rados").psize).toString() );

ioctx.destroy();
cluster.shutdown();

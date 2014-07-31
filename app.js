var rados = require('./build/Release/rados');

var cluster = new rados.Rados( "ceph", "client.admin", "/etc/ceph/ceph.conf");
cluster.connect(function (err) {
	if (err) {
		console.log("Error " + err);
		throw err;
	}
	console.log( "fsid : " + cluster.get_fsid() );
	
	var ioctx = new rados.Ioctx(cluster, "data");
	ioctx.write_full("testfile1", new Buffer("01234567"));
	console.log( "Read data : " + 
		ioctx.read("testfile1", ioctx.stat("node-rados").psize).toString() );

	ioctx.aio_write("testfile2", new Buffer("01234567")
		, function (err) {
			if (err) {
				console.log("Error " + err);
				throw err;
			}
			console.log( "aio_write sent" );
		}
		, function (err) {
			console.log( "--> in memory" );
		}
		, function (err) {
			console.log( "--> on disk" );
		});

	ioctx.destroy();
});

cluster.shutdown();

var rados = require('./build/Release/rados');

//==================================
//     Connect to cluster
//==================================
var cluster = new rados.Rados( "ceph", "client.admin", "/etc/ceph/ceph.conf");

var err = cluster.connect();
if (err) {
	// On connection error
	console.log("Error " + err);
	throw err;
}

// Print cluster FSID
console.log( "fsid : " + cluster.get_fsid() );


//==================================
//     Create IOCTX
//==================================
var ioctx = new rados.Ioctx(cluster, "data");

console.log(" --- Sync Read / Write --- ");
// Sync write_full
ioctx.write_full("testfile1", new Buffer("01234567"));

// Sync Read
console.log( "Read data : " + 
	ioctx.read("testfile1", ioctx.stat("testfile1").psize).toString() );

// Remove
ioctx.remove("testfile1");

console.log(" --- ASync Read / Write --- ");
// ASync write_full
ioctx.aio_write_full("testfile2", new Buffer("01234"), 4
	, function (err) {
		if (err) {
			// On write error
			console.log("Error " + err);
			throw err;
		}

		// ASync append
		ioctx.aio_append("testfile2", new Buffer("5678"), 4
		, function (err) {
			if (err) {
				// On write error
				console.log("Error " + err);
				throw err;
			}

			ioctx.aio_write("testfile2", new Buffer("XX"), 2, 3, function (err) {} , function (err) {} , function (err) {});

			ioctx.aio_read("testfile2", ioctx.stat("testfile2").psize, 0, function (err) {}, function (err, data) {
				console.log( "Read Callback : " + data);
			} );
			
		}
		, function (err) {} 
		, function (err) {});

	}
	, function () {
		// In memory callback
		console.log( "--> (async) in memory" );
	}
	, function () {
		// On disk callback
		console.log( "--> (async) on disk" );
	});

ioctx.destroy();

cluster.shutdown();
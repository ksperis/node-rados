var rados = require('./build/Release/rados');

// EXEMPLE FILE

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

// Print cluster FSID, pools
console.log( "fsid : " + cluster.get_fsid() );
console.log( "ls pools : " + cluster.pool_list() );


//==================================
//     Create IOCTX
//==================================
var ioctx = new rados.Ioctx(cluster, "data");

console.log(" --- RUN Sync Read / Write --- ");
// Sync write_full
ioctx.write_full("testfile1", new Buffer("01234567ABCDEF"));

// Sync Read
console.log( "Read data : " + 
	ioctx.read("testfile1", ioctx.stat("testfile1").psize).toString() );

// Remove
ioctx.remove("testfile1");

console.log(" --- RUN ASync Read / Write --- ");
// ASync write_full
ioctx.aio_write("testfile2", new Buffer("1234567879ABCD"), 16, 0, function (err) {
	if (err) {
	  throw err;
	}

	ioctx.aio_read("testfile2", 16, 0, function (err, data) {
	if (err) {
	  throw err;
	}

	 console.log("[async callback] data = " + data.toString());

	});

});


//==================================
//     Read / Write Attributes
//==================================

console.log(" --- RUN Attributes Read / Write --- ");

ioctx.setxattr("testfile3", "attr1", "first attr");
ioctx.setxattr("testfile3", "attr2", "second attr");
ioctx.setxattr("testfile3", "attr3", "last attr value");

var attrs = ioctx.getxattrs("testfile2");

console.log("testfile3 xattr = %j", attrs);


// destroy ioctx and close cluster after aio_flush
ioctx.aio_flush_async(function (err) {
	ioctx.destroy();
	cluster.shutdown();
});


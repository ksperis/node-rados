var rados = require('./build/Release/rados');

// EXAMPLE FILE

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
var ioctx = new rados.Ioctx(cluster, "rbd");

console.log(" --- RUN Sync Write / Read --- ");
// Sync write_full
ioctx.write_full("testfile1", new Buffer("01234567ABCDEF"));

// Sync Read

console.log( "Read data : " + 
	ioctx.read("testfile1", ioctx.stat("testfile1").psize).toString() );

// Remove
ioctx.remove("testfile1");

console.log(" --- RUN ASync Write / Read --- ");
// ASync write_full
ioctx.aio_write("testfile2", new Buffer("1234567879ABCD"), 14, 0, function (err) {
	if (err) {
	  throw err;
	}

	ioctx.aio_read("testfile2", 14, 0, function (err, data) {
	if (err) {
	  throw err;
	}

	 console.log("[async callback] data = " + data.toString());

	});

});


//==================================
//     Read / Write Attributes
//==================================

console.log(" --- RUN Attributes Write / Read --- ");

ioctx.setxattr("testfile3", "attr1", "first attr");
ioctx.setxattr("testfile3", "attr2", "second attr");
ioctx.setxattr("testfile3", "attr3", "last attr value");

var attrs = ioctx.getxattrs("testfile3");

console.log("testfile3 xattr = %j", attrs);


// destroy ioctx and close cluster after aio_flush
ioctx.aio_flush_async(function (err) {
	ioctx.destroy();
	cluster.shutdown();
});


process.exit(code=0)

// OTHER EXAMPLES

//   Read Sync file in chunks
var file = "testfile";
var	fileSize = ioctx.stat(file).psize,
	chunkSize = 512,
    bytesRead = 0;


while (bytesRead < fileSize) {
    if ((bytesRead + chunkSize) > fileSize) {
        chunkSize = (fileSize - bytesRead);
    }
    var buffer = ioctx.read(file, chunkSize, bytesRead);
    bytesRead += chunkSize;
    process.stdout.write(buffer.toString());
}


//   Read Async file in chunks
var file = "testfile";
var	fileSize = ioctx.stat(file).psize,
	chunkSize = 512,
    bytesRead = 0;


while (bytesRead < fileSize) {
    if ((bytesRead + chunkSize) > fileSize) {
        chunkSize = (fileSize - bytesRead);
    }
    ioctx.aio_read(file, chunkSize, bytesRead, function (err, data) {
    	process.stdout.write(data.toString());
    });
    bytesRead += chunkSize;
}


//   Write Async file in chunks
var filename="/tmp/bigfile"
var object = "bigfile";


fs.open(filename, 'r', function(err, fd) {
    if (err) { throw "Read error"; }

    fs.fstat(fd, function(err, stats) {
        if (err) { throw "Stats error"; }

        var fileSize=stats.size,
            chunkSize=51200,
            buffer=new Buffer(chunkSize),
            totalBytesRead = 0;
            bytesRead = 0;

        while (totalBytesRead < fileSize) {
            bytesRead = fs.readSync(fd, buffer, 0, chunkSize, totalBytesRead);
            ioctx.aio_write(object, buffer, bytesRead, totalBytesRead, function (err) {
                if (err) { throw err; }
            });

            totalBytesRead += bytesRead;
        }
        fs.close(fd);
        ioctx.aio_flush_async(function (err) {
            ioctx.destroy();
            cluster.shutdown();
        });
    });
});


//   Use snapshot
ioctx.write_full("testfile10", new Buffer("version1"));
ioctx.snap_create("snaptest1");
ioctx.write_full("testfile10", new Buffer("version2"));
ioctx.snap_create("snaptest2");
ioctx.write_full("testfile10", new Buffer("version3"));
ioctx.snap_create("snaptest3");

ioctx.snap_rollback("testfile10", "snaptest2");
console.log(ioctx.read("testfile10").toString());

ioctx.snap_remove("snaptest1");
ioctx.snap_remove("snaptest2");
ioctx.snap_remove("snaptest3");

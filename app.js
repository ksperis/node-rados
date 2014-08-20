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

console.log(" --- Sync Read / Write --- ");
// Sync write_full
ioctx.write_full("testfile1", new Buffer("01234567ABCDEF"));

// Sync Read
console.log( "Read data : " + 
	ioctx.read("testfile1", ioctx.stat("testfile1").psize).toString() );

// Remove
ioctx.remove("testfile1");


//==================================
//     Read / Write Attributes
//==================================

ioctx.setxattr("testfile2", "attr1", "first attr");
ioctx.setxattr("testfile2", "attr2", "second attr");
ioctx.setxattr("testfile2", "attr3", "last attr value");


var attrs = ioctx.getxattrs("testfile2");

console.log(" --- testfile2 xattr : --- ");
for(var attr in attrs) {
    if(attrs.hasOwnProperty(attr)){
        console.log(attr + ': ' + attrs[attr]);
    }
}



ioctx.destroy();

cluster.shutdown();
node-rados
==========

Ceph rados client for node

Installation
-------------

	sudo apt-get install nodejs node-gyp librados-dev
	git clone https://github.com/ksperis/node-rados
	cd node-rados
	node-gyp rebuild

Usage
-----

Connect to cluster :

	cluster = new rados.Rados(cluster, user, conffile)	// return Rados
	cluster.connect()									// return err code
	cluster.shutdown()									// return err code
	cluster.get_fsid()									// return String (null on error)
	cluster.pool_list()									// return Array (null on error)

Create Ioctx :

	ioctx = new rados.Ioctx(cluster, poolname)

Sync Buffered functions :

	ioctx.write_full(oid, buffer)	// return err code
	ioctx.read(oid, size)			// return Buffer (null on error)
	ioctx.remove(oid)				// return err code
	ioctx.stat(oid)					// return err code

Testing functions :

	ioctx.aio_write(oid, buffer, size, offset, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_append(oid, buffer, size, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_write_full(oid, buffer, size, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_read(oid, size, function (err) {}, function complete (err) {})

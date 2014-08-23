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

Load module :

	var rados = require('./build/Release/rados');
	// or, if installed whith npm in node_modules
	var rados = require('rados');


Connect to cluster :

	cluster = new rados.Rados(cluster, user, conffile)	// return Rados object
	cluster.connect()									// on err, return err code
	cluster.shutdown()									// on err, return err code

	cluster.get_fsid()									// return String (null on error)
	cluster.pool_list()									// return Array (null on error)


Create / Delete Ioctx :

	ioctx = new rados.Ioctx(cluster, poolname)			// on error, throw error
	ioctx.delete()


Manage snapshots :

	ioctx.snap_create(snapname)							// on err, return err code
	ioctx.snap_remove(snapname)							// on err, return err code
	ioctx.snap_rollback(oid, snapname)					// on err, return err code


Sync Buffered functions :

	ioctx.read(oid, [size], [offset])					// return Buffer (null on error)
	ioctx.write(oid, buffer, [size], [offset])			// on err, return err code
	ioctx.write_full(oid, buffer, [size])				// on err, return err code
	ioctx.clone_range(dst, dst_off, src, src_off, size)	// on err, return err code
	ioctx.append(oid, buffer, [size])					// on err, return err code
	ioctx.trunc(oid, size)								// on err, return err code
	ioctx.remove(oid)									// on err, return err code
	ioctx.stat(oid)										// return Object with attr psize, pmtime


Manage Attr :

	ioctx.getxattr(oid, attr, [size])					// return String (null on error)
	ioctx.setxattr(oid, attr, value, [size])			// on err, return err code
	ioctx.rmxattr(oid, attr)							// on err, return err code
	ioctx.getxattrs(oid)								// return object with attributes (null on error)


AIO functions (testing) :

	ioctx.aio_read(oid, size, offset, function (err, data) {})
	ioctx.aio_write(oid, buffer, size, offset, function (err) {})
	ioctx.aio_append(oid, buffer, size, function (err) {})
	ioctx.aio_write_full(oid, buffer, size, function (err) {})
	ioctx.aio_flush()
	ioctx.aio_flush_async(function (err) {})

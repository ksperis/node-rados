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

	cluster = new rados.Rados(cluster, user, conffile)	// return Rados object
	cluster.connect()									// on err, return err code
	cluster.shutdown()									// on err, return err code

	cluster.get_fsid()									// return String (null on error)
	cluster.pool_list()									// return Array (null on error)


Create Ioctx :

	ioctx = new rados.Ioctx(cluster, poolname)			// on error, throw error


Sync Buffered functions :

	ioctx.read(oid, size[,offset])						// return Buffer (null on error)
	ioctx.write(oid, buffer, [size], [offset])			// on err, return err code
	ioctx.write_full(oid, buffer, [size])				// on err, return err code
	ioctx.clone_range(src, src_off, dst, dst_off, size)	// on err, return err code
	ioctx.append(oid, buffer, [size])					// on err, return err code
	ioctx.trunk(oid, size)								// on err, return err code
	ioctx.remove(oid)									// on err, return err code
	ioctx.stat(oid)										// return Object with attr psize, pmtime


Manage Attr :

	ioctx.getxattr(oid, attr, [size])					// return String (null on error)
	ioctx.setxattr(oid, attr, value, [size])			// on err, return err code
	ioctx.rmxattr(oid, attr)							// on err, return err code
	ioctx.getxattrs(oid)								// return object with attributes (null on error)


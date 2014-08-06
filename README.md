node-rados
==========

Ceph rados client for node

	sudo apt-get install nodejs node-gyp librados-dev
	git clone https://github.com/ksperis/node-rados
	cd node-rados
	node-gyp rebuild


	cluster = new rados.Rados(cluster, user, conffile)
	cluster.connect(function (err) {})
	cluster.shutdown()
	cluster.get_fsid()

	ioctx = new rados.Ioctx(cluster, poolname)
	ioctx.write_full(oid, buffer)
	ioctx.read(oid, size)
	ioctx.remove(oid)
	ioctx.stat(oid)
	ioctx.aio_write(oid, buffer, size, offset, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_append(oid, buffer, size, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_write_full(oid, buffer, size, function (err) {}, function complete (err) {}, function safe (err) {})
	ioctx.aio_read(oid, size, function (err) {}, function complete (err) {})

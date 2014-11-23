{
  "targets": [
    {
      "target_name": "rados",
      "sources": [ "module.cc", "rados.cc" ],
      "include_dirs" : [ "<!(node -e \"require('nan')\")" ],
      'libraries': [ "-lrados" ]
    }
  ]
}

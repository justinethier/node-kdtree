# Before this will work, libkdtree needs to be installed:
#
# make install PREFIX=/usr
#

import Utils

srcdir = 'src'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  conf.env.append_value("LIBPATH_KD", '/usr/lib')
  conf.env.append_value("LIB_KD", 'kdtree')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'kdtree'
  obj.source = 'src/node-kdtree.cc'
  obj.uselib = 'KD'

#
# TODO: see this link
# http://debuggable.com/posts/unit-testing-with-node-js:4b647d40-34e4-435a-a880-3b04cbdd56cb
#
# Change our tests over to use node.js assert and call into *all* the tests from 
# node-waf test
#
def test(tst):
  status = Utils.exec_command('node tests/test.js && node tests/test2.js');
  if status != 0:
     raise Utils.WafError('tests failed');

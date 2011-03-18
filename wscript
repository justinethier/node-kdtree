# Before this will work, libkdtree needs to be installed:
#
# make install PREFIX=/usr
#

import Utils
from logging import fatal

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  if not conf.check(lib='kdtree'):
     if not conf.check(lib="kdtree", libpath=['/usr/local/lib', '/opt/local/lib'], uselib_store="KD"):
                   fatal("libkdtree not found.")
  conf.env.append_value("LIBPATH_KD", '/usr/lib')
  conf.env.append_value("LIB_KD", 'kdtree')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'kdtree'
  obj.source = 'src/node-kdtree.cc'
  obj.uselib = 'KD'

#
# Added test command, based upon assert testing. See:
# http://debuggable.com/posts/unit-testing-with-node-js:4b647d40-34e4-435a-a880-3b04cbdd56cb
#
def test(tst):
  status = Utils.exec_command('node tests/baseline-test.js && node tests/large-dataset-test.js && node tests/loop-test.js');
  if status != 0:
     raise Utils.WafError('tests failed');

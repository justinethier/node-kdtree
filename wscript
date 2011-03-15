# Before this will work, libkdtree needs to be installed:
#
# make install PREFIX=/usr
#
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

{
  'targets': [
    {
      'target_name': 'kdtree',
      'sources': [ 'src/node-kdtree.cc' ],
#      'conditions' : [
#        ['OS=="linux"', {
#          'link_settings': {
#            'libraries': [
#              '-lkdtree'
#            ]
#          }
#        }]
#      ]
    }
  ]
}

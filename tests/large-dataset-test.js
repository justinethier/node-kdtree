/**
 * Test to demonstrate search on a large dataset.
 *
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */
var assert = require('assert');
var kd = require('kdtree');
var tree = new kd.KDTree(4);

console.log( "Loading data into tree, this may take awhile..." );
var i = 0;
for (var x = 0; x < 75; x++){
  for (var y = 0; y < 75; y++){
    for (var z = 0; z < 75; z++){
      tree.insert(x, y, z, 4, "element #" + i++); }}}
console.log( "Done loading the tree" );
assert.deepEqual( tree.nearest(9.5, 19.5, 31.5, 0), [10, 19, 32, 4, 'element #57707'] );
assert.deepEqual( tree.nearest(0, 0, 0, 0), [0, 0, 0, 4, 'element #0'] );

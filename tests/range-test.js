/**
 * Test to verify all members of a large dataset.
 *
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */
var assert = require('assert');
var kd = require('../build/Release/kdtree');
var tree = new kd.KDTree(2);

console.log( "Loading data into tree, this may take awhile..." );
var i = 0;
for (var x = 0; x < 75; x++){
  for (var y = 0; y < 75; y++){
      tree.insert(x, y); }}

assert.deepEqual(
  tree.nearestRange(0, 0, 3), 
  [ [ 2, 2 ],
    [ 2, 1 ],
    [ 1, 2 ],
    [ 1, 1 ],
    [ 0, 3 ],
    [ 0, 2 ],
    [ 3, 0 ],
    [ 2, 0 ],
    [ 1, 0 ],
    [ 0, 1 ],
    [ 0, 0 ] ]);

//console.log( tree.nearestRange(-10, -10, 2));

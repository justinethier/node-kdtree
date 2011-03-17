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
var kd = require('kdtree');
var tree = new kd.KDTree(2);

console.log( "Loading data into tree, this may take awhile..." );
var i = 0;
for (var x = 0; x < 75; x++){
  for (var y = 0; y < 75; y++){
      tree.insert(x, y); }}

for (var x = 0; x < 75; x++){
  for (var y = 0; y < 75; y++){
    assert.deepEqual( tree.nearest(x, y), [x, y], "Failed test for [" + x + "," + y + "]. Nearest is" + tree.nearest(x, y) ); }}


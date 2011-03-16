/**
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */
var assert = require('assert');
var kd = require('kdtree');
var tree = new kd.KDTree();
// TODO: put all  insert data into a big list, and print that list out here...
tree.insert(1, 2, 3);
tree.insert(1, 1.9, 3);
//tree.insert(1, 2, 3);
tree.insert(1, 11.1, 4);
tree.insert(10, 20, 30);
console.log( tree.nearest(9, 19, 31) );
console.log( tree.nearest(0, 0, 0) );
console.log( tree.nearest(0.5, 2, 4) );
console.log( tree.nearest(1, 11, 4) );
console.log( tree.nearest(100, 11000, 4000) );
console.log( tree.nearest(100, -11000, 4000) );
console.log( tree.nearest(1, 0, 0) );

// Test using a second tree of different dimension
var tree2 = new kd.KDTree(4);
tree2.insert(10, 10, 10, 10, "10x4 test");
tree2.insert(1, 2, 3, 4, "A 4-dimensional point");
tree2.insert(1, 1.9, 3, 4, "Another 4-dimensional point");
assert.deepEqual(tree2.nearest(10, 10, 10, 10),
                 [ 10, 10, 10, 10, '10x4 test' ],
                 "Searching on a new tree");
assert.deepEqual(tree2.nearest(1, 10, 10, 1),
                 [1, 2, 3, 4, "A 4-dimensional point"]);

// Test that first tree still works...
assert.deepEqual( tree.nearest(10, 10, 10),
                  [ 1, 11.1, 4 ],
                  "Verify original tree can still be searched");

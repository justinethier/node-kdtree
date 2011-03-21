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
tree.insert(1, 2, 3);
tree.insert(1, 1.9, 3);
//tree.insert(1, 2, 3);
tree.insert(1, 11.1, 4);
tree.insert(10, 20, 30);
assert.deepEqual( tree.nearest(9, 19, 31), [10, 20, 30]);
assert.deepEqual( tree.nearest(0, 0, 0), [1, 1.9, 3]);
assert.deepEqual( tree.nearest(0.5, 2, 4), [1, 2, 3]);
assert.deepEqual( tree.nearest(1, 11, 4), [1, 11.1, 4]);
assert.deepEqual( tree.nearest(100, 11000, 4000), [10, 20, 30]);
assert.deepEqual( tree.nearest(100, -11000, 4000), [1, 1.9, 3]);
assert.deepEqual( tree.nearest(1, 0, 0), [1, 1.9, 3]);

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

assert.equal( tree.dimensions(), 3);
assert.equal( tree2.dimensions(), 4);

// Test shortcut methods for point / value
var treeS = new kd.KDTree();
treeS.insert(1, 1, 1, "test");
treeS.insert(2, 2, 2);
assert.deepEqual( treeS.nearestValue(0, 0, 0), ["test"] );
assert.deepEqual( treeS.nearestValue(3, 3, 3), [] );
assert.deepEqual( treeS.nearestPoint(0, 0, 0), [1, 1, 1] );
assert.deepEqual( treeS.nearestPoint(3, 3, 3), [2, 2, 2] );

var kd = require('kdtree');
var tree = new kd.KDTree();
tree.test();
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

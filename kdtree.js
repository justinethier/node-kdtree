//var Kdtree_lib = require('./kdtree').KDTree;
//module.exports = new Kdtree_lib();
//module.exports = new kdtree_lib.KDTree();
var klib = require("kdtree");
module.exports = new klib.KDTree;
/*
> k = require("kdtree");
{ KDTree: [Function: KDTree] }
> a = new k.KDTree;
{}
> a
{}
> a = new k.KDTree();
{}
> a
{}
> a.test();
inserting 10 random vectors... range query returned 6 items
6
*/

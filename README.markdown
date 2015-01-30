[<img src="https://raw.githubusercontent.com/justinethier/node-kdtree/master/doc/node-kdtree-logo.png" alt="">](https://github.com/justinethier/node-kdtree) 
#node-kdtree

node-kdtree is a node.js addon that defines a wrapper to libkdtree, allowing one to work with KD trees directly in node. A [KD tree](http://en.wikipedia.org/wiki/Kd-tree) is a data structure that organizes points in a multi-dimensional space, and in particular is useful for performing efficient nearest neighbor searches.

## Dependencies
The [kdtree](http://code.google.com/p/kdtree/) C library is required. In order to install, get the latest version from [here](http://code.google.com/p/kdtree/downloads/list) and run the following commands:

    ./configure
    make
    sudo make install PREFIX=/usr 

## Installation
The easiest way to install node-kdtree is to use the [npm](https://github.com/isaacs/npm) package manager:

    npm install kdtree

## Usage

###Creating a tree
You may create a tree by instantiating a new `KDTree` object:

    var kd = require('kdtree');
    var tree = new kd.KDTree(3); // A new tree for 3-dimensional points

When creating a new tree we can specify the dimensions of the data. For example, a three-dimensional tree will contain points of the form (x, y, z). If a dimension is not specified, the tree defaults to three dimensions.

###Adding data to a tree
Data may be added to the tree using the `insert` method:

    tree.insert(1, 2, 3);
    tree.insert(10, 20, 30);

There must be one argument for each dimension of the data - for example, a three dimensional tree would have three arguments to `insert`. An optional data parameter may also be specified to store a data value alongside the point data:

    tree.insert(39.285785, -76.610262, "USS Constellation");

###Nearest neighbor searches
The `nearest` method is used to find the point in the tree that is closest to a target point. For example:

    > tree.nearest(39.273889, -76.738056);
    [39.272051, -76.731917, "Bill's Music, Inc."]

`nearest` will return an array containing closest point, or an empty array if no points were found. As shown above, if the point contains a data value, that value will also be returned at the end of the array.


A `nearestRange` method is also provided, which allows us to find all of the points within a given range. For example:

    > tree.nearestRange(0, 0, 3);
    [ [ 1, 1 ],
      [ 0, 2 ],
      [ 2, 0 ],
      [ 1, 0 ],
      [ 0, 1 ],
      [ 0, 0 ] ]

The first arguments to `nearestRange` are the components of the point to begin searching at. The last argument is the search range.

##API

[API documentation](https://github.com/justinethier/node-kdtree/blob/master/doc/API.markdown)

##Credits

node-kdtree is developed by [Justin Ethier](http://github.com/justinethier).

Thanks to John Tsiombikas for developing libkdtree!

Patches are welcome; please send via pull request on github.

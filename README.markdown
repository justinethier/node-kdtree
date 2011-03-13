node-kdtree
====================================

## DESCRIPTION
node-kdtree is a node.js Addon that defines a wrapper to libkdtree, allowing one to work with KD trees directly in node. A [KD tree](http://en.wikipedia.org/wiki/Kd-tree) is may be used to organize points in a multi-dimensional space, and in particular is useful for performing nearest neighbor searches.

## Dependencies
The [kdtree](http://code.google.com/p/kdtree/) library is required. In order to install, get the latest version from [here](http://code.google.com/p/kdtree/downloads/list) and run the following commands:

   ./configure
   make
   sudo make install PREFIX=/usr 


##dimensions
Get the dimensions of the tree.

    var dimensions = tree.dimensions();

##insert
Add a point to the tree. A value may optionally be associated with the point.

    tree.insert( p1, p2, ..., value);

##nearest
Find the nearest point in the tree.
Returns the point and an associated value.

    var n = tree.nearest( p1, p2, ...);

##nearestPoint
Find the nearest point in the tree, and return only that point.

    var point = tree.nearest( p1, p2, ...);

##nearestValue
Find the nearest point in the tree, and only return its associated value.

    var value = tree.nearest( p1, p2, ...);

##nearestRange
Find all points within the tree within a particular range of the given point.

    var results = tree.nearest( p1, p2, ..., range);


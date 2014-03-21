##dimensions
Get the dimensions of the tree.

    var dimensions = tree.dimensions();

##insert
Add a point to the tree. A value may optionally be associated with the point.

    tree.insert( p1, p2, ..., value);

##nearest
Find the nearest point in the tree.
Returns the point and an associated value, or an empty array if no point is found.

    var n = tree.nearest( p1, p2, ...);

##nearestPoint
Find the nearest point in the tree, and return an array containing only the coordinates of that point.
An empty array is returned if no point is found.

    var point = tree.nearestPoint( p1, p2, ...);

##nearestValue
Find the nearest point in the tree, and return its associated value.
If a point is not found or if the found point does not have an associated value, this method returns null.

    var value = tree.nearestValue( p1, p2, ...);

##nearestRange
Find all points within the tree within a particular range of the given point.
Returns an array which contains sub-arrays. Each sub array contains the coordinates of the found point as well as any associated data value.

    var results = tree.nearestRange( p1, p2, ..., range);


# quadtree
quadtree, with shape classes {rectangle, circle}, overlap between shapes can be detected, auto extend, auto shrink, auto deleted

## todo
 - implement remove entry function
 - nodes will have cumulative entry count
 - when remove entry, node whose entry count is lower than capacity/2 will shrink (cancel split)
 - support overlap between entries

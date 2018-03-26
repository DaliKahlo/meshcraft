//$c1   XRL 07/24/2012 Created 
//========================================================================//
//
// FindNodeClass class implementation 
//
//=========================================================================

#include "FindNodeVistor.h"

FindNodeVisitor::FindNodeVisitor(const std::string &searchName)
{
	setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
	searchForName.assign(searchName);
}

void FindNodeVisitor::setNameToFind(const std::string &searchName)
{
	searchForName.assign(searchName);
}


void FindNodeVisitor::apply(osg::Node& node)
{
	const std::string curName = node.getName();

	if (curName.compare(0, searchForName.length(), searchForName) == 0) {
		foundNodeList.push_back(&node);
	}

	traverse( node );
}

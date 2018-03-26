//$c1   XRL 02/01/2012 Created 
//========================================================================//
//
// instance.h
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_INSTANCE_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define AFX_INSTANCE_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include <osg/Matrix>

class Instance  
{
public:
	Instance(int,int);
	virtual ~Instance();

	osg::Matrix * transform() { return &m_transf; }
	void setTransform(double,double,double,double,
					  double,double,double,double,
					  double,double,double,double,
					  double,double,double,double);
	void postMult(osg::Matrix &);
	bool isIdentity();

	int body_index() { return m_bodyIndex; }
	int occurence() { return m_occurence; }

private:

	osg::Matrix m_transf;
	bool m_identity;		// true if it is an identical transf
	int m_occurence;		// we use this as identifier, starting from 1
	int m_bodyIndex;		// 
};

#endif

//$c1   XRL 03/11/2014 Created 
//========================================================================//
//
// Post processing implementation 
//
//=========================================================================
#include "stdafx.h"
#include "Post.h"
#include "util.h"
#include <assert.h>
#include <sstream>
#include <iomanip>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/LightModel>
#include <osgText/Font>

#define NOT_EXIST 1.0e14

extern osg::ref_ptr<osgText::Font> g_font;

cPOST::cPOST()
{
	attached =  false;
	m_colorlegend = NULL;
	m_postsw = NULL;
	minVal = 0.0;
	maxVal = 1.0;
}

cPOST::~cPOST()
{
	// need clean up m_colormap, m_lineplot and m_colorlegend ???
	// detach them from their parents, and set to NULL
	assert(!attached);
}

void cPOST::clear(osg::Group *p, osg::Camera *hud)
{
	detachParent(p, hud);
	if(m_postsw.valid()) {
		int nl = m_postsw->getNumChildren();
		m_postsw->removeChildren(0,nl-1); 
		m_postsw = NULL;
	}
	minVal = 0.0;
	maxVal = 1.0;
}


void cPOST::attachParent(osg::Group *p, osg::Camera *hud, float winWidth, float winHeight)
{
	if(!attached) {
		p->addChild(m_postsw);
		drawHUDColorLegend(hud, winWidth, winHeight);
		attached = true;
	}
}

void cPOST::detachParent(osg::Group *p, osg::Camera *hud)
{
	if( attached ) {
		p->removeChild(m_postsw);
		clearHUDColorLegend(hud);
		attached = false;
	}
}

void cPOST::draw_colormap(int nv, int nf, double *xyz, int *tria, double *values)
{
	///// always clear line_plot when redraw color map (colormap is drawn earlier)
	if(m_postsw.valid()) {
		int nl = m_postsw->getNumChildren();
		m_postsw->removeChildren(0,nl-1); 
	}

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> colormap = new osg::Geometry;

	////// create vertex and normal array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(3*nf);
	colormap->setVertexArray( v.get() );

	////// create normal array
	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(3*nf); 
	colormap->setNormalArray(norms.get());
	colormap->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	////// set colors 
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(3*nf);
	colormap->setColorArray(colors.get());
	colormap->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	// set normals, vertices and primitives
	osg::Vec3 norVec;
	osg::Vec4 color;
	int i, j, k, count = 0;
	double coord[3][3], nor[3];

	for(i=0; i<nf; i++) {
		for(j=0; j<3; j++) {
			k = tria[3*i+j] - 1;
			coord[j][0] = xyz[3*k];
			coord[j][1] = xyz[3*k+1];
			coord[j][2] = xyz[3*k+2];
		}
		if(norm(coord[1], coord[2], coord[0], nor) == false)
			continue;

		norVec.set(nor[0],nor[1],nor[2]);
		computeValueColor(values[2*i+1], color);

		for(j=0; j<3; j++) {
			(*v)[count].set(coord[j][0],coord[j][1],coord[j][2]); 
			(*norms)[count] = norVec;
			(*colors)[count] = color;
			count++;
		}
		colormap->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
	}

	int id = 1;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->setName("ColorMap_" + std::to_string((long long)id));
	geode->addDrawable( colormap );

	osg::LightModel* ltModel = new osg::LightModel; 
	osg::StateSet* st = geode->getOrCreateStateSet(); 
	ltModel->setTwoSided(true); 
	st->setAttribute(ltModel); 

	if(!m_postsw.valid()) {
		m_postsw = new osg::Switch;
		m_postsw->setName("PostSW");
	}

	m_postsw->addChild(geode);
	//m_colormap->setAllChildrenOn();
	//m_lineplot->setAllChildrenOff();
}


void cPOST::draw_lineplot(int np, double *xyz, double *proj)
{
	double minPt[3], maxPt[3], len, tmp;
	int i;
	boundingbox(np,xyz,minPt,maxPt);
	len = maxPt[0] - minPt[0];
	for(i=1; i<3; i++) {
		tmp = maxPt[i] - minPt[i];
		if(tmp > len)
			len = tmp;
	}
	tmp = 0.05*len / (maxVal - minVal);

	int id = 1;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->setName("LinePlot_" + std::to_string((long long)id));
    osg::LineWidth* lw = new osg::LineWidth( 1. );
    geode->getOrCreateStateSet()->setAttributeAndModes( lw, osg::StateAttribute::ON );

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> barGeom = new osg::Geometry;

	////// create vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(np*2);
	barGeom->setVertexArray( v.get() );

	//// create color array
	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array(np*2);
	barGeom->setColorArray(c);
	barGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	///// fill vertex array
	osg::Vec4 color;
	double v0[3], v1[3];
	for(i=0; i<np; i++) {
		computeValueColor(proj[4*i+3], color);

		v0[0] = proj[4*i];
		v0[1] = proj[4*i+1];
		v0[2] = proj[4*i+2];
		v1[0] = (xyz[3*i] - v0[0]) * tmp + v0[0];
		v1[1] = (xyz[3*i+1] - v0[1]) * tmp + v0[1];
		v1[2] = (xyz[3*i+2] - v0[2]) * tmp + v0[2];
		
		(*v)[2*i].set(v0[0],v0[1],v0[2]);
		(*c)[2*i] = color;
		(*v)[2*i + 1].set(v1[0],v1[1],v1[2]);
		(*c)[2*i + 1] = color;
	}

	// create and add a DrawArray line primitives
	osg::DrawElementsUInt* lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
	for(i=0; i<np; i++) {
		lines->push_back(2*i);
		lines->push_back(2*i+1);
	}
	barGeom->addPrimitiveSet(lines);

	barGeom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(1.0f));
	barGeom->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

	geode->addDrawable( barGeom );

	// the color map geode node is always already attached
	m_postsw->addChild(geode);
	m_postsw->setValue(0, false);
	m_postsw->setValue(1, true);
	return;
}

int cPOST::showcolormap()
{
	if(!m_postsw.valid())
		return 0;
	int nc = m_postsw->getNumChildren();
	if(nc == 0)
		return 0;
	m_postsw->setAllChildrenOff();
	m_postsw->setValue(0,true);
	return 1;
}

int cPOST::showlineplot()
{
	if(!m_postsw.valid())
		return 0;
	int nc = m_postsw->getNumChildren();
	if(nc <= 1)
		return 0;
	m_postsw->setAllChildrenOff();
	m_postsw->setValue(1,true);
	return 1;
}

/////////////////////////////////////////////////////
//
//  ColorLegend (i.e. SosgSim calarBar)
//
////////////////////////////////////////////////////
void cPOST::clearHUDColorLegend(osg::Camera *hud)
{
	if( !m_colorlegend.valid() )
		return;

	int num = m_colorlegend->getNumDrawables();
	if( num>0 ) {
		m_colorlegend->removeDrawables(0,num);
	}
	hud->removeChild(m_colorlegend);
	m_colorlegend = NULL;
}

void cPOST::drawHUDColorLegend(osg::Camera *hud, float winWidth, float winHeight)
{
	if( !m_colorlegend.valid() ) {
		m_colorlegend = new osg::Geode;
		hud->addChild( m_colorlegend.get() );
	}

	// 
	static const  osg::Vec4 red = osg::Vec4 ( 1.0f, 0.0f, 0.0f, 1.0f );
	static const  osg::Vec4 green = osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f );
	static const  osg::Vec4 blue = osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f );
	static const  osg::Vec4 yellow = osg::Vec4 ( 1.0f,1.0f,0.0f,1.0f );
	static const  osg::Vec4 cyan = osg::Vec4( 0.0f, 1.0f, 1.0f, 1.0f );
	static const  osg::Vec4 purple = osg::Vec4 ( 1.0f,0.0f,1.0f,1.0f );

	float xmin = 0.93f* winWidth; 
	float xmax = 0.95f* winWidth;
	float ymin = 0.65f * winHeight; 
	float ymax = 0.98f * winHeight;

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;

	int i, k;
	float y=ymin, dy = (ymax - ymin) / 4.0f;
	for(i=0; i<5; i++) {
		v->push_back(osg::Vec3(xmin,y,0));
		v->push_back(osg::Vec3(xmax,y,0));
		y += dy;
	}

	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array; 
	norms->push_back( osg::Vec3(0.0f,0.0f,1.0f) );

	osg::ref_ptr<osg::Geometry> colorLegend = new osg::Geometry;
	colorLegend->setVertexArray( v.get() );
	colorLegend->setNormalArray( norms.get() );
	colorLegend->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
	c->push_back( blue );	
	c->push_back( blue );	
	c->push_back( cyan );	
	c->push_back( cyan );
	c->push_back( green );	
	c->push_back( green );	
	c->push_back( yellow );	
	c->push_back( yellow );
	c->push_back( red );	
	c->push_back( red );
	colorLegend->setColorArray(c.get());
	colorLegend->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
	k = 0;
	for(i=0; i<4; i++) {
		quads->push_back(k);
		quads->push_back(k+1);
		quads->push_back(k+3);
		quads->push_back(k+2);
		k += 2;
	}
	colorLegend->addPrimitiveSet(quads);
	
	m_colorlegend->addDrawable( colorLegend.get() );

	// add texts
	osg::Vec3 pos;
	float ts = 0.008*(winWidth - 1280) + 10.0f;
	float dx = 0.001*winWidth;
	double dv = (maxVal - minVal) / 4.0;
	double val = minVal;
	y = ymin; 
	for(i=0; i<5; i++) {
		std::stringstream oss;
		pos.set(osg::Vec3(xmax+dx,y,0));
		if(val >= 0.0) 
			oss << std::setprecision(5) << " " << val;		
		else
			oss << std::setprecision(5) << val;
		m_colorlegend->addDrawable( createText(pos, oss.str(), ts, 2, osg::Vec4(0.0f,0.0f,0.0f,1.0f)) );
		y += dy;
		val += dv;
	}

	return;
}

void cPOST::setValueRangle(double minV, double maxV)
{
	minVal = minV;
	maxVal = maxV;
}

void cPOST::computeValueColor(double val, osg::Vec4 &c)
{
	float red, green, blue;

	if(val > NOT_EXIST-1.0) {
		c.set( FCOLOR_R, FCOLOR_G, FCOLOR_B, 1.0f );  
		return;
	}

	double mean = 0.5*(minVal + maxVal);
	double upper_quartile = 0.5*(mean + maxVal);
	double lower_quartile = 0.5*(mean + minVal);

	upper_quartile = 0.5*(mean + upper_quartile);
	lower_quartile = 0.5*(mean + upper_quartile);

	if(val >= upper_quartile) {
		// red -> yellow: 
		// R: 1.0f  G: 0.0f -> 1.0f  B: 0.0f
		red = 1.0f;
		green = (float) ((maxVal - val) / (maxVal - upper_quartile));
		blue = 0.0f;
	} else if(val >= mean) {
		// yellow ->green, red changes from 1.0 to 0.0
		red = (float) ((val - mean)/(upper_quartile - mean));
		green =  1.0f;
		blue = 0.0f;
	} else if(val >= lower_quartile) {
		// green -> cyan
		red = 0.0f;
		green = 1.0f;
		blue = (float) ((mean - val) / (mean - lower_quartile));
	} else {
		// cyan ->blue
		red = 0.0f;
		green = (float) ((val - minVal)/(lower_quartile - minVal));
		blue = 1.0f;
	}
	c.set( red, green, blue, 1.0f );
}

osgText::Text* cPOST::createText(const osg::Vec3& pos, const std::string& content,
						         float size, int dir,  const osg::Vec4& color)
{
	osg::ref_ptr<osgText::Text> text = new osgText::Text;
	text->setDataVariance( osg::Object::DYNAMIC );
	text->setFont( g_font.get() );
	text->setCharacterSize( size );
	switch(dir) {
	case 0: text->setAxisAlignment( osgText::TextBase::YZ_PLANE ); break;
	case 1: text->setAxisAlignment( osgText::TextBase::XZ_PLANE ); break;
	case 2: text->setAxisAlignment( osgText::TextBase::XY_PLANE ); break;
	}
	text->setPosition( pos );
	text->setText( content );
	//text->setDrawMode( osgText::Text::TEXT | osgText::Text::BOUNDINGBOX );
#ifdef BLACK_WHITE_MODE
	text->setColor( osg::Vec4(1.0f-BKCOLOR_R, 1.0f-BKCOLOR_G, 1.0f-BKCOLOR_B, 1.0f) );
#else
	text->setColor( color );
#endif
	return text.release();
}


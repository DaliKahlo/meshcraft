//$c8   XRL 03/27/2014 annotatation (savePick, drawAnnotatationLine, annotate)
//$c7   XRL 02/25/2014 rect picking (rectangularPick, drawRectangularPick)
//$c6   XRL 10/31/2013 add PointIntersector and showPOINT 
//$c5   XRL 06/09/2013 account for clip plane
//$c4   XRL 11/08/2012 interrogate mesh database using indices of sg node/drawable/primitive
//$c3   XRL 09/21/2012 highlight picked entity
//$c2   XRL 09/20/2012 display what's picked using HUD text
//$c1   XRL 09/17/2012 Created referring osgkeyboardmouse.cpp
//========================================================================
//
// PickHandler.cpp
//
//========================================================================
#include "stdafx.h"
#include <osg/io_utils>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/PolytopeIntersector>
#include <osgViewer/Viewer>
#include <osg/Geometry>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "PickHandler.h"
#include "SelectionMgr.h"
#include "util.h"

#include "../MeshWorks.h"
#include "../MeshWorkDoc.h"
#include "../MeshWorkOsgView.h"

PickHandler::PickHandler(cOSG* p) :
  _mx(0.0), _my(0.0), _mx_normalized(0.0), _my_normalized(0.0), _mode(10), _mOSG(p)
{
	_annotate = 0;
	_last[0] = 0.0;
	_last[1] = 0.0;
	_last[2] = 0.0;
}

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if (!viewer) return false;

	switch(ea.getEventType())
	{
	case(osgGA::GUIEventAdapter::KEYUP):
		{
			if (ea.getKey()=='v')
			{ _mode=0; }               
			else if (ea.getKey()=='e')
			{ _mode=1; }
			else if (ea.getKey()=='f')
			{ _mode=2; }
			else if (ea.getKey()=='p')
			{ _mode=10; }

			return false;
		}
	case(osgGA::GUIEventAdapter::PUSH):
		{
			_mx = ea.getX();
			_my = ea.getY();
			_mx_normalized = ea.getXnormalized();
			_my_normalized = ea.getYnormalized();
			break;
		}
	case(osgGA::GUIEventAdapter::RELEASE):
		{
			if (_mx == ea.getX() && _my == ea.getY()) 
			{
				switch(_annotate) {
				case 0: 
				case 1:
					pick(ea,viewer);  break; // only do a pick if the mouse hasn't moved
				case 2: annotate(ea,viewer); break;
				}
			} 
			else if(ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
			{
				rectangularPick(ea,viewer);
			}
			return true;
		}  
	case(osgGA::GUIEventAdapter::DRAG):
		{
			if((ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL) && _mx != ea.getX() && _my != ea.getY()) {
				drawRectangularPick(ea);
			}
		}
	case(osgGA::GUIEventAdapter::MOVE):
		{
			if(_annotate == 2) {
				drawAnnotatationLine(ea, viewer);
			}
		}
	default:
		break;
	}
	return false;
}

void PickHandler::pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
{
    osg::Node* scene = viewer->getSceneData();
    if (!scene) return;

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();
	CMeshWorkDoc *pDoc = osgview->GetDocument();
	if(!pDoc) return;

	bool _useLineSegmentIntersector = true;
	SCENETYPE _sceneType = _mOSG->currentscene();

	switch(_sceneType) {
	case __COLORMAP: _useLineSegmentIntersector=true; break;
	case __LINEPLOT: _useLineSegmentIntersector=false; break;
	case __MESH: {
		if(_mode < 10) 
			_useLineSegmentIntersector=true;
		else
			_useLineSegmentIntersector=false;
		break;
				 }
	}

	if(_useLineSegmentIntersector) 
    {
        osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::PROJECTION, ea.getXnormalized(),ea.getYnormalized() );
		osgUtil::IntersectionVisitor iv(picker.get());
		iv.setTraversalMask( ~0x1 );				// match setNodeMask in InitCameraAndViewer
        viewer->getCamera()->accept(iv);

        if (picker->containsIntersections())
        {
			// get the closest visual intersection 
			osgUtil::LineSegmentIntersector::Intersection intersection;
			osg::Vec4 v;
			if( _mOSG->GetClipPlane(v) ) {
				bool visinterflag=false;
				osgUtil::LineSegmentIntersector::Intersections intersections = picker->getIntersections();
				osgUtil::LineSegmentIntersector::Intersections::iterator itrI;
				for(itrI = intersections.begin(); itrI != intersections.end(); itrI++) {
					osg::Vec3 point = itrI->getWorldIntersectPoint();
					double f = v[0]*point[0] + v[1]*point[1] + v[2]*point[2] + v[3];
					if(f > 0.0) {
						visinterflag = true;
						break;
					}
				}
				if(!visinterflag)
					return;
				intersection = *itrI;
			}
			else {
				 intersection = picker->getFirstIntersection();
			}

            //osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl;

			// do corresponding operations
			if(_annotate == 0) {
				switch(_mode) {
				case 0: showVERTEX(intersection); break;
				case 1: showEDGE(intersection); break;
				case 2: showFACE(intersection); break;
				}
			} else if(_annotate == 1) {
				if(savePick(intersection))
					_annotate ++;
			}
        }

    } else {  // point pick
		float bias = _mOSG->GetBSphereRadius() * 0.01;
		osg::ref_ptr<PointIntersector> intersector = new PointIntersector(osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
		intersector->setPickBias(bias);
		osgUtil::IntersectionVisitor iv( intersector.get() );
		iv.setTraversalMask( ~0x1 );
		viewer->getCamera()->accept( iv );
            
        if ( intersector->containsIntersections() )
        {
			double c[3];
			PointIntersector::Intersection result;
			PointIntersector::Intersections results = intersector->getIntersections();
			PointIntersector::Intersections::iterator itrP;

			double minDist = 1e10;
			int primitiveIndex = 0;
			for(itrP = results.begin(); itrP != results.end(); itrP++) {
				result = *itrP;
				if(minDist > result.ratio) {
					osg::Vec3d xyz = result.getWorldIntersectPoint();
					c[0] = xyz[0]; c[1] = xyz[1]; c[2] = xyz[2]; 
					minDist = result.ratio;
					primitiveIndex = result.primitiveIndex;
				}
			}
			if(minDist != 1e10) {
				if(_annotate == 0) {
					pDoc->getSelectionMgr()->appendPoint(c,0,0,0);
					pDoc->getSelectionMgr()->showPoints(_mOSG, bias);
				} else if(_annotate == 1) {
					double xyz[4][3], val;
					// primitiveIndex is not returned correctly. Use coordinates
					xyz[0][0] = c[0];
					xyz[0][1] = c[1];
					xyz[0][2] = c[2];
					// in return: xyz[0] is the given point cloud 
					//            xyz[1] theory values
					// annotation points to the theory (projection) location
					if(lookupLinePlot(primitiveIndex, xyz, &val)) {
						_last[0] = xyz[1][0];
						_last[1] = xyz[1][1];
						_last[2] = xyz[1][2];
						_data[0] = xyz[0][0];
						_data[1] = xyz[0][1];
						_data[2] = xyz[0][2];
						_data[3] = val;
						_annotate ++;
					}
				}
			}
        }
	}

}

void PickHandler::drawRectangularPick(const osgGA::GUIEventAdapter& ea)
{
	float currentX = ea.getX();
	float currentY = ea.getY();
	if(_mx == currentX || _my == currentY)
		return;

	// convert into WINDOW coordinate system (required as a point on the screen)  See page 244 chapt 9
	int w,h;
	float ts;
	_mOSG->GetScreenAndTextSize(&w, &h,&ts);

	float beginX = 0.5* (_mx_normalized+1.0) *(float)w;
	float beginY = 0.5* (_my_normalized+1.0) *(float)h;
	currentX = 0.5* (ea.getXnormalized()+1.0) *(float)w;
	currentY = 0.5* (ea.getYnormalized()+1.0) *(float)h;

	//osg::Viewport* viewport = _mOSG->GetViewport();
	//currentX = viewport->x() + (int)((double )viewport->width()*(ea.getXnormalized()*0.5+0.5));
	//currentY = viewport->y() + (int)((double )viewport->height()*(ea.getYnormalized()*0.5+0.5));

	//beginX = viewport->x() + (int)((double )viewport->width()*(_mx_normalized*0.5+0.5));
	//beginY = viewport->x() + (int)((double )viewport->width()*(_my_normalized*0.5+0.5));

	// draw a rectangular on HUD plane
	float xMin = MIN(beginX, currentX);
    float yMin = MIN(beginY, currentY);
    float xMax = MAX(beginX, currentX);
    float yMax = MAX(beginY, currentY);

	//TRACE("draw: X=%f ~ %f, Y=%f ~ %f  (%f,%f)\n",xMin, xMax, yMin, yMax, x, y);
	_mOSG->clearHudDrawing();
	_mOSG->drawHUDRect(xMin, xMax, yMin, yMax);
}

void PickHandler::rectangularPick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
{
	float currentX = ea.getXnormalized();
	float currentY = ea.getYnormalized();

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();
	CMeshWorkDoc *pDoc = osgview->GetDocument();
	if(!pDoc) return;

	float xMin = MIN(_mx_normalized, currentX);
    float yMin = MIN(_my_normalized, currentY);
    float xMax = MAX(_mx_normalized, currentX);
    float yMax = MAX(_my_normalized, currentY);

	//TRACE("Pick: X=%f ~ %f, Y=%f ~ %f\n",xMin, xMax, yMin, yMax);

    osgUtil::PolytopeIntersector* recPicker = new osgUtil::PolytopeIntersector(osgUtil::PolytopeIntersector::PROJECTION,
                                                                               xMin, yMin, xMax, yMax);
    osgUtil::IntersectionVisitor iv(recPicker);
    viewer->getCamera()->accept(iv); 

    if(recPicker->containsIntersections())
    {
		double c[3];
		osg::Node* node = 0;
		osg::Group* grandparent = 0;
        // iterate over all intersections to get all intersection data
		osgUtil::PolytopeIntersector::Intersections intersections = recPicker->getIntersections();
		osgUtil::PolytopeIntersector::Intersections::iterator itrI;
		//TRACE("Intersections: %d\n",intersections.size());
		for(itrI = intersections.begin(); itrI != intersections.end(); itrI++) {
			if(itrI->numIntersectionPoints == 1) {
				// we deal with points only. osg polytope pick can produce huge number of strange data
				osg::notify(osg::NOTICE)<<"Picked "<<itrI->localIntersectionPoint<<std::endl
							<<"  Distance to ref. plane "<<itrI->distance
							<<", max. dist "<<itrI->maxDistance
							<<", primitive index "<<itrI->primitiveIndex
							<<", numIntersectionPoints "<<itrI->numIntersectionPoints
							<<std::endl;
	
				osg::NodePath nodePath = itrI->nodePath;
				node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
				grandparent = (nodePath.size()>=4)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-4]):0;

				int part, occ, fcID;
				sceneNameToAssmID(grandparent,node,itrI->drawable,&part,&occ,&fcID);

				osg::Vec3 worldxyz = itrI->localIntersectionPoint * osg::computeLocalToWorld(nodePath);
				c[0] = worldxyz[0]; 
				c[1] = worldxyz[1]; 
				c[2] = worldxyz[2]; 

				pDoc->getSelectionMgr()->appendPoint(c,occ,fcID,itrI->primitiveIndex);
			}
		}
		pDoc->getSelectionMgr()->showPoints(_mOSG, 0.01*_mOSG->GetBSphereRadius());

    }

	_mOSG->clearHudDrawing();
    return;

}

void PickHandler::showPOINT( int i, PointIntersector::Intersection& result )
{
	osg::Vec3d xyz = result.getWorldIntersectPoint();
}

void PickHandler::showVERTEX( osgUtil::LineSegmentIntersector::Intersection& intersection )
{
}

void PickHandler::showEDGE( osgUtil::LineSegmentIntersector::Intersection& intersection )
{
    osg::Node* node = 0;
    osg::Group* parent = 0;

	osg::NodePath& nodePath = intersection.nodePath;
    node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
    parent = (nodePath.size()>=4)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-4]):0;

	if (node) {
		std::string dname = intersection.drawable->getName();
		std::string nname = node->getName();
		osg::Vec3 point = intersection.getWorldIntersectPoint();
				
		if( nname.size() > 5 && dname.size() > 5) {
			double fxyz[4][3];
			int ids[6];
			bool found = lookupDatabase(parent, node, intersection.drawable, intersection.primitiveIndex, ids, fxyz);

			double pxyz[3], dsq[3], dsqmin;
			int i, j, k;
			pxyz[0] = point.x(); pxyz[1] = point.y(); pxyz[2] = point.z(); 
			dsq[0] = P_sqrDistToSegment(fxyz[0],fxyz[1],pxyz);
			dsq[1] = P_sqrDistToSegment(fxyz[0],fxyz[2],pxyz);
			dsq[2] = P_sqrDistToSegment(fxyz[1],fxyz[2],pxyz);
			i=0; dsqmin = dsq[0]; 
			if(dsq[1] < dsqmin) { i=1; dsqmin=dsq[1]; }
			if(dsq[2] < dsqmin) { i=2; dsqmin=dsq[2]; }

			switch(i) {
			case 0: j = 0; k = 1; break; 
			case 1: j = 2; k = 0; break; 
			case 2: j = 1; k = 2; break; 
			}

			int w,h, dh;
			float x,y,ts;
			_mOSG->GetScreenAndTextSize(&w, &h, &ts);
			_mOSG->clearHudText();
			dh = ts+3.0f;
			x = 0.004*w;
			if(found) {
				y = h-dh;	
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Edge " + std::to_string((long long)ids[j+2]) + " " + std::to_string((long long)ids[k+2]));
				
				std::stringstream oss;
				oss << std::setprecision(7) << "(" << fxyz[j][0] << "  " << fxyz[j][1] << "  " << fxyz[j][2] << ")\n";	
				oss << std::setprecision(7) << "(" << fxyz[k][0] << "  " << fxyz[k][1] << "  " << fxyz[k][2] << ")";	
				y -= dh;
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());

				double angle;
				std::string str = nname.substr(5);
				int occurance = atoi( str.c_str() );	
				int nf = _mOSG->crossEdgeAngle(occurance,ids[j+2],ids[k+2],&angle);

				std::stringstream ossl;
				double vec[3];
				VectorSubtract(fxyz[j],fxyz[k],vec);
				ossl << std::setprecision(7) << "Length:" << sqrt( VectorDot(vec,vec) );
				y -= dh;
				y -= dh;
				if(nf == 2) {
					ossl << std::setprecision(7) << "\nNormal change:" << angle;
				} else {
					ossl << "\nNeighboring faces:" << nf;
				}
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f), ossl.str());

				_mOSG->clearHighlight();
				double radius = 0.12*sqrt(max(max(dsq[0],dsq[1]),dsq[2]));
				_mOSG->highlightP(fxyz[j],radius,0);
				_mOSG->highlightP(fxyz[k],radius,0);
			}
			_mOSG->Render(_mOSG);
		}
	} 
    return;
}

void PickHandler::showFACE( osgUtil::LineSegmentIntersector::Intersection& intersection )
{
    osg::Node* node = 0;
    osg::Group* parent = 0;

	osg::NodePath& nodePath = intersection.nodePath;
    node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
    parent = (nodePath.size()>=4)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-4]):0;

	CString message;
	if (node) {
		std::string dname = intersection.drawable->getName();
		std::string nname = node->getName();
				
		if( nname.size() > 5 && dname.size() > 5) {
		double fxyz[4][3];
		int ids[6];
		bool found = lookupDatabase(parent, node, intersection.drawable, intersection.primitiveIndex, ids, fxyz);

		int w,h;
		float x,y,ts, dh;
		_mOSG->GetScreenAndTextSize(&w, &h, &ts);
		_mOSG->clearHudText();
		dh = ts + 3.0f;
		osg::notify(osg::NOTICE)<<"width="<<w<<"    height"<<h<<"   dh="<<dh<<std::endl;
		x = 0.004*w;
		y = h-dh;		_mOSG->addHudText(osg::Vec3(x, y, 0.0f), nname); 
		y -= dh;		_mOSG->addHudText(osg::Vec3(x, y, 0.0f), dname);
		if(found) 
		{ y -= dh;		_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Element " + std::to_string((long long)ids[0])); }
		else 
		{ y -= dh;		_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Primitive " + std::to_string((long long)intersection.primitiveIndex)); }
		if( intersection.indexList.size() == 3 ) {
			y -= dh;
			if(found) {
				if(ids[1] == 3) {
					_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Node " + std::to_string((long long)ids[2])
																+ " " + std::to_string((long long)ids[3])
																+ " " + std::to_string((long long)ids[4]) );
				} else if(ids[1] == 4) {
					_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Node " + std::to_string((long long)ids[2])
																+ " " + std::to_string((long long)ids[3])
																+ " " + std::to_string((long long)ids[4])
																+ " " + std::to_string((long long)ids[5]) );
				}
						
				std::stringstream oss;
				for( int i=0; i<ids[1]; i++ ) {
					oss << std::setprecision(7) << "(" << fxyz[i][0] << "  " << fxyz[i][1] << "  " << fxyz[i][2] << ")\n";	
				}
				y -= dh;
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());

				double vec[3], l12, l13, l23, shape=0.0;
				std::stringstream ossl;
				VectorSubtract(fxyz[0],fxyz[1],vec);
				l12 = sqrt( VectorDot(vec,vec) );
				VectorSubtract(fxyz[0],fxyz[2],vec);
				l13 = sqrt( VectorDot(vec,vec) );
				VectorSubtract(fxyz[1],fxyz[2],vec);
				l23 = sqrt( VectorDot(vec,vec) );

				fMeanRatio(fxyz,&shape);

				ossl << std::setprecision(6) << "Length: "<< l12 << " " << l13 << " " << l23 <<"\n";
				ossl << std::setprecision(6) << "Shape:  "<< shape;
				y -= ids[1]*dh;
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f), ossl.str());

				_mOSG->clearHighlight();
				_mOSG->highlight(ids[1],fxyz);

			} else {  // not found, it's a face of tets
				_mOSG->addHudText(osg::Vec3(x, y, 0.0f),"Node " + std::to_string((long long)intersection.indexList[0])
															+ " " + std::to_string((long long)intersection.indexList[1])
															+ " " + std::to_string((long long)intersection.indexList[2]) );

				osg::ref_ptr<osg::Geometry> geom = intersection.drawable->asGeometry();
				if( geom ) {	
					int i;
	
					osg::Vec3 v[3];
					osg::ref_ptr<osg::Vec3Array> varray = (osg::Vec3Array *)geom->getVertexArray();
					int dim = varray->getDataSize();
					for( i=0; i<dim; i++ ) {
						std::stringstream oss;
						v[i] = varray->at( intersection.indexList[i] );
						oss << std::setprecision(6) << "(" << v[i][0] << " " << v[i][1] << " " << v[i][2] << ")";
						y -= dh;
						_mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());
					}
					for(i=0; i<3; i++) {
						fxyz[i][0] = v[i].x();
						fxyz[i][1] = v[i].y();
						fxyz[i][2] = v[i].z();
					}
					_mOSG->clearHighlight();
					_mOSG->highlight(3,fxyz);
				}
			}
		}
		_mOSG->Render(_mOSG);

		//message.Format("Picked (%f, %f, %f)\n\
		//			   drawable type: %s\n\
		//			   drawable name: %s\n\
		//			   primitive index: %d\n\
		//			   vertex index: (%d, %d, %d) \n\
		//			   node path size: %d \n\
		//			   node type: %s \n\
		//			   node name: %s",
		//				intersection.localIntersectionPoint.x(),
		//				intersection.localIntersectionPoint.y(),
		//				intersection.localIntersectionPoint.z(),
		//				intersection.drawable->className(),
		//				dname.c_str(),
		//				intersection.primitiveIndex,
		//				intersection.indexList[0], 
		//				intersection.indexList[1], 
		//				intersection.indexList[2],
		//				nodePath.size(), 
		//				node->className(),
		//				nname.c_str());
		}
		else
		{
			osg::notify(osg::NOTICE) << "node name: " << nname << std::endl;
			osg::notify(osg::NOTICE) << "drawable name: " << nname << std::endl;
		}
	} 
	//AfxMessageBox(message);
}

// return 1 if picking succeeds and position is saved
//        0 if picking fails
int PickHandler::savePick( osgUtil::LineSegmentIntersector::Intersection& intersection )
{
    osg::Node* node = 0;
    osg::Group* parent = 0;

	osg::NodePath& nodePath = intersection.nodePath;
    node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
    parent = (nodePath.size()>=4)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-4]):0;

	int ret = 0;
	if (node) {
		std::string dname = intersection.drawable->getName();
		std::string nname = node->getName();
		
		double val, fxyz[4][3];
		bool found = false;

		if( nname == "ColorMap_1" ) {
			found = lookupColorMap(intersection.primitiveIndex, fxyz, &val);
		}
		else if( nname == "LinePlot_1") {
			printf("should not get here.\n");
			found = lookupLinePlot(intersection.primitiveIndex, fxyz, &val);
		}

		// colormap always picking face
		// line plot always picking vertex or point
		// graphic table with green for passing red for fail

		if(found) {
			//switch(_mode) {
			//case 0: {  // picked a vertex
			//		}
			//case 1: {  // picked an edge
			//	double pxyz[3], dsq[3], dsqmin;
			//	int i, j, k;
			//	osg::Vec3 point = intersection.getWorldIntersectPoint();
			//	pxyz[0] = point.x(); pxyz[1] = point.y(); pxyz[2] = point.z(); 
			//	dsq[0] = P_sqrDistToSegment(fxyz[0],fxyz[1],pxyz);
			//	dsq[1] = P_sqrDistToSegment(fxyz[0],fxyz[2],pxyz);
			//	dsq[2] = P_sqrDistToSegment(fxyz[1],fxyz[2],pxyz);
			//	i=0; dsqmin = dsq[0]; 
			//	if(dsq[1] < dsqmin) { i=1; dsqmin=dsq[1]; }
			//	if(dsq[2] < dsqmin) { i=2; dsqmin=dsq[2]; }

			//	switch(i) {
			//	case 0: j = 0; k = 1; break; 
			//	case 1: j = 2; k = 0; break; 
			//	case 2: j = 1; k = 2; break; 
			//	}

			//	_last[0] = 0.5 * (fxyz[j][0] + fxyz[k][0]);
			//	_last[1] = 0.5 * (fxyz[j][1] + fxyz[k][1]);
			//	_last[2] = 0.5 * (fxyz[j][2] + fxyz[k][2]);
			//	ret = 1;
			//	break;
			//		}
			//case 2: {  // picked a face
				int i;
				for( i=0; i<3; i++ )	_last[i] = 0.0f;
				for( i=0; i<3; i++ ) {
					_last[0] += fxyz[i][0];
					_last[1] += fxyz[i][1];
					_last[2] += fxyz[i][2];
				}
				for( i=0; i<3; i++ )	_last[i] /= (3.0f);
				_data[0] = val;
				ret = 1;
			//	break;
			//		}
			//}
		}
	} 
	return ret;
}

void PickHandler::drawAnnotatationLine(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
{
	// convert into WINDOW coordinate system (required as a point on the screen)  See page 244 chapt 9
	int w,h;
	float ts;
	_mOSG->GetScreenAndTextSize(&w, &h,&ts);

	float currentX = ea.getX();
	float currentY = ea.getY();
	currentX = 0.5* (ea.getXnormalized()+1.0) *(float)w;
	currentY = 0.5* (ea.getYnormalized()+1.0) *(float)h;
	
	// important! since drawn in hud viewport, can not use camera's viewport
	osg::Camera * camera = viewer->getCamera();
	osg::Matrix MVPW( camera->getViewMatrix() *
                      camera->getProjectionMatrix());// * 
                      //camera->getViewport()->computeWindowMatrix());
	osg::Vec3 posIn3D(_last[0],_last[1],_last[2]);
	osg::Vec3 posIn2D = posIn3D * MVPW;
	float winX = 0.5* (posIn2D.x() + 1.0) *(float)w;
	float winY = 0.5* (posIn2D.y() + 1.0) *(float)h;

	_mOSG->clearHudDrawing();
	_mOSG->drawHUDLine(winX, winY, currentX, currentY);
}

void PickHandler::annotate(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
{
	_annotate = 1;    // ready for next annotation
	_mOSG->clearHudDrawing();

	double xyz[64][3];
	xyz[0][0] = _last[0];
	xyz[0][1] = _last[1];
	xyz[0][2] = _last[2];

	float cx = ea.getX();
	float cy = ea.getY();

	osg::Camera * camera = viewer->getCamera();
	osg::Matrix MVPW( camera->getViewMatrix() *
                      camera->getProjectionMatrix() *
                      camera->getViewport()->computeWindowMatrix());
	osg::Matrixd inverseMVPW = osg::Matrixd::inverse(MVPW);

	//// draw line
    osg::Vec3 near_point = osg::Vec3(cx,cy,0.0f) * inverseMVPW;
    osg::Vec3 far_point  = osg::Vec3(cx,cy,1.0f) * inverseMVPW;

	double t, segm[2][3];
	segm[0][0] = near_point.x();
	segm[0][1] = near_point.y();
	segm[0][2] = near_point.z();
	segm[1][0] = far_point.x();
	segm[1][1] = far_point.y();
	segm[1][2] = far_point.z();

	t = ParOnLinearEdge(segm,_last);
	for(int i=0; i<3; i++) 
		xyz[1][i] = segm[0][i] + t * (segm[1][i] - segm[0][i]);

	_mOSG->highlightPolyLine(2,xyz,0.5,osg::Vec4(0.0f,0.0f,0.0f,1.0));

	//// text
	std::stringstream oss;
	osg::Vec3 pos(xyz[1][0], xyz[1][1], xyz[1][2]);

	if(_mOSG->currentscene() == __COLORMAP) {
		oss << std::setprecision(3) << _T("  Error: ") << _data[0] << _T("  \n");
		oss << std::setprecision(3) << _T("  X:  ") << _last[0] << _T("  \n");
		oss << std::setprecision(3) << _T("  Y:  ") << _last[1] << _T("  \n");
		oss << std::setprecision(3) << _T("  Z:  ") << _last[2] << _T("  \n");
	} else {
		oss << std::setprecision(3) << _T("    Theory  Measure  Error  \n");
		oss << std::setprecision(3) << _T(" X  ") << _last[0]  << "  " << _data[0] << "  " << _last[0] - _data[0] << _T("  \n");
		oss << std::setprecision(3) << _T(" Y  ") << _last[1]  << "  " << _data[1] << "  " << _last[1] - _data[1] << _T("  \n");
		oss << std::setprecision(3) << _T(" Z  ") << _last[2]  << "  " << _data[2] << "  " << _last[2] - _data[2] << _T("  \n");
		oss << std::setprecision(3) << _T(" Total                         ") <<_data[3] << _T(" \n");
	}

	//// compute alignement
	int w,h;
	float ts;
	_mOSG->GetScreenAndTextSize(&w, &h,&ts);
	osg::Matrix MVPW2( camera->getViewMatrix() *
                      camera->getProjectionMatrix());
	osg::Vec3 posIn3D(_last[0],_last[1],_last[2]);
	osg::Vec3 posIn2D = posIn3D * MVPW2;
	float lastX = 0.5* (posIn2D.x() + 1.0) *(float)w;
	float lastY = 0.5* (posIn2D.y() + 1.0) *(float)h;

	if(lastX < cx && lastY < cy)
		_mOSG->addText(pos,oss.str(),osgText::TextBase::LEFT_BOTTOM);
	else if(lastX > cx && lastY > cy)
		_mOSG->addText(pos,oss.str(),osgText::TextBase::RIGHT_TOP);
	else if(lastX < cx && lastY > cy)
		_mOSG->addText(pos,oss.str(),osgText::TextBase::LEFT_TOP);
	else if(lastX > cx && lastY < cy)
		_mOSG->addText(pos,oss.str(),osgText::TextBase::RIGHT_BOTTOM);
		
}


// input:	dname -> body id
//			nnmae -> face if
//			primitiveIndex -> the ith tri or quad in the face container
// return:  ids[0]  the id of the tri or quad
//			ids[1]  the number of vertices, 3 or 4
//			ids[2-5] the ids of the bounding verices
bool PickHandler::lookupDatabase(osg::Group* grandpa, osg::Node* node, osg::Drawable* drawable, int primitiveIndex, int ids[6], double xyz[4][3])
{
	int part,occurance,face_id;
	sceneNameToAssmID(grandpa,node,drawable,&part,&occurance,&face_id);

	return _mOSG->findMeshEntityIDs(occurance, face_id, primitiveIndex, ids, xyz);
}
// the rules to associate graphics primitive with database are defined in scenefraphfactory.cpp
//			1. ddname is named in terms of occurance   <---- we have bugs now for assembly
//			2. nname is named in terms of permanent face id (not the temp id) or face_null
// (CSGFactory::createSwitchAndFaceGeode)
void PickHandler::sceneNameToAssmID(osg::Group* grandpa, osg::Node* node, osg::Drawable* drawable, int *part, int *occ, int *fcID)
{
	std::string drawablename = drawable->getName();
	std::string nodename = node->getName();
	std::string str = nodename.substr(5);
	*occ = atoi( str.c_str() );	

	*fcID = 0;
	str = drawablename.substr(5);
	if( drawablename != "Face_null" && 
		drawablename != "BEdges" && 
		drawablename != "MoVertices" && 
		drawablename != "PtCloud")
		*fcID = atoi( str.c_str() ); 

	*part = 0;
	if(grandpa) {
		std::string graphpaname = grandpa->getName();
		str = graphpaname.substr(5);
		*part = atoi( str.c_str() ); 
	}
}

bool PickHandler::lookupColorMap(int primitiveIndex, double xyz[4][3], double *value)
{
	return _mOSG->loopupPostData(FACE_SCALAR2, primitiveIndex, xyz, value);
}

bool PickHandler::lookupLinePlot(int primitiveIndex, double xyz[2][3], double *value)
{
	return _mOSG->loopupPostData(VERTEX_VECTOR_SCALAR, primitiveIndex, xyz, value);
}

//To transform screen position to world position, use this code:
//
//     osg::Matrix MVPW( camera->getViewMatrix() *
//                       camera->getProjectionMatrix() *
//                       camera->getViewport()->computeWindowMatrix());
//
//     osg::Matrixd inverseMVPW = osg::Matrixd::inverse(MVPW);
//
//     osg::Vec3 near_point = osg::Vec3(x,y,0.0f) * inverseMVPW;
//     osg::Vec3 far_point  = osg::Vec3(x,y,1.0f) * inverseMVPW;
//
//Note that what you get is actually a line that goes from the near plane 
//to the far plane, since you're transforming 2D coordinates into 3D it 
//means that a point in 2D becomes a line in 3D.
//
//To transform a 3D position into 2D window coordinates you would just do 
//the opposite of that, using the same MVPW as above:
//
//     osg::Vec3 posIn2D = posIn3D * MVPW;
//
//Just make sure the posIn3D is in world space and not in some object's 
//local coordinate system.
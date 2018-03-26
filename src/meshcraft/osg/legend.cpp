//$c1   XRL 03/11/2014 Created 
//========================================================================//
//
// Post processing implementation 
//
//=========================================================================
#include "stdafx.h"

	MyHud::MyHud ( ) {
		hud = createHud ();
		// Title
		addNode ( (osg::Drawable*)(createTextNode("Pathway:", osg::Vec3( 5.0f, 975.0f, 0.0f )).get()) );
		pathway = createTextNode("", osg::Vec3(250.0f, 975.0f, 0.0f ));
		addNode ( (osg::Drawable*)pathway.get() );
				
		addNode ( (osg::Drawable*)(createTextNode ( 
			"Node ID: ", osg::Vec3 (5.0f, 950.0f, 0.0f ) ).get()) );
		id = createTextNode("", osg::Vec3(250.0f, 950.0f, 0.0f ));
		addNode ( (osg::Drawable*)id.get() );
		
		addNode ( (osg::Drawable*)(createTextNode ( 
			"Name: ", osg::Vec3 (5.0f, 925.0f, 0.0f ) ).get()) );
		name = createTextNode ( "", osg::Vec3( 250.0f, 925.0f, 0.0f ) );
		addNode ( (osg::Drawable*)name.get()  );


		addNode ( (osg::Drawable*)(createTextNode ( 
			"Node Location: ", osg::Vec3 (5.0f, 900.0f, 0.0f ) ).get()) );
		location = createTextNode("", osg::Vec3(250.0f, 900.0f, 0.0f ));
		addNode ( (osg::Drawable*)location.get() );
		

		// Initialize the legend
		osg::Vec3 position(10.0f,10.0f,0.0f);
		osg::Vec3 delta(0.0f, 35.0f,0.0f);
		createLegendEntry ( white, position, "Other" );
		position += delta;
		createLegendEntry ( green, position, "Cytosol" );
		position += delta;
		createLegendEntry ( yellow, position, "Nucleus" );
		position += delta;
		createLegendEntry ( red, position, "Plasma Membrane" );
		position += delta;
		createLegendEntry ( purple, position, "Extracellular" );
		position += delta;
		//createLegendEntry ( osg::Vec4(.2,1.0,1.0,1.0), position, "Selected Node" );
		//position += delta;
		addNode ( (osg::Drawable*)(createTextNode ( 
			"Nodes: ", position ).get()) );
		//position += delta;

		//createLegendEntry ( red, position, "Inhibitory" );
		//position += delta;
		//createLegendEntry ( green, position, "Excitatory" );
		//position += delta;
		//addNode ( (osg::Drawable*)(createTextNode ( 
		//	"Edges: ", position ).get()) );
	}
	
	osg::Node* MyHud::getHud () {
		return hud;
	}
	void MyHud::setName ( std::string namestr ) {
		name.get()->setText ( namestr );
	}
	
	void MyHud::setPathway ( std::string pathwaystr ) {
	    pathway.get()->setText ( pathwaystr );
	}

	void MyHud::setId ( std::string idstr ) {
		id.get()->setText ( idstr );
	}

	void MyHud::setLocation ( std::string locstr ) {
		location.get()->setText ( locstr );
	}

	

	osg::Node* MyHud::createHud()
	{
/*
        // This shows up on all 4 screens
		osg::Camera* hudCamera = new osg::Camera;
		hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hudCamera->setProjectionMatrixAsOrtho2D(0,1280,0,1024);
		hudCamera->setViewMatrix(osg::Matrix::identity());
		hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
		hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
        DOSG::getScene()->addChild ( hudCamera );   
        return hudCamera;
        */
        
    // Projection->MatrixTransform->Geode = Segmentation Fault?
	osg::Projection* _projection = new osg::Projection;
    _projection->setMatrix(osg::Matrix::ortho2D(0, 1280, 0, 1024));
    _projection->setNodeMask(1) ;
/*
	osg::Light* light = new osg::Light();
	osg::LightSource* lightSource = new osg::LightSource();
	light->setAmbient ( osg::Vec4d(1.0, 1.0, 1.0, 1.0) );
	light->setConstantAttenuation ( .05 );
    lightSource->setLight(light);

    lightSource->setLocalStateSetModes(osg::StateAttribute::ON);

    // get a stateset in the node
    osg::StateSet *stateSet = DOSG::getScene()->getOrCreateStateSet() ;
    lightSource->setStateSetModes(*stateSet,osg::StateAttribute::ON);


    osg::LightModel* lightModel = new osg::LightModel;
// get specular highlights to look correct across pipes
    lightModel->setLocalViewer(true) ;

    // turn on two sided lighting if not backface culling
    lightModel->setTwoSided(false) ;

    stateSet->setAttribute(lightModel);

    DOSG::getScene()->addChild(lightSource);
*/


    // create HUD text
    osg::MatrixTransform *modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());

	_projection->addChild(modelview_abs) ;  

	DOSG::getScene()->addChild(_projection) ;
 
    osg::Geode *geode = new osg::Geode ;
    modelview_abs->addChild(geode) ;

	return geode;

//return hudCamera;
    }

	void MyHud::addNode ( osg::Drawable* drawable ) {
//		osg::Geode* geode = new osg::Geode();
//		osg::StateSet* stateset = geode->getOrCreateStateSet();
//		stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
//		stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
//		geode->setName("The text label");
//		geode->addDrawable( drawable );
//		std::cout << "Adding child\n";
		((osg::Geode*)hud)->addDrawable(drawable);
				std::cout << "Child added\n";
	}

	osg::ref_ptr<osgText::Text> MyHud::createTextNode (std::string text, osg::Vec3 position) {
		osg::ref_ptr<osgText::Text> updateText = new osgText::Text;
		
		osg::ref_ptr<osgText::Font> f = osgText::readFontFile("fonts/VeraMono.ttf");

        updateText->setFont ( f.get() );
		
		updateText->setCharacterSize(28);
		updateText->setFontResolution(512,512);
		updateText->setPosition ( position );
	
		updateText->setBackdropType(osgText::Text::OUTLINE);
       	updateText->setColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        updateText->setBackdropColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
        updateText->setBackdropOffset(0.06f);	   
	
		updateText->setText(text);
		updateText->setDataVariance(osg::Object::DYNAMIC);
		return updateText;

	}

	void MyHud::createLegendEntry ( osg::Vec4 color, osg::Vec3 position, std::string text ) {
		osg::Vec3 dy(0.0f,25.0f,0.0f);
        osg::Vec3 dx(25.0f,0.0f,0.0f);
		osg::Vec3 dtext(40.0f, 5.0f, 0.0f );
 //       osg::Geode* geode = new osg::Geode();
  //      osg::StateSet* stateset = geode->getOrCreateStateSet();
        
		osg::Geometry *quad=new osg::Geometry;
		osg::StateSet* stateset = new osg::StateSet;
  //      stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
  stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
  //osg::Texture2D* HUDTexture = new osg::Texture2D;
    //HUDTexture->setDataVariance(osg::Object::DYNAMIC);
    //HUDTexture->setImage(osgDB::readImageFile("images/master_chief.jpg"));
    //stateset->setTextureAttributeAndModes(0,HUDTexture,osg::StateAttribute::ON);
      quad->setStateSet ( stateset );
		addNode ( (osg::Drawable*)(createTextNode ( 
			text, position + dtext ).get()) );

//		geode->setName("NodeLegendEntry" + text);
        osg::Vec3Array* vertices = new osg::Vec3Array(4); // 1 quad
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors = new osg::Vec4Array;
        colors->push_back(color);
        quad->setColorArray(colors);
        quad->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
        (*vertices)[0]=position;
        (*vertices)[1]=position+dx;
        (*vertices)[2]=position+dx+dy;
        (*vertices)[3]=position+dy;
        quad->setVertexArray(vertices);
        quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
//        geode->addDrawable(quad);

		((osg::Geode*)hud)->addDrawable(quad);
	}
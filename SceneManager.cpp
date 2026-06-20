///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////
// does this work????
#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

// setting up textures
//bringing in brick for when I build the house
//bringing in wood for the posts
//update the wood looks horrible
//using darkwood for the posts instead
//metal imported
void SceneManager::LoadSceneTextures() {
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"../../Utilities/textures/RealBrick.jpg",
		"brick");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/wood.jpg",
		"wood");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/darkwood.jpg",
		"dwood");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/metal.jpg",
		"metal");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/darkmetal.jpg",
		"dmetal");
	bReturn = CreateGLTexture(
		"../../Utilities/textures/grass.jpg",
		"grass");
	//adding moon texture!
	bReturn = CreateGLTexture(
		"../../Utilities/textures/abstract.jpg",
		"moon");
	BindGLTextures();
	//adding stars textures
	bReturn = CreateGLTexture(
		"../../Utilities/textures/star.jpg",
		"star");
	BindGLTextures();
}
//Configuring materials
void SceneManager::DefineObjectMaterials() {

	//adding dark wood materials

	OBJECT_MATERIAL darkwoodMaterial;
	darkwoodMaterial.ambientColor = glm::vec3(0.5f, 0.4f, 0.3f);
	darkwoodMaterial.ambientStrength = 0.2f;
	darkwoodMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	darkwoodMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	darkwoodMaterial.shininess = 0.6;
	darkwoodMaterial.tag = "dwood";

	m_objectMaterials.push_back(darkwoodMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.5f, 0.4f, 0.3f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	woodMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	woodMaterial.shininess = 0.6;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	//moon material shader
	OBJECT_MATERIAL moonMaterial;
	moonMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.2f);
	moonMaterial.ambientStrength = 0.9f;
	moonMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.3f);
	moonMaterial.specularColor = glm::vec3(0.7f, 0.6f, 0.5f);
	moonMaterial.shininess = 10.0;
	moonMaterial.tag = "moon";

	m_objectMaterials.push_back(moonMaterial);

	//moon material shader
	OBJECT_MATERIAL starMaterial;
	starMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.2f);
	starMaterial.ambientStrength = 0.9f;
	starMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.3f);
	starMaterial.specularColor = glm::vec3(0.7f, 0.6f, 0.5f);
	starMaterial.shininess = 10.0;
	starMaterial.tag = "star";

	m_objectMaterials.push_back(starMaterial);

	OBJECT_MATERIAL gMaterial;
	gMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.2f);
	gMaterial.ambientStrength = 0.9f;
	gMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.3f);
	gMaterial.specularColor = glm::vec3(0.7f, 0.6f, 0.5f);
	gMaterial.shininess = 10.0;
	gMaterial.tag = "grass";

	m_objectMaterials.push_back(gMaterial);

	OBJECT_MATERIAL mMaterial;
	mMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.2f);
	mMaterial.ambientStrength = 0.4f;
	mMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.3f);
	mMaterial.specularColor = glm::vec3(0.7f, 0.6f, 0.5f);
	mMaterial.shininess = 23.0;
	mMaterial.tag = "metal";

	m_objectMaterials.push_back(mMaterial);

}
//lights
void SceneManager::SetupSceneLights() {

	m_pShaderManager->setVec3Value("lightSources[0].position", 0.0f, 40.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.1f, 0.1f, 0.5f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 30.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.01f);

	m_pShaderManager->setVec3Value("lightSources[1].position", 0.0f, 30.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 30.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 5.0f);

	//adding lights to radiant from the moon
	m_pShaderManager->setVec3Value("lightSources[1].position", 15.0f, 52.0f, -10.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 30.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 5.0f);

	

	m_pShaderManager->setBoolValue("bUseLighting", true);

}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->DrawConeMesh();
	
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(60.0f, 0.1f, 60.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	//ZrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(1, 1, 1, 1);
	SetShaderTexture("grass");
	//i think the wood shader just looks better
	SetShaderMaterial("dwood");

	//grass floor
	// draw the mesh with transformation values
	//relunctacly switched this to a box mesh because it looks better even though the shaders are misbehaving in a different way
	//m_basicMeshes->DrawBoxMesh();
	//changed my mind but kept this because it could be used down the road
	m_basicMeshes->DrawPlaneMesh();

	//first pole
	//scaling
	scaleXYZ = glm::vec3(.2f, 10.0f, 0.2f);

	//rotation
	XrotationDegrees = -11.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(9.0f, 0.0f, -1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//pole cross bar
	//scaling
	scaleXYZ = glm::vec3(0.1f, 1.5f, 0.1f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	//positioning
	positionXYZ = glm::vec3(10.0f, 8.5f, -2.4f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");
	//Load cross bar
	m_basicMeshes->DrawCylinderMesh();

	//second pole
	//scaling
	scaleXYZ = glm::vec3(.2f, 10.0f, 0.2f);

	//rotation
	XrotationDegrees = -17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(12.0f, 0.0f, -1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole 2
	m_basicMeshes->DrawCylinderMesh();

	//pole cross bar2
//scaling
	scaleXYZ = glm::vec3(0.1f, 1.5f, 0.1f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 90.0f;
	//positioning
	positionXYZ = glm::vec3(12.5f, 7.5f, -2.9f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Load cross bar
	m_basicMeshes->DrawCylinderMesh();

	//power box 
	//scaling
	scaleXYZ = glm::vec3(0.7f, 1.0f, 0.7f);

	//rotation
	XrotationDegrees = -17.0f;
	YrotationDegrees = 60.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(11.5f, 2.0f, -1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("metal");
	SetShaderMaterial("metal");

	//Load power box 
	m_basicMeshes->DrawBoxMesh();

	//power box control panel
	//scaling
	scaleXYZ = glm::vec3(0.3f, 0.6f, 0.3f);

	//rotation
	XrotationDegrees = -17.0f;
	YrotationDegrees = 60.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(11.15f, 2.0f, -1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//black
	//SetShaderColor(0, 0, 0, 1);
	SetShaderTexture("dmetal");
	SetShaderMaterial("metal");

	//Load power panel
	m_basicMeshes->DrawBoxMesh();

	//Pole One Power outlet
	//scaling
	scaleXYZ = glm::vec3(0.17f, 0.17f, 0.17f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(10.0f, 8.5f, -2.4f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("metal");
	SetShaderMaterial("metal");

	//Load power outlet
	m_basicMeshes->DrawSphereMesh();

	//power outlet 2
	//scaling
	scaleXYZ = glm::vec3(0.17f, 0.17f, 0.17f);

	//rotation
	XrotationDegrees = -17.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(12.5f, 7.5f, -2.9f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("metal");
	SetShaderMaterial("metal");

	//Load power outlet 2
	m_basicMeshes->DrawSphereMesh();



	//house

	//base
	//house base
	//scaling
	//fixing the house so the lights match properly by adjusting rotation and sizing
	scaleXYZ = glm::vec3(10.0f, 10.0f, 20.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-6.0f, 5.0f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("dwood");

	//Load house base
	m_basicMeshes->DrawBoxMesh();

	//door
	//updated door values to match lighting due to rotation and sizing
	scaleXYZ = glm::vec3(2.0f, 6.0f, 3.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-6.3f, 4.0f, 4.3f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	
	SetShaderTexture("wood");
	SetShaderMaterial("wood");

	//Load house door
	m_basicMeshes->DrawBoxMesh();

	//house roof
	//changing roof values and textures
	scaleXYZ = glm::vec3(10.0f, 5.0f, 20.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	//updated positioning based on scale values offset
	positionXYZ = glm::vec3(-6.0f, 12.5f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("wood");
	SetShaderMaterial("wood");

	//Load house roof
	m_basicMeshes->DrawPyramid4Mesh();


	//house sign post
	scaleXYZ = glm::vec3(0.3f, 4.0f, 0.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(0.0f, 1.0f, 6.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("wood");
	SetShaderMaterial("wood");

	//Load house post
	m_basicMeshes->DrawCylinderMesh();

	//house sign 
	scaleXYZ = glm::vec3(0.3f, 4.0f, 1.0f);

	//rotation
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	//positioning
	positionXYZ = glm::vec3(1.0f, 4.0f, 6.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Load house sign
	m_basicMeshes->DrawBoxMesh();

	//house door step
	
	//scaling
	scaleXYZ = glm::vec3(20.0f, 2.0f, 4.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-6.0f, 0.0f, 5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//grey
	//SetShaderColor(.5, .5, .5, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Load door step
	m_basicMeshes->DrawBoxMesh();

	//door handle
	scaleXYZ = glm::vec3(0.2f, 0.2f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-5.5f, 3.5f, 5.3f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load door handle
	m_basicMeshes->DrawSphereMesh();

	//--------------------------------------------------------------------------------------------------------------------------------------------//
	//This is where I am adding new objects to the project//
	//--------------------------------------------------------------------------------------------------------------------------------------------//

	//Okay so far I fixed the lighting, updated and added textures; the project finally is in a point where I am willing to move forward with adding objects.
	//here is what I want to add on top of the graphical enhancments
	//Moon, starry night back drop, trees, road, driveway and a car
	

	//moon
	scaleXYZ = glm::vec3(10.0f, 10.0f, 10.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(15.0f, 52.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("moon");
	SetShaderMaterial("wood");

	//Load moon
	m_basicMeshes->DrawSphereMesh();


	


	//stars
	scaleXYZ = glm::vec3(50.0f, 50.0f, 50.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("star");
	SetShaderMaterial("wood");

	//Load stars
	m_basicMeshes->DrawSphereMesh();


	//road
	scaleXYZ = glm::vec3(100.0f, 1.0f, 20.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(0.0f, 0.0f, 25.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("dmetal");
	SetShaderMaterial("wood");

	//Load road
	m_basicMeshes->DrawBoxMesh();

	//driveway
	scaleXYZ = glm::vec3(25.0f, 1.0f, 15.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(22.0f, 0.0f, 2.5f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("dmetal");
	SetShaderMaterial("wood");

	//Load driveway
	m_basicMeshes->DrawBoxMesh();

	

	//truck base
	scaleXYZ = glm::vec3(7.0f, 5.0f, 10.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(22.0f, 4.0f, 4.5f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("dmetal");
	SetShaderMaterial("wood");

	//Load truck base
	m_basicMeshes->DrawBoxMesh();

	//truck grill
	scaleXYZ = glm::vec3(5.0f, 3.0f, 0.4f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(22.0f, 4.5f, 9.5f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck grill
	m_basicMeshes->DrawBoxMesh();


	//truck roof
	scaleXYZ = glm::vec3(5.0f, 3.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(22.0f, 7.0f, 3.5f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck roof
	m_basicMeshes->DrawBoxMesh();

	//truck bed
	scaleXYZ = glm::vec3(7.0f, 2.5f, 10.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(22.0f, 2.8f, -5.5f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("dmetal");
	SetShaderMaterial("wood");

	//Load truck bed
	m_basicMeshes->DrawBoxMesh();

	//truck wheel1
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(19.0f, 2.0f, 9.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel1
	m_basicMeshes->DrawSphereMesh();



	//truck wheel2
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(19.0f, 2.0f, -5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel2
	m_basicMeshes->DrawSphereMesh();


	//truck wheel3
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(25.0f, 2.0f, -5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel3
	m_basicMeshes->DrawSphereMesh();

	//truck wheel4
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(25.0f, 2.0f, 9.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel4
	m_basicMeshes->DrawSphereMesh();

	//truck wheel5
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(25.0f, 2.0f, -9.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel5
	m_basicMeshes->DrawSphereMesh();

	//truck wheel6
	scaleXYZ = glm::vec3(1.3f, 1.3f, 1.3f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(19.0f, 2.0f, -9.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("metal");
	SetShaderMaterial("wood");

	//Load truck wheel6
	m_basicMeshes->DrawSphereMesh();

	//tree1

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-23.0f, 0.0f, 5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-23.0f, 15.0f, 5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-23.0f, 10.0f, 5.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//tree2

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 0.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 15.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 10.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//tree3

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-37.0f, 0.0f, 1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-37.0f, 15.0f, 1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-37.0f, 10.0f, 1.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();
	
	//tree4

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 0.0f, -20.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 15.0f, -20.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-25.0f, 10.0f, -20.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//tree5

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-15.0f, 0.0f, -25.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-15.0f, 15.0f, -25.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-15.0f, 10.0f, -25.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();


	//tree6

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-5.0f, 0.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-5.0f, 15.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(-5.0f, 10.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();



	//tree7

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(15.0f, 0.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(15.0f, 15.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(15.0f, 10.0f, -15.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();



	//tree8

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(30.0f, 0.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(30.0f, 15.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(30.0f, 10.0f, -10.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();



	//tree9

	scaleXYZ = glm::vec3(.2f, 15.0f, 0.2f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(35.0f, 0.0f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("dwood");
	SetShaderMaterial("wood");

	//Base Pole
	m_basicMeshes->DrawCylinderMesh();

	//leaves

	scaleXYZ = glm::vec3(5.0f, 5.0f, 5.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(35.0f, 15.0f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();

	//leaves2

	scaleXYZ = glm::vec3(7.0f, 7.0f, 7.0f);

	//rotation
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	//positioning
	positionXYZ = glm::vec3(35.0f, 10.0f, 0.0f);
	//set them
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	//brown
	//SetShaderColor(.4, .25, .1, 1);
	SetShaderTexture("grass");
	SetShaderMaterial("grass");

	//load leaves
	m_basicMeshes->DrawPyramid4Mesh();


	

	

	/****************************************************************/
}

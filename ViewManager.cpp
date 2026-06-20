///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include "sqlitea/sqlite3.h";
using namespace std;
//including vectors for data storage
#include <vector>



// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    



// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters

	g_pCamera->Position = glm::vec3(0.0f, 7.0f, 25.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
		//tracking
	if (gFirstMouse) {
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
		

	}
		//seeing offsets 
		float xOffset = xMousePos - gLastX;
		float yOffset = gLastY - yMousePos;

		//setting positions
		gLastX = xMousePos;
		gLastY = yMousePos;

		//moving camera
		g_pCamera -> ProcessMouseMovement(xOffset * 8, yOffset * 8);

	
}


/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/

//--------------------------------------------------------------------------------

//hold value so we are not registering multiple clicks per click on locational buttons. They only need to be clicked once so this solves that issue
int hold = 0;
//I decided to repeat this process for all the found locations because the way this program is set up
//every click registers multiple click at a time which leads to output clogging the logs.
//it is an unorthidox fix but it works
int truck = 0;
int moon = 0;
int cabin = 0;
int trees = 0;
int power = 0;
int road = 0;
int sign = 0;
int door = 0;
int ans = 0;
//vector for storing locations we have found
std::vector<std::string> foundAreas = {};
//------------


//creating database functions
//create database
static int createDat(const char* d) {
	sqlite3* db;
	int stop = 0;

	stop = sqlite3_open(d, &db);
	sqlite3_close(db);
	std::cout << "Database created"<<"\n";
	return 0;
}
//create table
static int createTab(const char* d) {
	sqlite3* db;
	//creating a table
	string sql = "CREATE TABLE IF NOT EXISTS PROGRESS("
		"TOTAL INTEGER PRIMARY KEY AUTOINCREMENT,"
		"LOCATIONS TEXT NOT NULL);";
	int stop = 0;
	
	//opening and testing exec
	stop = sqlite3_open(d, &db);
	char* error;
	stop = sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
	//loop to see if it worked
	if (stop == SQLITE_OK) {
		cout << "Table is created" << "\n";
		
	}
	else {
		cout << "Error with table" << "\n";
		
	}
		

	return 0;
	
}
//insert data
static int insertDat(const char* d) {
	sqlite3* db;
	char* error;
	int stop = sqlite3_open(d, &db);
	if (truck == 1) {
		truck++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Truck');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout<<"\n" << "Insert truck to database succesfull" << "\n";
		}
		else cout << "insert truck to database unsuccesfull" << "\n";
		
	}
	if (moon == 1) {
		moon++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Moon');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert moon to database succesfull" << "\n";
		}
		else cout << "insert moon to database unsuccesfull" << "\n";
	}
	if (cabin == 1) {
		cabin++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Cabin');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert cabin to database succesfull" << "\n";
		}
		else cout << "insert cabin to database unsuccesfull" << "\n";
	}
	if (power == 1) {
		power++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Powerlines');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert powerlines to database succesfull" << "\n";
		}
		else cout << "insert powerlines to database unsuccesfull" << "\n";
	}
	if (trees == 1) {
		trees++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Trees');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert trees to database succesfull" << "\n";
		}
		else cout << "insert trees to database unsuccesfull" << "\n";
	}
	if (road == 1) {
		road++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Road');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert road to database succesfull" << "\n";
		}
		else cout << "insert road to database unsuccesfull" << "\n";
	}
	if (sign == 1) {
		sign++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Sign');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert sign to database succesfull" << "\n";
		}
		else cout << "insert sign to database unsuccesfull" << "\n";
	}
	if (door == 1) {
		door++;
		string sql("INSERT INTO PROGRESS(TOTAL,LOCATIONS) VALUES('Door');");
		sqlite3_exec(db, sql.c_str(), NULL, 0, &error);
		if (stop == SQLITE_OK) {
			cout << "\n" << "Insert door to database succesfull" << "\n";
		}
		else cout << "insert door to database unsuccesfull" << "\n";
	}
	return 0;
}
//select data
static int selectDat(const char* d) {
	sqlite3* db;
	char* error;
	int stop = sqlite3_open(d, &db);
	string sql = "SELECT * FROM PROGRESS;";
	sqlite3_exec(db, sql.c_str(),NULL,0,&error);
	if (stop == SQLITE_OK) {
		cout << "\n"<<"Selecting data Succefull" << "\n";
	}
	else {
		cout << "Selecting data unsuccesfull";
	}
	return 0;
}



//The main gameplay loop lies inside this ProcessKeyboardEvents function
//Found locations are stored inside the vector and a relayed to the player alongside an int count of locations they have found and how many are left via cout

void ViewManager::ProcessKeyboardEvents()
{



	
	//printing welcome message
	if (hold <= 0) {
		hold++;

		
		std::cout << "--------------------------------------------------------------------------------------" << "\n";
		//creting directory and calling create functions
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		//creating table and database
		createDat(directory);
		createTab(directory);
		
		
		
		
		
		



		
		std::cout << "-------------------------------------------------------------------------------------"<<"\n"<< "Welcome to The Cabin, this is a game where you will be looking for key locations in a crime scene. You will be taking notes of everything you see at the scene. You cannot leave untill you have found everything. You can't shake the creepy feeling this place gives you. Use the left click button on your mouse to read the tutorial"<<"\n" << "If you want the cheat sheet use right click on the mouse";
	}

	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		//selecting data because escape will close the game so the data should be stored here
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		selectDat(directory);
		//letting the player know what data was stored
		std::cout << "Locations requested to save: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		
		

		glfwSetWindowShouldClose(m_pWindow, true);
		
		
	}
	//setting up keyboard functions
	//updated speed functions for movement

	//----------------------------------------------------------------------------------------------
	//adding input for mouse clicks
	//adding input for keyboard clicks
	
	//this will serve as the tutorial
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS&&hold <=1 )
	{
		
		hold++;
		//adding seperation lines so it is easier for the player to see new messages
		std::cout << "\n" << "---------------------------------------------------------------------------------------"<<"\n";
		std::cout  << "You found the tutorial, you must look for objects around the scene and type the first letter of them to add them to key the locations you have found (For example, if you see a truck then type T because that is the first letter in truck). You must find all the key locations before you can return to your boss. Hint: use wasd to move and e and q to go up and down, key locations can be found high and low." << "\n";
	}
	//cheat sheet if you just want to see that the program works without looking for clue
	//this will serve as the tutorial
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && ans <= 0)
	{
		ans++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout <<"Location Letters : T, M, C, F, P, R, S and D " << "\n";
	}

	//Truck location
	if (glfwGetKey(m_pWindow, GLFW_KEY_T) == GLFW_PRESS && truck <= 0)
	{

		//---------------------------------------------------
		truck++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "You found an old Ford Ranger, the windows are so dirty you cannot see inside, the tires are flat and it looks like it has not been driven in years. It gives you the creeps"<<"\n";
		foundAreas.push_back("Truck");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout<< foundAreas << " ";
		}
		
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for truck
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
		
		


		
		
	}

	//Moon location
	if (glfwGetKey(m_pWindow, GLFW_KEY_M) == GLFW_PRESS && moon <= 0)
	{
		moon++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "You look up at the moon, it is a solar eclipse today, its color is red and it is not helping the unease you feel." << "\n";
		foundAreas.push_back("Moon");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout<< foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//-----------------------------------------------------

		//insert for moon
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}

	//Cabin location
	if (glfwGetKey(m_pWindow, GLFW_KEY_C) == GLFW_PRESS && cabin <= 0)
	{
		cabin++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "There is a cabin, it looks like nobody has lived there for a while." << "\n";
		foundAreas.push_back("Cabin");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for cabin
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}
	
	//Forest location
	if (glfwGetKey(m_pWindow, GLFW_KEY_F) == GLFW_PRESS && trees <= 0)
	{
		trees++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "You are in a forest, as the wind sways past you and brushes through the trees you feel at peace. Nature is beautiful." << "\n";
		foundAreas.push_back("Forest");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for forest
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}
	//Powerlines location
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS && power <= 0)
	{
		power++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "You see a set of powerlines outside the house. They are about to fall over. I hope noones turns those back on any time soon they do not look safe." << "\n";
		foundAreas.push_back("Powerlines");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for powerline
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}

	//Road location
	if (glfwGetKey(m_pWindow, GLFW_KEY_R) == GLFW_PRESS && road <= 0)
	{
		road++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "The road is old and does not have any markings on it." << "\n";
		foundAreas.push_back("Road");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for road
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}
	//Sign location
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS && sign <= 0)
	{
		sign++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "There is a sign on the front of the house, whatever was written on it is no longer legible." << "\n";
		foundAreas.push_back("Sign");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for sign
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);
	}
	//Door location
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS && door <= 0)
	{
		door++;
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
		std::cout << "The door is very old fashioned, you can almost see through the cracks in the planks." << "\n";
		foundAreas.push_back("Door");
		std::cout << "Found locations: ";
		for (std::string foundAreas : foundAreas) {
			std::cout << foundAreas << " ";
		}
		std::cout << foundAreas.size() << "/8" << "\n";
		//insert for door
		const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
		sqlite3* db;
		insertDat(directory);

	}
	//Close the game when the player finds all the key locations

	

	if (foundAreas.size()>=8)
	{
		
		std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
	std::cout << "Well your work here is done. You did not find much and you are unsure why you were sent here in the first place. You turn back to head to your police cruiser but... wait a minute... where is it??? You could have sworn you parked just down the road. You hear a door creak behind you and you are afraid to turn around. You look nothing is there. You turn back around and someone is standing where you parked holding your keys. To be continued...";
	std::cout << "\n" << "---------------------------------------------------------------------------------------" << "\n";
	//selecting data to table
	const char* directory = "C:\\Users\\kyler\\Desktop\\WilesKylerCapstone\\CS330Content\\Projects\\WilesKylerCapstone\\OpenGlDatabase.db";
	sqlite3* db;
	selectDat(directory);
	std::cout << "Locations requested to save: ";
	//letting the player know what data was added to database
	for (std::string foundAreas : foundAreas) {
		std::cout << foundAreas << " ";
	}
	
		glfwSetWindowShouldClose(m_pWindow, true);
	}
	
	//----------------------------------------------------------------------------------------------

	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime*6);
		
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime*6);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime*6);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime*6);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime*6);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(UP, gDeltaTime*6);
	}



}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	//adding if statements for orthographic and perspective view
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS) {
		projection = glm::perspective(glm::radians(90.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS) {
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	}
	
	
	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}

}



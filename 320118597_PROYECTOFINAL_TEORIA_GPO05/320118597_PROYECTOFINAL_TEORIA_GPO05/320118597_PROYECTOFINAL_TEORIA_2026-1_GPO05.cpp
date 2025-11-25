//320118597
// PROYECTOFINAL2026-1
//GPO05


//Se cargan todas las librerias vistas con las practicas
#include <iostream>
#include <cmath>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include "stb_image.h"		// Para cargar imagenes
#include <glm/glm.hpp>		//Para vectores y matrices
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Funciones para controlar
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode); //Teclado para encender/apagar animaciones
void MouseCallback(GLFWwindow *window, double xPos, double yPos);					// Mouse para la cámara
void DoMovement();																	// Movimiento de cámara con WASD
void animacion();																	 // Para las secuencias de las pokébolas

// Dimensiones de la pantalla
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camara
Camera  camera(glm::vec3(-100.0f, 2.0f, -45.0f));								//Posicion inicial
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
float range = 0.0f;
float rot = 0.0f;

//ON/OFF de las animaciones
bool anim2_on = true;   // Secuencias de las pokebolas
bool anim3_on = true;  // TV (shader Anim3)
bool anim4_on = true;  // Pantalla PC (shader AnimPC)
bool anim5_on = true;  // Foco (shader Anim4)

// para congelar el tiempo cuando se apaga
float t_hold_anim3 = 0.0f;
float t_hold_animPC = 0.0f;
float t_hold_foco = 0.0f;



// Secuencia de las pokebolas
static const float groundY = -2.0f;
static const float FLOOR_Y = groundY;

const int NUM_BALLS = 6; //Total de pokebolas
struct Ball {
	glm::vec3 base;  // posición en el “suelo”
	glm::vec3 pos;   // posición animada
	float t;         // tiempo dentro del salto 
	float spin;      // giro en el aire
};
Ball balls[NUM_BALLS];
int activeIdx = 0;

// Física/tiempo (ajusta a tu look actual)
const float GRAVY = -19.6f;           // gravedad usada en anteriores practicas
const float V0_SEQ = 9.3f;             // altura/velocidad
const float JUMP_T = 1.0f;             // duración final por bola
const float SPIN_DEG_PER_SEC = 100.0f; // giro final por bola

float voltorbTime = 0.0f;
bool voltorbGoingRight = true;   // dirección
float voltorbX = -103.18f;       // posición base


// VOLTORB (animación)
float v_pos = 0.0f;        // posición actual
float v_speed = 3.0f;      // velocidad de movimiento
float v_min = -0.38f;       // límite izquierda
float v_max = 5.29f;       // límite derecha
bool v_right = true;       // dirección
float v_pause = 0.0f;      // contador de espera
float v_waitTime = 1.0f;   // segundos que espera en el extremo

float v_rot = 0.0f;        // giro actual
float v_rotSpeed = 191.0f; // velocidad de giro

//Switch para la animacion
bool anim1_on = true;  // Voltorb



// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f); //Posicion del cubo con luz 
glm::vec3 PosIni(-100.0f, 1.0f, -47.0f);	//Posicion inicial
bool active;


// Deltatime
GLfloat deltaTime = 0.0f;	// Tiempo entre frames
GLfloat lastFrame = 0.0f;  	// Tiempo desde el utlimo frame, lo uso para que jamas se queden quietos

float posX;		//Variable para PosicionX
float posY;		//Variable para PosicionY
float posZ;		//Variable para PosicionZ

// Posiciones utilizadas para las 4 luces, que se veran en las ventanas y en la pantalla 
glm::vec3 pointLightPositions[] = {
	glm::vec3(posX,posY,posZ),
	glm::vec3(0,0,0),
	glm::vec3(0,0,0),
	glm::vec3(0,0,0)
};

glm::vec3 LightP1;








int main()
{
	// Init GLFW
	glfwInit();		//Inicia GLFW
	

	//Creo la ventana principal con el nombre del proyecto
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "320118597_PROYECTOFINAL2026-1_GPO08", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;			// Si falla, cierro GLFW y salgo
		glfwTerminate();

		return EXIT_FAILURE;
	}

	//Funciones base de otras practicas que "atrapan" el cursor y teclado dentro de la pantalla
	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	printf("%f", glfwGetTime());

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	// Shaders principales 
	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader SkyBoxshader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");

	//TV
	Shader Anim3("Shaders/anim3.vs", "Shaders/anim3.frag");
	Shader AnimPC("Shaders/animPC.vs", "Shaders/animPC.frag");

	//Ventanas
	Shader Glass("Shaders/glass.vs", "Shaders/glass.frag");
	Glass.Use();
	glUniform1i(glGetUniformLocation(Glass.Program, "uTex"), 0); // sampler a la unidad 0

	// Habilita blending 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Config inicial la animacion para la pantalla de la PC
	AnimPC.Use();
	glUniform1i(glGetUniformLocation(AnimPC.Program, "uTex"), 0);	// La pantalla PC usa la textura en unidad 0

	// Config inicial de Anim3 (TV)
	Anim3.Use();
	glUniform1i(glGetUniformLocation(Anim3.Program, "uTexA"), 0);
	glUniform1i(glGetUniformLocation(Anim3.Program, "uTexB"), 1);
	glUniform1i(glGetUniformLocation(Anim3.Program, "canal1"), 0);
	glUniform1i(glGetUniformLocation(Anim3.Program, "canal2"), 1);
	glUniform1f(glGetUniformLocation(Anim3.Program, "uSpeed"), 1.0f);
	glUniform1f(glGetUniformLocation(Anim3.Program, "uWidth"), 0.15f);
	glUniform1f(glGetUniformLocation(Anim3.Program, "uScan"), 0.35f);
	glUniform1f(glGetUniformLocation(Anim3.Program, "uDist"), 0.001f);
	glUniform1f(glGetUniformLocation(Anim3.Program, "uFlick"), 0.02f);

	// Foco (Anim4)
	Shader Anim4("Shaders/anim4.vs", "Shaders/anim4.frag");
	Anim4.Use();
	glUniform3f(glGetUniformLocation(Anim4.Program, "uColor"), 1.0f, 0.1f, 0.1f); // rojo
	glUniform1f(glGetUniformLocation(Anim4.Program, "uMinI"), 0.15f);
	glUniform1f(glGetUniformLocation(Anim4.Program, "uMaxI"), 1.00f);
	glUniform1f(glGetUniformLocation(Anim4.Program, "uSpeed"), 0.6f); // más alto = más rápido
	glUniform1f(glGetUniformLocation(Anim4.Program, "uSoft"), 0.20f);
	glUniform1i(glGetUniformLocation(Anim4.Program, "uUseRadial"), 1); // 1 = LED redondito; 0 = todo el mesh




	// Carga de modelos .obj
	Model Centro((char*)"Models/CentroPokemon/CentroPokemonFinalTSV.obj");
	Model Ventanas((char*)"Models/CentroPokemon/Ventanas2.obj");
	Model Puerta((char*)"Models/CentroPokemon/Puerta2.obj");
	Model Libreria((char*)"Models/Libreria/Libreria3.obj");
	Model Mapa((char*)"Models/Mapa/Mapa2.obj");
	Model Mostrador((char*)"Models/Mostrador/Mostrador3.obj");
	Model Planta((char*)"Models/Planta/Planta3.obj");
	Model Borde((char*)"Models/Tele/Borde.obj");
	Model Pantalla((char*)"Models/Tele/Pantalla.obj");
	Model Sanador((char*)"Models/Sanador/Sanador2.obj");
	Model PC((char*)"Models/PC/PCSF2.obj");
	Model PantallaPC((char*)"Models/PC/PantallaPC.obj");
	Model Foco((char*)"Models/PC/Foco.obj");
	Model Poke((char*)"Models/Poke/Poke.obj");
	Model Voltorb((char*)"Models/Voltorb/Voltorb2.obj");
	Model Letrero((char*)"Models/Letrero/Letrero.obj");
	Model Piso((char*)"Models/Piso/Piso.obj");
	Model Lab((char*)"Models/Lab/Lab4.obj");
	Model V2((char*)"Models/Lab/V4.obj");
	Model PC2((char*)"Models/PC2/PC2.obj");
	Model MesaMadera((char*)"Models/MesaMadera/MesaMadera.obj");
	Model Poke2((char*)"Models/Poke2/Poke2.obj");
	Model Mesa((char*)"Models/Mesa/Mesa.obj");
	Model Capsula((char*)"Models/Capsula/Capsula.obj");
	Model LibreriaB((char*)"Models/LibreriaB/LibreriaB2.obj");

	



	// La geometria para las luces con forma de cubo
	GLfloat vertices[] =
	{
		// Positions            // Normals              // Texture Coords
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,  	1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,

		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f
	};

	//Uso las mismas propiedades de base que el skybox anterior, por lo que solo debo de cambiar las imagenes
	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	//Dejo los siguientes puntos para compatibilidad, ya no uso las posiciones que me dan
	GLuint indices[] =
	{  
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15,
		16,17,18,19,
		20,21,22,23,
		24,25,26,27,
		28,29,30,31,
		32,33,34,35
	};

	// Positions all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};


	// VBO, VAO y EBO para las luces
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	// Normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Texture Coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	



	//SkyBox
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1,&skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices),&skyboxVertices,GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);


	// Aqui es donde estaban las imagenes para el skybox de la practica, tan solo tuve que renombrarlos por otras imagenes
	vector<const GLchar*> faces;
	faces.push_back("SkyBox/right.tga");
	faces.push_back("SkyBox/left.tga");
	faces.push_back("SkyBox/top.tga"); //top
	faces.push_back("SkyBox/bottom.tga");
	faces.push_back("SkyBox/back.tga");
	faces.push_back("SkyBox/front.tga");
	
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	// Texturas de la pantalla
	auto LoadTexture2D = [](const char* path)->GLuint {
		int w, h, ch;
		stbi_set_flip_vertically_on_load(true); // Invierto en Y las imagenes para que no salgan volteadas
		unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
		if (!data) { std::cout << "No se pudo cargar: " << path << std::endl; return 0; }
		GLuint tex; glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		stbi_image_free(data);
		return tex;
		};

	GLuint canalATex = LoadTexture2D("Textures/canal1.png");
	GLuint canalBTex = LoadTexture2D("Textures/canal2.png"); //Aqui en realidad utilice la misma imagen, solamente le puse menos brillo para que simulara una pantalla con interferencia


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

	// Bosiciones base de las 6 pokebolas
	balls[0].base = glm::vec3(-102.215f, groundY, -105.986f);
	balls[1].base = glm::vec3(-103.850f, groundY, -105.986f);
	balls[2].base = glm::vec3(-103.850f, groundY, -107.986f);
	balls[3].base = glm::vec3(-102.215f, groundY, -107.986f);
	balls[4].base = glm::vec3(-103.850f, groundY, -109.986f);
	balls[5].base = glm::vec3(-102.215f, groundY, -109.986f);

	for (int i = 0; i < NUM_BALLS; i++) {
		balls[i].pos = balls[i].base;	// Empieza en base
		balls[i].t = 0.0f;				// Reinicio tiempo local
		balls[i].spin = 0.0f;			// Giro inicial
	}

	activeIdx = 0;		// Arranca la secuencia desde la primera bola
	

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calcula el tiempo entre frames
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// ANIMACION VOLTORB
		if (anim1_on) {			// Solo avanza si la animación 1 está encendida (tecla 1)
			if (v_pause > 0.0f) {
				v_pause -= deltaTime;			// Cuenta regresiva en el extremo
			}
			else {
				// // Movimiento lineal derecha/izquierda + giro tipo rueda
				if (v_right) v_pos += v_speed * deltaTime;
				else         v_pos -= v_speed * deltaTime;

				if (v_right) v_rot -= v_rotSpeed * deltaTime;
				else         v_rot += v_rotSpeed * deltaTime;

				// Checar límites y cambiar de sentido con una pausa
				if (v_pos >= v_max) { v_pos = v_max; v_pause = v_waitTime; v_right = false; }
				if (v_pos <= v_min) { v_pos = v_min; v_pause = v_waitTime; v_right = true; }
			}
		}
		// si está apagado, se queda en su última posición/rotación




		 // Entrada y animaciones de frame
		glfwPollEvents();
		DoMovement();
		if (anim2_on) animacion();



		// Limpiar buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Shader de iluminación principal (Phong)
		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
	
		
		// Luz direccional (tipo sol general)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.10f, 0.10f, 0.10f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 1.0f, 0.95f, 0.85f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 1.0f, 0.98f, 0.9f);

		// Luz tipo “sol” entrando por ventanas 
		pointLightPositions[0] = glm::vec3(-111.0f, 13.2f, -113.2f);  // ventana trasera izquierda
		pointLightPositions[1] = glm::vec3(-100.0f, 13.2f, -113.2f);  // ventana trasera derecha
		pointLightPositions[2] = glm::vec3(-89.0f, 13.2f, -113.2f);  // ventana trasera en medio
		pointLightPositions[3] = glm::vec3(-100.0f, 7.8f, -112.2f);  // Pantalla
		


		//Luces ya dadas y funcionales
		// Point light 1 (warm white)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"),
			pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.02f, 0.03f, 0.05f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.60f, 0.70f, 1.00f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.60f, 0.70f, 1.00f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0075f);

		


		// Point light 1 (warm white)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"),
			pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.08f, 0.08f, 0.08f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 1.00f, 0.95f, 0.85f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.00f, 0.95f, 0.85f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);

		// Point light 2 (warm white)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"),
			pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.08f, 0.08f, 0.08f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 1.00f, 0.95f, 0.85f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 1.00f, 0.95f, 0.85f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.032f);

		// Point light 3 (warm white)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"),
			pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.08f, 0.08f, 0.08f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 1.00f, 0.95f, 0.85f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 1.00f, 0.95f, 0.85f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.032f);

		// Point light 4 (warm white)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"),
			pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].ambient"), 0.08f, 0.08f, 0.08f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"), 1.00f, 0.95f, 0.85f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"), 1.00f, 0.95f, 0.85f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.032f);


		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();


		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	

		glBindVertexArray(VAO);
		glm::mat4 tmp = glm::mat4(1.0f); //Temp



		
		
		//Carga de modelos

		//Centro
		view = camera.GetViewMatrix();
		glm::mat4 model(1);
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-100.0f, -8.0f, -75.0f)); 
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::scale(model, glm::vec3(1.6f, 1.50f, 1.6f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Centro.Draw(lightingShader);


		
		
		


		// Ventanas con transparencia (shader Glass)
		Glass.Use();
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// tinte azul y transparencia
		glUniform3f(glGetUniformLocation(Glass.Program, "uTint"), 0.72f, 0.86f, 1.00f); // azul suave
		glUniform1f(glGetUniformLocation(Glass.Program, "uAlpha"), 0.45f);              // nivel de transparencia
		glDepthMask(GL_FALSE);   
		Ventanas.Draw(Glass);		
		glDepthMask(GL_TRUE);
		lightingShader.Use();		// Regreso al shader de iluminación para no afectar a los demas objetos



		

		//  Puerta con transparencia (shader Glass)
		Glass.Use();
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1f(glGetUniformLocation(Glass.Program, "uAlpha"), 0.70f);
		glDepthMask(GL_FALSE);
		Puerta.Draw(Glass);
		// restaurar
		glDepthMask(GL_TRUE);
		lightingShader.Use();





		//Lab
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-85.0f, -4.2f, -91.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Lab.Draw(lightingShader);

		// Ventanas del Lab (V2) con shader Glass 
		Glass.Use();
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(Glass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		
		glUniform3f(glGetUniformLocation(Glass.Program, "uTint"), 0.72f, 0.86f, 1.00f);
		glUniform1f(glGetUniformLocation(Glass.Program, "uAlpha"), 0.2f);

		glDepthMask(GL_FALSE);      
		V2.Draw(Glass);             
		glDepthMask(GL_TRUE);

		// regreso al shader de iluminación normal
		lightingShader.Use();





		//Libreria2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-56.8f, -4.1f, -112.0f)); // 
		model = glm::scale(model, glm::vec3(1.4f, 2.8f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		LibreriaB.Draw(lightingShader);









		//Libreria2 En frente derecha
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-52.5f, -4.2f, -94.0f)); // 
		model = glm::scale(model, glm::vec3(1.0f, 2.0f, 1.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		LibreriaB.Draw(lightingShader);

		//Libreria2 En frente derecha iz
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-55.7f, -4.2f, -94.0f)); // 
		model = glm::scale(model, glm::vec3(1.0f, 2.0f, 1.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		LibreriaB.Draw(lightingShader);



		//Libreria2 En frente Izquierda
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-72.1f, -4.2f, -94.0f)); // 
		model = glm::scale(model, glm::vec3(1.0f, 2.0f, 1.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		LibreriaB.Draw(lightingShader);

		//Libreria2 En frente Izquierda
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-68.9f, -4.2f, -94.0f)); // 
		model = glm::scale(model, glm::vec3(1.0f, 2.0f, 1.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		LibreriaB.Draw(lightingShader);




		//Mesa
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-55.7f, -5.9f, -113.0f)); // 
		model = glm::scale(model, glm::vec3(1.1f, 1.6f, 1.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Mesa.Draw(lightingShader);



		//MesaMadera
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-70.3f, -5.0f, -123.2f)); // 
		model = glm::scale(model, glm::vec3(1.12f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		MesaMadera.Draw(lightingShader);

		//PC2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-70.3f, -5.0f, -123.2f)); // 
		model = glm::scale(model, glm::vec3(1.12f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		PC2.Draw(lightingShader);

		//MesaMadera
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-64.1f, -5.0f, -123.2f)); // 
		model = glm::scale(model, glm::vec3(1.12f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		MesaMadera.Draw(lightingShader);

		


		//	Capsula
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-57.2f, -4.97f, -106.6f)); // 
		model = glm::scale(model, glm::vec3(1.0f, 1.2f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Capsula.Draw(lightingShader);



		//	Poke2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-54.25f, -2.65f, -96.2f)); // 
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Poke2.Draw(lightingShader);

		//	Poke2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-52.25f, -2.65f, -96.2f)); // 
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Poke2.Draw(lightingShader);

		//	Poke2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-50.25f, -2.65f, -96.2f)); // 
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Poke2.Draw(lightingShader);



		//Libreria
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-111.0f, -4.65f, -112.0f)); // 
		model = glm::scale(model, glm::vec3(1.2f, 2.2f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Libreria.Draw(lightingShader);



		//Piso
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-100.0f, -4.5, -95.0f)); // ← fija, frente a cámar
		model = glm::scale(model, glm::vec3(2.0f));
		model = glm::scale(model, glm::vec3(1.6f, 1.50f, 1.6f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);

		//Piso2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-2.9f, -4.5, -95.0f)); // ← fija, frente a cámara
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f));
		model = glm::scale(model, glm::vec3(1.6f, 1.50f, 1.6f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);



		//Mapa
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-91.2f, -4.5f, -103.6f));
		model = glm::scale(model, glm::vec3(1.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Mapa.Draw(lightingShader);
		
		
		
		//Mostrador
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-100.0f, -6.85f, -90.2f)); 
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 3.0f, 3.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Mostrador.Draw(lightingShader);


		//Letrero
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-100.0f, -4.85f, -90.2f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.4f, 2.0f, 3.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Letrero.Draw(lightingShader);



		//Planta
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-114.85f, -3.92f, -109.8f)); 
		model = glm::scale(model, glm::vec3(1.8f, 1.8, 1.8f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Planta.Draw(lightingShader);

		//Planta 2
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-85.85f, -3.92f, -83.3f)); 
		model = glm::scale(model, glm::vec3(1.8f, 1.8, 1.8f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Planta.Draw(lightingShader);

		//Planta 3
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-114.85f, -3.92f, -83.3f)); 
		model = glm::scale(model, glm::vec3(1.8f, 1.8, 1.8f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Planta.Draw(lightingShader);


		//Borde (separe el borde de la pantalla para aplicar el shader solo a la pantalla, lo mismo hice con la PC
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-100.0f, -1.0f, -100.9f));
		model = glm::scale(model, glm::vec3(1.6f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Borde.Draw(lightingShader);


		// Pantalla con shader
		view = camera.GetViewMatrix();
		glm::mat4 modelPant = glm::mat4(1.0f);
		modelPant = glm::translate(modelPant, glm::vec3(-100.0f, -1.0f, -100.9f));
		modelPant = glm::scale(modelPant, glm::vec3(1.6f));

		Anim3.Use();

		// matrices + tiempo 
		glUniformMatrix4fv(glGetUniformLocation(Anim3.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(Anim3.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(Anim3.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPant));
		float t_tv = anim3_on ? (float)glfwGetTime() : t_hold_anim3;
		glUniform1f(glGetUniformLocation(Anim3.Program, "uTime"), t_tv);

		glUniform1i(glGetUniformLocation(Anim3.Program, "uTexA"), 8);
		glUniform1i(glGetUniformLocation(Anim3.Program, "uTexB"), 9);

		glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D, canalATex); // canal1.png
		glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_2D, canalBTex); // canal2.png

		Pantalla.Draw(Anim3);
		Pantalla.Draw(Anim3);

		// vuelve a lightingShader para el resto
		lightingShader.Use();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPant)); 


		//Sanador
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-103.18f, -5.45f, -108.5f)); 
		model = glm::scale(model, glm::vec3(1.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Sanador.Draw(lightingShader);

		// Voltorb animado 
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		// Movimiento sobre X
		model = glm::translate(model,
			glm::vec3(-103.18f + v_pos, -2.5f, -94.5f));
		// Giro en Z según dirección
		model = glm::rotate(model, glm::radians(v_rot), glm::vec3(0.0f, 0.0f, 1.0f));
		// Mantener orientación original
		model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.38f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Voltorb.Draw(lightingShader);



		//PC
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-94.92f, -2.17f, -108.5f)); // ← fija, frente a cámara
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.42f, 0.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		PC.Draw(lightingShader);


		//PantallaPC
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-94.92f, -2.17f, -108.5f)); // ← fija, frente a cámara
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.42f, 0.4f));

		// Pantalla PC con shader

		AnimPC.Use();

		// matrices
		glUniformMatrix4fv(glGetUniformLocation(AnimPC.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(AnimPC.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(AnimPC.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// tiempo para la animación
		float t_pc = anim4_on ? (float)glfwGetTime() : t_hold_animPC;
		glUniform1f(glGetUniformLocation(AnimPC.Program, "uTime"), t_pc);
		// textura
		glActiveTexture(GL_TEXTURE0);
		PantallaPC.Draw(AnimPC);
		// restaurar
		lightingShader.Use();



		// Foco 
		{
			glm::mat4 modelFoco = glm::mat4(1.0f);
			modelFoco = glm::translate(modelFoco, glm::vec3(-94.92f, -2.17f, -108.5f));
			modelFoco = glm::rotate(modelFoco, glm::radians(-90.0f), glm::vec3(0, 1, 0));
			modelFoco = glm::scale(modelFoco, glm::vec3(0.4f, 0.42f, 0.4f));

			Anim4.Use();
			glUniform1i(glGetUniformLocation(Anim4.Program, "uUseRadial"), 0);
			glUniformMatrix4fv(glGetUniformLocation(Anim4.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(Anim4.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(Anim4.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelFoco));
			float t_foco = anim5_on ? (float)glfwGetTime() : t_hold_foco;
			glUniform1f(glGetUniformLocation(Anim4.Program, "uTime"), t_foco);


			Foco.Draw(Anim4);

			// restaurar para lo siguiente
			lightingShader.Use();
			
		}



		// Dibujar las 6 pokébolas
		for (int i = 0; i < NUM_BALLS; ++i) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, balls[i].pos);			// Posición animada (salto)

			// Rotación en Y con tu multiplicador 3.5
			model = glm::rotate(model, glm::radians(balls[i].spin * 3.5f),
				glm::vec3(0.0f, 1.0f, 0.0f));

			model = glm::scale(model, glm::vec3(0.4f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Poke.Draw(lightingShader);
		}

		// restaurar estado
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		glBindVertexArray(0);

		//Cubos de luz 
		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		//model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		glBindVertexArray(lightVAO);
		for (GLuint i = 0; i < 4; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);


		// Draw skybox as last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		SkyBoxshader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default




		// Swap the screen buffers
		glfwSwapBuffers(window);
	}




	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();




	return 0;
}


void animacion()
{
	Ball& b = balls[activeIdx];

	// avanzar tiempo
	b.t += deltaTime;
	float t = b.t;

	// parabólica que regresa al suelo en JUMP_T (con altura definida por V0_SEQ y GRAVY)
	float y0 = groundY;
	// Integra con misma forma que usas en proyectos base
	b.pos.y = y0 + V0_SEQ * t + 0.5f * GRAVY * t * t;
	if (b.pos.y < y0) b.pos.y = y0;

	// giro en Y (roll visual)
	b.spin += SPIN_DEG_PER_SEC * deltaTime;

	// fin de salto por bola
	if (t >= JUMP_T) {
		b.t = 0.0f;
		b.spin = 0.0f;
		b.pos = b.base;
		activeIdx = (activeIdx + 1) % NUM_BALLS;
	}
}




// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	


	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	// Encendido/Apagado por número (solo al presionar)
	if (action == GLFW_PRESS) {
		switch (key) {
			// Animación 1: Voltorb
		case GLFW_KEY_1: anim1_on = true;  break;
		case GLFW_KEY_2: anim1_on = false; break;

			// Animación 2: Secuenciador Pokébolas
		case GLFW_KEY_3: anim2_on = true;  break;
		case GLFW_KEY_4: anim2_on = false; break;

			// Animación 3: TV (Anim3)
		case GLFW_KEY_5: anim3_on = true;  break;
		case GLFW_KEY_6: anim3_on = false; break;

			// Animación 4: Pantalla PC (AnimPC)
		case GLFW_KEY_7: anim4_on = true;  break;
		case GLFW_KEY_8: anim4_on = false; break;

			// Animación 5: Foco (Anim4)
		case GLFW_KEY_9: anim5_on = true;  break;
		case GLFW_KEY_0: anim5_on = false; break;
		}
	}

	// Encendido/Apagado del secuenciador con 3/4 (solo al presionar)
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_3) anim2_on = true;   // Encender
		if (key == GLFW_KEY_4) anim2_on = false;  // Apagar
	}
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) { keys[key] = true; }
		else if (action == GLFW_RELEASE) { keys[key] = false; }
	}



	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
			LightP1 = glm::vec3(1.0f, 0.0f, 0.0f);
		else
			LightP1 = glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{

	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

// Moves/alters the camera positions based on user input
void DoMovement()
{

	
	



	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}






}


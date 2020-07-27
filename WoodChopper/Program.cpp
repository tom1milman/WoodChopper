//Include imports
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>

using namespace sf;

//Enum - player/branch location
enum class side { LEFT, RIGHT, NONE };

//Function declaration
int SetUpSound(Sound& sound, SoundBuffer& soundBuffer, String filePath);
void StartGame(bool& paused, int& score, float& timeRemaining, Sprite& ripSprite, Sprite& playerSprite, bool& acceptInput);
void OutOfTime(bool& paused, Text& messageText, Sound& outOfTime);
void MovePlayer(bool right, side& playerSide, int& score, float& timeRemaining, Sprite& axeSprite, Sprite& playerSprite, Sprite& logSprite, float& logSpeedX, bool& logActive, bool& acceptInput, Sound& chop);
void MoveBee(Sprite& beeSprite, bool& beeActive, float& beeSpeed, Time dt);
void MoveClouds(Time dt);
void UpdateBranches(int seed);
void MoveBranches();
void MoveFlyingLog(Sprite& logSprite, bool& logActive, float logSpeedX, float logSpeedY, Time dt);
void GameOver(Sprite& playerSprite, Sprite& ripSprite, Text& messageText, Sound& death, bool& paused, bool& acceptInput);

//Global variables
RenderWindow window;

const int GAME_VOLUME = 4;

const float AXE_POSITION_LEFT = 700;
const float AXE_POSITION_RIGHT = 1075;

const int NUM_CLOUDS = 3;
Sprite clouds[NUM_CLOUDS];
bool cloudActive[NUM_CLOUDS];
float cloudSpeeds[NUM_CLOUDS];

const int NUM_BRANCHES = 6;
Sprite branches[NUM_BRANCHES];
side branchPositions[NUM_BRANCHES];

int main()
{
	//Create video mode object
	VideoMode vm(1920, 1080);

	//Create + open game window
	RenderWindow window(vm, "Wood Chopper!", Style::Fullscreen);

	//Create and load background texture
	Texture backgroundTexture;
	if (backgroundTexture.loadFromFile("graphics/background.png") == 0) {
		return 1;
	}

	//Set backgorund sprite
	Sprite backgroundSprite;
	backgroundSprite.setTexture(backgroundTexture);
	backgroundSprite.setPosition(0, 0);

	//Set tree texture + sprite
	Texture treeTexture;
	treeTexture.loadFromFile("graphics/tree.png");
	Sprite treeSprite;
	treeSprite.setTexture(treeTexture);
	treeSprite.setPosition(810, 0);

	//Bee set up
	Texture beeTexture;
	beeTexture.loadFromFile("graphics/bee.png");
	Sprite beeSprite;
	beeSprite.setTexture(beeTexture);
	beeSprite.setPosition(0, 800);
	bool beeActive = false;
	float beeSpeed = 0.0f;

	//Clouds set up
	Texture cloudTexture;
	cloudTexture.loadFromFile("graphics/cloud.png");

	for (int i = 0; i < NUM_CLOUDS; i++) 
	{
		clouds[i].setTexture(cloudTexture);
		clouds[i].setPosition(50, i * 250);
		cloudActive[i] = false;
		cloudSpeeds[i] = 0.0f;
	}

	//Time control
	Clock clock;

	//Time bar
	RectangleShape timeBar;
	float timeBarStartWidth = 400;
	float timeBarHeight = 80;
	timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
	timeBar.setFillColor(Color::Red);
	timeBar.setPosition((1920 / 2) - timeBarStartWidth / 2, 980);

	Time gameTimeTotal;
	float timeRemaining = 6.0f;
	float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

	bool paused = true;

	//Score
	int score = 0;
	sf::Text messageText;
	sf::Text scoreText;

	Font font;
	font.loadFromFile("fonts/KOMIKAP_.ttf");

	//Assign font
	messageText.setFont(font);
	scoreText.setFont(font);

	//Assign messages
	messageText.setString("Press enter to start");
	scoreText.setString("Score = 0");

	//Assign size
	messageText.setCharacterSize(75);
	scoreText.setCharacterSize(90);

	//Assign color
	messageText.setFillColor(Color::White);
	scoreText.setFillColor(Color::White);

	//Position Text
	FloatRect textRect = messageText.getLocalBounds();
	messageText.setOrigin(
		textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);
	messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

	scoreText.setPosition(20, 20);

	//Prepare branches
	Texture branchTexture;
	branchTexture.loadFromFile("graphics/branch.png");

	for (int i = 0; i < NUM_BRANCHES; i++)
	{
		branches[i].setTexture(branchTexture);
		branches[i].setPosition(-2000, -2000);
		branches[i].setOrigin(220, 20);
	}

	//Prepare Player
	Texture playerTexture;
	playerTexture.loadFromFile("graphics/player.png");
	
	Sprite playerSprite;
	playerSprite.setTexture(playerTexture);
	playerSprite.setPosition(580, 720);

	side playerSide = side::LEFT;

	//Gravestone
	Texture ripTexture;
	ripTexture.loadFromFile("graphics/rip.png");
	
	Sprite ripSprite;
	ripSprite.setTexture(ripTexture);
	ripSprite.setPosition(600, 860);

	//Axe
	Texture axeTexture;
	axeTexture.loadFromFile("graphics/axe.png");
	Sprite axeSprite;
	axeSprite.setTexture(axeTexture);
	axeSprite.setPosition(700, 830);

	//Flying log
	Texture logTexture;
	logTexture.loadFromFile("graphics/log.png");
	Sprite logSprite;
	logSprite.setTexture(logTexture);
	logSprite.setPosition(810, 720);

	bool logActive = false;
	float logSpeedX = 1000;
	float logSpeedY = -1500;

	//Controls player input
	bool acceptInput = false;

	//Prepare sounds
	SoundBuffer chopBuffer;
	Sound chop;
	SetUpSound(chop, chopBuffer, "sound/chop.wav");

	SoundBuffer ootBuffer;
	Sound outOfTime;
	SetUpSound(outOfTime, ootBuffer, "sound/out_of_time.wav");

	SoundBuffer deathBuffer;
	Sound death;
	SetUpSound(death, deathBuffer, "sound/death.wav");

	while (window.isOpen()) {
		/*
			--- Player Input ---
		*/
#pragma region Input

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyReleased && !paused) 
			{
				acceptInput = true;

				axeSprite.setPosition(2000, axeSprite.getPosition().y);
			}
		}

		//Close game
		if (Keyboard::isKeyPressed(Keyboard::Escape)) 
			window.close();	

		//Start game
		if (Keyboard::isKeyPressed(Keyboard::Enter))		
			StartGame(paused, score, timeRemaining, ripSprite, playerSprite, acceptInput);

		//gameplay input
		if (acceptInput)
		{
			if (Keyboard::isKeyPressed(Keyboard::Right))
				MovePlayer(true, playerSide, score, timeRemaining, axeSprite, playerSprite, logSprite, logSpeedX, logActive, acceptInput, chop);

			if (Keyboard::isKeyPressed(Keyboard::Left))
				MovePlayer(false, playerSide, score, timeRemaining, axeSprite, playerSprite, logSprite, logSpeedX, logActive, acceptInput, chop);
		}

#pragma endregion


#pragma region Scene update
		if (!paused)
		{

			Time dt = clock.restart();

			//Time update
			timeRemaining -= dt.asSeconds();
			timeBar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));

			//Out of time
			if (timeRemaining <= 0.0f)
				OutOfTime(paused, messageText, outOfTime);


			//Score update
			std::stringstream ss;
			ss << "Score = " << score;
			scoreText.setString(ss.str());

			MoveBee(beeSprite, beeActive, beeSpeed, dt);
			MoveClouds(dt);
			MoveBranches();
			MoveFlyingLog(logSprite, logActive, logSpeedX, logSpeedY, dt);

			//Checks for player-branch collision
			if (branchPositions[5] == playerSide)
				GameOver(playerSprite, ripSprite, messageText, death, paused, acceptInput);

		} // End if (!paused)
#pragma endregion

#pragma region Draw scene
		//clears last frame
		window.clear();

		//Draw background
		window.draw(backgroundSprite);

		//Draw clouds
		for (int i = 0; i < NUM_CLOUDS; i++)
			window.draw(clouds[i]);

		//draw branches
		for (int i = 0; i < NUM_BRANCHES; i++)
		{
			window.draw(branches[i]);
		}

		//Draw tree
		window.draw(treeSprite);

		//Draw Player
		window.draw(playerSprite);
		
		//Draw axe
		window.draw(axeSprite);
		
		//draw flying log
		window.draw(logSprite);

		//draw gravestone
		window.draw(ripSprite);

		//Draw bees
		window.draw(beeSprite);

		//Draw score
		window.draw(scoreText);

		//Draw timebar
		window.draw(timeBar);

		if (paused) 
		{
			window.draw(messageText);
		}

		//Draws frame
		window.display();
#pragma endregion

	}

	return 0;
}

//Sets sound settings
int SetUpSound(Sound& sound, SoundBuffer& soundBuffer, String filePath)
{
	if (!soundBuffer.loadFromFile(filePath))
		return 1;

	sound.setBuffer(soundBuffer);
	sound.setVolume(GAME_VOLUME);

	return 0;
}

void StartGame(bool& paused, int& score, float& timeRemaining, Sprite& ripSprite, Sprite& playerSprite, bool& acceptInput)
{
	paused = false;

	score = 0;
	timeRemaining = 6;

	//preset branches
	for (int i = 1; i < NUM_BRANCHES; i++)
	{
		branchPositions[i] = side::NONE;
	}

	//hides gravestone
	ripSprite.setPosition(675, 2000);

	//places player
	playerSprite.setPosition(580, 720);
	acceptInput = true;
}

void MovePlayer(bool right, side& playerSide, int& score, float& timeRemaining, Sprite& axeSprite, Sprite& playerSprite, Sprite& logSprite, float& logSpeedX, bool& logActive, bool& acceptInput, Sound& chop)
{

	if (right) 
	{
		playerSide = side::RIGHT;
		axeSprite.setPosition(AXE_POSITION_RIGHT, axeSprite.getPosition().y);
		logSprite.setPosition(810, 720);
		playerSprite.setPosition(1200, 720);
		logSpeedX = -5000;

	}
	else 
	{
		playerSide = side::LEFT;
		axeSprite.setPosition(AXE_POSITION_LEFT, axeSprite.getPosition().y);
		playerSprite.setPosition(580, 720);
		logSprite.setPosition(810, 720);
		logSpeedX = 5000;

	}

	score++;

	timeRemaining += (2 / score) + 0.15;

	UpdateBranches(score);

	logActive = true;

	acceptInput = false;

	//play sound
	chop.play();
}

void OutOfTime(bool& paused, Text& messageText, Sound& outOfTime)
{
	paused = true;

	messageText.setString("Out of time");

	FloatRect textRect = messageText.getLocalBounds();
	messageText.setOrigin(textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);

	messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

	outOfTime.play();
}

//Sets and moves the bee
void MoveBee(Sprite& beeSprite, bool& beeActive, float& beeSpeed, Time dt)
{
	if (!beeActive)
	{
		//bee speed
		srand((int)time(0) * 10);
		beeSpeed = (rand() % 200) + 200;

		//Bee hieght
		srand((int)time(0) * 10);
		float height = (rand() % 500) + 500;
		beeSprite.setPosition(2000, height);
		beeActive = true;
	}
	else //Moving bee 
	{
		beeSprite.setPosition(
			beeSprite.getPosition().x - (beeSpeed * dt.asSeconds()),
			beeSprite.getPosition().y);

		//Did bee reached right edge of screen
		if (beeSprite.getPosition().x < -100)
			beeActive = false;
	}
}

//Sets and moves clouds
void MoveClouds(Time dt) 
{
	for (int i = 0; i < NUM_CLOUDS; i++)
	{
		if (!cloudActive[i])
		{
			int randomSeedMultiplier = (i + 1) * 10;
			int heightModifier = i + 1;

			//cloud speed
			srand((int)time(0) * randomSeedMultiplier);
			cloudSpeeds[i] = (rand() % 100) + 5;

			//Cloud hieght
			srand((int)time(0) * randomSeedMultiplier);
			float height = (rand() % (heightModifier * 150));

			if (heightModifier > 1)
				height -= 150;

			clouds[i].setPosition(-200, height);
			cloudActive[i] = true;
		}
		else //Moving cloud
		{
			clouds[i].setPosition(
				clouds[i].getPosition().x + (cloudSpeeds[i] * dt.asSeconds()),
				clouds[i].getPosition().y);

			//Did bee reached right edge of screen
			if (clouds[i].getPosition().x > 1920)
				cloudActive[i] = false;
		}
	}
}

void UpdateBranches(int seed) 
{
	for (int j = NUM_BRANCHES - 1; j > 0; j--)
	{
		branchPositions[j] = branchPositions[j - 1];
	}

	//Spawn new branch
	srand((int)time(0) + seed);
	int r = (rand() % 5);

	switch (r)
	{
	case 0:
		branchPositions[0] = side::LEFT;
		break;
	case 1:
		branchPositions[0] = side::RIGHT;
		break;
	default:
		branchPositions[0] = side::NONE;
		break;
	}
}

//Moves the braches down
void MoveBranches() 
{
	for (int i = 0; i < NUM_BRANCHES; i++)
	{
		float height = i * 150;

		if (branchPositions[i] == side::LEFT)
		{
			branches[i].setPosition(610, height);
			branches[i].setRotation(180);
		}

		else if (branchPositions[i] == side::RIGHT)
		{
			branches[i].setPosition(1330, height);
			branches[i].setRotation(0);
		}
		else
			branches[i].setPosition(3000, height);
	}
}

//Moves the flying log accross the screen
void MoveFlyingLog(Sprite& logSprite, bool& logActive, float logSpeedX, float logSpeedY, Time dt)
{
	if (logActive)
	{
		logSprite.setPosition(
			logSprite.getPosition().x + (logSpeedX * dt.asSeconds()),
			logSprite.getPosition().y + (logSpeedY * dt.asSeconds())
		);

		//Is log
		if (logSprite.getPosition().x < -100 || logSprite.getPosition().x > 2000)
		{
			logActive = false;
			logSprite.setPosition(810, 720);
		}

	}
}

//finishes the game
void GameOver(Sprite& playerSprite, Sprite& ripSprite, Text& messageText, Sound& death, bool& paused, bool& acceptInput)
{
	paused = true;
	acceptInput = false;

	ripSprite.setPosition(525, 760);
	playerSprite.setPosition(2000, 660);

	messageText.setString("SQUISHED");

	FloatRect textRect = messageText.getLocalBounds();
	messageText.setOrigin(
		textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);

	messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

	death.play();
}
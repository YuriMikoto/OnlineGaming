/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_net, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <stdio.h>
#include <string>
#include <iostream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const unsigned short PORT = 1234;
const unsigned short BUFFER_SIZE = 512;
const unsigned short MAX_SOCKETS = 3;
const unsigned short MAX_CLIENTS = MAX_SOCKETS - 1;

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};
//The dot that will move around on the screen
class Dot
{
public:
	//The dimensions of the dot
	static const int DOT_WIDTH = 20;
	static const int DOT_HEIGHT = 20;

	//Maximum axis velocity of the dot
	static const int DOT_VEL = 2;

	//Used to determine which player this dot is. Player 1 (host) is drawn in red; Player 2 (guest) is drawn in blue.
	int m_playerNum; 

	//Initializes the variables
	Dot(int playerNum);

	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);

	bool handleCollision(Dot other);

	//Moves the dot
	void move();

	//Shows the dot on the screen
	void render();

	int getX();
	int getY();

	void setPosition(int x, int y);

private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;

	//The velocity of the dot
	int mVelX, mVelY;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture redDotTexture;
LTexture blueDotTexture;
LTexture greenDotTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}


	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}


Dot::Dot(int playerNum)
{
	m_playerNum = playerNum;

	if (m_playerNum == 1) 
	{//Initialize starting positions.
		mPosX = 0;
		mPosY = 0;
	}
	else if (m_playerNum == 2)
	{//Each player starts in a different position.
		mPosX = (SCREEN_WIDTH / 3) - DOT_WIDTH;
		mPosY = (SCREEN_HEIGHT / 3) - DOT_HEIGHT;
	}
	else if (m_playerNum == 3)
	{
		mPosX = (SCREEN_WIDTH * 2 / 3) - DOT_WIDTH;
		mPosY = (SCREEN_HEIGHT * 2 / 3) - DOT_HEIGHT;
	}

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= DOT_VEL; break;
		case SDLK_DOWN: mVelY += DOT_VEL; break;
		case SDLK_LEFT: mVelX -= DOT_VEL; break;
		case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += DOT_VEL; break;
		case SDLK_DOWN: mVelY -= DOT_VEL; break;
		case SDLK_LEFT: mVelX += DOT_VEL; break;
		case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

bool Dot::handleCollision(Dot other)
{
	//Use the distance formula to calculate the distance between the two circles.
	int distance = sqrt(((other.getX() - mPosX) * (other.getX() - mPosX)) + ((other.getY() - mPosY) * (other.getY() - mPosY)));

	if (distance <= DOT_WIDTH)
	{//If they're close enough, respond. Otherwise, do nothing.
		std::cout << "COLLIDING ";
		return true;
	}
	return false;
}

void Dot::move()
{
	//Move the dot left or right
	mPosX += mVelX;

	//If the dot moved off the left side.
	if ((mPosX < -DOT_WIDTH))
	{
		//Wrap to right.
		mPosX = SCREEN_WIDTH;
	}
	//If the dot moved off the right side.
	else if (mPosX > SCREEN_WIDTH)
	{
		//Wrap to left.
		mPosX = -DOT_WIDTH;
	}

	//Move the dot up or down
	mPosY += mVelY;

	//If the dot moved off the left side.
	if (mPosY < -DOT_HEIGHT)
	{
		//Wrap to right.
		mPosY = SCREEN_HEIGHT;
	}
	//If the dot moved off the right side.
	else if (mPosY > SCREEN_HEIGHT)
	{
		//Wrap to left.
		mPosY = -DOT_HEIGHT;
	}
}

void Dot::render()
{
	//Show the dot. Player 1's is red, 2's is blue, 3's is green. Determine which is which, then draw.
	if (m_playerNum == 1)
		redDotTexture.render(mPosX, mPosY);
	else if (m_playerNum == 2)
		blueDotTexture.render(mPosX, mPosY);
	else if (m_playerNum == 3)
		greenDotTexture.render(mPosX, mPosY);
}

int Dot::getX()
{
	return mPosX;
}

int Dot::getY()
{
	return mPosY;
}

void Dot::setPosition(int x, int y)
{
	mPosX = x;
	mPosY = y;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		SDLNet_Init();
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load dot textures
	if (!redDotTexture.loadFromFile("reddot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}
	if (!blueDotTexture.loadFromFile("bluedot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}
	if (!greenDotTexture.loadFromFile("greendot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	redDotTexture.free();
	blueDotTexture.free();
	greenDotTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{	
			IPaddress ip;
			TCPsocket sock;
			SDLNet_SocketSet socks = SDLNet_AllocSocketSet(MAX_SOCKETS);
			Uint16 port;
			char buffer[BUFFER_SIZE];
			int received = 0;

			int playerID;
			bool gameState = false; //Determines whether or not the game itself should run.
			//False at program opening; game cannot begin until enough players have joined.
			//Becomes true when three players enter the game. At this point, the players can control their dot.
			//Becomes false again after triggering a win condition. All control freezes, winner is displayed.

			//Main loop flag
			bool quit = false;

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//// Note to self: Set this project up as client. See C:\Users\gameuser\Documents\Projects (OG)\OnlineGaming\Moving.Dot.SDL\Moving.Dot.SDL.Dot\Del\S E R V E R\Del for server.
			/// Objective 1-1: COMPLETE
			///   Get this client to receive the message the server sends out the moment a connection is formed. This gives us the player ID.
			///   - This is used to determine which dot to control. Change "playerID" to this value. 
			/// Objective 2-1: COMPLETE
			///   Every time this player dot moves, send a message to the server stating that this dot has moved, which will send this information to the other client.
			/// Objective 2-2: COMPLETE
			///   Every update, check for a message that states the other player has moved. On receipt, change that dot's position on this client's side.
			/// Objective 3-1: NOT DONE
			///   Every move, check to see if the two players have collided. If so, P1 wins. If not, increment a timer; once timer's value reaches a preset max, P2 wins.
			///   - The collision check is already complete, all that's needed now is to correct the response using data sending to let the other player know. 
			/// 
			/// NEXT STEP: Project Version
			/// Objective 4-1: COMPLETE
			///	  Add a third player. Make this one green.
			/// Objective 4-2: COMPLETE
			///   Implement collision response.
			///   - P1 is pursued by P2 and P3, who work together. If either make contact with P1, P2/3 win. If P2 and P3 collide, nothing happens. If P1 is not caught in a certain time, P1 wins.
			/// Objective 4-3: --
			///   Polish. Make the game look a little better.
			/// Objective 4-4: --
			///   Ensure the game works on multiple computers.
			// TEST: Run server indicated before, then CTRL+F5 this solution, once for each player. One client may automatically close; if this happens, close everything and start over.
			// Working on multiple computers is not tested, but in theory, changing the IP address below to your server computer's IP should allow this one to connect to it. Hopefully?
			
			int please = SDLNet_ResolveHost(&ip, "149.153.106.167", 1234);
			sock = SDLNet_TCP_Open(&ip); 
			SDLNet_TCP_AddSocket(socks, sock);
			SDLNet_TCP_Recv(sock, buffer, BUFFER_SIZE); //If you get an access violation error here, set up the server first.
			sscanf_s(buffer, "0 %d", &playerID);

			//Event handler
			SDL_Event e;

			//The dots that will be moving around on the screen
			Dot player1(1);
			Dot player2(2);
			Dot player3(3);

			//If Player 1 survives for a full minute, they win.
			int timer = 0;
			const int ENDGAME_TIME = 1800;

			//While application is running
			while (!quit)
			{
				//Receive and interpret messages; move the other player's dot according to this.
				while (SDLNet_CheckSockets(socks, 0) > 0)
				{
					if (SDLNet_SocketReady(sock)) {
						memset(buffer, 0, sizeof(buffer)); //Clear the buffer first before receiving.
						SDLNet_TCP_Recv(sock, buffer, BUFFER_SIZE); 
						int num = buffer[0] - '0';

						if (num == 1)
						{

							std::cout << buffer << std::endl;

							int otherID;
							int newX;
							int newY;
							sscanf_s(buffer, "1 %d %d %d", &otherID, &newX, &newY);
							std::cout << "(" << newX << ", " << newY << ")" << std::endl;
							if (otherID == 1)
							{
								player1.setPosition(newX, newY);
							}
							else if (otherID == 2)
							{
								player2.setPosition(newX, newY);
							}
							else if (otherID == 3)
							{
								player3.setPosition(newX, newY);
							}

							std::cout << "P1: (" << player1.getX() << ", " << player1.getY() << ")" << std::endl <<
										 "P2: (" << player2.getX() << ", " << player2.getY() << ")" << std::endl <<
										 "P3: (" << player3.getX() << ", " << player3.getY() << ")" << std::endl <<
										 "Time: " << timer << std::endl;

						}

						if (num == 3)
						{
							std::cout << buffer << std::endl;
							int winner;
							sscanf_s(buffer, "3 %d", &winner);

							if (playerID == 1 && winner == 1)
							{//You as Player 1 have won.
								std::cout << "YOU ALONE HAVE WON" << std::endl;
							}
							else if ((playerID == 2 || playerID == 3) && winner == 2)
							{//You as Player 2 or 3 have won.
								std::cout << "YOUR TEAM HAS WON" << std::endl;
							}
							else
							{//You, regardless of player, have lost.
								std::cout << "YOU HAVE LOST" << std::endl;
							}
							gameState = false;
						}

						if (num == 4)
						{//Command to start game has been received.
							gameState = true;
							std::cout << "START GAME" << std::endl;
						}
					}
				}

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
					{
						quit = true;
					}

					if (gameState)
					{//Handle input for the dot only if the game is in progress.
						if (playerID == 1)
						{
							player1.handleEvent(e);
						}
						else if (playerID == 2)
						{
							player2.handleEvent(e);
						}
						else if (playerID == 3)
						{
							player3.handleEvent(e);
						}
					}
				}

				if (gameState)
				{//Move the dot and send its new location to the other player only if the game is in progress.
					player1.move();
					player2.move();
					player3.move();

					std::string msg;

					if (playerID == 1)
					{
						msg = "1 1 " + std::to_string(player1.getX()) + " " + std::to_string(player1.getY()) + '\0';
					}
					else if (playerID == 2)
					{
						msg = "1 2 " + std::to_string(player2.getX()) + " " + std::to_string(player2.getY()) + '\0';
					}
					else if (playerID == 3)
					{
						msg = "1 3 " + std::to_string(player3.getX()) + " " + std::to_string(player3.getY()) + '\0';
					}
					memcpy(buffer, msg.c_str(), msg.size());
					SDLNet_TCP_Send(sock, buffer, strlen(buffer) + 1);

					if (player1.handleCollision(player2) || player1.handleCollision(player3))
					{//Player 1 has been caught by either Player 2 or Player 3. It doesn't matter which; Player 1 loses, and the other two win as a team.
						msg = "3 2" + '\0';
						memcpy(buffer, msg.c_str(), msg.size());
						SDLNet_TCP_Send(sock, buffer, strlen(buffer) + 1);
					}

					//Increment game timer and check for game end.
					timer++;

					if (timer >= ENDGAME_TIME)
					{//Game time has elapsed, and Player 1 has eluded the others. Player 1 wins, and the other two lose as a team.
						msg = "3 1" + '\0';
						memcpy(buffer, msg.c_str(), msg.size());
						SDLNet_TCP_Send(sock, buffer, strlen(buffer) + 1);
					}

				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render objects
				player1.render();
				player2.render();
				player3.render();

				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	SDLNet_Quit();
	close();

	return 0;
}
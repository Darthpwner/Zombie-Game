// zombies.cpp

#include <iostream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cassert>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;              // max number of rows in the arena
const int MAXCOLS = 20;              // max number of columns in the arena
const int MAXZOMBIES = 100;          // max number of zombies allowed

const int NORTH = 0;
const int EAST  = 1;
const int SOUTH = 2;
const int WEST  = 3;
const int NUMDIRS = 4;

const int EMPTY      = 0;
const int HAS_BRAIN  = 1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
              // type name, since it's mentioned in the Zombie declaration.

class Zombie
{
  public:
      // Constructor
    Zombie(Arena* ap, int r, int c);

      // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

      // Mutators
    void move();

  private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
	int	   m_affected;			//For zombies that ate poisoned brains
	int	   m_brainCounter;		//Counts the amount of brains eaten
	bool   m_zDead;				//Returns if zombie is dead
};

class Player
{
  public:
      // Constructor
    Player(Arena *ap, int r, int c);

      // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

      // Mutators
    string dropBrain();
    string move(int dir);
    void   setDead();

  private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool   m_dead;
	Zombie* m_zombies[MAXZOMBIES];
	int		m_nZombies;
};

class Arena
{
  public:
      // Constructor/destructor
    Arena(int nRows, int nCols);
    ~Arena();

      // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     zombieCount() const;
    int     getCellStatus(int r, int c) const;
    int     numberOfZombiesAt(int r, int c) const;
    void    display(string msg) const;

      // Mutators
    void setCellStatus(int r, int c, int status);
    bool addZombie(int r, int c);
    bool addPlayer(int r, int c);
    void moveZombies();

  private:
    int     m_grid[MAXROWS][MAXCOLS];
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Zombie* m_zombies[MAXZOMBIES];
    int     m_nZombies;
    int     m_turns;

      // Helper functions
    void checkPos(int r, int c) const;
};

class Game
{
  public:
      // Constructor/destructor
    Game(int rows, int cols, int nZombies);
    ~Game();

      // Mutators
    void play();

  private:
    Arena* m_arena;

      // Helper functions
    string takePlayerTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int randInt(int lowest, int highest);
bool charToDir(char ch, int& dir);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Zombie implementation
///////////////////////////////////////////////////////////////////////////

Zombie::Zombie(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** A zombie must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "***** Zombie created with invalid coordinates (" << r << ","
             << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
	m_brainCounter = 0;		//Brain counter is initially 0
	m_zDead = false;			//Zombies are initially alive
}

int Zombie::row() const
{
    return m_row;
}

int Zombie::col() const
{
    return m_col;
}

bool Zombie::isDead() const
{
    return m_zDead;		
}

void Zombie::move()	
{	
	//For affected zombies
	if (m_affected % 2 == 1)	//Zombies that eat one brain move every other turn, so they can move on odd turns
	{
		m_affected++;
		int dir = randInt(0,3);		//global constants NORTH, EAST, SOUTH, WEST values
		attemptMove(*m_arena, dir, m_row, m_col);

	if(m_arena -> getCellStatus(row(), col() ) == HAS_BRAIN)	//Zombie eats one brain
		{
		m_arena -> setCellStatus(m_row, m_col, EMPTY);	//empties the coordinate if zombie eats the brain
		m_affected = 2;		//Start at 2 so that the zombie moves every other turn when using modulus 2
		m_brainCounter++;	//Counter for the brain to determine if a zombie dies
		}
	}
	
	else if (m_affected % 2 == 0)	//Zombies that eat one brain move every other turn, so they don't move on even turns
		m_affected++;

	else	//Normal zombie movement
	{
		int dir = randInt(0,3);		//global constants NORTH, EAST, SOUTH, WEST values
		attemptMove(*m_arena, dir, m_row, m_col);	

		if(m_arena -> getCellStatus(row(), col() ) == HAS_BRAIN)	//Zombie eats the brain
		{
			m_arena -> setCellStatus(m_row, m_col, EMPTY);	//empties the coordinate if zombie eats the brain
			m_affected = 2;		//Start at 2 so that the zombie moves every other turn when using modulus 2
			m_brainCounter++;	//Counter for the brain to determine if a zombie dies
		}
	}

	if (m_brainCounter == 2)	//zombie dies after eating 2 brains
		m_zDead = true; 
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** The player must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "**** Player created with invalid coordinates (" << r
             << "," << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_dead = false;
}

int Player::row() const
{
	return m_row;  
}

int Player::col() const
{
    return m_col; 
}

string Player::dropBrain()
{
    if (m_arena->getCellStatus(m_row, m_col) == HAS_BRAIN)
        return "There's already a brain at this spot.";
    m_arena->setCellStatus(m_row, m_col, HAS_BRAIN);
    return "A brain has been dropped.";
}

string Player::move(int dir)	
{
	bool callAttemptMove = attemptMove(*m_arena, dir, m_row, m_col);
	if (m_arena -> numberOfZombiesAt(m_row, m_col) > 0)		//Player will walk into a zombie and die
			{
				m_dead = true;
				return "Player walked into a zombie and died.";
			}
	else if (callAttemptMove == false)
		return "Player couldn't move; player stands.";

	else if (dir == NORTH) 
		return "Player moved north.";
	else if (dir == SOUTH) 
		return "Player moved south."; 
	else if (dir == WEST) 
		return "Player moved west."; 
	else if (dir == EAST) 
		return "Player moved east.";

	return "Player couldn't move; player stands.";	//So all control paths return a value
}

bool Player::isDead() const
{
	return m_dead;  
}

void Player::setDead()
{
    m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
    if (nRows <= 0  ||  nCols <= 0  ||  nRows > MAXROWS  ||  nCols > MAXCOLS)
    {
        cout << "***** Arena created with invalid size " << nRows << " by "
             << nCols << "!" << endl;
        exit(1);
    }
    m_rows = nRows;
    m_cols = nCols;
    m_player = nullptr;
    m_nZombies = 0;
    m_turns = 0;
    for (int r = 1; r <= m_rows; r++)
        for (int c = 1; c <= m_cols; c++)
            setCellStatus(r, c, EMPTY);
}

Arena::~Arena()		//Destructor releases the player and all remaining dynamically allocated zombies
{
    for (int a = 0; a != m_nZombies; a++)
		delete m_zombies[a];
		delete m_player;
}

int Arena::rows() const
{
    return m_rows;  
}

int Arena::cols() const
{
    return m_cols;  
}

Player* Arena::player() const
{
    return m_player;
}

int Arena::zombieCount() const
{
	return m_nZombies; 
}

int Arena::getCellStatus(int r, int c) const
{
    checkPos(r, c);
    return m_grid[r-1][c-1];
}

int Arena::numberOfZombiesAt(int r, int c) const
{
	int numOfZombies = 0;	//Counts number of zombies at each coordinate point (row r, column c)
	for (int k = 0; k != m_nZombies; k++)
	if (r == (m_zombies[k] -> row()) && c == (m_zombies[k] -> col()) ) //Checks if the r and c correspond to the same coordinate point
		numOfZombies++;
	return numOfZombies;	//return number of zombies at each coordinate point (row r, column c)
}

void Arena::display(string msg) const
{
    char displayGrid[MAXROWS][MAXCOLS];
    int r, c;
      // Fill displayGrid with dots (empty) and stars (brains)
    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++)
            displayGrid[r-1][c-1] = (getCellStatus(r,c) == EMPTY ? '.' : '*');
	
      // Indicate each zombie's position
	for (int r = 1; r <= rows(); r++)
	{
		for (int c = 1; c <= cols(); c++)	
		{
			if (Arena::numberOfZombiesAt(r, c) == 0)		//Display grid is not affected if there are no zombies at that position
				;
			else if (Arena::numberOfZombiesAt(r, c) == 1)
				displayGrid[r-1][c-1] = 'Z';
			else if (Arena::numberOfZombiesAt(r, c) == 2)
				displayGrid[r-1][c-1] = '2';
			else if (Arena::numberOfZombiesAt(r, c) == 3)
				displayGrid[r-1][c-1] = '3';
			else if (Arena::numberOfZombiesAt(r, c) == 4)
				displayGrid[r-1][c-1] = '4';
			else if (Arena::numberOfZombiesAt(r, c) == 5)
				displayGrid[r-1][c-1] = '5';
			else if (Arena::numberOfZombiesAt(r, c) == 6)
				displayGrid[r-1][c-1] = '6';
			else if (Arena::numberOfZombiesAt(r, c) == 7)
				displayGrid[r-1][c-1] = '7';
			else if (Arena::numberOfZombiesAt(r, c) == 8)
				displayGrid[r-1][c-1] = '8';
			else 
				displayGrid[r-1][c-1] = '9';
		}
	}

      // Indicate player's position
    if (m_player != nullptr)
        displayGrid[m_player->row()-1][m_player->col()-1] = (m_player->isDead() ? 'X' : '@');

      // Draw the grid
    clearScreen();
    for (r = 1; r <= rows(); r++)
    {
        for (c = 1; c <= cols(); c++)
            cout << displayGrid[r-1][c-1];
        cout << endl;
    }
    cout << endl;

      // Write message, zombie, and player info
    if (msg != "")
        cout << msg << endl;
    cout << "There are " << zombieCount() << " zombies remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player!" << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
    cout << m_turns << " turns have been taken." << endl;
}

void Arena::setCellStatus(int r, int c, int status)
{
    checkPos(r, c);
    m_grid[r-1][c-1] = status;
}

bool Arena::addZombie(int r, int c)
{
    if (m_nZombies == MAXZOMBIES)
        return false;
    m_zombies[m_nZombies] = new Zombie(this, r, c);
    m_nZombies++;
    return true;
}

bool Arena::addPlayer(int r, int c)
{
      // Don't add a player if one already exists
    if (m_player != nullptr)
        return false;

      // Dynamically alocate a new Player and add it to the arena
    m_player = new Player(this, r, c);
    return true;
}

void Arena::moveZombies()
{
	// Move all zombies
	for (int a = 0; a < m_nZombies ; a++)
		{
			m_zombies[a] -> Zombie::move();		//Move each individual zombie in the game
			if (m_zombies[a] -> isDead() == true)
			{
				delete m_zombies[a];		//Dynamically release the zombie that died
				m_nZombies--;				//Decrement the total number of zombies to fit the array 
					for (int x = a; x < m_nZombies; x++)
						m_zombies[x] = m_zombies[x+1];		//Shifts each individual zombie back 1 position to replace the zombie that died.
			}
		}

	for (int b = 0; b < m_nZombies; b++)
		{
			if (m_player -> row() == m_zombies[b] -> row() && m_player -> col() == m_zombies[b] -> col() )	//If player is in the same position as a zombie, player dies
			m_player -> setDead();
		}
	
      // Another turn has been taken
    m_turns++;
}

void Arena::checkPos(int r, int c) const
{
    if (r < 1  ||  r > m_rows  ||  c < 1  ||  c > m_cols)
    {
        cout << "***** " << "Invalid arena position (" << r << ","
             << c << ")" << endl;
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nZombies)
{
    if (nZombies < 0  ||  nZombies > MAXZOMBIES)
    {
        cout << "***** Game created with invalid number of zombies:  "
             << nZombies << endl;
        exit(1);
    }
    int nEmpty = rows * cols - nZombies - 1;  // 1 for Player
    if (nEmpty < 0)
    {
        cout << "***** Game created with a " << rows << " by "
             << cols << " arena, which is too small too hold a player and "
             << nZombies << " zombies!" << endl;
        exit(1);
    }

      // Create arena
    m_arena = new Arena(rows, cols);

      // Add player
    int rPlayer;
    int cPlayer;
    do
    {
        rPlayer = randInt(1, rows);
        cPlayer = randInt(1, cols);
    } while (m_arena->getCellStatus(rPlayer, cPlayer) != EMPTY);
    m_arena->addPlayer(rPlayer, cPlayer);

      // Populate with zombies
    while (nZombies > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (r == rPlayer && c == cPlayer)
            continue;
        m_arena->addZombie(r, c);
        nZombies--;
    }
}

Game::~Game()
{
    delete m_arena;
}

string Game::takePlayerTurn()
{
    for (;;)
    {
        cout << "Your move (n/e/s/w/x or nothing): ";
        string playerMove;
        getline(cin, playerMove);

        Player* player = m_arena->player();
        int dir;

        if (playerMove.size() == 0)
        {
            if (recommendMove(*m_arena, player->row(), player->col(), dir))
                return player->move(dir);
            else
                return player->dropBrain();
        }
        else if (playerMove.size() == 1)
        {
            if (tolower(playerMove[0]) == 'x')
                return player->dropBrain();
            else if (charToDir(playerMove[0], dir))
                return player->move(dir);
        }
        cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
    }
}

void Game::play()
{
    m_arena->display("");
    while ( ! m_arena->player()->isDead()  &&  m_arena->zombieCount() > 0)
    {
        string msg = takePlayerTurn();
        Player* player = m_arena->player();
        if (player->isDead())
            break;
        m_arena->moveZombies();
        m_arena->display(msg);
    }
    if (m_arena->player()->isDead())
        cout << "You lose." << endl;
    else
        cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementation
///////////////////////////////////////////////////////////////////////////

  // Return a uniformly distributed random int from lowest to highest, inclusive
int randInt(int lowest, int highest)
{
    if (highest < lowest)
        swap(highest, lowest);
    return lowest + (rand() % (highest - lowest + 1));
}

bool charToDir(char ch, int& dir)
{
    switch (tolower(ch))
    {
      default:  return false;
      case 'n': dir = NORTH; break;
      case 'e': dir = EAST;  break;
      case 's': dir = SOUTH; break;
      case 'w': dir = WEST;  break;
    }
    return true;
}

bool attemptMove(const Arena& a, int dir, int& r, int& c)	
{
	//Call attemptMove in Player::Move and Zombie::Move
	if (dir == NORTH) 
	{ 
		if (r == 1) 
			return false;
		else
		{
			r--; 
			return true;
		}
	} 
if (dir == SOUTH) 
	{ 
		if (r == a.rows() ) 
			return false;
		else
		{
			r++; 
			return true; 
		}
	} 
if (dir == WEST) 
	{ 
		if (c == 1) 
			return false; 
		else
		{
			c--; 
			return true; 
		}
	} 
 if (dir == EAST) 
	{ 
		if (c == a.cols() ) 
			return false;
		else
		{
			c++; 
			return true; 
		}
	}
 return false;	//So all control paths return a value
}

bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
	if (a.numberOfZombiesAt(r + 1, c) == 0 && a.numberOfZombiesAt(r - 1, c) == 0 && a.numberOfZombiesAt(r, c + 1) == 0 && a.numberOfZombiesAt(r, c - 1) == 0)	//Don't move if there are no zombies next to you
		return false;
	if (a.numberOfZombiesAt(r + 1, c) > 0 && a.numberOfZombiesAt(r - 1, c) > 0 && a.numberOfZombiesAt(r, c + 1) > 0 && a.numberOfZombiesAt(r, c - 1) > 0)	//Don't move if zombies surround you on all four positions
		return false;
	int safeNumberOfZombies = 100;	//assumes that the max number of zombies is in the game and that if you move, all 100 will surround the player
	int checkNumberOfZombies;	//Used to check number of zombies and if less than 100, reinitialize safeNumberOfZombies

	if (a.numberOfZombiesAt(r + 1, c) == 0 && r != a.rows() )	//If there are no zombies to the south and you aren't at the bottom of the Arena grid
	{
		checkNumberOfZombies = a.numberOfZombiesAt(r + 2, c) + a.numberOfZombiesAt(r + 1, c + 1) + a.numberOfZombiesAt(r + 1, c - 1);	//Checks number of zombies around player if player moves south
		if (checkNumberOfZombies < safeNumberOfZombies)
			{
				safeNumberOfZombies = checkNumberOfZombies;
				bestDir = SOUTH;
			}
	}

	if (a.numberOfZombiesAt(r - 1, c) == 0 && r != 1)	//If there are no zombies to the north and you aren't at the top of the Arena grid
	{
			checkNumberOfZombies = a.numberOfZombiesAt(r - 2, c) + a.numberOfZombiesAt(r - 1, c + 1) + a.numberOfZombiesAt(r - 1, c - 1);	//Checks number of zombies around player if player moves north
			if (checkNumberOfZombies < safeNumberOfZombies)
			{
				safeNumberOfZombies = checkNumberOfZombies;
				bestDir = NORTH;
			}
	}

	if (a.numberOfZombiesAt(r, c + 1) == 0 && c != a.cols() )	//If there are no zombies to the right and you aren't at the rightmost side of the Arena grid
	{
		checkNumberOfZombies = a.numberOfZombiesAt(r, c + 2) + a.numberOfZombiesAt(r + 1, c + 1) + a.numberOfZombiesAt(r - 1, c + 1);	//Checks number of zombies around player if player moves east
		if (checkNumberOfZombies < safeNumberOfZombies)
			{
				safeNumberOfZombies = checkNumberOfZombies;
				bestDir = EAST;
			}
	}

		if (a.numberOfZombiesAt(r, c - 1) == 0 && c != 1)	//If there are no zombies to the left and you aren't at the leftmost side of the Arena grid
		{
			checkNumberOfZombies = a.numberOfZombiesAt(r, c - 2) + a.numberOfZombiesAt(r + 1, c - 1) + a.numberOfZombiesAt(r - 1, c - 1);	//Checks number of zombies around player if player moves west
			if (checkNumberOfZombies < safeNumberOfZombies)
			{
				safeNumberOfZombies = checkNumberOfZombies;
				bestDir = WEST;
			}
		}
		if (safeNumberOfZombies < 100)	//Moves player if next to a zombie(s) AND in the direction with the least number of zombies surrounding the player after the player moves.
			return true;	//Execute the move if it is safe
		return false;	//So all control paths return a value
}
  
// DO NOT MODIFY THE CODE BETWEEN HERE AND THE MAIN ROUTINE
#ifdef _MSC_VER  //  Microsoft Visual C++

#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
                                                        &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not Microsoft Visual C++, so assume UNIX interface

#include <cstring>

void clearScreen()
{
    static const char* term = getenv("TERM");
    static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
    if (term == nullptr  ||  strcmp(term, "dumb") == 0)
        cout << endl;
     else
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
}

#endif

///////////////////////////////////////////////////////////////////////////
//	Basic Tests
///////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <cstdlib>

using namespace std;

void thisFunctionWillNeverBeCalled()
{
      // If the student deleted or changed the manifest constants, this
      // won't compile.

    const bool b1 = (MAXROWS == 20 && MAXCOLS == 20 && MAXZOMBIES == 100 &&
                     EMPTY == 0 && HAS_BRAIN == 1 &&
                     NORTH == 0 && EAST == 1 && SOUTH == 2 && WEST == 3);
    char a1[b1 ? 1 : -1];  // illegal negative size array if b1 is false

      // If the student deleted or changed the interfaces to the public
      // functions, this won't compile.  (This uses magic beyond the scope
      // of CS 31.)

    Zombie z(static_cast<Arena*>(0), 1, 1);
    int  (Zombie::*pz1)() const = &Zombie::row;
    int  (Zombie::*pz2)() const = &Zombie::col;
    bool (Zombie::*pz3)() const = &Zombie::isDead;
    void (Zombie::*pz4)()       = &Zombie::move;

    Player p(static_cast<Arena*>(0), 1, 1);
    int  (Player::*pp1)() const = &Player::row;
    int  (Player::*pp2)() const = &Player::col;
    bool (Player::*pp3)() const = &Player::isDead;
    string (Player::*pp4)()     = &Player::dropBrain;
    string (Player::*pp5)(int)  = &Player::move;
    void (Player::*pp6)()       = &Player::setDead;

    Arena a(1, 1);
    int  (Arena::*pa1)() const       = &Arena::rows;
    int  (Arena::*pa2)() const       = &Arena::cols;
    Player* (Arena::*pa3)() const    = &Arena::player;
    int (Arena::*pa4)() const        = &Arena::zombieCount;
    int (Arena::*pa5)(int,int) const = &Arena::getCellStatus;
    int (Arena::*pa6)(int,int) const = &Arena::numberOfZombiesAt;
    void (Arena::*pa7)(string) const = &Arena::display;
    void (Arena::*pa8)(int,int,int)  = &Arena::setCellStatus;
    bool (Arena::*pa9)(int,int)      = &Arena::addZombie;
    bool (Arena::*pa10)(int,int)     = &Arena::addPlayer;
    void (Arena::*pa11)()            = &Arena::moveZombies;

    Game g(1,1,1);
    void (Game::*pg1)() = &Game::play;
}

void findTheZombie(const Arena& a, int& r, int& c)
{
	if      (a.numberOfZombiesAt(r-1, c) == 1) r--;
	else if (a.numberOfZombiesAt(r+1, c) == 1) r++;
	else if (a.numberOfZombiesAt(r, c-1) == 1) c--;
	else if (a.numberOfZombiesAt(r, c+1) == 1) c++;
	else assert(false);
}

void doBasicTests()
{
    {
        Arena a(10, 20);
        a.addPlayer(2, 5);
        Player* pp = a.player();
        assert(pp->row() == 2  &&  pp->col() == 5  && ! pp->isDead());
        assert(pp->move(NORTH) == "Player moved north.");
        assert(pp->row() == 1  &&  pp->col() == 5  && ! pp->isDead());
        assert(pp->move(NORTH) == "Player couldn't move; player stands.");
        assert(pp->row() == 1  &&  pp->col() == 5  && ! pp->isDead());
        pp->setDead();
        assert(pp->row() == 1  &&  pp->col() == 5  && pp->isDead());
    }
    {
        Arena a(10, 20);
	int r = 4;
	int c = 4;
        a.setCellStatus(r-1, c, HAS_BRAIN);
        a.setCellStatus(r+1, c, HAS_BRAIN);
        a.setCellStatus(r, c-1, HAS_BRAIN);
        a.setCellStatus(r, c+1, HAS_BRAIN);
        a.addZombie(r, c);
        a.addPlayer(8, 18);
        assert(a.zombieCount() == 1  &&  a.numberOfZombiesAt(r, c) == 1);
	a.moveZombies();
        assert(a.zombieCount() == 1  &&  a.numberOfZombiesAt(r, c) == 0);
	findTheZombie(a, r, c);
	assert(a.getCellStatus(r, c) != HAS_BRAIN);
	a.moveZombies();
        assert(a.zombieCount() == 1  &&  a.numberOfZombiesAt(r, c) == 1);
	a.moveZombies();
        assert(a.zombieCount() == 1  &&  a.numberOfZombiesAt(r, c) == 0);
	findTheZombie(a, r, c);
	a.moveZombies();
        assert(a.zombieCount() == 1  &&  a.numberOfZombiesAt(r, c) == 1);
        a.setCellStatus(r-1, c, HAS_BRAIN);
        a.setCellStatus(r+1, c, HAS_BRAIN);
        a.setCellStatus(r, c-1, HAS_BRAIN);
        a.setCellStatus(r, c+1, HAS_BRAIN);
	a.moveZombies();
        assert(a.zombieCount() == 0  &&  a.numberOfZombiesAt(r, c) == 0);
	assert(a.numberOfZombiesAt(r-1, c) == 0);
	assert(a.numberOfZombiesAt(r+1, c) == 0);
	assert(a.numberOfZombiesAt(r, c-1) == 0);
	assert(a.numberOfZombiesAt(r, c+1) == 0);

        for (int k = 0; k < MAXZOMBIES/4; k++)
        {
            a.addZombie(7, 18);
            a.addZombie(9, 18);
            a.addZombie(8, 17);
            a.addZombie(8, 19);
        }
        assert(! a.player()->isDead());
        a.moveZombies();
        assert(a.player()->isDead());
    }
    cout << "Passed all basic tests" << endl;
    exit(0);
}

///////////////////////////////////////////////////////////////////////////
// main()
///////////////////////////////////////////////////////////////////////////

int main()
{
      // Initialize the random number generator
    srand(static_cast<unsigned int>(time(0)));

      // Create a game
      // Use this instead to create a mini-game:   Game g(3, 5, 2);
    Game g(10, 12, 40);
	//Game g(5, 5, 5);

 //     // Play the game
    g.play();
	/*doBasicTests();*/
}




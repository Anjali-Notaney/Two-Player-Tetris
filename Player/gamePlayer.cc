#include "Player.h"
#include "gamePlayer.h"
#include "../Blocks/OBlock.h"
#include "../Blocks/JBlock.h"
#include "../Blocks/IBlock.h"
#include "../Blocks/SBlock.h"
#include "../Blocks/ZBlock.h"
#include "../Blocks/TBlock.h"
#include "../Blocks/LBlock.h"
#include "../Level/Level.h"
#include "../Level/Level0.h"
#include "../Level/Level1.h"
#include "../Level/Level2.h"
#include "../Level/Level3.h"
#include "../Level/Level4.h"
#include <sstream>
#include <string>


//GamePlayer::GamePlayer(xWindow &w){}
GamePlayer::GamePlayer(Grid* grid, Level *level, bool id):Player(grid, level, id){
    nextBlock = getNextBlock();
}
//GamePlayer::GamePlayer(Grid* grid, Level *level, bool id):grid{grid}, level{level}, playerId{id}{}

GamePlayer::~GamePlayer(){
  delete grid;
  delete level;
  delete currBlock;
}

bool GamePlayer::getPlayerId(){
    return this->playerId;
}

Player* GamePlayer::getBasePlayer() {
  return this;
}


std::vector<GridCell>* GamePlayer::getRow(int rowNum){
  return grid->getRow(rowNum);
}

void GamePlayer::printRow (int rowNum) {
    std::vector<GridCell> gridRow = *(this->grid->getRow(rowNum));
    for (auto cells : gridRow){
        std::cout << cells;
    }
}

int GamePlayer::getLevel() {
  return level->getLevel();
}

int GamePlayer::getScore() {
  return score;
}

char GamePlayer::getNextBlock() {
  return level->getNextBlock();
}

void GamePlayer::setNextBlock() {
  switch (nextBlock)
  {
    case 'S':
      currBlock = new SBlock(grid);
      break;
    case 'Z':
      currBlock = new ZBlock(grid);
      break;
    case 'T':
      currBlock = new TBlock(grid);
      break;
    case 'L':
      currBlock = new LBlock(grid);
      break;
    case 'J':
      currBlock = new JBlock(grid);
      break;
    case 'O':
      currBlock = new OBlock(grid);
      break;
    case 'I':
      currBlock = new IBlock(grid);
      break;
  }
  nextBlock = getNextBlock();
}

void GamePlayer::moveLeft(int times) {
  for(int i = times; i > 0; i--){
    currBlock->moveLeft();
  }
}

void GamePlayer::moveRight(int times) {
  for(int i = times; i > 0; i--){
    currBlock->moveRight();
  }
}

bool GamePlayer::moveDown(int times) {
  for(int i = times; i > 0; i--){
    if(currBlock->moveDown() == false){
      return false;
    };
  }
  return true;
}

void GamePlayer::rotate(std::string direction, int times){
  for(int i = times; i > 0; i--){
    currBlock->rotate(direction);
  }
}

void GamePlayer::setNextBlockChar(char c){
  nextBlock = c;
}

char GamePlayer::getNextBlockChar(){
  return nextBlock;
}

void GamePlayer::levelUp(){
  int levelNum = level->getLevel();
  if(levelNum != 4){
    delete level;
    switch(levelNum){
      case 0:
        level = new Level1();
        break;
      case 1:
        level = new Level2();
        break;
      case 2:
        level = new Level3();
        break;
      case 3:
        level = new Level4(grid);
        break;
    }
  }
}

void GamePlayer::levelDown(){
  int levelNum = level->getLevel();
  if(levelNum != 0){
    delete level;
    switch(levelNum){
      case 1:
        //If differennt sequencefile is defined and player 1
        #ifdef scriptfile1
        if(playerId == 1) level = new Level0("scriptfile1");
        break;
        #endif
        //Otherwise just use sequence1.txt
        if(playerId == 1) level = new Level0("sequence1.txt");

        //If different sequencefile is defined and player 2
        #ifdef scriptfile2
        if(playerId == 0) level = new Level0("scriptfile2");
        break;
        #endif
        //Otherwise just use sequence2.txt
        if(playerId == 0) level = new Level0("sequence2.txt");
        break;
      case 2:
        level = new Level1();
        break;
      case 3:
        level = new Level2();
        break;
      case 4:
        level = new Level3();
        break;
    }
  }
}

void GamePlayer::noRandom(std::string sequencefile){
  if(level->getLevel() == 3 || level->getLevel() == 4){
    level->noRandom(sequencefile);
  }
}

void GamePlayer::random(){
  if(level->getLevel() == 3 || level->getLevel() == 4){
    level->random();
  }
}


//When a block is dropped, we go through all the rows of the grid.
//If the row is full then we loop through the row
//When we loop through the row we call drop(x,y) on every block in the grid
//Drop will check to see if the cell is in the block, if it is it will delete it
//Every cell will shift one block downwards

//isFull(std::vector<GridCell> vec) checks to see if all the cells in
// the given vector are used up
// @param std::vector<GridCell> vec: a row vector given to the grid
bool isFull(std::vector<GridCell>* vec){
  for(auto i: *vec){
      if(!i.isUsed){
          return false;
      }
  }
  return true;
}

//Shift all cells above rowCleared down by one
void GamePlayer::shiftCellsDown(int rowCleared){
  for(int y = rowCleared-1; y >= 3; y--){
    for(int x = 0; x < 11; x++){
      if(grid->getGridCell(x,y)->isUsed) {
        grid->getGridCell(x,y+1)->isUsed = true;
      } else {
        grid->getGridCell(x,y+1)->isUsed = false;
      }
      grid->getGridCell(x,y+1)->setType(grid->getGridCell(x,y)->getType());        
    }
  }

  //Set the block cells to one cell below the current
  // if they're above the cleared row
  for(auto i : blocksOnBoard){
    i->moveCellsDown(rowCleared);
  }
  
}

//If the block is empty (i.e has no cells) delete it
void GamePlayer::removeEmptyBlocks(){
  for(int i = 0; i < blocksOnBoard.size(); i++){
    if(blocksOnBoard.at(i)->numCells() == 0){
      blocksOnBoard.erase(blocksOnBoard.begin() + i);
      i--;
    }
  }
}

int GamePlayer::drop(int times) {
  while(currBlock->moveDown());
  blocksOnBoard.emplace_back(currBlock);
  int numRowsCleared = 0;
  //Go through each row
  for(int y = 17; y >=3; y--){
    //Check to see if the row is full
    
    if(isFull(grid->getRow(y))){
      //Go through every cell in the row
      
      std::vector<GridCell>* row = grid->getRow(y);
      for(int i = 0; i < 11; i++){

        //Go through every block on the board
        for(auto j: blocksOnBoard){
          //Call drop with the coordinates of the cell on the board
          //If the cell is in the block it will be removed from the block
          j->removeCellFromBlock(row->at(i).getX(), row->at(i).getY());
        }

        //Set each cell in the full row to false
        //Set the character to ' '
        row->at(i).isUsed = false;
        row->at(i).setType(' ');
      }
      
      //Remove any blocks that may have been emptied
      removeEmptyBlocks();

      //Move all cells above current row down one
      shiftCellsDown(y);

      //Increment numRowsCleared;
      numRowsCleared++;

      //Start from a row lower if the row you are on got cleared
      y++;
    }
  }
  //Add to the score
  if(numRowsCleared!= 0) score += level->getScore(numRowsCleared);
  
  //change the player
  playerOnePlaying = !playerOnePlaying;

  //Return the num rows cleared
  return numRowsCleared;
}


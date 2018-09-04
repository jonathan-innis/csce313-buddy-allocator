/* 
    File: my_allocator.cpp
*/
#include "BuddyAllocator.h"
#include <iostream>
#include <stdexcept>
#include <math.h>
using namespace std;

LinkedList::LinkedList(){
  length = 0;
}

void LinkedList::insert(BlockHeader* b){
  b->nextBlock = head;
  head = b;
  length++;
}

void LinkedList::remove(BlockHeader* b){
  BlockHeader* temp = head;
  if (temp == b){
    head = b;
    length--;
    return;
  }
  while (temp->nextBlock && temp->nextBlock != b){
    cout << &temp << endl;
  }
  if (temp->nextBlock->nextBlock)
    temp->nextBlock = temp->nextBlock->nextBlock;
  else
    temp->nextBlock = nullptr;
  length--;
}

BuddyAllocator::BuddyAllocator (uint _basic_block_size, uint _total_memory_length){
  front = new char [_total_memory_length];
  BlockHeader* bh = (BlockHeader*) front;
  bh->free = true; bh->blockSize = _total_memory_length; bh->nextBlock = nullptr;
	basicBlockSize = _basic_block_size;
  totalSize = _total_memory_length;
  freeListLength = int(log2(totalSize/basicBlockSize));
  freeList = new LinkedList[freeListLength];
  freeList[this->freeListPosition(totalSize)].insert(bh);
}

BuddyAllocator::~BuddyAllocator (){
	delete [] front;
  delete [] freeList;
}

char* BuddyAllocator::getbuddy(char* addr){
  BlockHeader* bh = (BlockHeader*) addr;
  uint bs = bh->blockSize;
  return ((&addr - &(this->front)) ^ bs) + this->front;
}

bool BuddyAllocator::isvalid(char* addr){
  return false;
}

bool BuddyAllocator::arebuddies(char* block1, char* block2){
  return this->getbuddy(block1) == block2;
}

char* BuddyAllocator::merge(char* block1, char* block2){
  char* left;
  char* right;
  if (&block1 < &block2){
    left = block1;
    right = block2;
  }
  else{
    left = block2;
    right = block1;
  }

  BlockHeader* right_bh = (BlockHeader*) right;
  BlockHeader* left_bh = (BlockHeader*) left;
  left_bh->blockSize = left_bh->blockSize + right_bh->blockSize;

  return left;
}

char* BuddyAllocator::split(char* block){
  BlockHeader* left_bh = (BlockHeader*) block;
  uint oldSize = left_bh->blockSize;
  freeList[this->freeListPosition(oldSize)].remove(left_bh); //Remove old size from the free list
  uint newSize = oldSize/2;
  char* right = block + newSize;
  BlockHeader* right_bh = (BlockHeader*) right;
  left_bh->free = true; left_bh->blockSize = newSize; left_bh->nextBlock = nullptr;
  right_bh->free = true; right_bh->blockSize = newSize; right_bh->nextBlock = nullptr;
  freeList[this->freeListPosition(newSize)].insert(left_bh); //Add the new sizes to the free lists
  freeList[this->freeListPosition(newSize)].insert(right_bh); //Add the new sizes to the free lists
  return (char*) left_bh;
}

char* BuddyAllocator::alloc(uint _length) {
  int i = 0;
  int j = 1;
  if (_length > totalSize - sizeof(BlockHeader)) throw runtime_error("There is not enough space in memory to allot the needed space.");
  while ((basicBlockSize * j) - sizeof(BlockHeader) < _length){
    j *= 2; 
    i++;
  }
  int neededBlockSize = basicBlockSize * j;
  if (freeList[i].getlength() > 0){
    BlockHeader* block = freeList[i].gethead();
    block->free = false;
    block->nextBlock = nullptr;
    freeList[i].remove(block);
  }
  else{
    char* splitBlock;
    while (freeList[i].getlength() == 0) i++;
    int currBlockSize = freeList[i].gethead()->blockSize;
    while (neededBlockSize != currBlockSize){
      char* block = (char*) freeList[i].gethead();
      splitBlock = this->split(block);
      block = splitBlock;
      currBlockSize /= 2;
    }
    freeList[i].remove((BlockHeader*) splitBlock);
    return splitBlock;
  }
}

int BuddyAllocator::free(char* _a) {
  /* Same here! */
  delete _a;
  return 0;
}

void BuddyAllocator::debug (){
  
}


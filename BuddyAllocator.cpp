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
  if (head) b->nextBlock = head;
  else b->nextBlock = nullptr;
  head = b;
  b->free = true;
  length++;
}

void LinkedList::remove(BlockHeader* b){
  BlockHeader* temp = head;
  if (temp == b) head = temp->nextBlock;
  else{
    while (temp->nextBlock != b){
      temp = temp->nextBlock;
    }
    temp->nextBlock = temp->nextBlock->nextBlock;
  }
  length--;
  b->free = false;
}

BuddyAllocator::BuddyAllocator (uint _basic_block_size, uint _total_memory_length){
  int j = 1;
  while ((_basic_block_size * j) < _total_memory_length){
    j *= 2; 
  }
  totalSize = _basic_block_size * j;
  front = new char [int(totalSize)];
  BlockHeader* bh = (BlockHeader*) front;
  bh->free = true; bh->blockSize = totalSize; bh->nextBlock = nullptr;
	basicBlockSize = _basic_block_size;
  freeListLength = int(log2(totalSize/basicBlockSize)) + 1;
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
  void* buddy = ((addr - front) ^ bs) + front;
  return ((addr - front) ^ bs) + front;
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
  if (block1 < block2){
    left = block1;
    right = block2;
  }
  else{
    left = block2;
    right = block1;
  }
  BlockHeader* right_bh = (BlockHeader*) right;
  BlockHeader* left_bh = (BlockHeader*) left;
  uint oldSize = left_bh->blockSize;
  uint newSize = oldSize * 2;

  freeList[this->freeListPosition(oldSize)].remove(left_bh);
  freeList[this->freeListPosition(oldSize)].remove(right_bh);
  freeList[this->freeListPosition(newSize)].insert(left_bh);
  left_bh->blockSize = newSize;

  return left;
}

char* BuddyAllocator::split(char* block){
  BlockHeader* left_bh = (BlockHeader*) block;
  uint oldSize = left_bh->blockSize;
  freeList[this->freeListPosition(oldSize)].remove(left_bh); //Remove old size from the free list
  uint newSize = oldSize/2;
  char* right = block + newSize;
  BlockHeader* right_bh = (BlockHeader*) right;
  left_bh->blockSize = newSize; left_bh->nextBlock = nullptr;
  right_bh->blockSize = newSize; right_bh->nextBlock = nullptr;
  freeList[this->freeListPosition(newSize)].insert(left_bh); //Add the new sizes to the free lists
  freeList[this->freeListPosition(newSize)].insert(right_bh); //Add the new sizes to the free lists
  return (char*) left_bh;
}

char* BuddyAllocator::alloc(uint _length) {
  cout << "Allocating" << endl;
  cout << "Length: " << _length << endl;
  int i = 0;
  int j = 1;
  if (_length > totalSize - sizeof(BlockHeader)) throw runtime_error("There is not enough space in memory to allocate the needed space.");
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
    char* returnedBlock = (char*) block;
    return (char*) (returnedBlock + sizeof(BlockHeader));
  }
  else{
    char* splitBlock;
    while (freeList[i].getlength() == 0){
      i++;
      cout << i << endl;
      if (i >= freeListLength) throw runtime_error("There is not enough space in memory to allocate the needed space");
    }
    int currBlockSize = freeList[i].gethead()->blockSize;
    while (neededBlockSize != currBlockSize){
      char* block = (char*) freeList[freeListPosition(currBlockSize)].gethead();
      splitBlock = this->split(block);
      block = splitBlock;
      currBlockSize /= 2;
    }
    freeList[freeListPosition(currBlockSize)].remove((BlockHeader*) splitBlock);
    BlockHeader* splitBlockHeader = (BlockHeader*) splitBlock;
    return (splitBlock + sizeof(BlockHeader));
  }
}

int BuddyAllocator::free(char* _a) {
  char* startAddr = _a - sizeof(BlockHeader);
  BlockHeader* bh = (BlockHeader*) startAddr;
  freeList[freeListPosition(bh->blockSize)].insert(bh);
  bool merging = true;
  uint newSize = bh->blockSize;
  while (merging){
    char* buddy = getbuddy(startAddr);
    BlockHeader* buddy_header = (BlockHeader*) buddy;
    if (buddy_header->free && buddy_header->blockSize == bh->blockSize){
      startAddr = merge(startAddr, buddy);
      bh = (BlockHeader*) startAddr;
    }
    else merging = false;
  }
}

void BuddyAllocator::debug (){
  for (int i = 0; i < freeListLength; i++){
    cout << i << ". ";
    if (freeList[i].getlength() > 0){
      BlockHeader* temp = freeList[i].gethead();
      for (int j = 0; j < freeList[i].getlength(); j++){
        cout << (void*) temp << "->";
        temp = temp->nextBlock;
      }
    }
    cout << "(" << freeList[i].getlength() << ")" << endl;
  }
}


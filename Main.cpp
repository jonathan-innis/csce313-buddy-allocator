#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <unistd.h>

int main(int argc, char ** argv) {
  int basic_block_size = 128;
  int memory_length = 524288;
  int c;
  try{
    while ((c = getopt (argc, argv, "b:s:")) != -1)
      switch (c){
        case 'b':
          basic_block_size = atoi(optarg);  
          break;
        case 's':
          memory_length = atoi(optarg);
          break;
      }
    // create memory manager
    BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);


    // test memory manager
    Ackerman* am = new Ackerman ();
    am->test(allocator); // this is the full-fledged test. 
    
    // destroy memory manager
    delete allocator;
  }
  catch (runtime_error e){
    cerr << e.what() << endl;
  }
}

#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>

struct cacheBlock{
	unsigned long int tag;
	struct cacheBlock* next;
};

int length(struct cacheBlock* cache){
	struct cacheBlock* ptr = cache;
	int length = 0;
	while(ptr != NULL){
		length++;	
		ptr = ptr->next;
	}
	return length;
}

void writeToCache(struct cacheBlock** cache, unsigned long int tag, unsigned int setIndex, long int numSets, int blocksPerSet){
	//struct node* new = malloc(sizeof(struct node));
	int index = setIndex; 
	struct cacheBlock* ptr = cache[index]; 
	struct cacheBlock* prev = NULL; 
	struct cacheBlock* new = malloc(sizeof(struct cacheBlock));

	

        if(ptr == NULL){
                cache[index] = new ;
                new->tag = tag;
                new->next = NULL;
                return;
        }

	 else{
while(ptr->next != NULL){
	printf("%lu->", ptr->tag);
	ptr = ptr->next;		
}               new->tag = tag;
		new->next = cache[index];
		cache[index] = new;
		
//printf(" written ");
	}
	assert(ptr == NULL);
	if(length(cache[index]) > blocksPerSet) {
		while(ptr->next != NULL){
			prev = ptr; 
			ptr = ptr->next;		
		}
		free(prev->next);
		prev->next = NULL;
	}




}

bool readFromCache(struct cacheBlock** cache, unsigned long int tag, unsigned int setIndex, long int numSets){
	
	int index = setIndex; 
        struct cacheBlock* ptr = cache[index];
        while(ptr != NULL){
                if(ptr->tag == tag){
                        return true;
		}
                ptr = ptr->next;
        }
	return false;
}

int main(int argc, char** argv){
 
	//read in arguments
	FILE* fp;
	if(!(fp = fopen(argv[6], "r"))){
		printf("error\n");
		return 0;
	}
	fp = fopen(argv[6], "r");
	long int cacheSize = atoi(argv[1]);
	long int blockSize = atoi(argv[2]);
	char* cachePolicy = argv[3];
	char* associativity = argv[4];
	long int prefetchSize = atoi(argv[5]);
	if(cacheSize == 0 || blockSize == 0 || (strcmp(cachePolicy, "fifo") != 0 && strcmp(cachePolicy, "lru")) != 0|| prefetchSize == 0) {
		printf("error\n");
		return 0;
	}
	

	//calculate cache parameters
	int numSets = 0;
	int blocksPerSet = 0;
	int numBlocks = (cacheSize * 1024) / blockSize;
	if(associativity[0] == 'd'){
		//direct
		numSets = numBlocks; 	
	} else if(associativity[6] == '\0'){
		//fully associative
		numSets = 1;
	} else{
		//n-way associative
		char temp[70];
		for(int i = 6, j = 0; associativity[i] != '\0';i++){
			temp[j] = associativity[i];
		}
		blocksPerSet = atoi(temp);
		numSets = numBlocks / blocksPerSet;
	}
	int numBitsSetIndex = log2(numSets);
	int numBitsOffset = log2(blockSize);
	
	
	
	//create hashtable and mask
	struct cacheBlock* cache[numSets];	
	for(int i = 0; i < numSets;i++){
		cache[i] = NULL;
	}
	unsigned int mask = (1 << numBitsSetIndex) - 1;	

	//process tracefiles
	int hits = 0, misses = 0, writes = 0, reads = 0, pReads = 0, pHits = 0, pMisses = 0;
	char ch;	
	unsigned long long int  address, prefetchAddress;
	int count = 0;
	while(fscanf(fp, "%c\t%llx\n", &ch, &address) && count < 2){
		unsigned int setIndex = (address >> numBitsOffset) & mask;
		unsigned long int tag = (address >> numBitsOffset) >> numBitsSetIndex;
		//unsigned int blockOffset = address & mask;
		prefetchAddress = address; 

		if(ch == 'R'){
			if(readFromCache(cache, tag, setIndex, numSets)){
				if(count == 1){
					pHits++;
				} else{
				hits++;
				}
				
			} else{
				writeToCache(cache, tag, setIndex, numSets, blocksPerSet);
				if(count == 1){
					for(int i = 0; i < prefetchSize; i++){
						prefetchAddress = prefetchAddress + blockSize;
						unsigned int pSetIndex = (prefetchAddress >> numBitsOffset) & mask;
						unsigned long int pTag = (prefetchAddress >> numBitsOffset) >> numBitsSetIndex;
						if(!readFromCache(cache, pTag, pSetIndex, numSets)){
							writeToCache(cache, pTag, pSetIndex, numSets, blocksPerSet);
							pReads++;
						}

					}
				pMisses++;
				pReads++;
				
				} else{
					misses++;
					reads++;
					pReads++;
				}
			

			}
		} else if(ch == 'W'){
			if(readFromCache(cache, tag, setIndex, numSets)){
				if(count == 1){
					pHits++;
				} else{
				hits++;
				}

				writes++;
			}
			else if(count == 1){
					for(int i = 0; i < prefetchSize; i++){
						prefetchAddress = prefetchAddress + blockSize;
						unsigned int pSetIndex = (prefetchAddress >> numBitsOffset) & mask;
						unsigned long int pTag = (prefetchAddress >> numBitsOffset) >> numBitsSetIndex;
						if(!readFromCache(cache, pTag, pSetIndex, numSets)){
							writeToCache(cache, pTag, pSetIndex, numSets, blocksPerSet);
							pReads++;
						}

				}
				pMisses++;
				pReads++;
				pReads++;
					
				}
			else {
				writeToCache(cache, tag, setIndex, numSets, blocksPerSet);
				writes++;
				misses++;
				reads++;
				pReads++;
				}
		} else if(ch == '#'){ 
			count++;
			fp = fopen(argv[6], "r");
			
		}
	}

	printf("no-prefetch\n");
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
	printf("with-prefetch\n");
	printf("Memory reads: %d\n", pReads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", pHits);
	printf("Cache misses: %d\n", pMisses);
	
	

	fclose(fp);
	return 0;
}

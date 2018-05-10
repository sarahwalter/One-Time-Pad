#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Main method */
int main(int argc, char **argv){
	int keylength;
	int i;
	int r;
	/* Set up error handling for argument issues */
	if(argc != 2){
		fprintf(stderr,"Error with the arguments\n");
		fprintf(stderr, "Please invoke this program in this way: keygen keylength\n");
		return -1;
	}
	/* Error handling for invalid key length */
	for(i = 0; argv[1][i] != '\0'; i++)
		if(argv[1][i]<'0' || argv[1][i]>'9'){
			fprintf(stderr, "Key length must be a positive number\n");
			return -1;
		}
	/* Generate the random characters */
	keylength = atoi(argv[1]);
	srand(time(NULL));
	for(i = 0; i < keylength; i++){
		r = rand()%27;
		if(r == 26)
			printf(" ");
		else
			printf("%c",'A'+r);
	}
	printf("\n");
	return 0;
}

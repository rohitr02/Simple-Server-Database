#include "pa3.h"

int main(int argc, char* argv[]){
    /* Input Validity */
    if(argc < 2 || argc > 2) return EXIT_FAILURE;                   // No arguments entered
    if(isLegalAtoiInput(argv[1]) == false || atoi(argv[1]) < 5000 || atoi(argv[1]) > 65536) return EXIT_FAILURE;
    bool errorInProgram = false;                        // Flag for the overall program. If for any reason a non-vital error is thrown set this to true and return EXIT_FAILURE at the end

    if(DEBUG) printf("Here\n");
    
    if( errorInProgram == true ) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
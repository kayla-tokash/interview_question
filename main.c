#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

// Lines may be seperated by whitespace
void applyFilterToString(char* regexPattern, char** pString) 
{   
    char* nlToken, *nlEndToken, *wsToken, *wsEndToken;
    regex_t regex;

    int isFirstLine = 1;
    const char dNewline[3] = "\n\n";
    const char dWhitespace[2] = " ";
    char* newString = (char*) malloc(strlen(*pString));
    char* tmpString = (char*) malloc(strlen(*pString));

    // Compile regular expression
    if (regcomp(&regex, regexPattern, 0))
    { 
        printf("There was a problem compiling the regex: %s", regexPattern);
        return;
    }

    // Empty the string to prevent garbage problem (needs more investigation)
    strcpy(newString, "");
    
    // Find new line seperated tokens first 
    strcpy(tmpString, *pString);
    nlToken = strtok_r(tmpString, dNewline, &nlEndToken);
    while (nlToken != NULL)
    {
        // Find space seperated tokens
        // Maintain file format
        if (isFirstLine)
        { 
            isFirstLine = 0;
        }
        else
        {
            strcat(newString, dNewline);
        }

        // Get first token from line
        int isFirstWord = 1; 
        char *tmpLine = (char*) malloc(strlen(nlToken));

        strcpy(tmpLine, nlToken);
        wsToken = strtok_r(tmpLine, dWhitespace, &wsEndToken);
        while (wsToken != NULL)
        {
            // Check token for match
            // Note: regexec retunrs true if there is no match
            if (regexec(&regex, wsToken, 0, NULL, 0))
            {
                // Maintain file format
                if (isFirstWord)
                {
                    isFirstWord = 0;
                }
                else
                {
                    strcat(newString, dWhitespace);
                }
                // Add unfiltered word
                strcat(newString, wsToken);
            }
            // Next token
            wsToken = strtok_r(NULL, dWhitespace, &wsEndToken);
        }
        // Next line token
        nlToken = strtok_r(NULL, dNewline, &nlEndToken);
        free(tmpLine);
    }
    
    // Clean up, and hand off filtered string
    free(*pString);
    *pString = newString;

    printf("newString (%u bytes): %s\n", strlen(newString), newString);
}


int main(int argc, char** argv)
{
    char buffer[BUFFER_SIZE];
    int  inputFileSize, result;
    char *fileContents, *inFilePath, *outFilePath, *filterFilePath;
    FILE *fFilter, *fIn, *fOut;

    if (argc != 4) 
    {
        printf("Usage: \n");
        printf("  %s <infile> <regex/swear file> <outfile>\n", argv[0]);
        return 1;
    }

    // Get command line args
    inFilePath = argv[1];
    filterFilePath = argv[2];
    outFilePath = argv[3];
    
    // Read "in" file
    fIn = fopen(inFilePath, "r");
    if (NULL == fIn)
    {
        // todo check errno
        printf("Error reading file. %s [%u]\n", strerror(errno), errno);
        return 1;
    }

    // Trick to get the filesize
    fseek(fIn, 0L, SEEK_END);
    inputFileSize = ftell(fIn);
    rewind(fIn);

    // Allocate a variable for the inputfile conents, and fille it
    fileContents = (char*) malloc(inputFileSize);
    int i = 0;
    char c = fgetc(fIn);
    while (c != EOF)
    {
        fileContents[i] = c;
        c = fgetc(fIn);
        ++i;
    }
    printf("Read %u bytes from file.\n", inputFileSize);

    // Close the file
    fclose(fIn);

    // Iterate through the filters, and apply them to the text
    fFilter = fopen(filterFilePath, "r");
    if (NULL == fFilter)
    { 
        printf("Error writing file. %s [%u]\n", strerror(errno), errno);
        return 1;
    }
    
    // Read in filters
    while (fscanf(fFilter, "%s", buffer) != EOF)
    {
        applyFilterToString(buffer, &fileContents);   
    }
    
    fclose(fFilter);

    // Write "out" file
    fOut = fopen(outFilePath, "w+");
    if (NULL == fOut)
    {   // todo check errno
        return 1;
    }
    fputs(fileContents, fOut);

    fclose(fOut);

    // clean up allocated memory
    free(fileContents);

    printf("Thank you for your time! ~Kayla\n");
}




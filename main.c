#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

/**
 * void applyFilterToString(char* regexPattern, char** pString)
 * Parameters:
 *   char*  regexPattern - The regex pattern supplied by the dirty words file.
 *   char** pString - The contents of the input file.
 **/
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

    // Empty the string to prevent garbage problem with strcat
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

    inFilePath = argv[1];
    filterFilePath = argv[2];
    outFilePath = argv[3];
   
    printf("Reading input file.\n");
 
    fIn = fopen(inFilePath, "r");
    if (NULL == fIn)
    {
        printf("Error reading file. %s [%u]\n", strerror(errno), errno);
        return 1;
    }

    fseek(fIn, 0L, SEEK_END);
    inputFileSize = ftell(fIn);
    rewind(fIn);

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

    fclose(fIn);

    printf("Applying \"dirty word\" filters.\n");

    fFilter = fopen(filterFilePath, "r");
    if (NULL == fFilter)
    { 
        printf("Error writing file. %s [%u]\n", strerror(errno), errno);
        return 1;
    }

    while (fscanf(fFilter, "%s", buffer) != EOF)
    {
        applyFilterToString(buffer, &fileContents);   
    }
    
    fclose(fFilter);

    printf("Writing to output file\n");

    fOut = fopen(outFilePath, "w+");
    if (NULL == fOut)
    {
        printf("Error writing to file. %s [%u]\n", strerror(errno), errno);
        return 1;
    }
    fputs(fileContents, fOut);
    fclose(fOut);

    free(fileContents);

    printf("Thank you for your time! ~Kayla\n");
}


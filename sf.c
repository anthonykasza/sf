#define AUTHORS "AK"
#define PROGRAM_NAME "sf - suffix"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  -f 	--filename	use the public suffix file located here
  -lX	--label-depth	print X number of labels, by default l1 is used
			l0 is the tld, l1 is the domain, l2 is the subdomain, etc.
  -e	--echo-names	include the domain name used as input and the result
*/


char* join(const char* s1, const char* s2, char delim[])
	{
	char* result = malloc(strlen(s1) + strlen(s2) + 1 + 1); // one for delim, one for \0

	if (result)
		{
		strcpy(result, s1);
		strcat(result, delim);
		strcat(result, s2);
		}
	return result;
}

char* rtrim(char* string, char junk)
	{
	char* original = string + strlen(string);
	while(*--original == junk);
	*(original + 1) = '\0';
	return string;
	}

char* ltrim(char *string, char junk)
	{
	char* original = string;
	char *p = original;
	int trimmed = 0;
	do 	{
		if (*original != junk || trimmed)
			{
			trimmed = 1;
			*p++ = *original;
			}
		} while (*original++ != '\0');
	return string;
	}

char* trim(char *str, char junk)
	{
	str = rtrim(str, junk);
	str = ltrim(str, junk);
	return str;
	}

char* remove_first_label(char* str, char delim[])
	{
	str = trim(str, '.');
	int first_flag = 1;

	char* result;
	char* label = strtok(str, delim);

	while (label != NULL)
		{
		if (first_flag)
			{
			label = strtok(NULL, delim);
			result = join("", "", "");
			first_flag = 0;
		} else	{
			result = join(result, label, delim);
			label = strtok(NULL, delim);
			}
		}
	result = trim(result, '.');
	return result;
	}

int main(int argc, char* argv[])
	{

	/*************************************************/
	/*      Read Public Suffix File                  */
	char* path = "/home/vmroot/c_stuff/public_suffix_list.dat";
	char** words = NULL;
	int lines_allocated = 128;
	int max_line_len = 253;

	words = (char **)malloc(sizeof(char*) * lines_allocated);
	if (words == NULL)
		{
		fprintf(stderr,"Out of memory (1).\n");
		exit(1);
		}

	FILE* fp;
	fp = fopen(path, "r");
	if (fp == NULL)
		{
		fprintf(stderr, "Could not open file: %s.\n", path);
		exit(2);
		}

	int i;
	for (i=0; 1; i++)
	{
		int j;
		// Have we gone over our line allocation?
		if (i >= lines_allocated)
			{
			int new_size;

			// Double our allocation and re-allocate
			new_size = lines_allocated*2;
			words = (char **)realloc(words,sizeof(char*)*new_size);
			if (words == NULL)
				{
				fprintf(stderr,"Out of memory.\n");
				exit(3);
				}
			lines_allocated = new_size;
			}
		// Allocate space for the next line
		words[i] = malloc(max_line_len);
		if (words[i] == NULL)
			{
			fprintf(stderr,"Out of memory (3).\n");
			exit(4);
			}
		if (fgets(words[i], max_line_len-1, fp) == NULL)
			{
			break;
			}

		// Get rid of CR or LF at end of line
		for (j=strlen(words[i])-1; j>=0 && (words[i][j]=='\n' || words[i][j]=='\r'); j--)
			{
			;
			}
		if (((words[i][0] == '/') && (words[i][1] == '/')) || strlen(words[i]) < 2)
			{
			// let's redo this index with the next line
			i--;
			continue;
			}
		words[i][j+1]='\0';
	}
	fclose(fp);

	/*************************************************/
	/*      Read Lines From STDIN                    */
	char* line = NULL;
	size_t size;
	while (getline(&line, &size, stdin) != -1)
		{
		// strip the newline character from the end of the line
		if (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r')
			{
			line[strlen(line)-1] = '\0';
			}

		// lowercase each letter in the line
		for(i = 0; line[i]; i++)
			{
			line[i] = tolower(line[i]);
			}

		/*************************************************/
		/*      Find the TLD of a line                   */
		char* name;
		name = malloc(sizeof(char) * strlen(line));
		strcpy(name, line);
		name = trim(name, '.');
		int flag = 0;
		while (strlen(name) >= 0)
			{
			int i = 0;
			while(words[i])
				{
				if(strcmp(words[i], name) == 0)
					{
					flag = 1;
					break;
					}
				i++;
			}
			if (flag)
				{
				break;
				}
			else
				{
				name = remove_first_label(name, ".");
				}

			}
		// if we didn;t find the TLD by the end of the public suffix file,
		//  there isn't one for the line entered.
		if (!flag)
			{
			name = "";
			}

		// there should be an arguement switch check here
		//   --echo-names / -e
		if (0)
			{
			printf("%s\t%s\n", line, name);
		} else	{
			printf("%s\n", name);
			}
		}

	for (; i>=0; i--)
		{
		free(words[i]);
		}
	free(words);
	return 0;
	}

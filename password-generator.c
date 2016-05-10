/*
    Copyright (C) 2016 cacahuatl < cacahuatl at autistici dot org >

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h> /* printf, f{open,read,close}, FILE */
#include <stdlib.h> /* malloc */
#include <string.h> /* strlen */
#include <err.h> /* warn... */
#include <math.h> /* ceil, log2, pow */

typedef unsigned int ui;

static ui wordcount = 0; /* total number of words in the list */
static ui wnum = 5; /* number of words in the passphrase */
static FILE *entropy = NULL; /* entropy source for rng */
static FILE *wordlist = NULL; /* file containing list of words */
static char **words = NULL; /* list of words read from file */

/* cleanup frees up the memory we've used */
static void cleanup(void) {
	if (entropy)
		fclose(entropy);
	if (wordlist)
		fclose(wordlist);
	if (words) {
		ui i = 0;
		while (i < wordcount)
			free(words[i++]);
		free(words);
	}
}

/* die emits message if not NULL and exits with code */
static void die(const char *reason, int code) {
	if (reason)
		fprintf(stderr, "FATAL ERROR: %s\n", reason);
	cleanup();
	exit(code);
}

/* rnd returns a random unsigned integer up to (excluding) max with minimal */
/* biasing, but some extra cost */
static ui rnd(ui max) {
	ui bits = (ui)ceil(log2((double)(max-1))),
	   bytes = bits / 8,
	   wastage = 0,
	   ret = 0;

	if (bits > sizeof(ret) * 8)
		die("Invalid range!", -1);
	if (!entropy)
		die("No entropy source open!", -1);
	if (bytes * 8 < bits)
		bytes++;
	wastage = (bytes * 8) - bits;
	for (;;) {
		if (fread(&ret, bytes, 1, entropy)) {
			ret >>= wastage;
			if (ret < max)
				break;
		} else {
			warn("fread");
			continue;
		}
	}
	return ret;
}

/* ee provides an estimate of shannon entropy for a 'worst case scenario' */
/* this is where they have our exact wordlist and try every permutation */
/* rather than a straight brute force attack */
static void ee(void) {
	fprintf(
		stderr,
		"Estimated entropy: %d~ bits\n",
		(int)floor(log2(pow((double)wordcount, (double)wnum)))
	);
}

int main(int argc, char **argv) {
	char word[512];
	ui i;

	wordlist = fopen("words", "r");
	if (!wordlist)
		die("Failed to open wordlist", -1);
	entropy = fopen("/dev/urandom", "r");
	if (!entropy)
		die("Failed to open entropy source", -1);
	while(fgets(word, 511, wordlist)) {
		if (!(words = realloc(words, (wordcount + 1) * sizeof(char *))))
			die("Failed to allocate memory", -1);
		if (word[strlen(word)-1] == '\n')
			word[strlen(word)-1] = 0;
		words[wordcount++] = strdup(word);
	}
	if (argc > 1) {
		if(1 > sscanf(argv[1], "%u", &wnum)) {
			die("Invalid arg", -1);
		}
	}
	ee();
	for(i = 0; i < wnum; i++) {
		printf("%s ", words[rnd(wordcount)]);
	}
	printf("\n");
	die(NULL, 0);
}

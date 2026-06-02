#pragma once


typedef struct Config Config; // Defined in context.h
							  

Config *loadConfig(char const *cpath);


void freeConfig(Config *config);

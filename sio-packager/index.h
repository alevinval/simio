/*
#ifndef INDEX_H
#define INDEX_H

typedef struct {
	unsigned char name[256];
	int history_count;
	unsigned char *receipts;
} Resource;

typedef struct {
	int size;
	Resource * resources;
} Index;

#endif /**  INDEX_H */

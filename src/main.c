#include <stdio.h>
#include <string.h>
#include "bar.h"

int main(void) {
	struct Bar bar; 
	strcpy(bar.timestamp, "2026-09-14");
	bar.open = 100;
	bar.high = 104;
	bar.low = 98;
	bar.close = 101;
	bar.volume = 1000;
	printf("%f\n", bar.close);
	return 0;
}


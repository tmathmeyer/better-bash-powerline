#include <stdio.h>
int main() {
	int i=0, brk=51;
	for(;i<16;i++) {
		if (i < 10) {
			printf("\e[48;5;%im%i   ", i, i);
		} else {
			printf("\e[48;5;%im%i  ", i, i);
		}
	}
	printf("\e[0m\n");
	for(;i<256;i++) {
		printf("\e[48;5;%im", i);
		if (i < 100) {
			printf("%i  ", i);
		} else {
			printf("%i ", i);
		}
		if (i==brk) {
			brk += 36;
			printf("\e[0m\n");
		}
	}
	printf("\e[0m\n");
    return 0;
}

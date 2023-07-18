#include <stdio.h>
#include <stdlib.h>

int main() {
    unsigned long long aaa[1024];
    int count = 0;

    for (unsigned long long a = 48; a < 50; a++) {
        for (unsigned long long b = 48; b < 50; b++) {
            for (unsigned long long c = 48; c < 50; c++) {
                for (unsigned long long d = 48; d < 50; d++) {
                    for (unsigned long long e = 48; e < 50; e++) {
                        for (unsigned long long f = 48; f < 50; f++) {
                            for (unsigned long long h = 48; h < 50; h++) {
                                for (unsigned long long i = 48; i < 50; i++) {
                                    for (unsigned long long j = 48; j < 50; j++) {
                                        for (unsigned long long k = 48; k < 50; k++) {
                                            aaa[count] = a + b * 256 + c * 256 * 256 + d * 256 * 256 * 256 +
                                                        e * 256 * 256 * 256 * 256 + f * 256 * 256 * 256 * 256 * 256 +
                                                        h * 256 * 256 * 256 * 256 * 256 * 256 +
                                                        i * 256 * 256 * 256 * 256 * 256 * 256 * 256 +
                                                        j * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 +
                                                        k * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256;
                                            count++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Print the generated values
    for (int i = 0; i < count; i++) {
        printf("%llu\n", aaa[i]);
    }

    return 0;
}

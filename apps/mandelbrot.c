#include "picomon.h"

// Fixed point shift (Q12.12 format: 12 bits for integer, 12 bits for fraction)
#define SHIFT 12
#define ONE (1 << SHIFT)

// Convert Int to Fixed
#define TO_FIX(x) ((x) << SHIFT)
// Multiply two Fixed numbers
#define MUL(a, b) ( ((long long)(a) * (b)) >> SHIFT )

void main() {
    print("\033[2J\033[H"); // Clear Screen
    print("Calculaing Mandelbrot Set (Fixed Point Q12.12)...\r\n");

    // Screen dimensions (Terminal size)
    int width = 70;
    int height = 24;

    // Viewport coordinates (Real numbers represented as Fixed Point)
    // x: -2.5 to 1.5, y: -1.5 to 1.5
    int min_re = TO_FIX(-2) - TO_FIX(1)/2;
    int max_re = TO_FIX(1) + TO_FIX(1)/2;
    int min_im = TO_FIX(-1) - TO_FIX(1)/2;
    int max_im = TO_FIX(1) + TO_FIX(1)/2;

    int re_factor = (max_re - min_re) / (width - 1);
    int im_factor = (max_im - min_im) / (height - 1);

    char charset[] = " .:-;!/>)|&IH%*#"; // Darkness gradient

    for (int y = 0; y < height; y++) {
        int c_im = max_im - y * im_factor;

        for (int x = 0; x < width; x++) {
            int c_re = min_re + x * re_factor;

            int z_re = c_re, z_im = c_im;
            int is_inside = 1;
            int n;

            // Iteration loop
            for (n = 0; n < 15; n++) {
                // Z_re2 = z_re * z_re
                int z_re2 = MUL(z_re, z_re);
                // Z_im2 = z_im * z_im
                int z_im2 = MUL(z_im, z_im);

                // if (z_re^2 + z_im^2 > 4) break
                if (z_re2 + z_im2 > TO_FIX(4)) {
                    is_inside = 0;
                    break;
                }

                // z_im = 2 * z_re * z_im + c_im
                z_im = MUL(z_re, z_im);
                z_im = (z_im << 1) + c_im; // * 2 + c_im

                // z_re = z_re^2 - z_im^2 + c_re
                z_re = z_re2 - z_im2 + c_re;
            }

            if (is_inside) {
                putc('#');
            } else {
                putc(charset[n]);
            }
        }
        putc('\r'); putc('\n');
    }
    
    print("\r\nDone.\r\n");
}

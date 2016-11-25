#include <stdio.h>
#include <string.h>
#include <math.h>
#pragma pack(1)

struct header {
	short format;
	int file_size;
	short reserved_1;
	short reserved_2;
	int offset;
};

struct DIB_header {
	int size;
	int width;
	int height;
	short planes;
	short bits_per_pixel;
	int compression_scheme;
	int image_size;
	int hor_resolution;
	int vert_resolution;
	int colors;
	int imp_colors;
};

struct pixel_rgb {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

int main(int argc, char *argv[]) {

	FILE *image;
	struct header image_header;
	struct DIB_header image_DIB_header;

	// Check that command line arguments are correct
	if (argc != 3) {
		printf("Proper usage: ./bmp_edit -flag FILENAME\n");
		printf("Now exiting...");
		return(1);
	}

	// Open image in binary r/w mode
	image = fopen(argv[2], "r+b");

	// Make sure image opens properly
	if (image == NULL) {
		printf("Error opening file.\n");
		printf("Now exiting...");
		return(1);
	}

	// Read in the first header
	fread(&image_header, sizeof(struct header), 1, image);

	// Check that format is BM
	if (image_header.format != 19778) {
		printf("Sorry, we do not support this file format.\n");
		printf("Now exiting...");
		return (1);
	}

	// Read in the second header
	fread(&image_DIB_header, sizeof(struct DIB_header), 1, image);

	// Check that size is 40 and file is 24bit RGB
	if (image_DIB_header.size != 40 || image_DIB_header.bits_per_pixel != 24) {
		printf("Sorry, we do not support this file format.\n");
		printf("Now exiting...");
		return (1);
	}

	// Move to beginning of pixel array
	fseek(image, image_header.offset, SEEK_SET);

	// Invert mode
	if (strcmp(argv[1], "-invert") == 0) {
		int i;
		int j;
		int skip = image_DIB_header.width % 4;
		int num;

		// Move through pixel array
		for (i = 0; i < image_DIB_header.height; i++) {
			for (j = 0; j < image_DIB_header.width; j++) {
				struct pixel_rgb pixel;

				// Read pixel
				fread(&pixel, sizeof(struct pixel_rgb), 1, image);

				// Invert pixel
				pixel.red = ~pixel.red;
				pixel.green = ~pixel.green;
				pixel.blue = ~pixel.blue;

				// Move back to beginning of pixel
				fseek(image, -3, SEEK_CUR);

				// Write pixel
				fwrite(&pixel, sizeof(struct pixel_rgb), 1, image);
			}

			// Skip padding
			if (skip != 0) {
				num = 4-skip;
				fseek(image, num, SEEK_CUR);
			}
		}
	}

	// Grayscale mode
	else if (strcmp(argv[1], "-grayscale") == 0) {
		int i;
		int j;
		int skip = image_DIB_header.width % 4;
		int num;
		float y, r, g, b;

		// Move through pixels
		for (i = 0; i < image_DIB_header.height; i++) {
			for (j = 0; j < image_DIB_header.width; j++) {
				struct pixel_rgb pixel;

				// Read pixel
				fread(&pixel, sizeof(struct pixel_rgb), 1, image);

				// Cast rgb values to float and divide by 255 to get value between 0 and 1
				r = (float) pixel.red / 255;
				g = (float) pixel.green / 255;
				b = (float) pixel.blue / 255;

				// Calculate y value
				y = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
				
				if (y <= 0.0031308) {
					r = g = b = (12.92*y);
				}

				else if (y > 0.0031308) {
					r = g = b = ((1.055 * pow(y, (1/2.4))) - 0.055);
				}

				else {
					printf("Error...");
					return(1);
				}

				// Convert back to char
				pixel.red = (char) (r*255);
				pixel.green = (char) (g*255);
				pixel.blue = (char) (b*255);

				// Move back to beginning of pixel
				fseek(image, -3, SEEK_CUR);

				// Write pixel
				fwrite(&pixel, sizeof(struct pixel_rgb), 1, image);
			}

			// Skip padding
			if (skip != 0) {
				num = 4-skip;
				fseek(image, num, SEEK_CUR);
			}
		}
	}

	else {
		printf("Invalid flag. Valid flags are -grayscale & -invert.\n");
		printf("Now exiting...\n");
		fclose(image);
		return(1);
	}

	fclose(image);
	return(0);
}
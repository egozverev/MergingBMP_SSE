#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <emmintrin.h> //SSE2 intrinsics
#include <mmintrin.h> // MMX (ancestor of SSE ) intrinsics
#include <smmintrin.h> //SSE 4.1 intrinsics

typedef struct {             // Total: 54 bytes
    uint16_t type;             // Magic identifier: 0x4d42
    uint32_t size;             // File size in bytes
    uint16_t reserved1;        // Not used
    uint16_t reserved2;        // Not used
    uint32_t offset;           // Offset to image data in bytes from beginning of file (54 bytes)
    uint32_t dib_header_size;  // DIB Header size in bytes (40 bytes)
    int32_t width_px;         // Width of the image
    int32_t height_px;        // Height of image
    uint16_t num_planes;       // Number of color planes
    uint16_t bits_per_pixel;   // Bits per pixel
    uint32_t compression;      // Compression type
    uint32_t image_size_bytes; // Image size in bytes
    int32_t x_resolution_ppm; // Pixels per meter
    int32_t y_resolution_ppm; // Pixels per meter
    uint32_t num_colors;       // Number of colors
    uint32_t important_colors; // Important colors
} __attribute__ ((packed)) BMPHeader;


unsigned char *LoadBitmapFile(const char *filename) {
    FILE *file_ptr;
    unsigned char *bmp_image_data;  // image data pointer, returned in the end

    file_ptr = fopen(filename, "rb"); //open filename in read binary mode
    if (file_ptr == NULL) { /* if smth went wrong whether it is wrong name or whatever */
        return NULL;
    }
    //read header first

    fseek(file_ptr, 0, SEEK_END); /* teleport to the end of the file */
    uint64_t file_length = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET); /* teleport to the beginning */


    //fread(bmp_header, sizeof(BMPHeader), 1, file_ptr);


    //just check that this is bmp file - using some kind of magic value
    /*if (bmp_header->type != 0x4D42) {
        fclose(file_ptr);
        return NULL;
    }*/


    //fseek(file_ptr, bmp_header->offset, SEEK_SET);
    //unsigned char * bmp_trash_data  = (unsigned char *) _mm_malloc(bmp_header->offset - sizeof(BMPHeader), 16);
    //fread(bmp_trash_data, bmp_header->offset - sizeof(BMPHeader), 1, file_ptr);
    //_mm_free(bmp_trash_data);
    //allocation of the memory
    //bmp_image_data = (unsigned char *) _mm_malloc(bmp_header->image_size_bytes, 16);
    // special malloc -
    unsigned char *file_all_data = (unsigned char *) _mm_malloc(file_length, 16);
    //check if malloc went wrong
    /*if (!bmp_image_data) {
        free(bmp_image_data);
        fclose(file_ptr);
        return NULL;
    }*/

    //read in the bitmap image data
    fread(file_all_data, file_length, 1, file_ptr);




    //close file and return bitmap iamge data
    fclose(file_ptr);
    return file_all_data;


}


void MergeBMPImages(const unsigned char *all_data1, const unsigned char *all_data2, const char *result_filename) {
    FILE *file_ptr = fopen(result_filename, "wb");
    BMPHeader *header1 = (BMPHeader *) all_data1;
    BMPHeader *header2 = (BMPHeader *) all_data2;
    fwrite(all_data1, header1->offset, 1, file_ptr);

    //unsigned char * bmp_trash_data  = (unsigned char *) _mm_malloc(fst_image->header.offset - sizeof(BMPHeader), 16);
    //fread(bmp_trash_data, fst_image->header.offset - sizeof(BMPHeader), 1, file_ptr);
    //_mm_free(bmp_trash_data);
    const unsigned char *data1 = all_data1 + header1->offset;
    const unsigned char *data2 = all_data2 + header2->offset;
    int32_t current_byte = 0;
    int32_t byte_string_length = header1->width_px * (header1->bits_per_pixel) / 8;
    //num of bytes of each pixel row
    int32_t padding_len = byte_string_length % 4 == 0 ? 0 : 4 - byte_string_length % 4;
    for (int32_t current_row = 0; current_row < header1->height_px; ++current_row) {
        int32_t current_pixel = 0;
        for (current_pixel = 0; current_pixel + 3 < header1->width_px; current_pixel += 4) {
            for (int32_t pixel = 0; pixel < 4; ++pixel) {
                unsigned char color[4];
                double alpha = data2[current_byte + 3] / 255.;
                for (int i = 0; i < 4; ++i) {
                    if (i == 3) {
                        color[3] = 255;
                    } else {
                        color[i] = (unsigned char)(data1[current_byte + i] * (1-alpha) + data2[current_byte + i] * alpha);
                    }
                }
                fwrite(color, header2->bits_per_pixel / 8, 1, file_ptr);

                current_byte += header1->bits_per_pixel / 8;
            }
        }
        if (current_pixel != header1->width_px) {
            for (int32_t pixel = 0; pixel < header1->width_px - current_pixel; ++pixel) {
                unsigned char color[4];
                double alpha = data2[current_byte + 3] / 255.;
                for (int i = 0; i < 4; ++i) {
                    if (i == 3) {
                        color[3] = 255;
                    } else {
                        color[i] = (unsigned char)(data1[current_byte + i] * (1-alpha) + data2[current_byte + i] * alpha);
                    }
                }
                fwrite(color, header2->bits_per_pixel / 8, 1, file_ptr);

                current_byte += header1->bits_per_pixel / 8;
            }
        }
        current_byte += padding_len;

    }
}

int main() {

    const unsigned char *all_data1 = LoadBitmapFile("/home/egor/C_projects/MergingBMP_SSE/example32.bmp");
    const unsigned char *all_data2 = LoadBitmapFile("/home/egor/C_projects/MergingBMP_SSE/second.bmp");
    MergeBMPImages(all_data1, all_data2, "/home/egor/C_projects/MergingBMP_SSE/not_devil_res.bmp");



    const unsigned char *all_data3 = LoadBitmapFile("/home/egor/C_projects/MergingBMP_SSE/devil1.bmp");
    const unsigned char *all_data4 = LoadBitmapFile("/home/egor/C_projects/MergingBMP_SSE/devil2.bmp");
    MergeBMPImages(all_data3, all_data4, "/home/egor/C_projects/MergingBMP_SSE/devil_result.bmp");


}
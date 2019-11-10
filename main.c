#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>

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
typedef struct {
    BMPHeader header;
    unsigned char *data;

} BMPImage;


unsigned char *LoadBitmapFile(char *filename, BMPHeader *bmp_header) {
    FILE *file_ptr;
    unsigned char *bmp_image_data;  // image data pointer, returned in the end

    file_ptr = fopen(filename, "rb"); //open filename in read binary mode
    if (file_ptr == NULL) { /* if smth went wrong whether it is wrong name or whatever */
        return NULL;
    }
    //read header first
    fread(bmp_header, sizeof(BMPHeader), 1, file_ptr);


    //just check that this is bmp file - using some kind of magic value
    if (bmp_header->type != 0x4D42) {
        fclose(file_ptr);
        return NULL;
    }


    //fseek(file_ptr, bmp_header->offset, SEEK_SET);

    //allocation of the memory
    bmp_image_data = (unsigned char *) _mm_malloc(bmp_header->image_size_bytes, 16);
    // special malloc -

    //check if malloc went wrong
    if (!bmp_image_data) {
        free(bmp_image_data);
        fclose(file_ptr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bmp_image_data, bmp_header->image_size_bytes, 1, file_ptr);
    for (size_t i = 0; i < bmp_header->size; ++i) {
        printf("%d ", bmp_image_data[i]);
    }
    printf("\n");

    int imageIdx = 0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0; imageIdx < bmp_header->image_size_bytes; imageIdx += 3) // fixed semicolon
    {
        //EACH CHAR = COLOR, 3 CHARS = PIXEL. xxd has 16bytes system - 00 means 0 <= 0 + 16 * 0 <= 255
        tempRGB = bmp_image_data[imageIdx];
        //bmp_image_data[imageIdx] = bmp_image_data[imageIdx + 2];
        //bmp_image_data[imageIdx + 2] = tempRGB;


    }

    //close file and return bitmap iamge data
    fclose(file_ptr);
    return bmp_image_data;


}

BMPImage ReadBMPImage(char *filename) {
    BMPImage bmp_image;
    bmp_image.data = LoadBitmapFile(filename, &bmp_image.header);
    return bmp_image;
}

/*void MergeBMPImages(const BMPImage *fst_image, const BMPImage *snd_image, char *result_filename) {
    FILE *file_ptr = fopen(result_filename, "wb");
    fwrite(&fst_image->header, sizeof(BMPHeader), 1, file_ptr);
    int32_t current_byte = 0;
    int32_t byte_string_length = fst_image->header.width_px * (fst_image->header.bits_per_pixel) / 4;
    //num of bytes of each pixel row
    int32_t padding_len = byte_string_length % 4 == 0 ? 0 : 4 - byte_string_length % 4;
    for (int32_t current_row = 0; current_row < fst_image->header.height_px; ++current_row){
        if(fst_image->header.width_px < 4){
            for(int32_t pixel = 0; pixel < fst_image->header.width_px; ++pixel){
                int8_t bool_is_second_zero = 1;
                for(int32_t i =0; i < fst_image ->header.bits_per_pixel/4; ++i){
                    if(snd_image -> data[current_byte + i] != 0){
                        bool_is_second_zero = 0;
                    }
                }
                if(bool_is_second_zero){
                    fwrite(snd_image->data + current_byte, snd_image->header.bits_per_pixel, 1, file_ptr);
                } else{
                    fwrite(fst_image->data + current_byte, fst_image->header.bits_per_pixel, 1, file_ptr);

                }
                current_byte += fst_image->header.bits_per_pixel;
            }
            current_byte += padding_len;
        }
        int32_t current_pixel = 0;
        for(current_pixel = 0; current_pixel + 3 < fst_image->header.width_px; current_pixel += 4 ){
            // while we can use see operations
        }
        if(current_pixel != fst_image->header.width_px){
            for(int32_t pixel = 0; pixel < fst_image->header.width_px - current_pixel; ++pixel){
                int8_t bool_is_second_zero = 1;
                for(int32_t i =0; i < fst_image ->header.bits_per_pixel/4; ++i){
                    if(snd_image -> data[current_byte + i] != 0){
                        bool_is_second_zero = 0;
                    }
                }
                if(bool_is_second_zero){
                    fwrite(snd_image->data + current_byte, snd_image->header.bits_per_pixel, 1, file_ptr);
                } else{
                    fwrite(fst_image->data + current_byte, fst_image->header.bits_per_pixel, 1, file_ptr);

                }
                current_byte += fst_image->header.bits_per_pixel;
            }
        }
        current_byte += padding_len;

    }
}*/

void MergeBMPImages(const BMPImage *fst_image, const BMPImage *snd_image, char *result_filename) {
    FILE *file_ptr = fopen(result_filename, "wb");
    fwrite(&fst_image->header, sizeof(BMPHeader), 1, file_ptr);
    int32_t current_byte = 0;
    int32_t byte_string_length = fst_image->header.width_px * (fst_image->header.bits_per_pixel) / 8;
    //num of bytes of each pixel row
    int32_t padding_len = byte_string_length % 4 == 0 ? 0 : 4 - byte_string_length % 4;
    for (int32_t current_row = 0; current_row < fst_image->header.height_px; ++current_row){
        int32_t current_pixel = 0;
        for(current_pixel = 0; current_pixel + 3 < fst_image->header.width_px; current_pixel += 4 ){
            for(int32_t pixel = 0; pixel < 4; ++pixel){
                int8_t bool_is_second_zero = 1;
                for(int32_t i =0; i < snd_image ->header.bits_per_pixel/8; ++i){
                    if(snd_image -> data[current_byte + i] != 0){
                        bool_is_second_zero = 0;
                    }
                }
                if(!bool_is_second_zero){
                    fwrite(snd_image->data + current_byte, snd_image->header.bits_per_pixel/8, 1, file_ptr);
                } else{
                    fwrite(fst_image->data + current_byte, fst_image->header.bits_per_pixel/8, 1, file_ptr);

                }
                current_byte += fst_image->header.bits_per_pixel/8;
            }
        }
        if(current_pixel != fst_image->header.width_px){
            for(int32_t pixel = 0; pixel < fst_image->header.width_px - current_pixel; ++pixel){
                int8_t bool_is_second_zero = 1;
                for(int32_t i =0; i < fst_image ->header.bits_per_pixel/8; ++i){
                    if(snd_image -> data[current_byte + i] != 0){
                        bool_is_second_zero = 0;
                    }
                }
                if(!bool_is_second_zero){
                    fwrite(snd_image->data + current_byte, snd_image->header.bits_per_pixel/8, 1, file_ptr);
                } else{
                    fwrite(fst_image->data + current_byte, fst_image->header.bits_per_pixel/8, 1, file_ptr);

                }
                current_byte += fst_image->header.bits_per_pixel/8;
            }
        }
        current_byte += padding_len;

    }
}
int main() {

    BMPImage image = ReadBMPImage("/home/egor/C_projects/MergingBMP_SSE/example32.bmp");
    BMPImage snd  = ReadBMPImage("/home/egor/C_projects/MergingBMP_SSE/second.bmp");
    MergeBMPImages(&image, &snd, "/home/egor/C_projects/MergingBMP_SSE/result.bmp");
    /*
    for(size_t i = 0; i < image.header.image_size_bytes; ++i){
        if(i < image.header.image_size_bytes / 4){
            image.data[i] = 115;
        }
        else if(i < image.header.image_size_bytes / 2){
            image.data[i] = 0;
        }
        else{
            image.data[i] = 200;
        }
    }

    FILE * snd_file = fopen("/home/egor/C_projects/MergingBMP_SSE/second_32.bmp", "wb");
    fwrite(&image.header, sizeof(BMPHeader), 1, snd_file);
    fwrite(image.data, image.header.image_size_bytes, 1, snd_file);
    printf("%d %d %d \n", image.header.height_px, image.header.width_px, image.header.bits_per_pixel);
    //now do what you want with it, later on i will show you how to display it in a normal window

    fclose(snd_file);
*/
}
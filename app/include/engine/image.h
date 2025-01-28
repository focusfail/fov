#ifndef __ENGINE_IMAGE_H__
#define __ENGINE_IMAGE_H__
#define STB_IMAGE_IMPLEMENTATION

#include "glad/glad.h"
#include "log.h"
#include "stb_image.h"

#include <stdbool.h>

typedef struct {
    int            width;
    int            height;
    int            channels;
    unsigned char* data;
} Image;

bool image_load(const char* filepath, Image* im)
{
    int w, h, channels;
    im->data = stbi_load(filepath, &w, &h, &channels, STBI_rgb_alpha);

    if (im->data == NULL) {
        stbi_image_free(im->data);
        log_error("Failed to load image: %s", filepath);
        return false;
    }

    im->width    = w;
    im->height   = h;
    im->channels = 4;

    return true;
}

void image_free(Image* im)
{
    stbi_image_free(im->data);
}

GLuint load_texture_from_image(Image* im)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load and generate the texture
    if (im->data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, im->width, im->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to load texture from image.");
    }

    return texture;
}

void _extrac_tiles(const Image* atlas, int tile_width, int tile_height, unsigned char** tiles)
{
    int tiles_per_row = atlas->width / tile_width;
    int tiles_per_col = atlas->height / tile_height;
    int tile_count    = tiles_per_row * tiles_per_col;

    for (int y = 0; y < tiles_per_col; y++) {
        for (int x = 0; x < tiles_per_row; x++) {
            int tile_index    = y * tiles_per_row + x;
            tiles[tile_index] = (unsigned char*)malloc(tile_width * tile_height * 4); // RGBA

            for (int ty = 0; ty < tile_height; ty++) {
                for (int tx = 0; tx < tile_width; tx++) {
                    int atlas_index      = ((y * tile_height + ty) * atlas->width + (x * tile_width + tx)) * 4;
                    int tile_pixel_index = (ty * tile_width + tx) * 4;
                    tiles[tile_index][tile_pixel_index + 0] = atlas->data[atlas_index + 0];
                    tiles[tile_index][tile_pixel_index + 1] = atlas->data[atlas_index + 1];
                    tiles[tile_index][tile_pixel_index + 2] = atlas->data[atlas_index + 2];
                    tiles[tile_index][tile_pixel_index + 3] = atlas->data[atlas_index + 3];
                }
            }
        }
    }
}

GLuint load_array_texture(const char* atlas_filepath, int tile_width, int tile_height)
{
    Image atlas;
    if (!image_load(atlas_filepath, &atlas)) {
        log_error("Failed to load atlas: %s", atlas_filepath);
        return 0;
    }

    int tiles_per_row = atlas.width / tile_width;
    int tiles_per_col = atlas.height / tile_height;
    int tile_count    = tiles_per_row * tiles_per_col;

    unsigned char** tiles = (unsigned char**)malloc(tile_count * sizeof(unsigned char*));
    _extrac_tiles(&atlas, tile_width, tile_height, tiles);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    // Use nearest neighbor for close-up, mipmaps for distance
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Adjust LOD for smoother transitions
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, 0.0f);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_LOD, 0.0f);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, 4.0f);

    int mip_levels = (int)(floor(log2(16))) + 1;

    // Allocate storage and upload texture data as before
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mip_levels, GL_RGBA8, tile_width, tile_height, tile_count);

    // Upload base level textures
    for (int i = 0; i < tile_count; i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tile_width, tile_height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                        tiles[i]);
    }

    // Generate mipmaps automatically
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Clean up
    for (int i = 0; i < tile_count; i++) {
        free(tiles[i]);
    }
    free(tiles);
    image_free(&atlas);

    return texture;
}

#endif // __ENGINE_IMAGE_H__
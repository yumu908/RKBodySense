//
// Created by Optical on 2024/3/25.
//

#ifndef NV_IMAGE_READER_H
#define NV_IMAGE_READER_H

#include "image_common.h"

// Abstract interface operations structure for image readers.
typedef struct _NVImageReaderOps {
    // Open/init device
    void* (*open)(const char* uri, int width, int height);
    // Close device and release resources
    int   (*close)(void* ctx);
    // Dequeue one frame from the device queue
    int   (*dequeue)(void* ctx, NVImage* image);
    // Enqueue processed image buffer back to the device
    int   (*enqueue)(void* ctx, NVImage* image, int index);
    // Get/print current format
    int   (*dump_format)(void* ctx);
} NVImageReaderOps;

typedef enum {
    NV_READER_TYPE_IMAGE,
    NV_READER_TYPE_V4L2,
    NV_READER_TYPE_CAMERA,
    NV_READER_TYPE_MAX,
} NVImageReaderType;

// Abstract image reader structure
typedef struct _NVImageReader {
    const NVImageReaderOps* ops;    // Pointer to operation function table
    void* context;                  // Opaque pointer to implementation-specific context
    // Additional common fields such as state, configuration, etc.
} NVImageReader;

NVImageReader* nv_image_reader_reader(NVImageReaderType readerType);

// Generic interface functions (optional, provide convenient call methods)
static inline void* nv_image_reader_open(NVImageReader* reader, const char* uri, int width, int height) {
    if (reader && reader->ops && reader->ops->open) {
        reader->context = reader->ops->open(uri, width, height);
        return reader->context;
    }
    return NULL;
}

static inline int nv_image_reader_dump_format(NVImageReader* reader) {
    if (reader && reader->ops && reader->ops->dump_format && reader->context) {
        return reader->ops->dump_format(reader->context);
    }
    return -1;
}

static inline int nv_image_reader_dequeue(NVImageReader* reader, NVImage* image) {
    if (reader && reader->ops && reader->ops->dequeue && reader->context) {
        return reader->ops->dequeue(reader->context, image);
    }
    return -1;
}

static inline int nv_image_reader_enqueue(NVImageReader* reader, NVImage* image, int index) {
    if (reader && reader->ops && reader->ops->enqueue && reader->context) {
        return reader->ops->enqueue(reader->context, image, index);
    }
    return -1;
}

static inline int nv_image_reader_close(NVImageReader* reader) {
    if (reader && reader->ops && reader->ops->close && reader->context) {
        int ret = reader->ops->close(reader->context);
        reader->context = nullptr;  // Clear context pointer
        return ret;
    }
    return -1;
}

#endif  // NV_IMAGE_READER_H

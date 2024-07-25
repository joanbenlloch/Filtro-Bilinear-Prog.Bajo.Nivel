#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/stddef.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include "zip.h"

#define FUSE_FILL_DIR_DEFAULTS 0
using namespace std;

static struct options {
    const char *zipFileName;
    zip_t *zip;
} options;

static const struct fuse_opt option_spec[] = {
    {"--zipfile=%s", offsetof(struct options, zipFileName), 1},
    FUSE_OPT_END
};

typedef enum {
    FILE_T, // tipo fichero
    DIR_T   // tipo directorio, en los ejemplos sólo se usa para el "raíz"
} fileType;

typedef struct {
    fileType type;
    vector<unsigned char> data;
} dirEntry_t;

map<string, dirEntry_t> dirEntries;

void process_zip_entries(const string& path, zip_t* zip) {
    int zip_entry_count = zip_total_entries(zip);
    for (int i = 0; i < zip_entry_count; i++) {
        zip_entry_openbyindex(zip, i);
        string entryName(zip_entry_name(zip));

        if (entryName.compare(0, path.length(), path) != 0) {
            zip_entry_close(zip);
            continue; // Saltar las entradas que no pertenecen al subdirectorio actual
        }

        string relativeEntryName = entryName.substr(path.length());

        // Verificar si es un directorio
        bool isDir = (relativeEntryName.back() == '/');
        if (isDir) {
            //relativeEntryName.pop_back();
        }

        // Crear una entrada nueva
        dirEntry_t newEntry;
        newEntry.type = isDir ? DIR_T : FILE_T;

        if (!isDir) {
            // Leer el contenido del archivo si no es un directorio
            size_t fileLength = 0;
            unsigned char* fileContents = NULL;
            zip_entry_read(zip, (void**)&fileContents, &fileLength);

            newEntry.data.resize(fileLength);
            memcpy(newEntry.data.data(), fileContents, fileLength);
        }

        // Agregar la entrada al mapa
        dirEntries[path + relativeEntryName] = newEntry;

        if (isDir) {
            // Procesar recursivamente los archivos y directorios dentro del subdirectorio
            process_zip_entries(path + relativeEntryName + "/", zip);
        }

        zip_entry_close(zip);
    }
}

static void *zip_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    cfg->kernel_cache = 1;
    dirEntries = {{"", {DIR_T, {}}}};

    // Imprimir todas las rutas dentro del zip
    int zip_entry_count = zip_total_entries(options.zip);
    cout << "Entries: " << zip_entry_count << endl;
    for (int i = 0; i < zip_entry_count; i++) {
        zip_entry_openbyindex(options.zip, i);
        const char* entry_name = zip_entry_name(options.zip);
        std::cout << entry_name << std::endl;
        if (entry_name) {
            std::cout << entry_name << std::endl;
        }
        zip_entry_close(options.zip);
    }
    
    //process_zip_entries("", options.zip);

    return NULL;
}

static int zip_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    memset(stbuf, 0, sizeof(struct stat));

    string sPath(path + 1);

    auto fIt = dirEntries.find(sPath);

    if (fIt == dirEntries.end())
        return -ENOENT;

    dirEntry_t entry = fIt->second;
    if (entry.type == DIR_T) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (entry.type == FILE_T) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = entry.data.size();
    }

    return 0;
}

static int zip_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    string sPath(path + 1);

    if (sPath != "" && dirEntries.find(sPath) == dirEntries.end())
        return -ENOENT;

    filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);

    for (auto &entry : dirEntries) {
        if (entry.first.compare(0, sPath.length(), sPath) == 0) {
            string name = entry.first.substr(sPath.length());
            if (name.find('/') == string::npos) {
                filler(buf, name.c_str(), NULL, 0, (fuse_fill_dir_flags)0);
            }
        }
    }

    return 0;
}

static int zip_open(const char *path, struct fuse_file_info *fi) {
    string sPath(path + 1);

    auto fIt = dirEntries.find(sPath);
    if (fIt == dirEntries.end() || fIt->second.type == DIR_T)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int zip_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    string sPath(path + 1);

    auto fIt = dirEntries.find(sPath);
    if (fIt == dirEntries.end() || fIt->second.type == DIR_T)
        return -ENOENT;

    size_t len = fIt->second.data.size();
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, fIt->second.data.data() + offset, size);
    } else
        size = 0;

    return size;
}

static const struct fuse_operations zip_oper = {
    .getattr    = zip_getattr,
    .open       = zip_open,
    .read       = zip_read,
    .readdir    = zip_readdir,
    .init       = zip_init
};

int main(int argc, char** argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    options.zipFileName = strdup("/tmp/prueba.zip");

    // parse options
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;
    // open zip file, check if exists
    options.zip = zip_open(options.zipFileName, ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
    if (options.zip == nullptr) {
        cout << "Error: Fichero " << options.zipFileName << " no encontrado" << endl;
        return 1;
    }
    int ret;
    ret = fuse_main(args.argc, args.argv, &zip_oper, NULL);
    fuse_opt_free_args(&args);

    return ret;
}

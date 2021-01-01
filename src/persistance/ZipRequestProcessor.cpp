
#include "ZipRequestProcessor.h"
#include <iostream>


void ZipRequestProcessor::loadingFinished(std::vector<unsigned char> * file) {
//    zip_source_t *src;
//    zip_t *za;
//    zip_error_t error;
//
//    zip_error_init(&error);
//    /* create source from buffer */
//    if ((src = zip_source_buffer_create(&file->at(0), file->size(), 1, &error)) == NULL) {
//        fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
////            free(data);
//        zip_error_fini(&error);
////            return 1;
//    }
//
//    /* open zip archive from source */
//    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
//        fprintf(stderr, "can't open zip from source: %s\n", zip_error_strerror(&error));
//        zip_source_free(src);
//        zip_error_fini(&error);
////            return 1;
//    }
//    zip_error_fini(&error);
//    zipArchive = za;

    /* we'll want to read the data back after zip_close */
//        zip_source_keep(src);
}

void ZipRequestProcessor::requestFile(std::string &fileName, CacheHolderType holderType,
                                      std::weak_ptr<PersistentFile> s_file) {
//    std::string s_fileName(fileName);
//    zip_error_t error;
//    struct zip_stat sb;
//    zip_file *zf;
//
//    zip_int64_t record = zip_name_locate(zipArchive, fileName, ZIP_FL_NOCASE);
//
//    if (record != -1) {
//        if (zip_stat_index(zipArchive, record, 0, &sb) != 0) {
//            throw "errror";
//        }
//        zf = zip_fopen_index(zipArchive, record, 0);
//
//        unsigned char *unzippedEntry = new unsigned char[sb.size];
//
//        int sum = 0;
//        while (sum != sb.size) {
//            zip_int64_t len = zip_fread(zf, &unzippedEntry[sum], 1024);
//            if (len < 0) {
//                //error
//                exit(102);
//            }
//            sum += len;
//        }
//
//        zip_fclose(zf);
//
//        m_fileRequester->provideFile(fileName, unzippedEntry, sum);
//        delete(unzippedEntry);
//    } else {
//        std::cout << "Could not load file " << std::string(fileName) << std::endl << std::flush;
//        m_fileRequester->rejectFile(fileName);
//    }
}

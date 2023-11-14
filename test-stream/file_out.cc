#include "test-stream/file_out.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int FileOut::counter = 0;

FileOut::FileOut(const char * path) : path(path), file(open(path, O_CREAT | O_WRONLY)) {counter++;}

FileOut::~FileOut() {
    close(file);
    counter--;
}

const char * FileOut::getPath() {
    return path;
}

int FileOut::count() {
    return counter;
}

void FileOut::flush() {
    write(file, buffer, pos);
    fsync(file);
    pos = 0;
}


int
inotify_remove_file(const char * path);

int inotify_insert_file(char * name, const char * path);

int reinotify_insert_directory(char * name, const char * path);

void *
start_inotify();

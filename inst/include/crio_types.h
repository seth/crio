#ifndef CRIO_TYPES_H_
#define CRIO_TYPES_H_

#define CRIO_OK   0
#define CRIO_EOF -1
#define CRIO_ERR -2

#define CRIO_FILT_FAIL  0
#define CRIO_FILT_PASS  1

#define CRIO_ERRBUF_SIZE 1024

typedef struct _crio_node * CrioNode;

struct crio_stream {
    void *file;
    char *filename;
    int (*read)(struct crio_stream *stream);
    int nread;
    int nfiltered;
    struct _crio_node *filter;
    void *ctx;
    char error_message[CRIO_ERRBUF_SIZE];
};

struct crio_filter {
    char* name;
    int (*filter)(struct crio_stream *stream, void *filter_ctx);
    void *filter_ctx;
    void (*finalizer)(void *filter_ctx);
};

#endif  /* CRIO_TYPES_H_ */

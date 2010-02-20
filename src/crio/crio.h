#ifndef CRIO_H_
#define CRIO_H_

#define CRIO_OK   0
#define CRIO_EOF -1
#define CRIO_ERR -2

struct crio_stream {
    void *file;
    int (*read)(void *, void *);
    int (*filter)(void *);
};

/* crio_stream_make initializes a new stream.  You must free the
   returned crio_stream pointer when finished.  Returns NULL if
   allocation fails.  */

struct crio_stream * crio_stream_make(int (*read)(void *fh, void *ctx),
                                      int (*filter)(void *ctx),
                                      void *fh);


/* user code will call crio_stream_next in a loop to process the
   stream */

int crio_stream_next(struct crio_stream *, void *ctx);

#endif  /* CRIO_H_ */

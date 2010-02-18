#ifndef CRIO_H_
#define CRIO_H_

#define CRIO_OK 0

struct crio_stream {
    void *file;
    int (*read)(void *, void *);
    int (*filter)(void *);
};

/* crio_stream_make initializes a new stream */
struct crio_stream * crio_stream_make(int (*read_record)(void *fh, void *ctx),
                                    int (*filter_record)(void *ctx),
                                    void *fh);

/* user code will call crio_stream_next in a loop to process
   the stream
*/
int crio_stream_next(struct crio_stream *, void *ctx);

#endif  /* CRIO_H_ */
